// file      : libbutl/uuid.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbutl/uuid.hxx>

#include <errno.h> // ENOTSUP

#include <cstdio>       // sprintf() scanf()
#include <cstring>      // strlen()
#include <stdexcept>
#include <system_error>

using namespace std;

namespace butl
{
  array<char, 37> uuid::
  c_string (bool upper) const
  {
    array<char, 37> r;

    sprintf (r.data (),
             (upper
              ? "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"
              : "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
             time_low,
             time_mid,
             time_hiv,
             clock_seq_hir,
             clock_seq_low,
             node[0], node[1], node[2], node[3], node[4], node[5]);

    return r;
  }

  void uuid::
  assign (const char* s)
  {
    if (s != nullptr && strlen (s) == 36 && s[8] == '-')
    {
      if (sscanf (s,
                  "%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
                  &time_low,
                  &time_mid,
                  &time_hiv,
                  &clock_seq_hir,
                  &clock_seq_low,
                  &node[0], &node[1], &node[2],
                  &node[3], &node[4], &node[5]) == 11)
        return;
    }

    throw invalid_argument ("invalid UUID string representation");
  }

  uuid_system_generator uuid::system_generator;

  // Utility function used by platform-specified uuid-*.cxx implementations.
  //
  void
  uuid_throw_weak ()
  {
    throw system_error (ENOTSUP,
                        generic_category (),
                        "strong UUID uniqueness cannot be guaranteed");
  }

#ifdef BUILD2_BOOTSTRAP
  uuid uuid_system_generator::
  generate (bool)
  {
    throw system_error (ENOTSUP,
                        generic_category (),
                        "no UUID generation support during bootstrap");
  }

  void uuid_system_generator::
  initialize ()
  {
  }

  void uuid_system_generator::
  terminate ()
  {
  }
#endif // BUILD2_BOOTSTRAP
}
