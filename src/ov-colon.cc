/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>

#include "error.h"
#include "pr-output.h"
#include "ov-colon.h"

int octave_magic_colon::t_id = -1;

const string octave_magic_colon::t_name ("magic-colon");

void
octave_magic_colon::print (ostream& os, bool) const
{
  indent (os);
  print_raw (os);
}

void
octave_magic_colon::print_raw (ostream& os, bool) const
{
  os << ":";
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
