/*

Copyright (C) 2009 John W. Eaton

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

#include <cstdlib>

#if defined (OCTAVE_USE_WINDOWS_API)
#include <Windows.h>
#elif defined (OCTAVE_USE_OS_X_API)
#include <CGDirectDisplay.h>
#include <CGDisplayConfiguration.h>
#elif defined (HAVE_X_WINDOWS)
#include <X11/Xlib.h>
#endif



#include "display.h"
#include "error.h"

display_info *display_info::instance = 0;

void
display_info::init (void)
{
#if defined (OCTAVE_USE_WINDOWS_API)

  HDC hdc = GetDC (0);

  if (hdc)
    {
      dp = GetDeviceCaps (hdc, BITSPIXEL)

      ht = GetDeviceCaps (hdc, VERTRES);
      wd = GetDeviceCaps (hdc, HORZRES);

      double ht_mm = GetDeviceCaps (hdc, VERTSIZE);
      double wd_mm = GetDeviceCaps (hdc, HORZSIZE);

      rx = wd * 25.4 / wd_mm;
      ry = ht * 25.4 / ht_mm;
    }
  else
    warning ("no graphical display found");

#elif defined (OCTAVE_USE_OS_X_API)

  CGDirectDisplayID display = CGMainDisplayID ();

  if (display)
    {
      dp = CGDisplayBitsPerPixel (display);

      ht = CGDisplayPixelsHigh (display);
      wd = CGDisplayPixelsWide (display);

      CGSize sz_mm = CGDisplayScreenSize (display);

      CGFloat ht_mm = sz_mm.height;
      CGFloat wd_mm = sz_mm.width;

      rx = wd * 25.4 / wd_mm;
      ry = ht * 25.4 / ht_mm;
    }
  else
    warning ("no graphical display found");

#elif defined (HAVE_X_WINDOWS)

  const char *display_name = getenv ("DISPLAY");

  if (display_name && *display_name)
    {
      Display *display = XOpenDisplay (display_name);

      if (display)
	{
	  Screen *screen = DefaultScreenOfDisplay (display);

	  if (screen)
	    {
	      dp = DefaultDepthOfScreen (screen);

	      ht = HeightOfScreen (screen);
	      wd = WidthOfScreen (screen);

	      int screen_number = XScreenNumberOfScreen (screen);

	      double ht_mm = DisplayHeightMM (display, screen_number);
	      double wd_mm = DisplayWidthMM (display, screen_number);

	      rx = wd * 25.4 / wd_mm;
	      ry = ht * 25.4 / ht_mm;
	    }
	  else
	    warning ("X11 display has no default screen");
	}
      else
	warning ("unable to open X11 DISPLAY");
    }
  else
    warning ("X11 DISPLAY environment variable not set");
#else

  warning ("no graphical display found");

#endif
}

bool
display_info::instance_ok (void)
{
  bool retval = true;

  if (! instance)
    instance = new display_info ();

  if (! instance)
    {
      ::error ("unable to create display_info object!");

      retval = false;
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
