// file      : libbutl/diagnostics.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef __cpp_modules_ts
#include <libbutl/diagnostics.mxx>
#endif

#ifndef _WIN32
#  include <unistd.h> // write()
#else
#  include <libbutl/win32-utility.hxx>
#  include <io.h> //_write()
#endif

#include <cassert>

#ifndef __cpp_lib_modules_ts
#include <utility>
#include <exception>

#include <ios>      // ios::failure
#include <mutex>
#include <string>
#include <cstddef>  // size_t
#include <iostream> // cerr
#endif

// Other includes.

#ifdef __cpp_modules_ts
module butl.diagnostics;

// Only imports additional to interface.
#ifdef __clang__
#ifdef __cpp_lib_modules_ts
import std.core;
import std.io;
#endif
#endif

import std.threading;
import butl.utility;
import butl.optional;
import butl.fdstream; // stderr_fd(), fdterm()
#else
#include <libbutl/utility.mxx>
#include <libbutl/optional.mxx>
#include <libbutl/fdstream.mxx>
#endif

using namespace std;

namespace butl
{
  ostream* diag_stream = &cerr;

  static mutex diag_mutex;

  string diag_progress;
  static string diag_progress_blank; // Being printed blanks out the line.
  static size_t diag_progress_size;  // Size of the last printed progress.

  static optional<bool> diag_term;

  // Print the progress string to STDERR. Ignore underlying OS errors (this is
  // a progress bar after all, and throwing from dtors wouldn't be nice). Must
  // be called with the diag_mutex being aquired.
  //
  // Note that the output will not interleave with that of independent writers,
  // given that the printed strings are not longer than PIPE_BUF for POSIX
  // (which is at least 512 bytes on all platforms).
  //
  // @@ Is there any atomicity guarantees on Windows?
  //
  static inline void
  progress_print (string& s)
  {
    if (!diag_term)
    try
    {
      diag_term = fdterm (stderr_fd());
    }
    catch (const ios::failure&)
    {
      diag_term = false;
    }

    // If we print to a terminal, and the new progress string is shorter than
    // the printed one, then we will complement it with the required number of
    // spaces (to overwrite the trailing junk) prior to printing, and restore
    // it afterwards.
    //
    size_t n (s.size ());

    if (*diag_term && n < diag_progress_size)
      s.append (diag_progress_size - n, ' ');

    if (!s.empty ())
    {
      s += *diag_term
        ? '\r'  // Position the cursor at the beginning of the line.
        : '\n';

      try
      {
#ifndef _WIN32
        if (write (stderr_fd(), s.c_str (), s.size ())) {} // Suppress warning.
#else
        _write (stderr_fd(), s.c_str (), static_cast<unsigned int> (s.size ()));
#endif
      }
      catch (const ios::failure&) {}

      s.resize (n);           // Restore the progress string.
      diag_progress_size = n; // Save the size of the printed progress string.
    }
  }

  diag_stream_lock::diag_stream_lock ()
  {
    diag_mutex.lock ();

    // If diagnostics shares the output stream with the progress bar, then
    // blank out the line prior to printing the diagnostics, if required.
    //
    if (diag_stream == &cerr && diag_progress_size != 0)
      progress_print (diag_progress_blank);
  }

  diag_stream_lock::~diag_stream_lock ()
  {
    // If diagnostics shares output stream with the progress bar and we use
    // same-line progress style, then reprint the current progress string
    // that was overwritten with the diagnostics.
    //
    if (diag_stream == &cerr    &&
        !diag_progress.empty () &&
        diag_term               &&
        *diag_term)
      progress_print (diag_progress);

    diag_mutex.unlock ();
  }

  diag_progress_lock::diag_progress_lock ()
  {
    assert (diag_stream == &cerr);
    diag_mutex.lock ();
  }

  diag_progress_lock::~diag_progress_lock ()
  {
    progress_print (diag_progress);
    diag_mutex.unlock ();
  }

  static void
  default_writer (const diag_record& r)
  {
    r.os.put ('\n');
    diag_stream_lock () << r.os.str ();

    // We can endup flushing the result of several writes. The last one may
    // possibly be incomplete, but that's not a problem as it will also be
    // followed by the flush() call.
    //
    // @@ Strange: why not just hold the lock for both write and flush?
    //
    diag_stream->flush ();
  }

  void (*diag_record::writer) (const diag_record&) = &default_writer;

  void diag_record::
  flush () const
  {
    if (!empty_)
    {
      if (epilogue_ == nullptr)
      {
        if (writer != nullptr)
          writer (*this);

        empty_ = true;
      }
      else
      {
        // Clear the epilogue in case it calls us back.
        //
        auto e (epilogue_);
        epilogue_ = nullptr;
        e (*this); // Can throw.
        flush ();  // Call ourselves to write the data in case it returns.
      }
    }
  }

  diag_record::
  ~diag_record () noexcept (false)
  {
    // Don't flush the record if this destructor was called as part of the
    // stack unwinding.
    //
#ifdef __cpp_lib_uncaught_exceptions
    if (uncaught_ == std::uncaught_exceptions ())
      flush ();
#else
    // Fallback implementation. Right now this means we cannot use this
    // mechanism in destructors, which is not a big deal, except for one
    // place: exception_guard. Thus the ugly special check.
    //
    if (!std::uncaught_exception () || exception_unwinding_dtor ())
      flush ();
#endif
  }
}
