// file      : libbutl/manifest-rewriter.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef __cpp_modules_ts
#include <libbutl/manifest-rewriter.mxx>
#endif

#include <cassert>

// C includes.

#ifndef __cpp_lib_modules_ts
#include <string>
#include <cstdint> // uint64_t
#include <cstddef> // size_t
#endif

// Other includes.

#ifdef __cpp_modules_ts
module butl.manifest_rewriter;

// Only imports additional to interface.
#ifdef __clang__
#ifdef __cpp_lib_modules_ts
import std.core;
#endif
import butl.path;
import butl.fdstream;
import butl.manifest_types;
#endif

import butl.utility;             // utf8_length()
import butl.manifest_serializer;
#else
#include <libbutl/utility.mxx>
#include <libbutl/manifest-serializer.mxx>
#endif

using namespace std;

namespace butl
{
  manifest_rewriter::
  manifest_rewriter (path p, bool long_lines)
      : path_ (move (p)),
        long_lines_ (long_lines),
        fd_ (fdopen (path_,
                     fdopen_mode::in  |
                     fdopen_mode::out |
                     fdopen_mode::exclusive))
  {
  }

  // Seek the file descriptor to the specified logical position and truncate
  // the file. Return the file suffix (cached prior to truncating) starting
  // from the specified position.
  //
  static string
  truncate (auto_fd& fd, uint64_t pos, uint64_t suffix_pos)
  {
    string r;
    {
      // Temporary move the descriptor into the stream.
      //
      ifdstream is (move (fd));
      fdbuf& buf (static_cast<fdbuf&> (*is.rdbuf ()));

      // Read suffix.
      //
      buf.seekg (suffix_pos);
      r = is.read_text ();

      // Seek to the specified position and move the file descriptor back.
      //
      buf.seekg (pos);
      fd = is.release ();
    }

    // Truncate the file starting from the current position. Note that we need
    // to use the physical position rather than logical.
    //
    fdtruncate (fd.get (), fdseek (fd.get (), 0, fdseek_mode::cur));
    return r;
  }

  void manifest_rewriter::
  replace (const manifest_name_value& nv)
  {
    assert (nv.colon_pos != 0); // Sanity check.

    // Truncate right after the value colon.
    //
    string suffix (truncate (fd_, nv.colon_pos + 1, nv.end_pos));

    // Temporary move the descriptor into the stream.
    //
    ofdstream os (move (fd_));

    if (!nv.value.empty ())
    {
      os << ' ';

      manifest_serializer s (os, path_.string (), long_lines_);

      // Note that the name can be surrounded with the ASCII whitespace
      // characters and the start_pos refers to the first character in the
      // line.
      //
      // Also note that we assume the already serialized name to be a valid
      // UTF-8 byte string and so utf8_length() may not throw.
      //
      s.write_value (nv.value,
                     static_cast<size_t> (nv.colon_pos - nv.start_pos) -
                     (nv.name.size () - utf8_length (nv.name)) + 2);
    }

    os << suffix;

    // Move the file descriptor back.
    //
    fd_ = os.release (); // Note: flushes the buffer.
  }

  void manifest_rewriter::
  insert (const manifest_name_value& pos, const manifest_name_value& nv)
  {
    assert (pos.end_pos != 0); // Sanity check.

    // We could have just started writing over the suffix but the truncation
    // doesn't hurt.
    //
    string suffix (truncate (fd_, pos.end_pos, pos.end_pos));

    // Temporary move the descriptor into the stream.
    //
    ofdstream os (move (fd_));
    os << '\n';

    manifest_serializer s (os, path_.string (), long_lines_);
    size_t n (s.write_name (nv.name));

    os << ':';

    if (!nv.value.empty ())
    {
      os << ' ';

      // Note that the name can be surrounded with the ASCII whitespace
      // characters and the start_pos refers to the first character in the
      // line.
      //
      s.write_value (nv.value,
                     static_cast<size_t> (nv.colon_pos - nv.start_pos) -
                     (nv.name.size () - n) + 2);
    }

    os << suffix;

    // Move the file descriptor back.
    //
    fd_ = os.release (); // Note: flushes the buffer.
  }
}
