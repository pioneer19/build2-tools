// file      : libbutl/manifest-serializer.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef __cpp_modules_ts
#include <libbutl/manifest-serializer.mxx>
#endif

#include <cassert>

#ifndef __cpp_lib_modules_ts
#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>

#include <ostream>
#endif

// Other includes.

#ifdef __cpp_modules_ts
module butl.manifest_serializer;

// Only imports additional to interface.
#ifdef __clang__
#ifdef __cpp_lib_modules_ts
import std.core;
import std.io;
#endif
import butl.manifest_types;
#endif

import butl.utf8;
import butl.utility;
#else
#include <libbutl/utf8.mxx>
#include <libbutl/utility.mxx>
#endif

using namespace std;

namespace butl
{
  using serialization = manifest_serialization;

  void manifest_serializer::
  write_next (const string& n, const string& v)
  {
    switch (s_)
    {
    case start:
      {
        if (!n.empty ())
          throw serialization (name_, "format version pair expected");

        if (v.empty ())
        {
          // End of manifests.
          //
          os_.flush ();
          s_ = end;
          break;
        }

        if (v != "1")
          throw serialization (name_, "unsupported format version " + v);

        os_ << ':';

        if (v != version_)
        {
          os_ << ' ' << v;
          version_ = v;
        }

        os_ << endl;
        s_ = body;
        break;
      }
    case body:
      {
        if (n.empty ())
        {
          s_ = start;

          // Start new manifest if the end-of-manifest pair is omitted.
          //
          if (!v.empty ())
            return next (n, v);

          break;
        }

        size_t l (write_name (n));
        os_ << ':';

        if (!v.empty ())
        {
          os_ << ' ';
          write_value (v, l + 2);
        }

        os_ << endl;
        break;
      }
    case end:
      {
        throw serialization (name_, "serialization after eos");
      }
    }
  }

  void manifest_serializer::
  comment (const string& t)
  {
    if (s_ == end)
      throw serialization (name_, "serialization after eos");

    string what;
    if (!utf8 (t, what, codepoint_types::graphic, U"\n\r\t"))
      throw serialization (name_, "invalid comment: " + what);

    os_ << '#';

    if (!t.empty ())
      os_ << ' ' << t;

    os_ << endl;
  }

  string manifest_serializer::
  merge_comment (const string& value, const string& comment)
  {
    string r;
    for (char c: value)
    {
      // Escape ';' character.
      //
      if (c == ';')
        r += '\\';

      r += c;
    }

    // Add the comment.
    //
    if (!comment.empty ())
    {
      r += "; ";
      r += comment;
    }

    return r;
  }

  size_t manifest_serializer::
  write_name (const string& n)
  {
    if (n.empty ())
      throw serialization (name_, "empty name");

    if (n[0] == '#')
      throw serialization (name_, "name starts with '#'");

    size_t r (0);
    pair<bool, bool> v;
    utf8_validator val (codepoint_types::graphic, U"\n\r\t");

    string what;
    for (char c: n)
    {
      v = val.validate (c, what);

      if (!v.first)
        throw serialization (name_, "invalid name: " + what);

      if (v.second) // Sequence last byte?
      {
        // Note: ASCII characters may not be a part of a multi-byte sequence.
        //
        switch (c)
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n': throw serialization (name_, "name contains whitespace");
        case ':':  throw serialization (name_, "name contains ':'");
        default:   break;
        }

        ++r;
      }
    }

    // Make sure that the last UTF-8 sequence is complete.
    //
    if (!v.second)
      throw serialization (name_, "invalid name: incomplete UTF-8 sequence");

    os_ << n;
    return r;
  }

  void manifest_serializer::
  write_value (const char* s, size_t n, size_t cl)
  {
    utf8_validator val (codepoint_types::graphic, U"\n\r\t");

    char c ('\0');
    bool b (true); // Begin of UTF-8 byte sequence.

    // The idea is to break on the 77th codepoint (i.e., write it
    // on the next line) which means we have written 76 codepoints
    // on this line plus 2 for '\' and '\n', which gives us 78.
    //
    string what;
    for (const char* e (s + n); s != e; s++)
    {
      char pc (c);
      c = *s;

      pair<bool, bool> v (val.validate (c, what));

      if (!v.first)
        throw serialization (name_, "invalid value: " + what);

      // Note that even the "hard" break (see below) is not that hard when it
      // comes to breaking the line right after the backslash. Doing so would
      // inject the redundant newline character, as the line-terminating
      // backslash would be escaped. So we delay breaking till the next
      // non-backslash character. We also delay until the beginning of a UTF-8
      // sequence.
      //
      if (pc != '\\' && b && !long_lines_)
      {
        bool br (false); // Break the line.

        // If this is a whitespace, see if it's a good place to break the
        // line.
        //
        if (c == ' ' || c == '\t')
        {
          // Find the next whitespace (or the end) and see if it is a better
          // place.
          //
          for (const char* w (s + 1); ; w++)
          {
            if (w == e || *w == ' ' || *w == '\t')
            {
              // Is this whitespace past where we need to break? Also see
              // below the "hard" break case for why we use 78 at the end.
              //
              if (cl + static_cast<size_t> (w - s) > (w != e ? 77U : 78U))
              {
                // Only break if this whitespace is close enough to
                // the end of the line.
                //
                br = (cl > 57);
              }

              break;
            }
          }
        }

        // Do we have to do a "hard" break (i.e., without a whitespace)?
        // If there is just one character left, then instead of writing
        // '\' and then the character on the next line, we might as well
        // write it on this line.
        //
        if (cl >= (s + 1 != e ? 77U : 78U))
          br = true;

        if (br)
        {
          os_ << '\\' << endl;
          cl = 0;
        }
      }

      os_ << c;

      b = v.second;

      if (b)
        ++cl;
    }

    // Make sure that the last UTF-8 sequence is complete.
    //
    if (!b)
      throw serialization (name_, "invalid value: incomplete UTF-8 sequence");

    // What comes next is always a newline. If the last character that
    // we have written is a backslash, escape it.
    //
    if (c == '\\')
      os_ << '\\';
  }

  void manifest_serializer::
  write_value (const string& v, size_t cl)
  {
    // Consider both \r and \n characters as line separators, and the
    // \r\n characters sequence as a single line separator.
    //
    auto nl = [&v] (size_t p = 0) {return v.find_first_of ("\r\n", p);};

    // Use the multi-line mode in any of the following cases:
    //
    // - column offset is too large (say greater than 39 (78/2) codepoints; we
    //   cannot start on the next line since that would start the multi-line
    //   mode)
    // - value contains newlines
    // - value contains leading/trailing whitespaces
    //
    // Should we use the multi-line mode for large column offsets if the long
    // lines flag is set? Probably yes, as this improves the manifest
    // readability, still allowing the user to easily copy the value which
    // seems to be the main reason for using the flag.
    //
    if (cl > 39 || nl () != string::npos ||
        v.front () == ' ' || v.front () == '\t' ||
        v.back () == ' ' || v.back () == '\t')
    {
      os_ << "\\" << endl; // Multi-line mode introductor.

      // Chunk the value into fragments separated by newlines.
      //
      for (size_t i (0), p (nl ()); ; p = nl (i))
      {
        if (p == string::npos)
        {
          // Last chunk.
          //
          write_value (v.c_str () + i, v.size () - i, 0);
          break;
        }

        write_value (v.c_str () + i, p - i, 0);
        os_ << endl;

        i = p + (v[p] == '\r' && v[p + 1] == '\n' ? 2 : 1);
      }

      os_ << endl << "\\"; // Multi-line mode terminator.
    }
    else
      write_value (v.c_str (), v.size (), cl);
  }

  // manifest_serialization
  //

  static inline string
  format (const string& n, const string& d)
  {
    string r;
    if (!n.empty ())
    {
      r += n;
      r += ": ";
    }
    r += "error: ";
    r += d;
    return r;
  }

  manifest_serialization::
  manifest_serialization (const string& n, const string& d)
      : runtime_error (format (n, d)), name (n), description (d)
  {
  }

  // serialize_manifest
  //
  void
  serialize_manifest (manifest_serializer& s,
                      const vector<manifest_name_value>& nvs,
                      bool eos)
  {
    s.next ("", "1"); // Start of manifest.

    for (const manifest_name_value& nv: nvs)
      s.next (nv.name, nv.value);

    s.next ("", ""); // End of manifest.

    if (eos)
      s.next ("", ""); // End of stream.
  }
}
