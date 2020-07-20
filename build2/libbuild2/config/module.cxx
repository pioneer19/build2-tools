// file      : libbuild2/config/module.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/config/module.hxx>

#include <libbuild2/scope.hxx>

using namespace std;

namespace build2
{
  namespace config
  {
    bool module::
    save_variable (const variable& var, uint64_t flags)
    {
      const string& n (var.name);

      // First try to find the module with the name that is the longest
      // prefix of this variable name.
      //
      auto& sm (saved_modules);
      auto i (sm.find_sup (n));

      // If no module matched, then create one based on the variable name.
      //
      if (i == sm.end ())
      {
        // Note: with 'config.' prefix.
        //
        i = sm.insert (string (n, 0, n.find ('.', 7))).first;
      }

      // Don't insert duplicates. The config.import.* vars are particularly
      // susceptible to duplication.
      //
      saved_variables& sv (i->second);
      auto j (sv.find (var));

      if (j != sv.end ())
      {
        assert (j->flags == flags);
        return false;
      }

      sv.push_back (saved_variable {var, flags});
      return true;
    }

    void module::
    save_variable (scope& rs, const variable& var, uint64_t flags)
    {
      if (module* m = rs.find_module<module> (module::name))
        m->save_variable (var, flags);
    }

    bool module::
    save_module (const char* name, int prio)
    {
      return saved_modules.insert (string ("config.") += name, prio).second;
    }

    void module::
    save_module (scope& rs, const char* name, int prio)
    {
      if (module* m = rs.find_module<module> (module::name))
        m->save_module (name, prio);
    }

    const string module::name ("config");
    const uint64_t module::version (1);
  }
}
