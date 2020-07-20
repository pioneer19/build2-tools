// file      : libbuild2/diagnostics.cxx -*- C++ -*-
// license   : MIT; see accompanying LICENSE file

#include <libbuild2/diagnostics.hxx>

#include <cstring>  // strchr()

#include <libbutl/process-io.mxx>

#include <libbuild2/scope.hxx>
#include <libbuild2/action.hxx>
#include <libbuild2/target.hxx>
#include <libbuild2/context.hxx>

using namespace std;

namespace build2
{
  // Diagnostics state (verbosity level, progress, etc). Keep disabled until
  // set from options.
  //
  uint16_t verb = 0;
  bool silent = true;

  optional<bool> diag_progress_option;

  bool diag_no_line = false;
  bool diag_no_column = false;

  bool stderr_term = false;

  void
  init_diag (uint16_t v, bool s, optional<bool> p, bool nl, bool nc, bool st)
  {
    assert (!s || v == 0);

    verb = v;
    silent = s;
    diag_progress_option = p;
    diag_no_line = nl;
    diag_no_column = nc;
    stderr_term = st;
  }

  // Stream verbosity.
  //
  const int stream_verb_index = ostream::xalloc ();

  void
  print_process (const char* const* args, size_t n)
  {
    diag_record dr (text);
    print_process (dr, args, n);
  }

  void
  print_process (diag_record& dr,
                 const char* const* args, size_t n)
  {
    dr << butl::process_args {args, n};
  }

  void
  print_process (const process_env& pe, const char* const* args, size_t n)
  {
    diag_record dr (text);
    print_process (dr, pe, args, n);
  }

  void
  print_process (diag_record& dr,
                 const process_env& pe, const char* const* args, size_t n)
  {
    if (pe.env ())
      dr << pe << ' ';

    dr << butl::process_args {args, n};
  }

  // Diagnostics stack.
  //
  static
#ifdef __cpp_thread_local
  thread_local
#else
  __thread
#endif
  const diag_frame* diag_frame_stack = nullptr;

  const diag_frame* diag_frame::
  stack () noexcept
  {
    return diag_frame_stack;
  }

  const diag_frame* diag_frame::
  stack (const diag_frame* f) noexcept
  {
    const diag_frame* r (diag_frame_stack);
    diag_frame_stack = f;
    return r;
  }

  // Diagnostic facility, project specifics.
  //

  void simple_prologue_base::
  operator() (const diag_record& r) const
  {
    stream_verb (r.os, sverb_);

    if (type_ != nullptr)
      r << type_ << ": ";

    if (mod_ != nullptr)
      r << mod_ << "::";

    if (name_ != nullptr)
      r << name_ << ": ";
  }

  void location_prologue_base::
  operator() (const diag_record& r) const
  {
    stream_verb (r.os, sverb_);

    if (!loc_.empty ())
    {
      r << loc_.file << ':';

      if (!diag_no_line)
      {
        if (loc_.line != 0)
        {
          r << loc_.line << ':';

          if (!diag_no_column)
          {
            if (loc_.column != 0)
              r << loc_.column << ':';
          }
        }
      }

      r << ' ';
    }

    if (type_ != nullptr)
      r << type_ << ": ";

    if (mod_ != nullptr)
      r << mod_ << "::";

    if (name_ != nullptr)
      r << name_ << ": ";
  }

  const basic_mark error ("error");
  const basic_mark warn  ("warning");
  const basic_mark info  ("info");
  const basic_mark text  (nullptr, nullptr, nullptr); // No type/data/frame.
  const fail_mark  fail  ("error");
  const fail_end   endf;

  // diag_do(), etc.
  //
  string
  diag_do (context& ctx, const action&)
  {
    const meta_operation_info& m (*ctx.current_mif);
    const operation_info& io (*ctx.current_inner_oif);
    const operation_info* oo (ctx.current_outer_oif);

    string r;

    // perform(update(x))   -> "update x"
    // configure(update(x)) -> "configure updating x"
    //
    if (m.name_do.empty ())
      r = io.name_do;
    else
    {
      r = m.name_do;

      if (io.name_doing[0] != '\0')
      {
        r += ' ';
        r += io.name_doing;
      }
    }

    if (oo != nullptr)
    {
      r += " (for ";
      r += oo->name;
      r += ')';
    }

    return r;
  }

  void
  diag_do (ostream& os, const action& a, const target& t)
  {
    os << diag_do (t.ctx, a) << ' ' << t;
  }

  string
  diag_doing (context& ctx, const action&)
  {
    const meta_operation_info& m (*ctx.current_mif);
    const operation_info& io (*ctx.current_inner_oif);
    const operation_info* oo (ctx.current_outer_oif);

    string r;

    // perform(update(x))   -> "updating x"
    // configure(update(x)) -> "configuring updating x"
    //
    if (!m.name_doing.empty ())
      r = m.name_doing;

    if (io.name_doing[0] != '\0')
    {
      if (!r.empty ()) r += ' ';
      r += io.name_doing;
    }

    if (oo != nullptr)
    {
      r += " (for ";
      r += oo->name;
      r += ')';
    }

    return r;
  }

  void
  diag_doing (ostream& os, const action& a, const target& t)
  {
    os << diag_doing (t.ctx, a) << ' ' << t;
  }

  string
  diag_did (context& ctx, const action&)
  {
    const meta_operation_info& m (*ctx.current_mif);
    const operation_info& io (*ctx.current_inner_oif);
    const operation_info* oo (ctx.current_outer_oif);

    string r;

    // perform(update(x))   -> "updated x"
    // configure(update(x)) -> "configured updating x"
    //
    if (!m.name_did.empty ())
    {
      r = m.name_did;

      if (io.name_doing[0] != '\0')
      {
        r += ' ';
        r += io.name_doing;
      }
    }
    else
      r += io.name_did;

    if (oo != nullptr)
    {
      r += " (for ";
      r += oo->name;
      r += ')';
    }

    return r;
  }

  void
  diag_did (ostream& os, const action& a, const target& t)
  {
    os << diag_did (t.ctx, a) << ' ' << t;
  }

  void
  diag_done (ostream& os, const action&, const target& t)
  {
    const meta_operation_info& m (*t.ctx.current_mif);
    const operation_info& io (*t.ctx.current_inner_oif);
    const operation_info* oo (t.ctx.current_outer_oif);

    // perform(update(x))   -> "x is up to date"
    // configure(update(x)) -> "updating x is configured"
    //
    if (m.name_done.empty ())
    {
      os << t;

      if (io.name_done[0] != '\0')
        os << ' ' << io.name_done;

      if (oo != nullptr)
        os << " (for " << oo->name << ')';
    }
    else
    {
      if (io.name_doing[0] != '\0')
        os << io.name_doing << ' ';

      if (oo != nullptr)
        os << "(for " << oo->name << ") ";

      os << t << ' ' << m.name_done;
    }
  }
}
