// file      : libbutl/string-parser.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef __cpp_modules_ts
#include <libbutl/string-parser.mxx>
#endif

// C includes.

#ifndef __cpp_lib_modules_ts
#include <string>
#include <vector>
#include <cstddef>
#include <utility>   // move()
#include <stdexcept>
#endif

// Other includes.

#ifdef __cpp_modules_ts
module butl.string_parser;

// Only imports additional to interface.
#ifdef __clang__
#ifdef __cpp_lib_modules_ts
import std.core;
#endif
#endif

#endif

using namespace std;

namespace butl
{
  namespace string_parser
  {
    // Utility functions.
    //
    inline static bool
    space (char c) noexcept
    {
      return c == ' ' || c == '\t';
    }

    vector<pair<string, size_t>>
    parse_quoted_position (const string& s, bool unquote)
    {
      vector<pair<string, size_t>> r;
      for (auto b (s.begin ()), i (b), e (s.end ()); i != e; )
      {
        for (; i != e && space (*i); ++i) ; // Skip spaces.

        if (i == e) // No more strings.
          break;

        string s;
        char quoting ('\0'); // Current quoting mode, can be used as bool.
        size_t pos (i - b);  // String position.

        for (; i != e; ++i)
        {
          char c (*i);

          if (!quoting)
          {
            if (space (c))             // End of string.
              break;

            if (c == '"' || c == '\'') // Begin of quoted substring.
            {
              quoting = c;

              if (!unquote)
                s += c;

              continue;
            }
          }
          else if (c == quoting)       // End of quoted substring.
          {
            quoting = '\0';

            if (!unquote)
              s += c;

            continue;
          }

          s += c;
        }

        if (quoting)
          throw invalid_string (i - b, "unterminated quoted string");

        r.emplace_back (move (s), pos);
      }

      return r;
    }

    vector<string>
    parse_quoted (const string& s, bool unquote)
    {
      vector<pair<string, size_t>> sp (parse_quoted_position (s, unquote));

      vector<string> r;
      r.reserve (sp.size ());
      for (auto& s: sp)
        r.emplace_back (move (s.first));

      return r;
    }

    string
    unquote (const string& s)
    {
      string r;
      char quoting ('\0'); // Current quoting mode, can be used as bool.

      for (auto i (s.begin ()), e (s.end ()); i != e; ++i)
      {
        char c (*i);

        if (!quoting)
        {
          if (c == '"' || c == '\'') // Begin of quoted substring.
          {
            quoting = c;
            continue;
          }
        }
        else if (c == quoting)       // End of quoted substring.
        {
          quoting = '\0';
          continue;
        }

        r += c;
      }

      return r;
    }

    vector<string>
    unquote (const vector<string>& v)
    {
      vector<string> r;
      r.reserve (v.size ());
      for (auto& s: v)
        r.emplace_back (unquote (s));

      return r;
    }
  }
}
