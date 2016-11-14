/*

Copyright (C) 2016 John W. Eaton

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

// areadlink is provided by gnulib.  We don't include gnulib headers
// directly in Octave's C++ source files to avoid problems that may be
// caused by the way that gnulib overrides standard library functions.

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#if defined (__WIN32__) && ! defined (__CYGWIN__)
#  include <stdlib.h>
#  include <string.h>
#  define WIN32_LEAN_AND_MEAN 1
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include "async-system-wrapper.h"

pid_t
octave_async_system_wrapper (const char *cmd)
{
  int retval = -1;

  if (! cmd)
    return retval;

#if defined (OCTAVE_USE_WINDOWS_API)

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory (&si, sizeof (si));
  ZeroMemory (&pi, sizeof (pi));

  char *xcmd = (char *) malloc (strlen (cmd) + 1);

  strcpy (xcmd, cmd);

  if (! CreateProcess (0, xcmd, 0, 0, FALSE, 0, 0, 0, &si, &pi))
    retval = -1;
  else
    {
      retval = pi.dwProcessId;

      CloseHandle (pi.hProcess);
      CloseHandle (pi.hThread);
    }

  free (xcmd);

#else

  pid_t pid = fork ();

  if (pid == 0)
    execl (SHELL_PATH, "sh", "-c", cmd, (char *) (0));
  else
    retval = pid;

#endif

  return retval;
}

