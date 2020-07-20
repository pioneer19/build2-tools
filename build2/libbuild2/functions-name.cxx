// file      : libbuild2/functions-name.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/scope.hxx>
#include <libbuild2/function.hxx>
#include <libbuild2/variable.hxx>
#include <libbuild2/algorithm.hxx>

using namespace std;

namespace build2
{
  // Convert name to target'ish name (see below for the 'ish part). Return
  // raw/unprocessed data in case this is an unknown target type (or called
  // out of scope). See scope::find_target_type() for details.
  //
  static pair<name, optional<string>>
  to_target_name (const scope* s, name&& n)
  {
    optional<string> e;

    if (s != nullptr)
    {
      auto rp (s->find_target_type (n, location ()));

      if (rp.first != nullptr)
        n.type = rp.first->name;

      e = move (rp.second);
    }

    return make_pair (move (n), move (e));
  }

  static const target&
  to_target (const scope& s, name&& n, name&& o)
  {
    if (const target* r = search_existing (n, s, o.dir))
      return *r;

    fail << "target "
         << (n.pair ? names {move (n), move (o)} : names {move (n)})
         << " not found" << endf;
  }

  void
  name_functions (function_map& m)
  {
    // These functions treat a name as a target/prerequisite name.
    //
    // While on one hand it feels like calling them target.name(), etc., would
    // have been more appropriate, on the other hand they can also be called
    // on prerequisite names. They also won't always return the same result as
    // if we were interrogating an actual target (e.g., the directory may be
    // relative). Plus we now have functions that can only be called on
    // targets (see below).
    //
    function_family fn (m, "name");

    fn["name"] = [](const scope* s, name n)
    {
      return to_target_name (s, move (n)).first.value;
    };
    fn["name"] = [](const scope* s, names ns)
    {
      return to_target_name (s, convert<name> (move (ns))).first.value;
    };

    // Note: returns NULL if extension is unspecified (default) and empty if
    // specified as no extension.
    //
    fn["extension"] = [](const scope* s, name n)
    {
      return to_target_name (s, move (n)).second;
    };
    fn["extension"] = [](const scope* s, names ns)
    {
      return to_target_name (s, convert<name> (move (ns))).second;
    };

    fn["directory"] = [](const scope* s, name n)
    {
      return to_target_name (s, move (n)).first.dir;
    };
    fn["directory"] = [](const scope* s, names ns)
    {
      return to_target_name (s, convert<name> (move (ns))).first.dir;
    };

    fn["target_type"] = [](const scope* s, name n)
    {
      return to_target_name (s, move (n)).first.type;
    };
    fn["target_type"] = [](const scope* s, names ns)
    {
      return to_target_name (s, convert<name> (move (ns))).first.type;
    };

    // Note: returns NULL if no project specified.
    //
    fn["project"] = [](const scope* s, name n)
    {
      return to_target_name (s, move (n)).first.proj;
    };
    fn["project"] = [](const scope* s, names ns)
    {
      return to_target_name (s, convert<name> (move (ns))).first.proj;
    };

    // Functions that can be called only on real targets.
    //
    function_family ft (m, "target");

    ft["path"] = [](const scope* s, names ns)
    {
      if (s == nullptr)
        fail << "target.path() called out of scope";

      // Most of the time we will have a single target so optimize for that.
      //
      small_vector<path, 1> r;

      for (auto i (ns.begin ()); i != ns.end (); ++i)
      {
        name& n (*i), o;
        const target& t (to_target (*s, move (n), move (n.pair ? *++i : o)));

        if (const auto* pt = t.is_a<path_target> ())
        {
          const path& p (pt->path ());

          if (&p != &empty_path)
            r.push_back (p);
          else
            fail << "target " << t << " path is not assigned";
        }
        else
          fail << "target " << t << " is not path-based";
      }

      // We want the result to be path if we were given a single target and
      // paths if multiple (or zero). The problem is, we cannot distinguish it
      // based on the argument type (e.g., name vs names) since passing an
      // out-qualified single target requires two names.
      //
      if (r.size () == 1)
        return value (move (r[0]));

      return value (paths (make_move_iterator (r.begin ()),
                           make_move_iterator (r.end ())));
    };

    // This one can only be called on a single target since we don't support
    // containers of process_path's (though we probably could).
    //
    fn["process_path"] = [](const scope* s, names ns)
    {
      if (s == nullptr)
        fail << "target.process_path() called out of scope";

      if (ns.empty () || ns.size () != (ns[0].pair ? 2 : 1))
        fail << "target.process_path() expects single target";

      name o;
      const target& t (
        to_target (*s, move (ns[0]), move (ns[0].pair ? ns[1] : o)));

      if (const auto* et = t.is_a<exe> ())
      {
        process_path r (et->process_path ());

        if (r.empty ())
          fail << "target " << t << " path is not assigned";

        return r;
      }
      else
        fail << "target " << t << " is not process_path-based" << endf;
    };

    // Name-specific overloads from builtins.
    //
    function_family fb (m, "builtin");

    fb[".concat"] = [](dir_path d, name n)
    {
      d /= n.dir;
      n.dir = move (d);
      return n;
    };
  }
}
