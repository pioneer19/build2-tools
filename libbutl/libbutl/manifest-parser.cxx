// file      : libbutl/manifest-parser.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef __cpp_modules_ts
#include <libbutl/manifest-parser.mxx>
#endif

#include <cassert>

#ifndef __cpp_lib_modules_ts
#include <string>
#include <vector>
#include <cstdint>
#include <utility>
#include <stdexcept>

#include <sstream>
#endif

// Other includes.

#ifdef __cpp_modules_ts
module butl.manifest_parser;

// Only imports additional to interface.
#ifdef __clang__
#ifdef __cpp_lib_modules_ts
import std.core;
import std.io;
#endif
import butl.optional;
import butl.char_scanner;
import butl.manifest_types;
#endif

#endif

using namespace std;

namespace butl
{
  using parsing = manifest_parsing;
  using name_value = manifest_name_value;

  void manifest_parser::
  parse_next (name_value& r)
  {
    if (s_ == end)
    {
      r = name_value {
        "", "", line, column, line, column, position, position, position};

      return;
    }

    auto clp (skip_spaces ());
    xchar c (clp.first);
    uint64_t start_pos (clp.second);

    // Here is the problem: if we are in the 'body' state (that is,
    // we are parsing inside the manifest) and we see the special
    // empty name, then before returning the "start" pair for the
    // next manifest, we have to return the "end" pair. One way
    // would be to cache the "start" pair and return it on the
    // next call of next(). But that would require quite a bit
    // of extra logic. The alternative is to detect the beginning
    // of the empty name before parsing too far. This way, the
    // next call to next() will start parsing where we left of
    // and return the "start" pair naturally.
    //
    if (s_ == body && c == ':')
    {
      s_ = start;

      r = name_value {"", "",
                      c.line, c.column, c.line, c.column,
                      start_pos, c.position, c.position};
      return;
    }

    r.name.clear ();
    r.value.clear ();

    // Regardless of the state, what should come next is a name,
    // potentially the special empty one.
    //
    r.start_pos = start_pos;

    parse_name (r);

    skip_spaces ();
    c = get ("manifest");

    if (eos (c))
    {
      // This is ok as long as the name is empty.
      //
      if (!r.name.empty ())
        throw parsing (name_, c.line, c.column, "':' expected after name");

      s_ = end;

      // The "end" pair.
      //
      r.value_line = r.name_line;
      r.value_column = r.name_column;
      r.colon_pos = r.start_pos;
      r.end_pos = r.start_pos;
      return;
    }

    if (c != ':')
      throw parsing (name_, c.line, c.column, "':' expected after name");

    r.colon_pos = c.position;

    skip_spaces ();
    parse_value (r);

    c = peek ("manifest");

    // The character after the value should be either a newline or eos.
    //
    assert (c == '\n' || eos (c));

    r.end_pos = c.position;

    if (c == '\n')
      get (c);

    // Now figure out whether what we've got makes sense, depending
    // on the state we are in.
    //
    if (s_ == start)
    {
      // Start of the (next) manifest. The first pair should be the
      // special empty name/format version.
      //
      if (!r.name.empty ())
        throw parsing (name_, r.name_line, r.name_column,
                       "format version pair expected");

      // The version value is only mandatory for the first manifest in
      // a sequence.
      //
      if (r.value.empty ())
      {
        if (version_.empty ())
          throw parsing (name_, r.value_line, r.value_column,
                         "format version value expected");
        r.value = version_;
      }
      else
      {
        version_ = r.value; // Update with the latest.

        if (version_ != "1")
          throw parsing (name_, r.value_line, r.value_column,
                         "unsupported format version " + version_);
      }

      s_ = body;
    }
    else
    {
      // Parsing the body of the manifest.
      //

      // Should have been handled by the special case above.
      //
      assert (!r.name.empty ());
    }
  }

  pair<string, string> manifest_parser::
  split_comment (const string& v)
  {
    using iterator = string::const_iterator;

    auto space = [] (char c) -> bool {return c == ' ' || c == '\t';};

    iterator i (v.begin ());
    iterator e (v.end ());

    string r;
    size_t n (0);
    for (char c; i != e && (c = *i) != ';'; ++i)
    {
      // Unescape ';' character.
      //
      if (c == '\\' && i + 1 != e && *(i + 1) == ';')
        c = *++i;

      r += c;

      if (!space (c))
        n = r.size ();
    }

    // Strip the value trailing spaces.
    //
    if (r.size () != n)
      r.resize (n);

    // Find beginning of a comment (i).
    //
    if (i != e)
    {
      // Skip spaces.
      //
      for (++i; i != e && space (*i); ++i);
    }

    return make_pair (move (r), string (i, e));
  }

  void manifest_parser::
  parse_name (name_value& r)
  {
    auto peek = [this] () {return manifest_parser::peek ("manifest name");};

    xchar c (peek ());

    r.name_line = c.line;
    r.name_column = c.column;

    for (; !eos (c); c = peek ())
    {
      if (c == ':' || c == ' ' || c == '\t' || c == '\n')
        break;

      r.name += c;
      get (c);
    }
  }

  void manifest_parser::
  parse_value (name_value& r)
  {
    auto peek = [this] () {return manifest_parser::peek ("manifest value");};

    // Here we don't always track the last peeked character.
    //
    auto get = [this] () {manifest_parser::get ("manifest value");};

    xchar c (peek ());

    r.value_line = c.line;
    r.value_column = c.column;

    string& v (r.value);
    string::size_type n (0); // Size of last non-space character (simple mode).

    // Detect the multi-line mode introductor.
    //
    bool ml (false);
    if (c == '\\')
    {
      get ();
      xchar p (peek ());

      if (p == '\n')
      {
        get (); // Newline is not part of the value so skip it.
        c = peek ();
        ml = true;
      }
      else if (eos (p))
        ml = true;
      else
        unget (c);
    }

    // Multi-line value starts from the line that follows the name.
    //
    if (ml)
    {
      r.value_line = c.line;
      r.value_column = c.column;
    }

    // The nl flag signals that the preceding character was a "special
    // newline", that is, a newline that was part of the milti-line mode
    // introductor or an escape sequence.
    //
    for (bool nl (ml); !eos (c); c = peek ())
    {
      // Detect the special "\n\\\n" sequence. In the multi-line mode,
      // this is a "terminator". In the simple mode, this is a way to
      // specify a newline.
      //
      // The key idea here is this: if we "swallowed" any characters
      // (i.e., called get() without a matching unget()), then we
      // have to restart the loop in order to do all the tests for
      // the next character. Also, for this to work, we can only
      // add one character to v, which limits us to maximum three
      // characters look-ahead: one in v, one "ungot", and one
      // peeked.
      //
      // The first block handles the special sequence that starts with
      // a special newline. In multi-line mode, this is an "immediate
      // termination" where we "use" the newline from the introductor.
      // Note also that in the simple mode the special sequence can
      // only start with a special (i.e., escaped) newline.
      //
      if (nl)
      {
        nl = false;

        if (c == '\\')
        {
          get ();
          xchar c1 (peek ());

          if (c1 == '\n' || eos (c1))
          {
            if (ml)
              break;
            else
            {
              if (c1 == '\n')
                get ();

              v += '\n'; // Literal newline.
              n = v.size ();
              continue; // Restart from the next character.
            }
          }
          else
            unget (c); // Fall through.
        }
      }

      if (c == '\n')
      {
        if (ml)
        {
          get ();
          xchar c1 (peek ());

          if (c1 == '\\')
          {
            get ();
            xchar c2 (peek ());

            if (c2 == '\n' || eos (c2))
              break;
            else
            {
              v += '\n';
              unget (c1);
              continue; // Restart from c1 (slash).
            }
          }
          else
            unget (c); // Fall through.
        }
        else
          break; // Simple value terminator.
      }

      // Detect the newline escape sequence. The same look-ahead
      // approach as above.
      //
      if (c == '\\')
      {
        get ();
        xchar c1 (peek ());

        if (c1 == '\n' || eos (c1))
        {
          if (c1 == '\n')
          {
            get ();
            nl = true; // This is a special newline.
          }
          continue; // Restart from the next character.
        }
        else if (c1 == '\\')
        {
          get ();
          xchar c2 (peek ());

          if (c2 == '\n' || eos (c1))
          {
            v += '\\';
            n = v.size ();
            // Restart from c2 (newline/eos).
          }
          else
          {
            v += '\\';
            n = v.size ();
            unget (c1); // Restart from c1 (second slash).
          }

          continue;
        }
        else
          unget (c); // Fall through.
      }

      get ();
      v += c;

      if (!ml && c != ' ' && c != '\t')
        n = v.size ();
    }

    // Cut off trailing whitespaces.
    //
    if (!ml)
      v.resize (n);
  }

  pair<manifest_parser::xchar, uint64_t> manifest_parser::
  skip_spaces ()
  {
    auto peek = [this] () {return manifest_parser::peek ("manifest");};

    xchar c (peek ());
    bool start (c.column == 1);
    uint64_t lp (c.position);

    for (; !eos (c); c = peek ())
    {
      switch (c)
      {
      case ' ':
      case '\t':
        break;
      case '\n':
        {
          // Skip empty lines.
          //
          if (!start)
            return make_pair (c, lp);

          lp = c.position + 1;
          break;
        }
      case '#':
        {
          // We only recognize '#' as a start of a comment at the beginning
          // of the line (sans leading spaces).
          //
          if (!start)
            return make_pair (c, lp);

          get (c);

          // Read until newline or eos.
          //
          for (c = peek (); !eos (c) && c != '\n'; c = peek ())
            get (c);

          continue;
        }
      default:
        return make_pair (c, lp); // Not a space.
      }

      get (c);
    }

    return make_pair (c, lp);
  }

  // manifest_parsing
  //

  static inline string
  format (const string& n, uint64_t l, uint64_t c, const string& d)
  {
    ostringstream os;
    if (!n.empty ())
      os << n << ':';
    os << l << ':' << c << ": error: " << d;
    return os.str ();
  }

  manifest_parsing::
  manifest_parsing (const string& n, uint64_t l, uint64_t c, const string& d)
      : runtime_error (format (n, l, c, d)),
        name (n), line (l), column (c), description (d)
  {
  }

  manifest_parsing::
  manifest_parsing (const string& d)
      : runtime_error (d),
        line (0), column (0), description (d)
  {
  }

  // parse_manifest
  //
  static bool
  try_parse_manifest (manifest_parser& p,
                      vector<manifest_name_value>& r,
                      bool allow_eos)
  {
    // Read the format version or eos pair. Note that the version is verified
    // by the parser.
    //
    manifest_name_value nv (p.next ());

    // Bail out if eos is reached and is allowed.
    //
    if (nv.empty () && allow_eos)
      return false;

    if (!nv.name.empty () || nv.empty ())
      throw manifest_parsing (p.name (),
                              nv.value_line, nv.value_column,
                              "start of manifest expected");

    for (nv = p.next (); !nv.empty (); nv = p.next ())
      r.push_back (move (nv));

    return true;
  }

  bool
  try_parse_manifest (manifest_parser& p, vector<manifest_name_value>& r)
  {
    return try_parse_manifest (p, r, true /* allow_eos */);
  }

  void
  parse_manifest (manifest_parser& p, std::vector<manifest_name_value>& r)
  {
    try_parse_manifest (p, r, false /* allow_eos */);
  }
}
