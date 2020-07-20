// file      : libbutl/base64.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#ifndef __cpp_modules_ts
#include <libbutl/base64.mxx>
#endif

// C includes.

#ifndef __cpp_lib_modules_ts
#include <string>
#include <vector>

#include <cstddef>   // size_t
#include <istream>
#include <ostream>
#include <iterator>  // {istreambuf, ostreambuf, back_insert}_iterator
#include <stdexcept> // invalid_argument
#endif

// Other includes.

#ifdef __cpp_modules_ts
module butl.base64;

// Only imports additional to interface.
#ifdef __clang__
#ifdef __cpp_lib_modules_ts
import std.core;
import std.io;
#endif
#endif

#endif

using namespace std;

namespace butl
{
  static const char codes[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  // base64-encode the data in the iterator range [i, e). Write the encoded
  // data starting at the iterator position o.
  //
  template <typename I, typename O>
  static void
  base64_encode (I& i, const I& e, O& o)
  {
    const size_t un (65); // Non-existing index of the codes string.
    for (size_t n (0); i != e; ++n)
    {
      if (n && n % 19 == 0)
        *o++ = '\n'; // Split into lines, like the base64 utility does.

      auto next = [&i] () {return static_cast<unsigned char> (*i++);};

      unsigned char c (next ());
      size_t i1 ((c >> 2) & 0x3F);
      size_t i2 ((c << 4) & 0x30);

      size_t i3 (un);
      if (i != e)
      {
        c = next ();
        i2 |= (c >> 4) & 0xF;
        i3 = (c << 2) & 0x3C;
      }

      size_t i4 (un);
      if (i != e)
      {
        c = next ();
        i3 |= (c >> 6) & 0x3;
        i4 = c & 0x3F;
      }

      *o++ = codes[i1];
      *o++ = codes[i2];
      *o++ = i3 == un ? '=' : codes[i3];
      *o++ = i4 == un ? '=' : codes[i4];
    }
  }

  static char
  index (char c)
  {
    switch (c)
    {
    case '/': return 63;
    case '+': return 62;
    default:
      {
        if (c >= 'A' && c <= 'Z')
          return c - 'A';
        else if (c >= 'a' && c <= 'z')
          return c - 'a' + 26;
        else if (c >= '0' && c <= '9')
          return c - '0' + 52;
        else
          throw invalid_argument ("invalid input");
      }
    }
  }

  // base64-decode the data in the iterator range [i, e). Write the decoded
  // data starting at the iterator position o. Throw invalid_argument if the
  // input data is invalid.
  //
  template <typename I, typename O>
  static void
  base64_decode (I& i, const I& e, O& o)
  {
    auto bad = []() {throw invalid_argument ("invalid input");};

    auto next = [&i, &e, &bad]() -> char
    {
      if (i == e)
        bad ();
      return *i++;
    };

    while (i != e)
    {
      char c (next ());
      if (c == '\n') // @@ Should we check for '\r' as well ?
        continue;

      char i1 = index (c);
      char i2 = index (next ());
      *o++ = (i1 << 2) | (i2 >> 4);

      c = next ();
      if (c == '=')
      {
        if (next () != '=' || i != e)
          bad ();
      }
      else
      {
        char i3 = index (c);
        *o++ = (i2 << 4) | (i3 >> 2);

        c = next ();
        if (c == '=')
        {
          if (i != e)
            bad ();
        }
        else
          *o++ = (i3 << 6) | index (c);
      }
    }
  }

  string
  base64_encode (istream& is)
  {
    if (!is.good ())
      throw invalid_argument ("bad stream");

    string r;
    istreambuf_iterator<char> i (is);
    back_insert_iterator<string> o (r);

    base64_encode (i, istreambuf_iterator<char> (), o);
    is.setstate (istream::eofbit);
    return r;
  }

  void
  base64_encode (ostream& os, istream& is)
  {
    if (!os.good () || !is.good ())
      throw invalid_argument ("bad stream");

    istreambuf_iterator<char> i (is);
    ostreambuf_iterator<char> o (os);
    base64_encode (i, istreambuf_iterator<char> (), o);

    if (o.failed ())
      os.setstate (istream::badbit);

    is.setstate (istream::eofbit);
  }

  string
  base64_encode (const vector<char>& v)
  {
    string r;
    back_insert_iterator<string> o (r);
    auto i (v.begin ());
    base64_encode (i, v.end (), o);
    return r;
  }

  void
  base64_decode (ostream& os, istream& is)
  {
    if (!os.good () || !is.good ())
      throw invalid_argument ("bad stream");

    istreambuf_iterator<char> i (is);
    ostreambuf_iterator<char> o (os);
    base64_decode (i, istreambuf_iterator<char> (), o);

    if (o.failed ())
      os.setstate (istream::badbit);

    is.setstate (istream::eofbit);
  }

  void
  base64_decode (ostream& os, const string& s)
  {
    if (!os.good ())
      throw invalid_argument ("bad stream");

    ostreambuf_iterator<char> o (os);
    auto i (s.cbegin ());
    base64_decode (i, s.cend (), o);

    if (o.failed ())
      os.setstate (istream::badbit);
  }

  vector<char>
  base64_decode (const string& s)
  {
    vector<char> r;
    back_insert_iterator<vector<char>> o (r);
    auto i (s.cbegin ());
    base64_decode (i, s.cend (), o);
    return r;
  }
}
