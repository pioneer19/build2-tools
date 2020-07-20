// file      : libbuild2/module.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/module.hxx>

#if !defined(BUILD2_BOOTSTRAP) && !defined(LIBBUILD2_STATIC_BUILD)
#  ifndef _WIN32
#    include <dlfcn.h>
#  else
#    include <libbutl/win32-utility.hxx>
#  endif
#endif

#include <libbuild2/file.hxx>  // import_*()
#include <libbuild2/scope.hxx>
#include <libbuild2/target.hxx>
#include <libbuild2/variable.hxx>
#include <libbuild2/operation.hxx>
#include <libbuild2/diagnostics.hxx>

// Core modules bundled with libbuild2.
//
#include <libbuild2/dist/init.hxx>
#include <libbuild2/test/init.hxx>
#include <libbuild2/config/init.hxx>
#include <libbuild2/install/init.hxx>

using namespace std;
using namespace butl;

namespace build2
{
  mutex loaded_modules_lock::mutex_;

  loaded_module_map loaded_modules;

  void
  load_builtin_module (module_load_function* lf)
  {
    for (const module_functions* i (lf ()); i->name != nullptr; ++i)
      loaded_modules[i->name] = i;
  }

  // Sorted array of bundled modules (excluding core modules bundled with
  // libbuild2; see below).
  //
  static const char* bundled_modules[] = {
    "bash",
    "bin",
    "c",
    "cc",
    "cxx",
    "in",
    "version"
  };

  static inline bool
  bundled_module (const string& mod)
  {
    return binary_search (
      bundled_modules,
      bundled_modules + sizeof (bundled_modules) / sizeof (*bundled_modules),
      mod);
  }

  // Note: also used by ad hoc recipes thus not static.
  //
  void
  create_module_context (context& ctx, const location& loc)
  {
    assert (ctx.module_context == nullptr);
    assert (*ctx.module_context_storage == nullptr);

    // Since we are using the same scheduler, it makes sense to reuse the
    // same global mutexes. Also disable nested module context for good
    // measure.
    //
    ctx.module_context_storage->reset (
      new context (ctx.sched,
                   ctx.mutexes,
                   false,                    /* match_only */
                   false,                    /* dry_run */
                   ctx.keep_going,
                   ctx.global_var_overrides, /* cmd_vars */
                   nullopt));                /* module_context */

    // We use the same context for building any nested modules that might be
    // required while building modules.
    //
    ctx.module_context = ctx.module_context_storage->get ();
    ctx.module_context->module_context = ctx.module_context;

    // Setup the context to perform update. In a sense we have a long-running
    // perform meta-operation batch (indefinite, in fact, since we never call
    // the meta-operation's *_post() callbacks) in which we periodically
    // execute update operations.
    //
    // Note that we perform each build in a separate update operation. Failed
    // that, if the same target is update twice (which may happen with ad hoc
    // recipes) we will see the old state.
    //
    if (mo_perform.meta_operation_pre != nullptr)
      mo_perform.meta_operation_pre ({} /* parameters */, loc);

    ctx.module_context->current_meta_operation (mo_perform);

    if (mo_perform.operation_pre != nullptr)
      mo_perform.operation_pre ({} /* parameters */, update_id);
  }

  // Note: also used by ad hoc recipes thus not static.
  //
  const target&
  update_in_module_context (context& ctx, const scope& rs, names tgt,
                            const location& loc, const path& bf)
  {
    // New update operation.
    //
    ctx.module_context->current_operation (op_update);

    // Un-tune the scheduler.
    //
    // Note that we can only do this if we are running serially because
    // otherwise we cannot guarantee the scheduler is idle (we could have
    // waiting threads from the outer context). This is fine for now since the
    // only two tuning level we use are serial and full concurrency (turns out
    // currently we don't really need this: we will always be called during
    // load or match phases and we always do parallel match; but let's keep it
    // in case things change).
    //
    auto sched_tune (ctx.sched.serial ()
                     ? scheduler::tune_guard (ctx.sched, 0)
                     : scheduler::tune_guard ());

    // Remap verbosity level 0 to 1 unless we were requested to be silent.
    // Failed that, we may have long periods of seemingly nothing happening
    // while we quietly update the module, which may look like things have
    // hung up.
    //
    // @@ CTX: modifying global verbosity level won't work if we have multiple
    //         top-level contexts running in parallel.
    //
    auto verbg = make_guard (
      [z = !silent && verb == 0 ? (verb = 1, true) : false] ()
      {
        if (z)
          verb = 0;
      });

    // Note that for now we suppress progress since it would clash with the
    // progress of what we are already doing (maybe in the future we can do
    // save/restore but then we would need some sort of diagnostics that we
    // have switched to another task).
    //
    action a (perform_update_id);
    action_targets tgs;

    mo_perform.search  ({},      /* parameters */
                        rs,      /* root scope */
                        rs,      /* base scope */
                        bf,      /* buildfile */
                        rs.find_target_key (tgt, loc),
                        loc,
                        tgs);

    mo_perform.match   ({},      /* parameters */
                        a,
                        tgs,
                        1,       /* diag (failures only) */
                        false    /* progress */);

    mo_perform.execute ({},      /* parameters */
                        a,
                        tgs,
                        1,       /* diag (failures only) */
                        false    /* progress */);

    assert (tgs.size () == 1);
    return tgs[0].as<target> ();
  }

  // Note: also used by ad hoc recipes thus not static.
  //
#if !defined(BUILD2_BOOTSTRAP) && !defined(LIBBUILD2_STATIC_BUILD)
  pair<void* /* handle */, void* /* symbol */>
  load_module_library (const path& lib, const string& sym, string& err)
  {
    // Note that we don't unload our modules since it's not clear what would
    // the benefit be.
    //
    void* h (nullptr);
    void* s (nullptr);

#ifndef _WIN32
    // Use RTLD_NOW instead of RTLD_LAZY to both speed things up (we are going
    // to use this module now) and to detect any symbol mismatches.
    //
    if ((h = dlopen (lib.string ().c_str (), RTLD_NOW | RTLD_GLOBAL)))
    {
      s = dlsym (h, sym.c_str ());

      if (s == nullptr)
        err = dlerror ();
    }
    else
      err = dlerror ();
#else
    if (HMODULE m = LoadLibrary (lib.string ().c_str ()))
    {
      h = static_cast<void*> (m);
      s = function_cast<void*> (GetProcAddress (m, sym.c_str ()));

      if (s == nullptr)
        err = win32::last_error_msg ();
    }
    else
      err = win32::last_error_msg ();
#endif

    return make_pair (h, s);
  }
#else
  pair<void*, void*>
  load_module_library (const path&, const string&, string&)
  {
    return pair<void*, void*> (nullptr, nullptr);
  }
#endif

  static module_load_function*
  import_module (
#if defined(BUILD2_BOOTSTRAP) || defined(LIBBUILD2_STATIC_BUILD)
    scope&,
#else
    scope& bs,
#endif
    const string& mod,
    const location& loc,
    bool boot,
    bool opt)
  {
    tracer trace ("import_module");

    // Take care of core modules that are bundled with libbuild2 in case they
    // are not pre-loaded by the driver.
    //
    if      (mod == "config")  return &config::build2_config_load;
    else if (mod == "dist")    return &dist::build2_dist_load;
    else if (mod == "install") return &install::build2_install_load;
    else if (mod == "test")    return &test::build2_test_load;

    bool bundled (bundled_module (mod));

    // Importing external modules during bootstrap is problematic: we haven't
    // loaded config.build nor entered all the variable overrides so it's not
    // clear what import() can do except confuse matters. So this requires
    // more thinking.
    //
    if (boot && !bundled)
    {
      fail (loc) << "unable to load build system module " << mod <<
        info << "loading external modules during bootstrap is not yet "
                 << "supported";
    }

    module_load_function* r (nullptr);

    // No dynamic loading of build system modules during bootstrap or if
    // statically-linked..
    //
#if defined(BUILD2_BOOTSTRAP) || defined(LIBBUILD2_STATIC_BUILD)
    if (!opt)
    {
      fail (loc) << "unknown build system module " << mod <<
#ifdef BUILD2_BOOTSTRAP
        info << "running bootstrap build system";
#else
        info << "running statically-linked build system";
#endif
    }
#else
    context& ctx (bs.ctx);

    // See if we can import a target for this module.
    //
    path lib;

    // If this is a top-level module update, then we use the nested context.
    // If, however, this is a nested module update (i.e., a module required
    // while updating a module), then we reuse the same module context.
    //
    // If you are wondering why don't we always use the top-level context, the
    // reason is that it might be running a different meta/operation (say,
    // configure or clean); with the nested context we always know it is
    // perform update.
    //
    // And the reason for not simply creating a nested context for each nested
    // module update is due to the no-overlap requirement of contexts: while
    // we can naturally expect the top-level project(s) and the modules they
    // require to be in separate configurations that don't shared anything,
    // the same does not hold for build system modules. In fact, it would be
    // natural to have a single build configuration for all of them and they
    // could plausibly share some common libraries.
    //
    bool nested (ctx.module_context == &ctx);

    // If this is one of the bundled modules, the project name is build2,
    // otherwise -- libbuild2-<mod>.
    //
    project_name proj;
    try
    {
      proj = project_name (bundled ? "build2" : "libbuild2-" + mod);
    }
    catch (const invalid_argument& e)
    {
      fail (loc) << "invalid build system module '" << mod << "': " << e;
    }

    // The target we are looking for is <prj>%libs{build2-<mod>}.
    //
    // We only search in subprojects if this is a nested module update
    // (remember, if it's top-level, then it must be in an isolated
    // configuration).
    //
    pair<name, optional<dir_path>> ir (
      import_search (bs,
                     name (proj, dir_path (), "libs", "build2-" + mod),
                     opt,
                     nullopt  /* metadata    */,
                     nested   /* subprojects */,
                     loc));

    if (ir.first.empty ())
    {
      assert (opt);
      return nullptr;
    }

    if (ir.second)
    {
      // What if a module is specified with config.import.<mod>.<lib>.libs?
      // Note that this could still be a project-qualified target.
      //
      if (ir.second->empty ())
        fail (loc) << "direct module target importation not yet supported";

      // We found the module as a target in a project. Now we need to update
      // the target (which will also give us the shared library path).
      //
      l5 ([&]{trace << "found " << ir.first << " in " << *ir.second;});

      // Create the build context if necessary.
      //
      if (ctx.module_context == nullptr)
      {
        if (!ctx.module_context_storage)
          fail (loc) << "unable to update build system module " << mod <<
            info << "building of build system modules is disabled";

        create_module_context (ctx, loc);
      }

      // Inherit loaded_modules lock from the outer context.
      //
      ctx.module_context->modules_lock = ctx.modules_lock;

      // "Switch" to the module context.
      //
      context& ctx (*bs.ctx.module_context);

      // Load the imported project in the module context.
      //
      pair<names, const scope&> lr (
        import_load (ctx, move (ir), false /* metadata */, loc));

      l5 ([&]{trace << "loaded " << lr.first;});

      // What happens next depends on whether this is a top-level or nested
      // module update.
      //
      if (nested)
      {
        // This could be initial or exclusive load.
        //
        // @@ TODO: see the ad hoc recipe case as a reference.
        //
        fail (loc) << "nested build system module updates not yet supported";
      }
      else
      {
        const target* l;
        {
          // Cutoff the existing diagnostics stack and push our own entry.
          //
          diag_frame::stack_guard diag_cutoff (nullptr);

          auto df = make_diag_frame (
            [&loc, &mod] (const diag_record& dr)
            {
              dr << info (loc) << "while loading build system module " << mod;
            });

          l = &update_in_module_context (
            ctx, lr.second, move (lr.first),
            loc, path ());
        }

        if (!l->is_a ("libs"))
          fail (loc) << "wrong export from build system module " << mod;

        lib = l->as<file> ().path ();

        l5 ([&]{trace << "updated " << lib;});
      }

      ctx.modules_lock = nullptr; // For good measure.
    }
    else
    {
      // No module project found. Form the shared library name (incorporating
      // build system core version) and try using system-default search
      // (installed, rpath, etc).

      // @@ This is unfortunate: it would have been nice to do something
      //    similar to what we've done for exe{}. While libs{} is in the bin
      //    module, we could bring it in (we've done it for exe{}). The
      //    problems are: it is intertwined with its group (lib{}) and we
      //    don't have any mechanisms to deal with prefixes, only extensions.
      //
      const char* pfx;
      const char* sfx;
#if   defined(__MINGW32__)
      pfx = "libbuild2-"; sfx = ".dll";
#elif defined(_WIN32)
      pfx = "build2-";    sfx = ".dll";
#elif defined(__APPLE__)
      pfx = "libbuild2-"; sfx = ".dylib";
#else
      pfx = "libbuild2-"; sfx = ".so";
#endif

      lib = path (pfx + mod + '-' + build_version_interface + sfx);

      l5 ([&]{trace << "system-default search for " << lib;});
    }

    // The build2_<mod>_load() symbol name.
    //
    string sym (sanitize_identifier ("build2_" + mod + "_load"));

    string err;
    pair<void*, void*> hs (load_module_library (lib, sym, err));

    if (hs.first != nullptr)
    {
      // I don't think we should ignore this even if the module is optional.
      //
      if (hs.second == nullptr)
        fail (loc) << "unable to lookup " << sym << " in build system module "
                   << mod << " (" << lib << "): " << err;

      r = function_cast<module_load_function*> (hs.second);
    }
    else if (!opt)
    {
      // Add import suggestion similar to import phase 2.
      //
      fail (loc) << "unable to load build system module " << mod << " ("
                 << lib << "): " << err <<
        info     << "use config.import." << proj.variable () << " command "
                 << "line variable to specify its project out_root";
    }
    else
      l5 ([&]{trace << "unable to load " << lib << ": " << err;});

#endif // BUILD2_BOOTSTRAP || LIBBUILD2_STATIC_BUILD

    return r;
  }

  static const module_functions*
  find_module (scope& bs,
               const string& smod,
               const location& loc,
               bool boot,
               bool opt)
  {
    tracer trace ("find_module");

    // Note that we hold the lock for the entire time it takes to build a
    // module.
    //
    loaded_modules_lock lock (bs.ctx);

    // Optional modules and submodules sure make this logic convoluted. So we
    // divide it into two parts: (1) find or insert an entry (for submodule
    // or, failed that, for the main module, the latter potentially NULL) and
    // (2) analyze the entry and issue diagnostics.
    //
    auto i (loaded_modules.find (smod)), e (loaded_modules.end ());

    if (i == e)
    {
      // If this is a submodule, get the main module name.
      //
      string mmod (smod, 0, smod.find ('.'));

      if (mmod != smod)
        i = loaded_modules.find (mmod);

      if (i == e)
      {
        module_load_function* f (import_module (bs, mmod, loc, boot, opt));

        if (f != nullptr)
        {
          // Enter all the entries noticing which one is our submodule. If
          // none are, then we notice the main module.
          //
          for (const module_functions* j (f ()); j->name != nullptr; ++j)
          {
            const string& n (j->name);

            l5 ([&]{trace << "registering " << n;});

            auto p (loaded_modules.emplace (n, j));

            if (!p.second)
              fail (loc) << "build system submodule name " << n << " of main "
                         << "module " << mmod << " is already in use";

            if (n == smod || (i == e && n == mmod))
              i = p.first;
          }

          // We should at least have the main module.
          //
          if (i == e)
            fail (loc) << "invalid function list in build system module "
                       << mmod;
        }
        else
          i = loaded_modules.emplace (move (mmod), nullptr).first;
      }
    }

    // Now the iterator points to a submodule or to the main module, the
    // latter potentially NULL.
    //
    if (!opt)
    {
      if (i->second == nullptr)
      {
        fail (loc) << "unable to load build system module " << i->first;
      }
      else if (i->first != smod)
      {
        fail (loc) << "build system module " << i->first << " has no "
                   << "submodule " << smod;
      }
    }

    // Note that if the main module exists but has no such submodule, we
    // return NULL rather than fail (think of an older version of a module
    // that doesn't implement some extra functionality).
    //
    return i->second;
  }

  void
  boot_module (scope& rs, const string& mod, const location& loc)
  {
    // First see if this modules has already been booted for this project.
    //
    module_map& lm (rs.root_extra->modules);
    auto i (lm.find (mod));

    if (i != lm.end ())
    {
      module_state& s (i->second);

      // The only valid situation here is if the module has already been
      // bootstrapped.
      //
      assert (s.boot);
      return;
    }

    // Otherwise search for this module.
    //
    const module_functions& mf (
      *find_module (rs, mod, loc, true /* boot */, false /* optional */));

    if (mf.boot == nullptr)
      fail (loc) << "build system module " << mod << " should not be loaded "
                 << "during bootstrap";

    i = lm.emplace (mod,
                    module_state {true, false, mf.init, nullptr, loc}).first;

    {
      module_boot_extra extra {i->second.module};
      i->second.first = mf.boot (rs, loc, extra);
    }

    rs.assign (rs.var_pool ().insert (mod + ".booted")) = true;
  }

  module_state*
  init_module (scope& rs,
               scope& bs,
               const string& mod,
               const location& loc,
               bool opt,
               const variable_map& hints)
  {
    // First see if this modules has already been inited for this project.
    //
    module_map& lm (rs.root_extra->modules);
    auto i (lm.find (mod));
    bool f (i == lm.end ());

    if (f)
    {
      // Otherwise search for this module.
      //
      if (const module_functions* mf = find_module (
            bs, mod, loc, false /* boot */, opt))
      {
        if (mf->boot != nullptr)
          fail (loc) << "build system module " << mod << " should be loaded "
                     << "during bootstrap";

        i = lm.emplace (
          mod,
          module_state {false, false, mf->init, nullptr, loc}).first;
      }
    }
    else
    {
      module_state& s (i->second);

      if (s.boot)
      {
        s.boot = false;
        f = true; // This is a first call to init.
      }
    }

    // Note: pattern-typed in context ctor as project visibility variables of
    // type bool.
    //
    // We call the variable 'loaded' rather than 'inited' because it is
    // buildfile-visible (where we use the term "load a module"; see the note
    // on terminology above)
    //
    auto& vp (rs.var_pool ());
    value& lv (bs.assign (vp.insert (mod + ".loaded")));
    value& cv (bs.assign (vp.insert (mod + ".configured")));

    bool l; // Loaded (initialized).
    bool c; // Configured.

    // Suppress duplicate init() calls for the same module in the same scope.
    //
    if (!lv.null)
    {
      assert (!cv.null);

      l = cast<bool> (lv);
      c = cast<bool> (cv);

      if (!opt)
      {
        if (!l)
          fail (loc) << "unable to load build system module " << mod;

        // We don't have original diagnostics. We could call init() again so
        // that it can issue it. But that means optional modules must be
        // prepared to be called again if configuring failed. Let's keep it
        // simple for now.
        //
        if (!c)
          fail (loc) << "build system module " << mod << " failed to "
                     << "configure";
      }
    }
    else
    {
      l = i != lm.end ();

      if ((c = l))
      {
        module_init_extra extra {i->second.module, hints};
        c = i->second.init (rs, bs, loc, f, opt, extra);
      }

      lv = l;
      cv = c;
    }

    return l && c ? &i->second : nullptr;
  }

  // @@ TODO: This is a bit of a fuzzy mess:
  //
  //    - The .loaded variable check: it's not clear if init_module()
  //      already has this semantics?
  //
  //    - Why do we use variable instead of the module map entry? Probably
  //      because of optional. Also entry present if only booted. Need to be
  //      careful here. Also root vs base!
  //
  // Note that it would have been nice to keep these inline but we need the
  // definition of scope for the variable lookup.
  //
  const shared_ptr<module>*
  load_module (scope& rs,
               scope& bs,
               const string& name,
               const location& loc,
               bool opt,
               const variable_map& hints)
  {
    if (cast_false<bool> (bs[name + ".loaded"]))
    {
      if (cast_false<bool> (bs[name + ".configured"]))
        return &rs.root_extra->modules.find (name)->second.module;
    }
    else
    {
      if (module_state* ms = init_module (rs, bs, name, loc, opt, hints))
        return &ms->module;
    }

    return nullptr;
  }

  const shared_ptr<module>&
  load_module (scope& rs,
               scope& bs,
               const string& name,
               const location& loc,
               const variable_map& hints)
  {
    //@@ TODO: shouldn't we also check for configured? What if the previous
    //   attempt to load it was optional?

    return cast_false<bool> (bs[name + ".loaded"])
      ? rs.root_extra->modules.find (name)->second.module
      : init_module (rs, bs, name, loc, false /* optional */, hints)->module;
  }
}
