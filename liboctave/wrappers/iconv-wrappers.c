/*

Copyright (C) 2018 Markus Mützel

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "iconv.h"

#include "iconv-wrappers.h"

iconv_t
octave_iconv_open_wrapper (const char *tocode, const char *fromcode)
{
  return iconv_open (tocode, fromcode);
}

int
octave_iconv_close_wrapper (iconv_t cd)
{
  return iconv_close (cd);
}
