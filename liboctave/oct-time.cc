/*

Copyright (C) 1999 John W. Eaton

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

#include <climits>
#include <cmath>

#include "lo-error.h"
#include "lo-utils.h"
#include "oct-time.h"

octave_time::octave_time (const octave_base_tm& tm)
{
  struct tm t;
  
  t.tm_sec = tm.sec ();
  t.tm_min = tm.min ();
  t.tm_hour = tm.hour ();
  t.tm_mday = tm.mday ();
  t.tm_mon = tm.mon ();
  t.tm_year = tm.year ();
  t.tm_wday = tm.wday ();
  t.tm_yday = tm.yday ();
  t.tm_isdst = tm.isdst ();

#if defined (HAVE_TM_ZONE)
  string s = tm.zone ();
  t.tm_zone = strsave (s.c_str ());
#endif

  ot_unix_time = mktime (&t);

  ot_usec = tm.usec ();

#if defined (HAVE_TM_ZONE)
  delete [] t.tm_zone;
#endif
}

string
octave_time::ctime (void) const
{
  return octave_localtime (*this) . asctime ();
}

void
octave_time::stamp (void)
{
#if defined (HAVE_GETTIMEOFDAY)

  struct timeval tp;

#if defined  (GETTIMEOFDAY_NO_TZ)
  gettimeofday (&tp);
#else
  gettimeofday (&tp, 0);
#endif

  ot_unix_time = tp.tv_sec;

  ot_usec = tp.tv_usec;

#else

  ot_unix_time = time (0);

#endif
}

#define DEFINE_SET_INT_FIELD_FCN(f, lo, hi) \
  octave_base_tm& \
  octave_base_tm::f (int v) \
  { \
    if (v < lo || v > hi) \
      (*current_liboctave_error_handler) \
	("invalid value specified for " #f); \
 \
    tm_ ## f = v; \
 \
    return *this; \
  }

DEFINE_SET_INT_FIELD_FCN (usec, 0, 1000000)
DEFINE_SET_INT_FIELD_FCN (sec, 0, 61)
DEFINE_SET_INT_FIELD_FCN (min, 0, 59)
DEFINE_SET_INT_FIELD_FCN (hour, 0, 23)
DEFINE_SET_INT_FIELD_FCN (mday, 1, 31)
DEFINE_SET_INT_FIELD_FCN (mon, 0, 11)
DEFINE_SET_INT_FIELD_FCN (year, INT_MIN, INT_MAX)
DEFINE_SET_INT_FIELD_FCN (wday, 0, 6)
DEFINE_SET_INT_FIELD_FCN (yday, 0, 365)
DEFINE_SET_INT_FIELD_FCN (isdst, 0, 1)

octave_base_tm&
octave_base_tm::zone (const string& s)
{
  tm_zone = s;
  return *this;
}

#if !defined STRFTIME_BUF_INITIAL_SIZE
#define STRFTIME_BUF_INITIAL_SIZE 128
#endif

string
octave_base_tm::strftime (const string& fmt) const
{
  string retval;

  struct tm t;
  
  t.tm_sec = tm_sec;
  t.tm_min = tm_min;
  t.tm_hour = tm_hour;
  t.tm_mday = tm_mday;
  t.tm_mon = tm_mon;
  t.tm_year = tm_year;
  t.tm_wday = tm_wday;
  t.tm_yday = tm_yday;
  t.tm_isdst = tm_isdst;

#if defined (HAVE_TM_ZONE)
  t.tm_zone = strsave (tm_zone.c_str ());
#endif

  const char *fmt_str = fmt.c_str ();

  char *buf = 0;
  size_t bufsize = STRFTIME_BUF_INITIAL_SIZE;
  size_t chars_written = 0;

  while (chars_written == 0)
    {
      delete [] buf;
      buf = new char[bufsize];
      buf[0] = '\0';

      chars_written = ::strftime (buf, bufsize, fmt_str, &t);

      bufsize *= 2;
    }

  retval = buf;

  delete [] buf;

#if defined (HAVE_TM_ZONE)
  delete [] t.tm_zone;
#endif

  return retval;
}

void
octave_base_tm::init (void *p)
{
  struct tm *t = static_cast<struct tm*> (p);
  
  tm_sec = t->tm_sec;
  tm_min = t->tm_min;
  tm_hour = t->tm_hour;
  tm_mday = t->tm_mday;
  tm_mon = t->tm_mon;
  tm_year = t->tm_year;
  tm_wday = t->tm_wday;
  tm_yday = t->tm_yday;
  tm_isdst = t->tm_isdst;

#if defined (HAVE_TM_ZONE)
  tm_zone = t->tm_zone;
#elif defined (HAVE_TZNAME)
  if (t->tm_isdst == 0 || t->tm_isdst == 1)
    tm_zone = tzname[t->tm_isdst];
#endif
}

void
octave_localtime::init (const octave_time& ot)
{
  tm_usec = ot.usec ();

  time_t t = ot;

  octave_base_tm::init (localtime (&t));
}

void
octave_gmtime::init (const octave_time& ot)
{
  tm_usec = ot.usec ();

  time_t t = ot;

  octave_base_tm::init (gmtime (&t));
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
