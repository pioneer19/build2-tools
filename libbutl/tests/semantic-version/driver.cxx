// file      : tests/semantic-version/driver.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <cassert>

#ifndef __cpp_lib_modules_ts
#include <iostream>
#endif

// Other includes.

#ifdef __cpp_modules_ts
#ifdef __cpp_lib_modules_ts
import std.core;
import std.io;
#endif
import butl.semantic_version;
#else
#include <libbutl/semantic-version.mxx>
#endif

using namespace std;
using namespace butl;

int
main ()
{
  using semver = semantic_version;
  using failed = const invalid_argument&;

  // Construction.
  //
  {
    semver v;
    assert (v.major == 0 && v.minor == 0 && v.patch == 0 && v.build.empty ());
  }

  {
    semver v (1, 2, 3);
    assert (v.major == 1 && v.minor == 2 && v.patch == 3 && v.build.empty ());
    assert (v.string () == "1.2.3");
  }
  {
    semver v (1, 2, 3, ".4");
    assert (v.major == 1 && v.minor == 2 && v.patch == 3 && v.build == ".4");
    assert (v.string ()     == "1.2.3.4");
    assert (v.string (true) == "1.2.3");
  }

  // Comparison.
  //
  assert (semver (2, 0 ,0)       > semver (1, 2, 3));
  assert (semver (1, 2, 0)       > semver (1, 1, 2));
  assert (semver (1, 1, 2)       > semver (1, 1, 1, ".2"));
  assert (semver (1, 1, 1, ".2") > semver (1, 1, 1, ".1"));
  assert (semver (1, 1, 1, ".1").compare (semver (1, 1, 1, ".2"), true) == 0);

  // String representation.
  //
  assert (semver ("1.2")            == semver (1, 2, 0));
  assert (semver ("1.2-3")          == semver (1, 2, 0, "-3"));
  assert (semver ("1.2.a1", "+-.")  == semver (1, 2, 0, ".a1"));
  assert (semver ("1.2.3")          == semver (1, 2, 3));
  assert (semver ("1.2.3-4")        == semver (1, 2, 3, "-4"));
  assert (semver ("1.2.3+4")        == semver (1, 2, 3, "+4"));
  assert (semver ("1.2.3.4", "+-.") == semver (1, 2, 3, ".4"));
  assert (semver ("1.2.3a",  "")    == semver (1, 2, 3, "a"));
  try {semver v ("1.2.3-4", false); assert (false);} catch (failed) {}
  try {semver v ("1.2.3.4");        assert (false);} catch (failed) {}
  try {semver v ("1.2.3a");         assert (false);} catch (failed) {}
  assert (!parse_semantic_version ("1.2.3.4"));

  // Numeric representation.
  //
  //               AAAAABBBBBCCCCC0000
  assert (semver (     100002000030000ULL)       == semver (1, 2, 3));
  assert (semver ( 9999999999999990000ULL, ".4") == semver (99999, 99999, 99999, ".4"));
  try { semver v (     100002000030001ULL);      assert (false);} catch (failed) {}
  try { semver v (10000000200003000000ULL);      assert (false);} catch (failed) {}
  assert (             100002000030000ULL        == semver (1, 2, 3).numeric ());
  assert (         9999999999999990000ULL        == semver (99999, 99999, 99999, ".4").numeric ());
  try { semver (999999, 0, 0).numeric ();        assert (false);} catch (failed) {}
}
