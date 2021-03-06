// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

#ifndef BPKG_COMMON_OPTIONS_HXX
#define BPKG_COMMON_OPTIONS_HXX

// Begin prologue.
//
//
// End prologue.

#include <list>
#include <deque>
#include <vector>
#include <iosfwd>
#include <string>
#include <cstddef>
#include <exception>

#ifndef CLI_POTENTIALLY_UNUSED
#  if defined(_MSC_VER) || defined(__xlC__)
#    define CLI_POTENTIALLY_UNUSED(x) (void*)&x
#  else
#    define CLI_POTENTIALLY_UNUSED(x) (void)x
#  endif
#endif

namespace bpkg
{
  namespace cli
  {
    class usage_para
    {
      public:
      enum value
      {
        none,
        text,
        option
      };

      usage_para (value);

      operator value () const 
      {
        return v_;
      }

      private:
      value v_;
    };

    class unknown_mode
    {
      public:
      enum value
      {
        skip,
        stop,
        fail
      };

      unknown_mode (value);

      operator value () const 
      {
        return v_;
      }

      private:
      value v_;
    };

    // Exceptions.
    //

    class exception: public std::exception
    {
      public:
      virtual void
      print (::std::ostream&) const = 0;
    };

    ::std::ostream&
    operator<< (::std::ostream&, const exception&);

    class unknown_option: public exception
    {
      public:
      virtual
      ~unknown_option () throw ();

      unknown_option (const std::string& option);

      const std::string&
      option () const;

      virtual void
      print (::std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string option_;
    };

    class unknown_argument: public exception
    {
      public:
      virtual
      ~unknown_argument () throw ();

      unknown_argument (const std::string& argument);

      const std::string&
      argument () const;

      virtual void
      print (::std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string argument_;
    };

    class missing_value: public exception
    {
      public:
      virtual
      ~missing_value () throw ();

      missing_value (const std::string& option);

      const std::string&
      option () const;

      virtual void
      print (::std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string option_;
    };

    class invalid_value: public exception
    {
      public:
      virtual
      ~invalid_value () throw ();

      invalid_value (const std::string& option,
                     const std::string& value,
                     const std::string& message = std::string ());

      const std::string&
      option () const;

      const std::string&
      value () const;

      const std::string&
      message () const;

      virtual void
      print (::std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string option_;
      std::string value_;
      std::string message_;
    };

    class eos_reached: public exception
    {
      public:
      virtual void
      print (::std::ostream&) const;

      virtual const char*
      what () const throw ();
    };

    class file_io_failure: public exception
    {
      public:
      virtual
      ~file_io_failure () throw ();

      file_io_failure (const std::string& file);

      const std::string&
      file () const;

      virtual void
      print (::std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string file_;
    };

    class unmatched_quote: public exception
    {
      public:
      virtual
      ~unmatched_quote () throw ();

      unmatched_quote (const std::string& argument);

      const std::string&
      argument () const;

      virtual void
      print (::std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string argument_;
    };

    class unexpected_group: public exception
    {
      public:
      virtual
      ~unexpected_group () throw ();

      unexpected_group (const std::string& argument,
                        const std::string& group);

      const std::string&
      argument () const;

      const std::string&
      group () const;

      virtual void
      print (std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string argument_;
      std::string group_;
    };

    class group_separator: public exception
    {
      public:
      virtual
      ~group_separator () throw ();

      // Note: either (but not both) can be empty.
      //
      group_separator (const std::string& encountered,
                       const std::string& expected);

      const std::string&
      encountered () const;

      const std::string&
      expected () const;

      virtual void
      print (std::ostream&) const;

      virtual const char*
      what () const throw ();

      private:
      std::string encountered_;
      std::string expected_;
    };

    // Command line argument scanner interface.
    //
    // The values returned by next() are guaranteed to be valid
    // for the two previous arguments up until a call to a third
    // peek() or next().
    //
    class scanner
    {
      public:
      virtual
      ~scanner ();

      virtual bool
      more () = 0;

      virtual const char*
      peek () = 0;

      virtual const char*
      next () = 0;

      virtual void
      skip () = 0;
    };

    class argv_scanner: public scanner
    {
      public:
      argv_scanner (int& argc, char** argv, bool erase = false);
      argv_scanner (int start, int& argc, char** argv, bool erase = false);

      int
      end () const;

      virtual bool
      more ();

      virtual const char*
      peek ();

      virtual const char*
      next ();

      virtual void
      skip ();

      private:
      int i_;
      int& argc_;
      char** argv_;
      bool erase_;
    };

    class vector_scanner: public scanner
    {
      public:
      vector_scanner (const std::vector<std::string>&, std::size_t start = 0);

      std::size_t
      end () const;

      void
      reset (std::size_t start = 0);

      virtual bool
      more ();

      virtual const char*
      peek ();

      virtual const char*
      next ();

      virtual void
      skip ();

      private:
      const std::vector<std::string>& v_;
      std::size_t i_;
    };

    class argv_file_scanner: public argv_scanner
    {
      public:
      argv_file_scanner (int& argc,
                         char** argv,
                         const std::string& option,
                         bool erase = false);

      argv_file_scanner (int start,
                         int& argc,
                         char** argv,
                         const std::string& option,
                         bool erase = false);

      argv_file_scanner (const std::string& file,
                         const std::string& option);

      struct option_info
      {
        // If search_func is not NULL, it is called, with the arg
        // value as the second argument, to locate the options file.
        // If it returns an empty string, then the file is ignored.
        //
        const char* option;
        std::string (*search_func) (const char*, void* arg);
        void* arg;
      };

      argv_file_scanner (int& argc,
                          char** argv,
                          const option_info* options,
                          std::size_t options_count,
                          bool erase = false);

      argv_file_scanner (int start,
                         int& argc,
                         char** argv,
                         const option_info* options,
                         std::size_t options_count,
                         bool erase = false);

      argv_file_scanner (const std::string& file,
                         const option_info* options = 0,
                         std::size_t options_count = 0);

      virtual bool
      more ();

      virtual const char*
      peek ();

      virtual const char*
      next ();

      virtual void
      skip ();

      // Return the file path if the peeked at argument came from a file and
      // the empty string otherwise. The reference is guaranteed to be valid
      // till the end of the scanner lifetime.
      //
      const std::string&
      peek_file ();

      // Return the 1-based line number if the peeked at argument came from
      // a file and zero otherwise.
      //
      std::size_t
      peek_line ();

      private:
      const option_info*
      find (const char*) const;

      void
      load (const std::string& file);

      typedef argv_scanner base;

      const std::string option_;
      option_info option_info_;
      const option_info* options_;
      std::size_t options_count_;

      struct arg
      {
        std::string value;
        const std::string* file;
        std::size_t line;
      };

      std::deque<arg> args_;
      std::list<std::string> files_;

      // Circular buffer of two arguments.
      //
      std::string hold_[2];
      std::size_t i_;

      bool skip_;

      static int zero_argc_;
      static std::string empty_string_;
    };

    class group_scanner: public scanner
    {
      public:
      group_scanner (scanner&);

      virtual bool
      more ();

      virtual const char*
      peek ();

      virtual const char*
      next ();

      virtual void
      skip ();

      // The group is only available after the call to next()
      // (and skip() -- in case one needs to make sure the group
      // was empty, or some such) and is only valid (and must be
      // handled) until the next call to any of the scanner
      // functions (including more()).
      //
      scanner&
      group ();

      // Escape an argument that is a group separator. Return the
      // passed string if no escaping is required.
      //
      static const char*
      escape (const char*);

      private:
      enum state
      {
        peeked,  // Argument peeked at with peek().
        scanned, // Argument scanned with next().
        skipped, // Argument skipped with skip()/initial.
      };

      enum separator
      {
        none,
        open,      // {
        close,     // }
        open_plus, // +{
        close_plus // }+
      };

      static separator
      sense (const char*);

      // If the state is scanned or skipped, then scan the
      // leading groups and save the next (unescaped) argument in
      // arg_. If the state is peeked, then scan the trailing
      // groups. In both cases set the new state.
      //
      void
      scan_group (state);

      scanner& scan_;
      state state_;

      // Circular buffer of two arguments.
      //
      std::string arg_[2];
      std::size_t i_;

      std::vector<std::string> group_;
      vector_scanner group_scan_;
    };

    template <typename X>
    struct parser;
  }
}

#include <set>

#include <bpkg/types.hxx>

#include <bpkg/options-types.hxx>

namespace bpkg
{
  class common_options
  {
    public:
    // Merge options from the specified instance appending/overriding
    // them as if they appeared after options in this instance.
    //
    void
    merge (const common_options&);

    // Option accessors.
    //
    const bool&
    v () const;

    const bool&
    V () const;

    const bool&
    quiet () const;

    const uint16_t&
    verbose () const;

    bool
    verbose_specified () const;

    const size_t&
    jobs () const;

    bool
    jobs_specified () const;

    const bool&
    no_result () const;

    const bool&
    no_progress () const;

    const path&
    build () const;

    bool
    build_specified () const;

    const strings&
    build_option () const;

    bool
    build_option_specified () const;

    const path&
    fetch () const;

    bool
    fetch_specified () const;

    const strings&
    fetch_option () const;

    bool
    fetch_option_specified () const;

    const size_t&
    fetch_timeout () const;

    bool
    fetch_timeout_specified () const;

    const butl::url&
    pkg_proxy () const;

    bool
    pkg_proxy_specified () const;

    const path&
    git () const;

    bool
    git_specified () const;

    const strings&
    git_option () const;

    bool
    git_option_specified () const;

    const path&
    sha256 () const;

    bool
    sha256_specified () const;

    const strings&
    sha256_option () const;

    bool
    sha256_option_specified () const;

    const path&
    tar () const;

    bool
    tar_specified () const;

    const strings&
    tar_option () const;

    bool
    tar_option_specified () const;

    const qualified_option<openssl_commands, path>&
    openssl () const;

    bool
    openssl_specified () const;

    const qualified_option<openssl_commands, strings>&
    openssl_option () const;

    bool
    openssl_option_specified () const;

    const bpkg::auth&
    auth () const;

    bool
    auth_specified () const;

    const std::set<string>&
    trust () const;

    bool
    trust_specified () const;

    const bool&
    trust_yes () const;

    const bool&
    trust_no () const;

    const string&
    pager () const;

    bool
    pager_specified () const;

    const strings&
    pager_option () const;

    bool
    pager_option_specified () const;

    const string&
    options_file () const;

    bool
    options_file_specified () const;

    const dir_path&
    default_options () const;

    bool
    default_options_specified () const;

    const bool&
    no_default_options () const;

    // Print usage information.
    //
    static ::bpkg::cli::usage_para
    print_usage (::std::ostream&,
                 ::bpkg::cli::usage_para = ::bpkg::cli::usage_para::none);

    static ::bpkg::cli::usage_para
    print_long_usage (::std::ostream&,
                      ::bpkg::cli::usage_para = ::bpkg::cli::usage_para::none);

    // Implementation details.
    //
    protected:
    common_options ();

    bool
    _parse (const char*, ::bpkg::cli::scanner&);

    public:
    bool v_;
    bool V_;
    bool quiet_;
    uint16_t verbose_;
    bool verbose_specified_;
    size_t jobs_;
    bool jobs_specified_;
    bool no_result_;
    bool no_progress_;
    path build_;
    bool build_specified_;
    strings build_option_;
    bool build_option_specified_;
    path fetch_;
    bool fetch_specified_;
    strings fetch_option_;
    bool fetch_option_specified_;
    size_t fetch_timeout_;
    bool fetch_timeout_specified_;
    butl::url pkg_proxy_;
    bool pkg_proxy_specified_;
    path git_;
    bool git_specified_;
    strings git_option_;
    bool git_option_specified_;
    path sha256_;
    bool sha256_specified_;
    strings sha256_option_;
    bool sha256_option_specified_;
    path tar_;
    bool tar_specified_;
    strings tar_option_;
    bool tar_option_specified_;
    qualified_option<openssl_commands, path> openssl_;
    bool openssl_specified_;
    qualified_option<openssl_commands, strings> openssl_option_;
    bool openssl_option_specified_;
    bpkg::auth auth_;
    bool auth_specified_;
    std::set<string> trust_;
    bool trust_specified_;
    bool trust_yes_;
    bool trust_no_;
    string pager_;
    bool pager_specified_;
    strings pager_option_;
    bool pager_option_specified_;
    string options_file_;
    bool options_file_specified_;
    dir_path default_options_;
    bool default_options_specified_;
    bool no_default_options_;
  };
}

// Print page usage information.
//
namespace bpkg
{
  ::bpkg::cli::usage_para
  print_bpkg_common_options_usage (::std::ostream&,
                                   ::bpkg::cli::usage_para = ::bpkg::cli::usage_para::none);

  ::bpkg::cli::usage_para
  print_bpkg_common_options_long_usage (::std::ostream&,
                                        ::bpkg::cli::usage_para = ::bpkg::cli::usage_para::none);
}

#include <bpkg/common-options.ixx>

// Begin epilogue.
//
//
// End epilogue.

#endif // BPKG_COMMON_OPTIONS_HXX
