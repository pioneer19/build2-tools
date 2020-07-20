// -*- C++ -*-
//
// This file was generated by CLI, a command line interface
// compiler for C++.
//

#ifndef BDEP_BDEP_OPTIONS_HXX
#define BDEP_BDEP_OPTIONS_HXX

// Begin prologue.
//
//
// End prologue.

#include <bdep/common-options.hxx>

namespace bdep
{
  class commands
  {
    public:
    commands ();

    // Return true if anything has been parsed.
    //
    bool
    parse (int& argc,
           char** argv,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int start,
           int& argc,
           char** argv,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int start,
           int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (::bdep::cli::scanner&,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    // Merge options from the specified instance appending/overriding
    // them as if they appeared after options in this instance.
    //
    void
    merge (const commands&);

    // Option accessors and modifiers.
    //
    const bool&
    help () const;

    bool&
    help ();

    void
    help (const bool&);

    const bool&
    new_ () const;

    bool&
    new_ ();

    void
    new_ (const bool&);

    const bool&
    init () const;

    bool&
    init ();

    void
    init (const bool&);

    const bool&
    sync () const;

    bool&
    sync ();

    void
    sync (const bool&);

    const bool&
    fetch () const;

    bool&
    fetch ();

    void
    fetch (const bool&);

    const bool&
    status () const;

    bool&
    status ();

    void
    status (const bool&);

    const bool&
    ci () const;

    bool&
    ci ();

    void
    ci (const bool&);

    const bool&
    release () const;

    bool&
    release ();

    void
    release (const bool&);

    const bool&
    publish () const;

    bool&
    publish ();

    void
    publish (const bool&);

    const bool&
    deinit () const;

    bool&
    deinit ();

    void
    deinit (const bool&);

    const bool&
    config () const;

    bool&
    config ();

    void
    config (const bool&);

    const bool&
    test () const;

    bool&
    test ();

    void
    test (const bool&);

    const bool&
    update () const;

    bool&
    update ();

    void
    update (const bool&);

    const bool&
    clean () const;

    bool&
    clean ();

    void
    clean (const bool&);

    // Print usage information.
    //
    static ::bdep::cli::usage_para
    print_usage (::std::ostream&,
                 ::bdep::cli::usage_para = ::bdep::cli::usage_para::none);

    // Option description.
    //
    static const ::bdep::cli::options&
    description ();

    // Implementation details.
    //
    protected:
    friend struct _cli_commands_desc_type;

    static void
    fill (::bdep::cli::options&);

    bool
    _parse (const char*, ::bdep::cli::scanner&);

    private:
    bool
    _parse (::bdep::cli::scanner&,
            ::bdep::cli::unknown_mode option,
            ::bdep::cli::unknown_mode argument);

    public:
    bool help_;
    bool new__;
    bool init_;
    bool sync_;
    bool fetch_;
    bool status_;
    bool ci_;
    bool release_;
    bool publish_;
    bool deinit_;
    bool config_;
    bool test_;
    bool update_;
    bool clean_;
  };

  class topics
  {
    public:
    topics ();

    // Return true if anything has been parsed.
    //
    bool
    parse (int& argc,
           char** argv,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int start,
           int& argc,
           char** argv,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int start,
           int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (::bdep::cli::scanner&,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    // Merge options from the specified instance appending/overriding
    // them as if they appeared after options in this instance.
    //
    void
    merge (const topics&);

    // Option accessors and modifiers.
    //
    const bool&
    common_options () const;

    bool&
    common_options ();

    void
    common_options (const bool&);

    const bool&
    default_options_files () const;

    bool&
    default_options_files ();

    void
    default_options_files (const bool&);

    const bool&
    projects_configs () const;

    bool&
    projects_configs ();

    void
    projects_configs (const bool&);

    // Print usage information.
    //
    static ::bdep::cli::usage_para
    print_usage (::std::ostream&,
                 ::bdep::cli::usage_para = ::bdep::cli::usage_para::none);

    // Option description.
    //
    static const ::bdep::cli::options&
    description ();

    // Implementation details.
    //
    protected:
    friend struct _cli_topics_desc_type;

    static void
    fill (::bdep::cli::options&);

    bool
    _parse (const char*, ::bdep::cli::scanner&);

    private:
    bool
    _parse (::bdep::cli::scanner&,
            ::bdep::cli::unknown_mode option,
            ::bdep::cli::unknown_mode argument);

    public:
    bool common_options_;
    bool default_options_files_;
    bool projects_configs_;
  };

  class options: public ::bdep::common_options
  {
    public:
    options ();

    // Return true if anything has been parsed.
    //
    bool
    parse (int& argc,
           char** argv,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int start,
           int& argc,
           char** argv,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (int start,
           int& argc,
           char** argv,
           int& end,
           bool erase = false,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    bool
    parse (::bdep::cli::scanner&,
           ::bdep::cli::unknown_mode option = ::bdep::cli::unknown_mode::fail,
           ::bdep::cli::unknown_mode argument = ::bdep::cli::unknown_mode::stop);

    // Merge options from the specified instance appending/overriding
    // them as if they appeared after options in this instance.
    //
    void
    merge (const options&);

    // Option accessors and modifiers.
    //
    const bool&
    help () const;

    bool&
    help ();

    void
    help (const bool&);

    const bool&
    version () const;

    bool&
    version ();

    void
    version (const bool&);

    // Print usage information.
    //
    static ::bdep::cli::usage_para
    print_usage (::std::ostream&,
                 ::bdep::cli::usage_para = ::bdep::cli::usage_para::none);

    // Option description.
    //
    static const ::bdep::cli::options&
    description ();

    // Implementation details.
    //
    protected:
    friend struct _cli_options_desc_type;

    static void
    fill (::bdep::cli::options&);

    bool
    _parse (const char*, ::bdep::cli::scanner&);

    private:
    bool
    _parse (::bdep::cli::scanner&,
            ::bdep::cli::unknown_mode option,
            ::bdep::cli::unknown_mode argument);

    public:
    bool help_;
    bool version_;
  };
}

// Print page usage information.
//
namespace bdep
{
  ::bdep::cli::usage_para
  print_bdep_usage (::std::ostream&,
                    ::bdep::cli::usage_para = ::bdep::cli::usage_para::none);
}

#include <bdep/bdep-options.ixx>

// Begin epilogue.
//
//
// End epilogue.

#endif // BDEP_BDEP_OPTIONS_HXX
