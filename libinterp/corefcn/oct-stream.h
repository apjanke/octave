/*

Copyright (C) 1996-2015 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if ! defined (octave_oct_stream_h)
#define octave_oct_stream_h 1

#include "octave-config.h"

#include <iosfwd>
#include <list>
#include <map>
#include <string>

// These are only needed as arguments to private functions, so they
// are also treated as private.

class scanf_format_elt;
class scanf_format_list;

class printf_format_elt;
class printf_format_list;

class delimited_stream;
class textscan_format_elt;
class textscan_format_list;

// These only appear as reference arguments or return values.

template <typename T> class Array;
class octave_value_list;
class string_vector;

#include "data-conv.h"
#include "mach-info.h"
#include "oct-refcount.h"

#include "Cell.h"
#include "ov.h"

// Main class to implement textscan.  Read data and parse it
// according to a format.
//
// The calling sequence is
//
//   textscan scanner ();
//   scanner.scan (...);

class
OCTINTERP_API
textscan
{
public:

  textscan (void);

  ~textscan (void) { }

  octave_value scan (std::istream& isp, const octave_value_list& args);

  octave_value scan (std::istream& isp, const octave_value_list& args,
                     octave_idx_type& count);

private:

  friend class textscan_format_list;

  std::string buf;

  // Three cases for delim_table and delim_list
  // 1. delim_table empty, delim_list empty:  whitespace delimiters
  // 2. delim_table = look-up table of delim chars, delim_list empty.
  // 3. delim_table non-empty, delim_list = Cell array of delim strings

  std::string whitespace_table;

  // delim_table[i] == '\0' if i is not a delimiter.
  std::string delim_table;

  // String of delimiter characters.
  std::string delims;

  Cell comment_style;

  // How far ahead to look to detect an open comment.
  int comment_len;

  // First character of open comment.
  int comment_char;

  octave_idx_type buffer_size;

  std::string date_locale;

  // 'inf' and 'nan' for formatted_double.
  Cell inf_nan;

  // Array of strings of delimiters.
  Cell delim_list;

  // Longest delimiter.
  int delim_len;

  octave_value empty_value;
  std::string exp_chars;
  int header_lines;
  Cell treat_as_empty;

  // Longest string to treat as "N/A".
  int treat_as_empty_len;

  std::string whitespace;

  short eol1;
  short eol2;
  short return_on_error;

  bool collect_output;
  bool multiple_delims_as_one;
  bool default_exp;
  bool numeric_delim;

  octave_idx_type lines;

  octave_value do_scan (std::istream& isp, textscan_format_list& fmt_list,
                        octave_idx_type ntimes);

  void parse_options (const octave_value_list& args,
                      textscan_format_list& fmt_list);

  int read_format_once (delimited_stream& isp, textscan_format_list& fmt_list,
                        std::list<octave_value>& retval,
                        Array<octave_idx_type> row, int& done_after);

  void scan_one (delimited_stream& is, const textscan_format_elt& fmt,
                 octave_value& ov, Array<octave_idx_type> row);

  // Methods to process a particular conversion specifier.
  double read_double (delimited_stream& is,
                      const textscan_format_elt& fmt) const;

  void scan_complex (delimited_stream& is, const textscan_format_elt& fmt,
                     Complex& val) const;

  int scan_bracket (delimited_stream& is, const std::string& pattern,
                    std::string& val) const;

  int scan_caret (delimited_stream& is, const std::string& pattern,
                  std::string& val) const;

  void scan_string (delimited_stream& is, const textscan_format_elt& fmt,
                    std::string& val) const;

  void scan_cstring (delimited_stream& is, const textscan_format_elt& fmt,
                     std::string& val) const;

  void scan_qstring (delimited_stream& is, const textscan_format_elt& fmt,
                     std::string& val);

  // Helper methods.
  std::string read_until (delimited_stream& is, const Cell& delimiters,
                          const std::string& ends) const;

  int lookahead (delimited_stream& is, const Cell& targets, int max_len,
                 bool case_sensitive = true) const;

  bool match_literal (delimited_stream& isp, const textscan_format_elt& elem);

  int skip_whitespace (delimited_stream& is, bool EOLstop = false);

  int skip_delim (delimited_stream& is);

  bool is_delim (unsigned char ch) const
  {
    return ((delim_table.empty () && (isspace (ch) || ch == eol1 || ch == eol2))
            || delim_table[ch] != '\0');
  }

  bool isspace (unsigned int ch) const { return whitespace_table[ch & 0xff]; }

  // True if the only delimiter is whitespace.
  bool whitespace_delim (void) const { return delim_table.empty (); }

  // No copying!

  textscan (const textscan&);

  textscan& operator = (const textscan&);
};

// Provide an interface for Octave streams.

class
OCTINTERP_API
octave_base_stream
{
  friend class octave_stream;

public:

  octave_base_stream (std::ios::openmode arg_md = std::ios::in|std::ios::out,
                      oct_mach_info::float_format ff
                        = oct_mach_info::native_float_format ())
    : count (0), md (arg_md), flt_fmt (ff), fail (false), open_state (true),
      errmsg ()
  { }

  virtual ~octave_base_stream (void) { }

  // The remaining functions are not specific to input or output only,
  // and must be provided by the derived classes.

  // Position a stream at OFFSET relative to ORIGIN.

  virtual int seek (off_t offset, int origin) = 0;

  // Return current stream position.

  virtual off_t tell (void) = 0;

  // Return TRUE if EOF has been reached on this stream.

  virtual bool eof (void) const = 0;

  // The name of the file.

  virtual std::string name (void) const = 0;

  // If the derived class provides this function and it returns a
  // pointer to a valid istream, scanf(), read(), getl(), and gets()
  // will automatically work for this stream.

  virtual std::istream *input_stream (void) { return 0; }

  // If the derived class provides this function and it returns a
  // pointer to a valid ostream, flush(), write(), and printf() will
  // automatically work for this stream.

  virtual std::ostream *output_stream (void) { return 0; }

  // Return TRUE if this stream is open.

  bool is_open (void) const { return open_state; }

  virtual void do_close (void) { }

  void close (void)
  {
    if (is_open ())
      {
        open_state = false;
        do_close ();
      }
  }

  virtual int file_number (void) const
  {
    // Kluge alert!

    if (name () == "stdin")
      return 0;
    else if (name () == "stdout")
      return 1;
    else if (name () == "stderr")
      return 2;
    else
      return -1;
  }

  bool ok (void) const { return ! fail; }

  // Return current error message for this stream.

  std::string error (bool clear, int& err_num);

protected:

  int mode (void) const { return md; }

  oct_mach_info::float_format float_format (void) const { return flt_fmt; }

  // Set current error state and set fail to TRUE.

  void error (const std::string& msg);
  void error (const std::string& who, const std::string& msg);

  // Clear any error message and set fail to FALSE.

  void clear (void);

  // Clear stream state.

  void clearerr (void);

private:

  // A reference count.
  octave_refcount<octave_idx_type> count;

  // The permission bits for the file.  Should be some combination of
  // std::ios::open_mode bits.
  int md;

  // Data format.
  oct_mach_info::float_format flt_fmt;

  // TRUE if an error has occurred.
  bool fail;

  // TRUE if this stream is open.
  bool open_state;

  // Should contain error message if fail is TRUE.
  std::string errmsg;

  // Functions that are defined for all input streams (input streams
  // are those that define is).

  std::string do_gets (octave_idx_type max_len, bool& err, bool strip_newline,
                       const std::string& who /* = "gets" */);

  std::string getl (octave_idx_type max_len, bool& err,
                    const std::string& who /* = "getl" */);
  std::string gets (octave_idx_type max_len, bool& err,
                    const std::string& who /* = "gets" */);
  off_t skipl (off_t count, bool& err, const std::string& who /* = "skipl" */);

  octave_value do_scanf (scanf_format_list& fmt_list, octave_idx_type nr,
                         octave_idx_type nc,
                         bool one_elt_size_spec, octave_idx_type& count,
                         const std::string& who /* = "scanf" */);

  octave_value scanf (const std::string& fmt, const Array<double>& size,
                      octave_idx_type& count, const std::string& who /* = "scanf" */);

  bool do_oscanf (const scanf_format_elt *elt, octave_value&,
                  const std::string& who /* = "scanf" */);

  octave_value_list oscanf (const std::string& fmt,
                            const std::string& who /* = "scanf" */);

  // Functions that are defined for all output streams (output streams
  // are those that define os).

  int flush (void);

  int do_numeric_printf_conv (std::ostream& os, const printf_format_elt *elt,
                              int nsa, int sa_1, int sa_2,
                              const octave_value& val,
                              const std::string& who);

  int do_printf (printf_format_list& fmt_list, const octave_value_list& args,
                 const std::string& who /* = "printf" */);

  int printf (const std::string& fmt, const octave_value_list& args,
              const std::string& who /* = "printf" */);

  int puts (const std::string& s, const std::string& who /* = "puts" */);

  // We can always do this in terms of seek(), so the derived class
  // only has to provide that.

  void invalid_operation (const std::string& who, const char *rw);

  // No copying!

  octave_base_stream (const octave_base_stream&);

  octave_base_stream& operator = (const octave_base_stream&);
};

class
OCTINTERP_API
octave_stream
{
public:

  octave_stream (octave_base_stream *bs = 0);

  ~octave_stream (void);

  octave_stream (const octave_stream&);

  octave_stream& operator = (const octave_stream&);

  int flush (void);

  std::string getl (octave_idx_type max_len, bool& err,
                    const std::string& who /* = "getl" */);
  std::string getl (const octave_value& max_len, bool& err,
                    const std::string& who /* = "getl" */);

  std::string gets (octave_idx_type max_len, bool& err,
                    const std::string& who /* = "gets" */);
  std::string gets (const octave_value& max_len, bool& err,
                    const std::string& who /* = "gets" */);

  off_t skipl (off_t count, bool& err, const std::string& who /* = "skipl" */);
  off_t skipl (const octave_value& count, bool& err,
               const std::string& who /* = "skipl" */);

  int seek (off_t offset, int origin);
  int seek (const octave_value& offset, const octave_value& origin);

  off_t tell (void);

  int rewind (void);

  bool is_open (void) const;

  void close (void);

  octave_value read (const Array<double>& size, octave_idx_type block_size,
                     oct_data_conv::data_type input_type,
                     oct_data_conv::data_type output_type,
                     octave_idx_type skip, oct_mach_info::float_format flt_fmt,
                     octave_idx_type& count);

  octave_idx_type write (const octave_value& data, octave_idx_type block_size,
                         oct_data_conv::data_type output_type,
                         octave_idx_type skip,
                         oct_mach_info::float_format flt_fmt);

  bool write_bytes (const void *data, size_t n_elts);

  bool skip_bytes (size_t n_elts);

  template <typename T>
  octave_idx_type write (const Array<T>& data, octave_idx_type block_size,
                         oct_data_conv::data_type output_type,
                         octave_idx_type skip,
                         oct_mach_info::float_format flt_fmt);

  octave_value scanf (const std::string& fmt, const Array<double>& size,
                      octave_idx_type& count, const std::string& who /* = "scanf" */);

  octave_value scanf (const octave_value& fmt, const Array<double>& size,
                      octave_idx_type& count, const std::string& who /* = "scanf" */);

  octave_value_list oscanf (const std::string& fmt,
                            const std::string& who /* = "scanf" */);

  octave_value_list oscanf (const octave_value& fmt,
                            const std::string& who /* = "scanf" */);

  int printf (const std::string& fmt, const octave_value_list& args,
              const std::string& who /* = "printf" */);

  int printf (const octave_value& fmt, const octave_value_list& args,
              const std::string& who /* = "printf" */);

  int puts (const std::string& s, const std::string& who /* = "puts" */);
  int puts (const octave_value& s, const std::string& who /* = "puts" */);

  bool eof (void) const;

  std::string error (bool clear, int& err_num);

  std::string error (bool clear = false)
  {
    int err_num;
    return error (clear, err_num);
  }

  // Set the error message and state.

  void error (const std::string& msg)
  {
    if (rep)
      rep->error (msg);
  }

  void error (const char *msg) { error (std::string (msg)); }

  int file_number (void) { return rep ? rep->file_number () : -1; }

  bool is_valid (void) const { return (rep != 0); }

  bool ok (void) const { return rep && rep->ok (); }

  operator bool () const { return ok (); }

  std::string name (void) const;

  int mode (void) const;

  oct_mach_info::float_format float_format (void) const;

  static std::string mode_as_string (int mode);

  std::istream *input_stream (void)
  {
    return rep ? rep->input_stream () : 0;
  }

  std::ostream *output_stream (void)
  {
    return rep ? rep->output_stream () : 0;
  }

  void clearerr (void) { if (rep) rep->clearerr (); }

private:

  // The actual representation of this stream.
  octave_base_stream *rep;

  bool stream_ok (bool clear = true) const
  {
    bool retval = true;

    if (rep)
      {
        if (clear)
          rep->clear ();
      }
    else
      retval = false;

    return retval;
  }

  void invalid_operation (const std::string& who, const char *rw)
  {
    if (rep)
      rep->invalid_operation (who, rw);
  }

  octave_value
  finalize_read (std::list<void *>& input_buf_list,
                 octave_idx_type input_buf_elts,
                 octave_idx_type elts_read,
                 octave_idx_type nr, octave_idx_type nc,
                 oct_data_conv::data_type input_type,
                 oct_data_conv::data_type output_type,
                 oct_mach_info::float_format ffmt);
};

class
OCTINTERP_API
octave_stream_list
{
protected:

  octave_stream_list (void) : list (), lookup_cache (list.end ()) { }

public:

  ~octave_stream_list (void) { }

  static bool instance_ok (void);

  static int insert (octave_stream& os);

  static octave_stream
  lookup (int fid, const std::string& who = "");

  static octave_stream
  lookup (const octave_value& fid, const std::string& who = "");

  static int remove (int fid, const std::string& who = "");
  static int remove (const octave_value& fid,
                     const std::string& who = "");

  static void clear (bool flush = true);

  static string_vector get_info (int fid);
  static string_vector get_info (const octave_value& fid);

  static std::string list_open_files (void);

  static octave_value open_file_numbers (void);

  static int get_file_number (const octave_value& fid);

private:

  typedef std::map<int, octave_stream> ostrl_map;

  ostrl_map list;

  mutable ostrl_map::const_iterator lookup_cache;

  static octave_stream_list *instance;

  static void cleanup_instance (void) { delete instance; instance = 0; }

  int do_insert (octave_stream& os);

  octave_stream do_lookup (int fid,
                           const std::string& who = "") const;
  octave_stream do_lookup (const octave_value& fid,
                           const std::string& who = "") const;

  int do_remove (int fid, const std::string& who = "");
  int do_remove (const octave_value& fid,
                 const std::string& who = "");

  void do_clear (bool flush = true);

  string_vector do_get_info (int fid) const;
  string_vector do_get_info (const octave_value& fid) const;

  std::string do_list_open_files (void) const;

  octave_value do_open_file_numbers (void) const;

  int do_get_file_number (const octave_value& fid) const;
};

#endif
