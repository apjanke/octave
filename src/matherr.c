/*

Copyright (C) 1997 John W. Eaton

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "oct-math.h"

#if defined (EXCEPTION_IN_MATH)
int
matherr (struct exception *x)
{
  /* Possibly print our own message someday.  Should probably be
     user-switchable. */

  switch (x->type)
    {
    case DOMAIN:
    case SING:
    case OVERFLOW:
    case UNDERFLOW:
    case TLOSS:
    case PLOSS:
    default:
      break;
    }

  /* But don't print the system message. */

  return 1;
}
#endif

#endif

/*
;;; Local Variables: ***
;;; mode: C ***
;;; End: ***
*/
