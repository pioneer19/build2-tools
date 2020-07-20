// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

// Begin prologue.
//
#include <bpkg/types-parsers.hxx>
//
// End prologue.

#include <bpkg/rep-info-options.hxx>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>

namespace bpkg
{
  namespace cli
  {
    template <typename X>
    struct parser
    {
      static void
      parse (X& x, bool& xs, scanner& s)
      {
        using namespace std;

        const char* o (s.next ());
        if (s.more ())
        {
          string v (s.next ());
          istringstream is (v);
          if (!(is >> x && is.peek () == istringstream::traits_type::eof ()))
            throw invalid_value (o, v);
        }
        else
          throw missing_value (o);

        xs = true;
      }

      static void
      merge (X& b, const X& a)
      {
        b = a;
      }
    };

    template <>
    struct parser<bool>
    {
      static void
      parse (bool& x, scanner& s)
      {
        s.next ();
        x = true;
      }

      static void
      merge (bool& b, const bool&)
      {
        b = true;
      }
    };

    template <>
    struct parser<std::string>
    {
      static void
      parse (std::string& x, bool& xs, scanner& s)
      {
        const char* o (s.next ());

        if (s.more ())
          x = s.next ();
        else
          throw missing_value (o);

        xs = true;
      }

      static void
      merge (std::string& b, const std::string& a)
      {
        b = a;
      }
    };

    template <typename X>
    struct parser<std::vector<X> >
    {
      static void
      parse (std::vector<X>& c, bool& xs, scanner& s)
      {
        X x;
        bool dummy;
        parser<X>::parse (x, dummy, s);
        c.push_back (x);
        xs = true;
      }

      static void
      merge (std::vector<X>& b, const std::vector<X>& a)
      {
        b.insert (b.end (), a.begin (), a.end ());
      }
    };

    template <typename X>
    struct parser<std::set<X> >
    {
      static void
      parse (std::set<X>& c, bool& xs, scanner& s)
      {
        X x;
        bool dummy;
        parser<X>::parse (x, dummy, s);
        c.insert (x);
        xs = true;
      }

      static void
      merge (std::set<X>& b, const std::set<X>& a)
      {
        b.insert (a.begin (), a.end ());
      }
    };

    template <typename K, typename V>
    struct parser<std::map<K, V> >
    {
      static void
      parse (std::map<K, V>& m, bool& xs, scanner& s)
      {
        const char* o (s.next ());

        if (s.more ())
        {
          std::string ov (s.next ());
          std::string::size_type p = ov.find ('=');

          K k = K ();
          V v = V ();
          std::string kstr (ov, 0, p);
          std::string vstr (ov, (p != std::string::npos ? p + 1 : ov.size ()));

          int ac (2);
          char* av[] =
          {
            const_cast<char*> (o),
            0
          };

          bool dummy;
          if (!kstr.empty ())
          {
            av[1] = const_cast<char*> (kstr.c_str ());
            argv_scanner s (0, ac, av);
            parser<K>::parse (k, dummy, s);
          }

          if (!vstr.empty ())
          {
            av[1] = const_cast<char*> (vstr.c_str ());
            argv_scanner s (0, ac, av);
            parser<V>::parse (v, dummy, s);
          }

          m[k] = v;
        }
        else
          throw missing_value (o);

        xs = true;
      }

      static void
      merge (std::map<K, V>& b, const std::map<K, V>& a)
      {
        for (typename std::map<K, V>::const_iterator i (a.begin ()); 
             i != a.end (); 
             ++i)
          b[i->first] = i->second;
      }
    };

    template <typename X, typename T, T X::*M>
    void
    thunk (X& x, scanner& s)
    {
      parser<T>::parse (x.*M, s);
    }

    template <typename X, typename T, T X::*M, bool X::*S>
    void
    thunk (X& x, scanner& s)
    {
      parser<T>::parse (x.*M, x.*S, s);
    }
  }
}

#include <map>
#include <cstring>

namespace bpkg
{
  // rep_info_options
  //

  rep_info_options::
  rep_info_options ()
  : name_ (),
    cert_fingerprint_ (),
    cert_name_ (),
    cert_organization_ (),
    cert_email_ (),
    repositories_ (),
    packages_ (),
    manifest_ (),
    deep_ (),
    repositories_file_ (),
    repositories_file_specified_ (false),
    packages_file_ (),
    packages_file_specified_ (false),
    type_ (),
    type_specified_ (false),
    directory_ (),
    directory_specified_ (false)
  {
  }

  bool rep_info_options::
  parse (int& argc,
         char** argv,
         bool erase,
         ::bpkg::cli::unknown_mode opt,
         ::bpkg::cli::unknown_mode arg)
  {
    ::bpkg::cli::argv_scanner s (argc, argv, erase);
    bool r = _parse (s, opt, arg);
    return r;
  }

  bool rep_info_options::
  parse (int start,
         int& argc,
         char** argv,
         bool erase,
         ::bpkg::cli::unknown_mode opt,
         ::bpkg::cli::unknown_mode arg)
  {
    ::bpkg::cli::argv_scanner s (start, argc, argv, erase);
    bool r = _parse (s, opt, arg);
    return r;
  }

  bool rep_info_options::
  parse (int& argc,
         char** argv,
         int& end,
         bool erase,
         ::bpkg::cli::unknown_mode opt,
         ::bpkg::cli::unknown_mode arg)
  {
    ::bpkg::cli::argv_scanner s (argc, argv, erase);
    bool r = _parse (s, opt, arg);
    end = s.end ();
    return r;
  }

  bool rep_info_options::
  parse (int start,
         int& argc,
         char** argv,
         int& end,
         bool erase,
         ::bpkg::cli::unknown_mode opt,
         ::bpkg::cli::unknown_mode arg)
  {
    ::bpkg::cli::argv_scanner s (start, argc, argv, erase);
    bool r = _parse (s, opt, arg);
    end = s.end ();
    return r;
  }

  bool rep_info_options::
  parse (::bpkg::cli::scanner& s,
         ::bpkg::cli::unknown_mode opt,
         ::bpkg::cli::unknown_mode arg)
  {
    bool r = _parse (s, opt, arg);
    return r;
  }

  void rep_info_options::
  merge (const rep_info_options& a)
  {
    CLI_POTENTIALLY_UNUSED (a);

    // common_options base
    //
    ::bpkg::common_options::merge (a);

    if (a.name_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->name_, a.name_);
    }

    if (a.cert_fingerprint_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->cert_fingerprint_, a.cert_fingerprint_);
    }

    if (a.cert_name_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->cert_name_, a.cert_name_);
    }

    if (a.cert_organization_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->cert_organization_, a.cert_organization_);
    }

    if (a.cert_email_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->cert_email_, a.cert_email_);
    }

    if (a.repositories_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->repositories_, a.repositories_);
    }

    if (a.packages_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->packages_, a.packages_);
    }

    if (a.manifest_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->manifest_, a.manifest_);
    }

    if (a.deep_)
    {
      ::bpkg::cli::parser< bool>::merge (
        this->deep_, a.deep_);
    }

    if (a.repositories_file_specified_)
    {
      ::bpkg::cli::parser< path>::merge (
        this->repositories_file_, a.repositories_file_);
      this->repositories_file_specified_ = true;
    }

    if (a.packages_file_specified_)
    {
      ::bpkg::cli::parser< path>::merge (
        this->packages_file_, a.packages_file_);
      this->packages_file_specified_ = true;
    }

    if (a.type_specified_)
    {
      ::bpkg::cli::parser< repository_type>::merge (
        this->type_, a.type_);
      this->type_specified_ = true;
    }

    if (a.directory_specified_)
    {
      ::bpkg::cli::parser< string>::merge (
        this->directory_, a.directory_);
      this->directory_specified_ = true;
    }
  }

  ::bpkg::cli::usage_para rep_info_options::
  print_usage (::std::ostream& os, ::bpkg::cli::usage_para p)
  {
    CLI_POTENTIALLY_UNUSED (os);

    if (p != ::bpkg::cli::usage_para::none)
      os << ::std::endl;

    os << "\033[1mREP-INFO OPTIONS\033[0m" << ::std::endl;

    os << std::endl
       << "\033[1m--name\033[0m|\033[1m-n\033[0m                Print the repository's name and location." << ::std::endl;

    os << std::endl
       << "\033[1m--cert-fingerprint\033[0m       Print the repository's certificate fingerprint or" << ::std::endl
       << "                         empty line if the repository is unsigned." << ::std::endl;

    os << std::endl
       << "\033[1m--cert-name\033[0m              Print the repository's certificate common name (CN) or" << ::std::endl
       << "                         empty line if the repository is unsigned." << ::std::endl;

    os << std::endl
       << "\033[1m--cert-organization\033[0m      Print the repository's certificate organization name" << ::std::endl
       << "                         (O) or empty line if the repository is unsigned." << ::std::endl;

    os << std::endl
       << "\033[1m--cert-email\033[0m             Print the repository's certificate email or empty line" << ::std::endl
       << "                         if the repository is unsigned." << ::std::endl;

    os << std::endl
       << "\033[1m--repositories\033[0m|\033[1m-r\033[0m        Print the list of complement and prerequisite" << ::std::endl
       << "                         repositories." << ::std::endl;

    os << std::endl
       << "\033[1m--packages\033[0m|\033[1m-p\033[0m            Print the list of available packages." << ::std::endl;

    os << std::endl
       << "\033[1m--manifest\033[0m               Instead of printing the information in the" << ::std::endl
       << "                         human-readable form, dump it (to \033[1mstdout\033[0m) as" << ::std::endl
       << "                         manifest(s). Normally you would use this option in" << ::std::endl
       << "                         combination with \033[1m--packages|-p\033[0m or \033[1m--repositories|-r\033[0m to" << ::std::endl
       << "                         only dump one of the manifests. If the \033[1m--deep\033[0m option" << ::std::endl
       << "                         is specified, then in the resulting packages manifest" << ::std::endl
       << "                         the \033[1m*-file\033[0m values are replaced with the contents of" << ::std::endl
       << "                         the referenced files. See also \033[1m--repositories-file\033[0m and" << ::std::endl
       << "                         \033[1m--packages-file\033[0m." << ::std::endl;

    os << std::endl
       << "\033[1m--deep\033[0m                   Verify files referenced by the \033[1m*-file\033[0m manifest values." << ::std::endl;

    os << std::endl
       << "\033[1m--repositories-file\033[0m \033[4mpath\033[0m If \033[1m--manifest\033[0m is specified, then save the repositories" << ::std::endl
       << "                         manifest to the specified file instead of dumping it" << ::std::endl
       << "                         to \033[1mstdout\033[0m." << ::std::endl;

    os << std::endl
       << "\033[1m--packages-file\033[0m \033[4mpath\033[0m     If \033[1m--manifest\033[0m is specified, then save the packages" << ::std::endl
       << "                         manifest to the specified file instead of dumping it" << ::std::endl
       << "                         to \033[1mstdout\033[0m." << ::std::endl;

    os << std::endl
       << "\033[1m--type\033[0m \033[4mtype\033[0m              Specify the repository type with valid values being" << ::std::endl
       << "                         \033[1mpkg\033[0m, \033[1mdir\033[0m, and \033[1mgit\033[0m. Refer to \033[1mbpkg-rep-add(1)\033[0m for" << ::std::endl
       << "                         details." << ::std::endl;

    os << std::endl
       << "\033[1m--directory\033[0m|\033[1m-d\033[0m \033[4mdir\033[0m       Use configuration in \033[4mdir\033[0m for the trusted certificate" << ::std::endl
       << "                         database." << ::std::endl;

    p = ::bpkg::cli::usage_para::option;

    // common_options base
    //
    p = ::bpkg::common_options::print_usage (os, p);

    return p;
  }

  typedef
  std::map<std::string, void (*) (rep_info_options&, ::bpkg::cli::scanner&)>
  _cli_rep_info_options_map;

  static _cli_rep_info_options_map _cli_rep_info_options_map_;

  struct _cli_rep_info_options_map_init
  {
    _cli_rep_info_options_map_init ()
    {
      _cli_rep_info_options_map_["--name"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::name_ >;
      _cli_rep_info_options_map_["-n"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::name_ >;
      _cli_rep_info_options_map_["--cert-fingerprint"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::cert_fingerprint_ >;
      _cli_rep_info_options_map_["--cert-name"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::cert_name_ >;
      _cli_rep_info_options_map_["--cert-organization"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::cert_organization_ >;
      _cli_rep_info_options_map_["--cert-email"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::cert_email_ >;
      _cli_rep_info_options_map_["--repositories"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::repositories_ >;
      _cli_rep_info_options_map_["-r"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::repositories_ >;
      _cli_rep_info_options_map_["--packages"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::packages_ >;
      _cli_rep_info_options_map_["-p"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::packages_ >;
      _cli_rep_info_options_map_["--manifest"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::manifest_ >;
      _cli_rep_info_options_map_["--deep"] =
      &::bpkg::cli::thunk< rep_info_options, bool, &rep_info_options::deep_ >;
      _cli_rep_info_options_map_["--repositories-file"] =
      &::bpkg::cli::thunk< rep_info_options, path, &rep_info_options::repositories_file_,
        &rep_info_options::repositories_file_specified_ >;
      _cli_rep_info_options_map_["--packages-file"] =
      &::bpkg::cli::thunk< rep_info_options, path, &rep_info_options::packages_file_,
        &rep_info_options::packages_file_specified_ >;
      _cli_rep_info_options_map_["--type"] =
      &::bpkg::cli::thunk< rep_info_options, repository_type, &rep_info_options::type_,
        &rep_info_options::type_specified_ >;
      _cli_rep_info_options_map_["--directory"] =
      &::bpkg::cli::thunk< rep_info_options, string, &rep_info_options::directory_,
        &rep_info_options::directory_specified_ >;
      _cli_rep_info_options_map_["-d"] =
      &::bpkg::cli::thunk< rep_info_options, string, &rep_info_options::directory_,
        &rep_info_options::directory_specified_ >;
    }
  };

  static _cli_rep_info_options_map_init _cli_rep_info_options_map_init_;

  bool rep_info_options::
  _parse (const char* o, ::bpkg::cli::scanner& s)
  {
    _cli_rep_info_options_map::const_iterator i (_cli_rep_info_options_map_.find (o));

    if (i != _cli_rep_info_options_map_.end ())
    {
      (*(i->second)) (*this, s);
      return true;
    }

    // common_options base
    //
    if (::bpkg::common_options::_parse (o, s))
      return true;

    return false;
  }

  bool rep_info_options::
  _parse (::bpkg::cli::scanner& s,
          ::bpkg::cli::unknown_mode opt_mode,
          ::bpkg::cli::unknown_mode arg_mode)
  {
    // Can't skip combined flags (--no-combined-flags).
    //
    assert (opt_mode != ::bpkg::cli::unknown_mode::skip);

    bool r = false;
    bool opt = true;

    while (s.more ())
    {
      const char* o = s.peek ();

      if (std::strcmp (o, "--") == 0)
      {
        opt = false;
      }

      if (opt)
      {
        if (_parse (o, s))
        {
          r = true;
          continue;
        }

        if (std::strncmp (o, "-", 1) == 0 && o[1] != '\0')
        {
          // Handle combined option values.
          //
          std::string co;
          if (const char* v = std::strchr (o, '='))
          {
            co.assign (o, 0, v - o);
            ++v;

            int ac (2);
            char* av[] =
            {
              const_cast<char*> (co.c_str ()),
              const_cast<char*> (v)
            };

            ::bpkg::cli::argv_scanner ns (0, ac, av);

            if (_parse (co.c_str (), ns))
            {
              // Parsed the option but not its value?
              //
              if (ns.end () != 2)
                throw ::bpkg::cli::invalid_value (co, v);

              s.next ();
              r = true;
              continue;
            }
            else
            {
              // Set the unknown option and fall through.
              //
              o = co.c_str ();
            }
          }

          // Handle combined flags.
          //
          char cf[3];
          {
            const char* p = o + 1;
            for (; *p != '\0'; ++p)
            {
              if (!((*p >= 'a' && *p <= 'z') ||
                    (*p >= 'A' && *p <= 'Z') ||
                    (*p >= '0' && *p <= '9')))
                break;
            }

            if (*p == '\0')
            {
              for (p = o + 1; *p != '\0'; ++p)
              {
                std::strcpy (cf, "-");
                cf[1] = *p;
                cf[2] = '\0';

                int ac (1);
                char* av[] =
                {
                  cf
                };

                ::bpkg::cli::argv_scanner ns (0, ac, av);

                if (!_parse (cf, ns))
                  break;
              }

              if (*p == '\0')
              {
                // All handled.
                //
                s.next ();
                r = true;
                continue;
              }
              else
              {
                // Set the unknown option and fall through.
                //
                o = cf;
              }
            }
          }

          switch (opt_mode)
          {
            case ::bpkg::cli::unknown_mode::skip:
            {
              s.skip ();
              r = true;
              continue;
            }
            case ::bpkg::cli::unknown_mode::stop:
            {
              break;
            }
            case ::bpkg::cli::unknown_mode::fail:
            {
              throw ::bpkg::cli::unknown_option (o);
            }
          }

          break;
        }
      }

      switch (arg_mode)
      {
        case ::bpkg::cli::unknown_mode::skip:
        {
          s.skip ();
          r = true;
          continue;
        }
        case ::bpkg::cli::unknown_mode::stop:
        {
          break;
        }
        case ::bpkg::cli::unknown_mode::fail:
        {
          throw ::bpkg::cli::unknown_argument (o);
        }
      }

      break;
    }

    return r;
  }
}

namespace bpkg
{
  ::bpkg::cli::usage_para
  print_bpkg_rep_info_usage (::std::ostream& os, ::bpkg::cli::usage_para p)
  {
    CLI_POTENTIALLY_UNUSED (os);

    if (p != ::bpkg::cli::usage_para::none)
      os << ::std::endl;

    os << "\033[1mSYNOPSIS\033[0m" << ::std::endl
       << ::std::endl
       << "\033[1mbpkg rep-info\033[0m [\033[4moptions\033[0m] \033[4mrep-loc\033[0m\033[0m" << ::std::endl
       << ::std::endl
       << "\033[1mDESCRIPTION\033[0m" << ::std::endl
       << ::std::endl
       << "The \033[1mrep-info\033[0m command prints various information about the specified repository." << ::std::endl
       << "By default it print the repository's name and location as the first line. If" << ::std::endl
       << "the repository is signed, the certificate information (name/organization/email)" << ::std::endl
       << "is printed as the next line followed by the certificate fingerprint. Then comes" << ::std::endl
       << "the list of complement and prerequisite repositories followed by the list of" << ::std::endl
       << "available packages." << ::std::endl
       << ::std::endl
       << "This default format can be altered in various ways using options listed below." << ::std::endl
       << "Note also that the information is written to \033[1mstdout\033[0m, not \033[1mstderr\033[0m." << ::std::endl
       << ::std::endl
       << "If the current working directory contains a \033[1mbpkg\033[0m configuration, then \033[1mrep-info\033[0m" << ::std::endl
       << "will use its certificate database for the repository authentication. That is," << ::std::endl
       << "it will trust the repository's certificate if it is already trusted by the" << ::std::endl
       << "configuration. Otherwise it will add the certificate to the configuration if" << ::std::endl
       << "you confirm it is trusted. You can specify an alternative configuration" << ::std::endl
       << "directory with the \033[1m--directory|-d\033[0m option. To disable using the configuration in" << ::std::endl
       << "the current working directory pass this option with an empty path." << ::std::endl;

    p = ::bpkg::rep_info_options::print_usage (os, ::bpkg::cli::usage_para::text);

    if (p != ::bpkg::cli::usage_para::none)
      os << ::std::endl;

    os << "\033[1mDEFAULT OPTIONS FILES\033[0m" << ::std::endl
       << ::std::endl
       << "See \033[1mbpkg-default-options-files(1)\033[0m for an overview of the default options files." << ::std::endl
       << "If the \033[1mrep-info\033[0m command uses an existing bpkg\033[0m configuration, then the start" << ::std::endl
       << "directory is the configuration directory. Otherwise, only the predefined" << ::std::endl
       << "directories (home, system, etc) are searched. The following options files are" << ::std::endl
       << "searched for in each directory and, if found, loaded in the order listed:" << ::std::endl
       << ::std::endl
       << "bpkg.options" << ::std::endl
       << "bpkg-rep-info.options" << ::std::endl
       << ::std::endl
       << "The following \033[1mrep-info\033[0m command options cannot be specified in the default" << ::std::endl
       << "options files:" << ::std::endl
       << ::std::endl
       << "--directory|-d" << ::std::endl;

    p = ::bpkg::cli::usage_para::text;

    return p;
  }
}

// Begin epilogue.
//
//
// End epilogue.

