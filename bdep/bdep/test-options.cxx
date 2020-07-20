// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

// Begin prologue.
//
#include <bdep/types-parsers.hxx>
//
// End prologue.

#include <bdep/test-options.hxx>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>

namespace bdep
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

namespace bdep
{
  // cmd_test_options
  //

  cmd_test_options::
  cmd_test_options ()
  : immediate_ (),
    recursive_ ()
  {
  }

  bool cmd_test_options::
  parse (int& argc,
         char** argv,
         bool erase,
         ::bdep::cli::unknown_mode opt,
         ::bdep::cli::unknown_mode arg)
  {
    ::bdep::cli::argv_scanner s (argc, argv, erase);
    bool r = _parse (s, opt, arg);
    return r;
  }

  bool cmd_test_options::
  parse (int start,
         int& argc,
         char** argv,
         bool erase,
         ::bdep::cli::unknown_mode opt,
         ::bdep::cli::unknown_mode arg)
  {
    ::bdep::cli::argv_scanner s (start, argc, argv, erase);
    bool r = _parse (s, opt, arg);
    return r;
  }

  bool cmd_test_options::
  parse (int& argc,
         char** argv,
         int& end,
         bool erase,
         ::bdep::cli::unknown_mode opt,
         ::bdep::cli::unknown_mode arg)
  {
    ::bdep::cli::argv_scanner s (argc, argv, erase);
    bool r = _parse (s, opt, arg);
    end = s.end ();
    return r;
  }

  bool cmd_test_options::
  parse (int start,
         int& argc,
         char** argv,
         int& end,
         bool erase,
         ::bdep::cli::unknown_mode opt,
         ::bdep::cli::unknown_mode arg)
  {
    ::bdep::cli::argv_scanner s (start, argc, argv, erase);
    bool r = _parse (s, opt, arg);
    end = s.end ();
    return r;
  }

  bool cmd_test_options::
  parse (::bdep::cli::scanner& s,
         ::bdep::cli::unknown_mode opt,
         ::bdep::cli::unknown_mode arg)
  {
    bool r = _parse (s, opt, arg);
    return r;
  }

  void cmd_test_options::
  merge (const cmd_test_options& a)
  {
    CLI_POTENTIALLY_UNUSED (a);

    // project_options base
    //
    ::bdep::project_options::merge (a);

    if (a.immediate_)
    {
      ::bdep::cli::parser< bool>::merge (
        this->immediate_, a.immediate_);
    }

    if (a.recursive_)
    {
      ::bdep::cli::parser< bool>::merge (
        this->recursive_, a.recursive_);
    }
  }

  ::bdep::cli::usage_para cmd_test_options::
  print_usage (::std::ostream& os, ::bdep::cli::usage_para p)
  {
    CLI_POTENTIALLY_UNUSED (os);

    if (p != ::bdep::cli::usage_para::none)
      os << ::std::endl;

    os << "\033[1mTEST OPTIONS\033[0m" << ::std::endl;

    os << std::endl
       << "\033[1m--immediate\033[0m|\033[1m-i\033[0m          Also test immediate dependencies." << ::std::endl;

    os << std::endl
       << "\033[1m--recursive\033[0m|\033[1m-r\033[0m          Also test all dependencies, recursively." << ::std::endl;

    p = ::bdep::cli::usage_para::option;

    // project_options base
    //
    p = ::bdep::project_options::print_usage (os, p);

    return p;
  }

  struct _cli_cmd_test_options_desc_type: ::bdep::cli::options
  {
    _cli_cmd_test_options_desc_type ()
    {
      ::bdep::cmd_test_options::fill (*this);
    }
  };

  void cmd_test_options::
  fill (::bdep::cli::options& os)
  {
    // project_options base
    //
    ::bdep::project_options::fill (os);

    // --immediate
    //
    {
      ::bdep::cli::option_names a;
      a.push_back ("-i");
      std::string dv;
      ::bdep::cli::option o ("--immediate", a, true, dv);
      os.push_back (o);
    }

    // --recursive
    //
    {
      ::bdep::cli::option_names a;
      a.push_back ("-r");
      std::string dv;
      ::bdep::cli::option o ("--recursive", a, true, dv);
      os.push_back (o);
    }
  }

  const ::bdep::cli::options& cmd_test_options::
  description ()
  {
    static _cli_cmd_test_options_desc_type _cli_cmd_test_options_desc_;
    return _cli_cmd_test_options_desc_;
  }

  typedef
  std::map<std::string, void (*) (cmd_test_options&, ::bdep::cli::scanner&)>
  _cli_cmd_test_options_map;

  static _cli_cmd_test_options_map _cli_cmd_test_options_map_;

  struct _cli_cmd_test_options_map_init
  {
    _cli_cmd_test_options_map_init ()
    {
      _cli_cmd_test_options_map_["--immediate"] =
      &::bdep::cli::thunk< cmd_test_options, bool, &cmd_test_options::immediate_ >;
      _cli_cmd_test_options_map_["-i"] =
      &::bdep::cli::thunk< cmd_test_options, bool, &cmd_test_options::immediate_ >;
      _cli_cmd_test_options_map_["--recursive"] =
      &::bdep::cli::thunk< cmd_test_options, bool, &cmd_test_options::recursive_ >;
      _cli_cmd_test_options_map_["-r"] =
      &::bdep::cli::thunk< cmd_test_options, bool, &cmd_test_options::recursive_ >;
    }
  };

  static _cli_cmd_test_options_map_init _cli_cmd_test_options_map_init_;

  bool cmd_test_options::
  _parse (const char* o, ::bdep::cli::scanner& s)
  {
    _cli_cmd_test_options_map::const_iterator i (_cli_cmd_test_options_map_.find (o));

    if (i != _cli_cmd_test_options_map_.end ())
    {
      (*(i->second)) (*this, s);
      return true;
    }

    // project_options base
    //
    if (::bdep::project_options::_parse (o, s))
      return true;

    return false;
  }

  bool cmd_test_options::
  _parse (::bdep::cli::scanner& s,
          ::bdep::cli::unknown_mode opt_mode,
          ::bdep::cli::unknown_mode arg_mode)
  {
    // Can't skip combined flags (--no-combined-flags).
    //
    assert (opt_mode != ::bdep::cli::unknown_mode::skip);

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

            ::bdep::cli::argv_scanner ns (0, ac, av);

            if (_parse (co.c_str (), ns))
            {
              // Parsed the option but not its value?
              //
              if (ns.end () != 2)
                throw ::bdep::cli::invalid_value (co, v);

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

                ::bdep::cli::argv_scanner ns (0, ac, av);

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
            case ::bdep::cli::unknown_mode::skip:
            {
              s.skip ();
              r = true;
              continue;
            }
            case ::bdep::cli::unknown_mode::stop:
            {
              break;
            }
            case ::bdep::cli::unknown_mode::fail:
            {
              throw ::bdep::cli::unknown_option (o);
            }
          }

          break;
        }
      }

      switch (arg_mode)
      {
        case ::bdep::cli::unknown_mode::skip:
        {
          s.skip ();
          r = true;
          continue;
        }
        case ::bdep::cli::unknown_mode::stop:
        {
          break;
        }
        case ::bdep::cli::unknown_mode::fail:
        {
          throw ::bdep::cli::unknown_argument (o);
        }
      }

      break;
    }

    return r;
  }
}

namespace bdep
{
  ::bdep::cli::usage_para
  print_bdep_test_usage (::std::ostream& os, ::bdep::cli::usage_para p)
  {
    CLI_POTENTIALLY_UNUSED (os);

    if (p != ::bdep::cli::usage_para::none)
      os << ::std::endl;

    os << "\033[1mSYNOPSIS\033[0m" << ::std::endl
       << ::std::endl
       << "\033[1mbdep test\033[0m [\033[4moptions\033[0m] [\033[4mpkg-spec\033[0m] [\033[4mcfg-spec\033[0m] [\033[4mcfg-var\033[0m...]\033[0m" << ::std::endl
       << ::std::endl
       << "\033[4mcfg-spec\033[0m = (\033[1m@\033[0m\033[4mcfg-name\033[0m | \033[1m--config\033[0m|\033[1m-c\033[0m \033[4mcfg-dir\033[0m)... | \033[1m--all\033[0m|\033[1m-a\033[0m" << ::std::endl
       << "\033[4mpkg-spec\033[0m = (\033[1m--directory\033[0m|\033[1m-d\033[0m \033[4mpkg-dir\033[0m)... | \033[4mprj-spec\033[0m" << ::std::endl
       << "\033[4mprj-spec\033[0m = \033[1m--directory\033[0m|\033[1m-d\033[0m \033[4mprj-dir\033[0m\033[0m" << ::std::endl
       << ::std::endl
       << "\033[1mDESCRIPTION\033[0m" << ::std::endl
       << ::std::endl
       << "The \033[1mtest\033[0m command tests the project packages in one or more build" << ::std::endl
       << "configurations. Additionally, immediate or all dependencies of the project" << ::std::endl
       << "packages can be tested by specifying the \033[1m--immediate\033[0m|\033[1m-i\033[0m\033[0m or \033[1m--recursive\033[0m|\033[1m-r\033[0m\033[0m" << ::std::endl
       << "options, respectively." << ::std::endl
       << ::std::endl
       << "Underneath \033[1mtest\033[0m executes the \033[1mbpkg-pkg-test(1)\033[0m command which itself is not much" << ::std::endl
       << "more than the build system \033[1mtest\033[0m operation (see \033[1mb(1)\033[0m for details). As a result," << ::std::endl
       << "the main utility of this command is the ability to refer to build" << ::std::endl
       << "configurations by names and to project packages implicitly via the current" << ::std::endl
       << "working directory as well as to test dependencies." << ::std::endl
       << ::std::endl
       << "If no project or package directory is specified, then the current working" << ::std::endl
       << "directory is assumed. If no configuration is specified, then the default" << ::std::endl
       << "configuration is assumed. See \033[1mbdep-projects-configs(1)\033[0m for details on" << ::std::endl
       << "specifying projects and configurations. Optional \033[4mcfg-var\033[0m...\033[0m are the additional" << ::std::endl
       << "configuration variables to pass to the build system." << ::std::endl;

    p = ::bdep::cmd_test_options::print_usage (os, ::bdep::cli::usage_para::text);

    if (p != ::bdep::cli::usage_para::none)
      os << ::std::endl;

    os << "\033[1mDEFAULT OPTIONS FILES\033[0m" << ::std::endl
       << ::std::endl
       << "See \033[1mbdep-default-options-files(1)\033[0m for an overview of the default options files." << ::std::endl
       << "For the \033[1mtest\033[0m command the search start directory is the project directory. The" << ::std::endl
       << "following options files are searched for in each directory and, if found," << ::std::endl
       << "loaded in the order listed:" << ::std::endl
       << ::std::endl
       << "bdep.options" << ::std::endl
       << "bdep-test.options" << ::std::endl
       << ::std::endl
       << "The following \033[1mtest\033[0m command options cannot be specified in the default options" << ::std::endl
       << "files:" << ::std::endl
       << ::std::endl
       << "--directory|-d" << ::std::endl;

    p = ::bdep::cli::usage_para::text;

    return p;
  }
}

// Begin epilogue.
//
//
// End epilogue.

