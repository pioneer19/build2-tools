// file      : libbuild2/functions-target-triplet.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/function.hxx>
#include <libbuild2/variable.hxx>

using namespace std;

namespace build2
{
  void
  target_triplet_functions (function_map& m)
  {
    function_family f (m, "target_triplet");

    f["string"] = [](target_triplet t) {return t.string ();};

    // Target triplet-specific overloads from builtins.
    //
    function_family b (m, "builtin");

    b[".concat"] = [](target_triplet l, string sr) {return l.string () + sr;};
    b[".concat"] = [](string sl, target_triplet r) {return sl + r.string ();};

    b[".concat"] = [](target_triplet l, names ur)
    {
      return l.string () + convert<string> (move (ur));
    };

    b[".concat"] = [](names ul, target_triplet r)
    {
      return convert<string> (move (ul)) + r.string ();
    };
  }
}
