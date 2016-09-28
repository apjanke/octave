/*

Copyright (C) 1996-2016 John W. Eaton

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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <cerrno>
#include <cstring>

#include "error.h"
#include "oct-fstrm.h"

octave_stream
octave_fstream::create (const std::string& nm_arg, std::ios::openmode arg_md,
                        octave::mach_info::float_format ff)
{
  return octave_stream (new octave_fstream (nm_arg, arg_md, ff));
}

octave_fstream::octave_fstream (const std::string& nm_arg,
                                std::ios::openmode arg_md,
                                octave::mach_info::float_format ff)
  : octave_base_stream (arg_md, ff), nm (nm_arg)
{
  fs.open (nm.c_str (), arg_md);

  if (! fs)
    // Note: error is inherited from octave_base_stream, not ::error.
    error (std::strerror (errno));
}

// Position a stream at OFFSET relative to ORIGIN.

int
octave_fstream::seek (off_t, int)
{
  // Note: error is inherited from octave_base_stream, not ::error.
  // This error function does not halt execution so "return ..." must exist.
  error ("fseek: invalid_operation");
  return -1;
}

// Return current stream position.

off_t
octave_fstream::tell (void)
{
  // Note: error is inherited from octave_base_stream, not ::error.
  // This error function does not halt execution so "return ..." must exist.
  error ("ftell: invalid_operation");
  return -1;
}

// Return nonzero if EOF has been reached on this stream.

bool
octave_fstream::eof (void) const
{
  return fs.eof ();
}

void
octave_fstream::do_close (void)
{
  fs.close ();
}

std::istream *
octave_fstream::input_stream (void)
{
  std::istream *retval = 0;

  if (mode () & std::ios::in)
    retval = &fs;

  return retval;
}

std::ostream *
octave_fstream::output_stream (void)
{
  std::ostream *retval = 0;

  if (mode () & std::ios::out)
    retval = &fs;

  return retval;
}

