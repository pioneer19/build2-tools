// file      : libbuild2/target.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/target.hxx>

#include <cstring> // strcmp()

#include <libbuild2/file.hxx>
#include <libbuild2/scope.hxx>
#include <libbuild2/search.hxx>
#include <libbuild2/algorithm.hxx>
#include <libbuild2/filesystem.hxx>
#include <libbuild2/diagnostics.hxx>

using namespace std;
using namespace butl;

namespace build2
{
  // target_type
  //
  bool target_type::
  is_a (const char* n) const
  {
    if (strcmp (name, n) == 0)
      return true;

    for (const target_type* b (base); b != nullptr; b = b->base)
      if (strcmp (b->name, n) == 0)
        return true;

    return false;
  }

  bool target_type::
  is_a_base (const target_type& tt) const
  {
    for (const target_type* b (base); b != nullptr; b = b->base)
      if (*b == tt)
        return true;

    return false;
  }

  // target_key
  //
  void target_key::
  as_name (names& r) const
  {
    string v;
    if (!name->empty ())
    {
      v = *name;
      target::combine_name (v, ext, false /* @@ TODO: what to do? */);
    }
    else
      assert (!ext);

    r.emplace_back (*dir, type->name, move (v));

    if (!out->empty ())
    {
      r.back ().pair = '@';
      r.emplace_back (*out);
    }
  }

  // target_state
  //
  static const char* const target_state_[] =
  {
    "unknown",
    "unchanged",
    "postponed",
    "busy",
    "changed",
    "failed",
    "group"
  };

  ostream&
  operator<< (ostream& os, target_state ts)
  {
    return os << target_state_[static_cast<uint8_t> (ts)];
  }

  // target
  //
  const target::prerequisites_type target::empty_prerequisites_;

  target::
  ~target ()
  {
    clear_data ();
  }

  const string& target::
  ext (string v)
  {
    ulock l (ctx.targets.mutex_);

    // Once the extension is set, it is immutable. However, it is possible
    // that someone has already "branded" this target with a different
    // extension.
    //
    optional<string>& e (*ext_);

    if (!e)
      e = move (v);
    else if (*e != v)
    {
      string o (*e);
      l.unlock ();

      fail << "conflicting extensions '" << o << "' and '" << v << "' "
           << "for target " << *this;
    }

    return *e;
  }

  group_view target::
  group_members (action) const
  {
    assert (false); // Not a group or doesn't expose its members.
    return group_view {nullptr, 0};
  }

  const scope& target::
  base_scope () const
  {
    // If this target is from the src tree, use its out directory to find
    // the scope.
    //
    return ctx.scopes.find (out_dir ());
  }

  const scope& target::
  root_scope () const
  {
    // This is tricky to cache so we do the lookup for now.
    //
    const scope* r (base_scope ().root_scope ());
    assert (r != nullptr);
    return *r;
  }

  pair<lookup, size_t> target::
  lookup_original (const variable& var, bool target_only) const
  {
    pair<lookup_type, size_t> r (lookup_type (), 0);

    ++r.second;
    {
      auto p (vars.lookup (var));
      if (p.first != nullptr)
        r.first = lookup_type (*p.first, p.second, vars);
    }

    const target* g (nullptr);

    if (!r.first)
    {
      ++r.second;

      // Skip looking up in the ad hoc group, which is semantically the
      // first/primary member.
      //
      if ((g = group == nullptr
           ? nullptr
           : group->adhoc_group () ? group->group : group))
      {
        auto p (g->vars.lookup (var));
        if (p.first != nullptr)
          r.first = lookup_type (*p.first, p.second, g->vars);
      }
    }

    // Delegate to scope's find_original().
    //
    if (!r.first)
    {
      if (!target_only)
      {
        auto p (base_scope ().lookup_original (
                  var,
                  &type (),
                  &name,
                  g != nullptr ? &g->type () : nullptr,
                  g != nullptr ? &g->name : nullptr));

        r.first = move (p.first);
        r.second = r.first ? r.second + p.second : p.second;
      }
      else
        r.second = size_t (~0);
    }

    return r;
  }

  value& target::
  append (const variable& var)
  {
    // Note: see also prerequisite::append() if changing anything here.

    // Note that here we want the original value without any overrides
    // applied.
    //
    auto l (lookup_original (var).first);

    if (l.defined () && l.belongs (*this)) // Existing var in this target.
      return vars.modify (l); // Ok since this is original.

    value& r (assign (var)); // NULL.

    if (l.defined ())
      r = *l; // Copy value (and type) from the outer scope.

    return r;
  }

  pair<lookup, size_t> target::opstate::
  lookup_original (const variable& var, bool target_only) const
  {
    pair<lookup_type, size_t> r (lookup_type (), 0);

    ++r.second;
    {
      auto p (vars.lookup (var));
      if (p.first != nullptr)
        r.first = lookup_type (*p.first, p.second, vars);
    }

    // Delegate to target's find_original().
    //
    if (!r.first)
    {
      auto p (target_->lookup_original (var, target_only));

      r.first = move (p.first);
      r.second = r.first ? r.second + p.second : p.second;
    }

    return r;
  }

  optional<string> target::
  split_name (string& v, const location& loc)
  {
    assert (!v.empty ());

    // We treat a single trailing dot as "specified no extension", double dots
    // as a single trailing dot (that is, an escape sequence which can be
    // repeated any number of times; in such cases we naturally assume there
    // is no default extension) and triple dots as "unspecified (default)
    // extension" (used when the extension in the name is not "ours", for
    // example, cxx{foo.test...} for foo.test.cxx). An odd number of dots
    // other than one or three is invalid.
    //
    optional<string> r;

    size_t p;
    if (v.back () != '.')
    {
      if ((p = path::traits_type::find_extension (v)) != string::npos)
        r = string (v.c_str () + p + 1);
    }
    else
    {
      if ((p = v.find_last_not_of ('.')) == string::npos)
        fail (loc) << "invalid target name '" << v << "'";

      p++;                      // Position of the first trailing dot.
      size_t n (v.size () - p); // Number of the trailing dots.

      if (n == 1)
        r = string ();
      else if (n == 3)
        ;
      else if (n % 2 == 0)
      {
        p += n / 2; // Keep half of the dots.
        r = string ();
      }
      else
        fail (loc) << "invalid trailing dot sequence in target name '"
                   << v << "'";
    }

    if (p != string::npos)
      v.resize (p);

    return r;
  }

  void target::
  combine_name (string& v, const optional<string>& e, bool de)
  {
    if (v.back () == '.')
    {
      assert (e && e->empty ());

      size_t p (v.find_last_not_of ('.'));
      assert (p != string::npos);

      p++;                      // Position of the first trailing dot.
      size_t n (v.size () - p); // Number of the trailing dots.
      v.append (n, '.');        // Double them.
    }
    else if (e)
    {
      v += '.';
      v += *e;  // Empty or not.
    }
    else if (de)
    {
      if (path::traits_type::find_extension (v) != string::npos)
        v += "...";
    }
  }

  // include()
  //
  include_type
  include_impl (action a,
                const target& t,
                const string& v,
                const prerequisite& p,
                const target* m)
  {
    context& ctx (t.ctx);

    include_type r (false);

    if      (v == "false") r = include_type::excluded;
    else if (v == "adhoc") r = include_type::adhoc;
    else if (v == "true")  r = include_type::normal;
    else
      fail << "invalid " << ctx.var_include->name << " variable value "
           << "'" << v << "' specified for prerequisite " << p;

    // Call the meta-operation override, if any (currently used by dist).
    //
    if (auto f = ctx.current_mif->include)
      r = f (a, t, prerequisite_member {p, m}, r);

    return r;
  }

  // target_set
  //
  const target* target_set::
  find (const target_key& k, tracer& trace) const
  {
    slock sl (mutex_);
    map_type::const_iterator i (map_.find (k));

    if (i == map_.end ())
      return nullptr;

    const target& t (*i->second);
    optional<string>& ext (i->first.ext);

    if (ext != k.ext)
    {
      ulock ul; // Keep locked for trace.

      if (k.ext)
      {
        // To update the extension we have to re-lock for exclusive access.
        // Between us releasing the shared lock and acquiring unique the
        // extension could change and possibly a new target that matches the
        // key could be inserted. In this case we simply re-run find ().
        //
        sl.unlock ();
        ul = ulock (mutex_);

        if (ext) // Someone set the extension.
        {
          ul.unlock ();
          return find (k, trace);
        }
      }

      l5 ([&]{
          diag_record r (trace);
          r << "assuming target ";
          to_stream (r.os,
                     target_key {&t.type (), &t.dir, &t.out, &t.name, ext},
                     stream_verb_max); // Always print the extension.
          r << " is the same as the one with ";

          if (!k.ext)
            r << "unspecified extension";
          else if (k.ext->empty ())
            r << "no extension";
          else
            r << "extension " << *k.ext;
        });

      if (k.ext)
        ext = k.ext;
    }

    return &t;
  }

  pair<target&, ulock> target_set::
  insert_locked (const target_type& tt,
                 dir_path dir,
                 dir_path out,
                 string name,
                 optional<string> ext,
                 bool implied,
                 tracer& trace)
  {
    target_key tk {&tt, &dir, &out, &name, move (ext)};
    target* t (const_cast<target*> (find (tk, trace)));

    if (t == nullptr)
    {
      // We sometimes call insert() even if we expect to find an existing
      // target in order to keep the same code (see cc/search_library()).
      //
      assert (ctx.phase != run_phase::execute);

      optional<string> e (
        tt.fixed_extension != nullptr
        ? string (tt.fixed_extension (tk, nullptr /* root scope */))
        : move (tk.ext));

      t = tt.factory (ctx, tt, move (dir), move (out), move (name));

      // Re-lock for exclusive access. In the meantime, someone could have
      // inserted this target so emplace() below could return false, in which
      // case we proceed pretty much like find() except already under the
      // exclusive lock.
      //
      ulock ul (mutex_);

      auto p (map_.emplace (target_key {&tt, &t->dir, &t->out, &t->name, e},
                            unique_ptr<target> (t)));

      map_type::iterator i (p.first);

      if (p.second)
      {
        t->ext_ = &i->first.ext;
        t->implied = implied;
        t->state.inner.target_ = t;
        t->state.outer.target_ = t;
        return pair<target&, ulock> (*t, move (ul));
      }

      // The "tail" of find().
      //
      t = i->second.get ();
      optional<string>& ext (i->first.ext);

      if (ext != e)
      {
        l5 ([&]{
            diag_record r (trace);
            r << "assuming target ";
            to_stream (
              r.os,
              target_key {&t->type (), &t->dir, &t->out, &t->name, ext},
              stream_verb_max); // Always print the extension.
            r << " is the same as the one with ";

            if (!e)
              r << "unspecified extension";
            else if (e->empty ())
              r << "no extension";
            else
              r << "extension " << *e;
          });

        if (e)
          ext = e;
      }

      // Fall through (continue as if the first find() returned this target).
    }

    if (!implied)
    {
      // The implied flag can only be cleared during the load phase.
      //
      assert (ctx.phase == run_phase::load);

      // Clear the implied flag.
      //
      if (t->implied)
        t->implied = false;
    }

    return pair<target&, ulock> (*t, ulock ());
  }

  ostream&
  to_stream (ostream& os, const target_key& k, optional<stream_verbosity> osv)
  {
    stream_verbosity sv (osv ? *osv : stream_verb (os));
    uint16_t dv (sv.path);
    uint16_t ev (sv.extension);

    // If the name is empty, then we want to print the last component of the
    // directory inside {}, e.g., dir{bar/}, not bar/dir{}.
    //
    bool n (!k.name->empty ());

    // Note: relative() returns empty for './'.
    //
    const dir_path& rd (dv < 1 ? relative (*k.dir) : *k.dir); // Relative.
    const dir_path& pd (n ? rd : rd.directory ());            // Parent.

    if (!pd.empty ())
    {
      if (dv < 1)
        os << diag_relative (pd);
      else
        to_stream (os, pd, true /* representation */);
    }

    const target_type& tt (*k.type);

    os << tt.name << '{';

    if (n)
    {
      os << *k.name;

      // If the extension derivation functions are NULL, then it means this
      // target type doesn't use extensions.
      //
      if (tt.fixed_extension != nullptr || tt.default_extension != nullptr)
      {
        // For verbosity level 0 we don't print the extension. For 1 we print
        // it if there is one. For 2 we print 'foo.?' if it hasn't yet been
        // assigned and 'foo.' if it is assigned as "no extension" (empty).
        //
        if (ev > 0 && (ev > 1 || (k.ext && !k.ext->empty ())))
        {
          os << '.' << (k.ext ? *k.ext : "?");
        }
      }
      else
        assert (!k.ext);
    }
    else
      to_stream (os,
                 rd.empty () ? dir_path (".") : rd.leaf (),
                 true /* representation */);

    os << '}';

    // If this target is from src, print its out.
    //
    if (!k.out->empty ())
    {
      if (dv < 1)
      {
        // Don't print '@./'.
        //
        const string& o (diag_relative (*k.out, false));

        if (!o.empty ())
          os << '@' << o;
      }
      else
        os << '@' << *k.out;
    }

    return os;
  }

  ostream&
  operator<< (ostream& os, const target_key& k)
  {
    if (auto p = k.type->print)
      p (os, k);
    else
      to_stream (os, k, stream_verb (os));

    return os;
  }

  // mtime_target
  //
  timestamp mtime_target::
  mtime () const
  {
    // Figure out from which target we should get the value.
    //
    const mtime_target* t (this);

    switch (ctx.phase)
    {
    case run_phase::load: break;
    case run_phase::match:
      {
        // Similar logic to matched_state_impl().
        //
        const opstate& s (state[action () /* inner */]);

        // Note: already synchronized.
        size_t o (s.task_count.load (memory_order_relaxed) - ctx.count_base ());

        if (o != offset_applied && o != offset_executed)
          break;
      }
      // Fall through.
    case run_phase::execute:
      {
        if (group_state (action () /* inner */))
          t = &group->as<mtime_target> ();

        break;
      }
    }

    return timestamp (duration (t->mtime_.load (memory_order_consume)));
  }

  // path_target
  //
  const string* path_target::
  derive_extension (bool search, const char* de)
  {
    // See also search_existing_file() if updating anything here.

    // Should be no default extension if searching.
    //
    assert (!search || de == nullptr);

    // The target should use extensions and they should not be fixed.
    //
    assert (de == nullptr || type ().default_extension != nullptr);

    if (const string* p = ext ())
      // Note that returning by reference is now MT-safe since once the
      // extension is specified, it is immutable.
      //
      return p;
    else
    {
      optional<string> e;

      // If the target type has the default extension function then try that
      // first. The reason for preferring it over what's been provided by the
      // caller is that this function will often use the 'extension' variable
      // which the user can use to override extensions. But since we pass the
      // provided default extension, the target type can override this logic
      // (see the exe{} target type for a use case).
      //
      if (auto f = type ().default_extension)
        e = f (key (), base_scope (), de, search);

      if (!e)
      {
        if (de != nullptr)
          e = de;
        else
        {
          if (search)
            return nullptr;

          fail << "no default extension for target " << *this << endf;
        }
      }

      return &ext (move (*e));
    }
  }

  const path& path_target::
  derive_path (const char* de, const char* np, const char* ns, const char* ee)
  {
    return derive_path_with_extension (derive_extension (de), np, ns, ee);
  }

  const path& path_target::
  derive_path_with_extension (const string& e,
                              const char* np,
                              const char* ns,
                              const char* ee)
  {
    path_type p (dir);

    if (np == nullptr || np[0] == '\0')
      p /= name;
    else
    {
      p /= np;
      p += name;
    }

    if (ns != nullptr)
      p += ns;

    return derive_path_with_extension (move (p), e, ee);
  }

  const path& path_target::
  derive_path (path_type p, const char* de, const char* ee)
  {
    return derive_path_with_extension (move (p), derive_extension (de), ee);
  }

  const path& path_target::
  derive_path_with_extension (path_type p, const string& e, const char* ee)
  {
    if (!e.empty ())
    {
      p += '.';
      p += e;
    }

    if (ee != nullptr)
    {
      p += '.';
      p += ee;
    }

    return path (move (p));
  }

  // Search functions.
  //

  const target*
  target_search (const target& t, const prerequisite_key& pk)
  {
    // The default behavior is to look for an existing target in the
    // prerequisite's directory scope.
    //
    return search_existing_target (t.ctx, pk);
  }

  const target*
  file_search (const target& t, const prerequisite_key& pk)
  {
    // First see if there is an existing target.
    //
    if (const target* e = search_existing_target (t.ctx, pk))
      return e;

    // Then look for an existing file in the src tree.
    //
    return search_existing_file (t.ctx, pk);
  }

  void
  target_print_0_ext_verb (ostream& os, const target_key& k)
  {
    stream_verbosity sv (stream_verb (os));
    if (sv.extension == 1) sv.extension = 0; // Remap 1 to 0.
    to_stream (os, k, sv);
  }

  void
  target_print_1_ext_verb (ostream& os, const target_key& k)
  {
    stream_verbosity sv (stream_verb (os));
    if (sv.extension == 0) sv.extension = 1; // Remap 0 to 1.
    to_stream (os, k, sv);
  }

  // type info
  //

  const target_type target::static_type
  {
    "target",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &target_search,
    false
  };

  const target_type mtime_target::static_type
  {
    "mtime_target",
    &target::static_type,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &target_search,
    false
  };

  const target_type path_target::static_type
  {
    "path_target",
    &mtime_target::static_type,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &target_search,
    false
  };

  extern const char file_ext_def[] = "";

  const target_type file::static_type
  {
    "file",
    &path_target::static_type,
    &target_factory<file>,
    &target_extension_fix<file_ext_def>,
    nullptr, /* default_extension */
    nullptr, /* pattern */
    &target_print_1_ext_verb, // Print extension even at verbosity level 0.
    &file_search,
    false
  };

  static const target*
  alias_search (const target& t, const prerequisite_key& pk)
  {
    // For an alias we don't want to silently create a target since it will do
    // nothing and it most likely not what the user intended.
    //
    const target* e (search_existing_target (t.ctx, pk));

    if (e == nullptr || e->implied)
      fail << "no explicit target for " << pk;

    return e;
  }

  const target_type alias::static_type
  {
    "alias",
    &target::static_type,
    &target_factory<alias>,
    nullptr, // Extension not used.
    nullptr,
    nullptr,
    nullptr,
    &alias_search,
    false
  };

  // dir
  //
  bool dir::
  check_implied (const scope& rs, const dir_path& d)
  {
    try
    {
      for (const dir_entry& e: dir_iterator (d, true /* ignore_dangling */))
      {
        switch (e.type ())
        {
        case entry_type::directory:
          {
            if (check_implied (rs, d / path_cast<dir_path> (e.path ())))
              return true;

            break;
          }
        case entry_type::regular:
          {
            if (e.path () == rs.root_extra->buildfile_file)
              return true;

            break;
          }
        default:
          break;
        }
      }
    }
    catch (const system_error& e)
    {
      fail << "unable to iterate over " << d << ": " << e << endf;
    }

    return false;
  }

  prerequisites dir::
  collect_implied (const scope& bs)
  {
    prerequisites_type r;
    const dir_path& d (bs.src_path ());

    try
    {
      for (const dir_entry& e: dir_iterator (d, true /* ignore_dangling */))
      {
        if (e.type () == entry_type::directory)
          r.push_back (
            prerequisite (nullopt,
                          dir::static_type,
                          dir_path (e.path ().representation ()), // Relative.
                          dir_path (), // In the out tree.
                          string (),
                          nullopt,
                          bs));
      }
    }
    catch (const system_error& e)
    {
      fail << "unable to iterate over " << d << ": " << e;
    }

    return r;
  }

  static const target*
  dir_search (const target& t, const prerequisite_key& pk)
  {
    tracer trace ("dir_search");

    // The first step is like in search_alias(): looks for an existing target.
    //
    const target* e (search_existing_target (t.ctx, pk));

    if (e != nullptr && !e->implied)
      return e;

    // If not found (or is implied), then try to load the corresponding
    // buildfile (which would normally define this target). Failed that, see
    // if we can assume an implied buildfile which would be equivalent to:
    //
    // ./: */
    //
    const scope& s (*pk.scope);
    const dir_path& d (*pk.tk.dir);

    // Note: this code is a custom version of parser::parse_include().

    // Calculate the new out_base. If the directory is absolute then we assume
    // it is already normalized.
    //
    dir_path out_base (d.relative ()
                       ? (s.out_path () / d).normalize ()
                       : d);

    // In our world modifications to the scope structure during search & match
    // should be "pure append" in the sense that they should not affect any
    // existing targets that have already been searched & matched.
    //
    // A straightforward way to enforce this is to not allow any existing
    // targets to be inside any newly created scopes (except, perhaps for the
    // directory target itself which we know hasn't been searched yet). This,
    // however, is not that straightforward to implement: we would need to
    // keep a directory prefix map for all the targets (e.g., in target_set).
    // Also, a buildfile could load from a directory that is not a
    // subdirectory of out_base. So for now we just assume that this is so.
    // And so it is.
    //
    bool retest (false);

    assert (t.ctx.phase == run_phase::match);
    {
      // Switch the phase to load.
      //
      phase_switch ps (t.ctx, run_phase::load);

      // This is subtle: while we were fussing around another thread may have
      // loaded the buildfile. So re-test now that we are in an exclusive
      // phase.
      //
      if (e == nullptr)
        e = search_existing_target (t.ctx, pk);

      if (e != nullptr && !e->implied)
        retest = true;
      else
      {
        // Ok, no luck, switch the scope.
        //
        pair<scope&, scope*> sp (
          switch_scope (*s.rw ().root_scope (), out_base));

        if (sp.second != nullptr) // Ignore scopes out of any project.
        {
          scope& base (sp.first);
          scope& root (*sp.second);

          const dir_path& src_base (base.src_path ());

          path bf (src_base / root.root_extra->buildfile_file);

          if (exists (bf))
          {
            l5 ([&]{trace << "loading buildfile " << bf << " for " << pk;});
            retest = source_once (root, base, bf);
          }
          else if (exists (src_base))
          {
            e = dir::search_implied (base, pk, trace);
            retest = (e != nullptr);
          }
        }
      }
    }

    assert (t.ctx.phase == run_phase::match);

    // If we loaded/implied the buildfile, examine the target again.
    //
    if (retest)
    {
      if (e == nullptr)
        e = search_existing_target (t.ctx, pk);

      if (e != nullptr && !e->implied)
        return e;
    }

    fail << "no explicit target for " << pk << endf;
  }

  static bool
  dir_pattern (const target_type&,
               const scope&,
               string& v,
               optional<string>&,
               const location&,
               bool r)
  {
    // Add/strip trailing directory separator unless already there.
    //
    bool d (path::traits_type::is_separator (v.back ()));

    if (r)
    {
      assert (d);
      v.resize (v.size () - 1);
    }
    else if (!d)
    {
      v += path::traits_type::directory_separator;
      return true;
    }

    return false;
  }

  const target_type dir::static_type
  {
    "dir",
    &alias::static_type,
    &target_factory<dir>,
    nullptr,              // Extension not used.
    nullptr,
    &dir_pattern,
    nullptr,
    &dir_search,
    false
  };

  const target_type fsdir::static_type
  {
    "fsdir",
    &target::static_type,
    &target_factory<fsdir>,
    nullptr,              // Extension not used.
    nullptr,
    &dir_pattern,
    nullptr,
    &target_search,
    false
  };

  static optional<string>
  exe_target_extension (const target_key&,
                        const scope&,
                        const char* e,
                        bool search)
  {
    // If we are searching for an executable that is not a target, then use
    // the host machine executable extension. Otherwise, if this is a target,
    // then we expect the rule to supply the target machine extension. But if
    // it doesn't, then fallback to no extension (e.g., a script).
    //
    return string (!search
                   ? (e != nullptr ? e : "")
                   :
#ifdef _WIN32
                   "exe"
#else
                   ""
#endif
    );
  }

#ifdef _WIN32
  static bool
  exe_target_pattern (const target_type&,
                      const scope&,
                      string& v,
                      optional<string>& e,
                      const location& l,
                      bool r)
  {
    if (r)
    {
      assert (e);
      e = nullopt;
    }
    else
    {
      e = target::split_name (v, l);

      if (!e)
      {
        e = "exe";
        return true;
      }
    }

    return false;
  }
#endif

  const target_type exe::static_type
  {
    "exe",
    &file::static_type,
    &target_factory<exe>,
    nullptr, /* fixed_extension */
    &exe_target_extension,
#ifdef _WIN32
    &exe_target_pattern,
#else
    nullptr,
#endif
    nullptr,
    &file_search,
    false
  };

  static const char*
  buildfile_target_extension (const target_key& tk, const scope* root)
  {
    // If the name is the special 'buildfile', then there is no extension,
    // otherwise it is 'build' (or 'build2file' and 'build2' in the
    // alternative naming scheme).

    // Let's try hard not to need the root scope by trusting the extensions
    // we were given.
    //
    // BTW, one way to get rid of all this root scope complication is to
    // always require explicit extension specification for buildfiles. Since
    // they are hardly ever mentioned explicitly, this should probably be ok.
    //
    if (tk.ext)
      return tk.ext->c_str ();

    if (root == nullptr)
    {
      // The same login as in target::root_scope().
      //
      // Note: we are guaranteed the scope is never NULL for prerequisites
      // (where out/dir could be relative and none of this will work).
      //
      // @@ CTX TODO
#if 0
      root = scopes.find (tk.out->empty () ? *tk.dir : *tk.out).root_scope ();
#endif

      if (root == nullptr || root->root_extra == nullptr)
        fail << "unable to determine extension for buildfile target " << tk;
    }

    return *tk.name == root->root_extra->buildfile_file.string ()
      ? ""
      : root->root_extra->build_ext.c_str ();
  }

  static bool
  buildfile_target_pattern (const target_type&,
                            const scope& base,
                            string& v,
                            optional<string>& e,
                            const location& l,
                            bool r)
  {
    if (r)
    {
      assert (e);
      e = nullopt;
    }
    else
    {
      e = target::split_name (v, l);

      if (!e)
      {
        const scope* root (base.root_scope ());

        if (root == nullptr || root->root_extra == nullptr)
          fail (l) << "unable to determine extension for buildfile pattern";

        if (v != root->root_extra->buildfile_file.string ())
        {
          e = root->root_extra->build_ext;
          return true;
        }
      }
    }

    return false;
  }

  const target_type buildfile::static_type
  {
    "buildfile",
    &file::static_type,
    &target_factory<buildfile>,
    &buildfile_target_extension,
    nullptr, /* default_extension */
    &buildfile_target_pattern,
    nullptr,
    &file_search,
    false
  };

  const target_type doc::static_type
  {
    "doc",
    &file::static_type,
    &target_factory<doc>,
    &target_extension_fix<file_ext_def>, // Same as file (no extension).
    nullptr, /* default_extension */
    nullptr, /* pattern */               // Same as file.
    &target_print_1_ext_verb,            // Same as file.
    &file_search,
    false
  };

  const target_type legal::static_type
  {
    "legal",
    &doc::static_type,
    &target_factory<legal>,
    &target_extension_fix<file_ext_def>, // Same as file (no extension).
    nullptr, /* default_extension */
    nullptr, /* pattern */               // Same as file.
    &target_print_1_ext_verb,            // Same as file.
    &file_search,
    false
  };

  static const char*
  man_extension (const target_key& tk, const scope*)
  {
    if (!tk.ext)
      fail << "man target " << tk << " must include extension (man section)";

    return tk.ext->c_str ();
  }

  const target_type man::static_type
  {
    "man",
    &doc::static_type,
    &target_factory<man>,
    &man_extension,           // Should be specified explicitly.
    nullptr, /* default_extension */
    nullptr,
    &target_print_1_ext_verb, // Print extension even at verbosity level 0.
    &file_search,
    false
  };

  extern const char man1_ext[] = "1"; // VC14 rejects constexpr.

  const target_type man1::static_type
  {
    "man1",
    &man::static_type,
    &target_factory<man1>,
    &target_extension_fix<man1_ext>,
    nullptr,  /* default_extension */
    &target_pattern_fix<man1_ext>,
    &target_print_0_ext_verb, // Fixed extension, no use printing.
    &file_search,
    false
  };

  static const char*
  manifest_target_extension (const target_key& tk, const scope*)
  {
    // If the name is special 'manifest', then there is no extension,
    // otherwise it is .manifest.
    //
    return *tk.name == "manifest" ? "" : "manifest";
  }

  static bool
  manifest_target_pattern (const target_type&,
                           const scope&,
                           string& v,
                           optional<string>& e,
                           const location& l,
                           bool r)
  {
    if (r)
    {
      assert (e);
      e = nullopt;
    }
    else
    {
      e = target::split_name (v, l);

      if (!e && v != "manifest")
      {
        e = "manifest";
        return true;
      }
    }

    return false;
  }

  const target_type manifest::static_type
  {
    "manifest",
    &doc::static_type,
    &target_factory<manifest>,
    &manifest_target_extension,
    nullptr, /* default_extension */
    &manifest_target_pattern,
    nullptr,
    &file_search,
    false
  };
}
