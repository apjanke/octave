/*

Copyright (C) 2011-2012 Jacob Dawid

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <clocale>
#include <string>

#include "octave.h"

#include "octave-main-thread.h"
#include "octave-link.h"

octave_main_thread::octave_main_thread () : QThread ()
{
}

void
octave_main_thread::run ()
{
  setlocale (LC_ALL, "en_US.UTF-8");

  emit ready ();

  octave_initialize_interpreter (octave_cmdline_argc, octave_cmdline_argv,
                                 octave_embedded);

  octave_execute_interpreter ();
}
