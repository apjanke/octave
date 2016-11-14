/*

Copyright (C) 1993-2016 John W. Eaton
Copyright (C) 2009 VZLU Prague

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "lo-error.h"
#include "unwind-prot.h"

namespace octave
{
  void
  unwind_protect_safe::warn_unhandled_exception (void) const
  {
    (*current_liboctave_warning_handler)
      ("unhandled exception in unwind_protect_safe handler.  "
       "It is a bug in Octave for this to happen.  "
       "Please help improve Octave by reporting it.");
  }
}

