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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#include "lo-error.h"
#include "oct-passwd.h"

#define NOT_SUPPORTED(nm) \
  nm ## ": not supported on this system"

string
octave_passwd::name (void) const
{
  if (! ok ())
    gripe_invalid ();

  return pw_name;
}

string
octave_passwd::passwd (void) const
{
  if (! ok ())
    gripe_invalid ();

  return pw_passwd;
}

uid_t
octave_passwd::uid (void) const
{
  if (! ok ())
    gripe_invalid ();

  return pw_uid;
}

gid_t
octave_passwd::gid (void) const
{
  if (! ok ())
    gripe_invalid ();

  return pw_gid;
}

string
octave_passwd::gecos (void) const
{
  if (! ok ())
    gripe_invalid ();

  return pw_gecos;
}

string
octave_passwd::dir (void) const
{
  if (! ok ())
    gripe_invalid ();

  return pw_dir;
}

string
octave_passwd::shell (void) const
{
  if (! ok ())
    gripe_invalid ();

  return pw_shell;
}

octave_passwd
octave_passwd::getpwent (void)
{
  string msg;
  return getpwent (msg);
}

octave_passwd
octave_passwd::getpwent (string& msg)
{
#if defined HAVE_GETPWENT
  msg = string ();
  return octave_passwd (::getpwent (), msg);
#else
  msg = NOT_SUPPORTED ("getpwent");
  return octave_passwd ();
#endif
}

octave_passwd
octave_passwd::getpwuid (uid_t uid)
{
  string msg;
  return getpwuid (uid, msg);
}

octave_passwd
octave_passwd::getpwuid (uid_t uid, string& msg)
{
#if defined (HAVE_GETPWUID)
  msg = string ();
  return octave_passwd (::getpwuid (uid), msg);
#else
  msg = NOT_SUPPORTED ("getpwuid");
  return octave_passwd ();
#endif
}

octave_passwd
octave_passwd::getpwnam (const string& nm)
{
  string msg;
  return getpwnam (nm, msg);
}

octave_passwd
octave_passwd::getpwnam (const string& nm, string& msg)
{
#if defined (HAVE_GETPWNAM)
  msg = string ();
  return octave_passwd (::getpwnam (nm.c_str ()), msg);
#else
  msg = NOT_SUPPORTED ("getpwnam");
  return octave_passwd ();
#endif
}

int
octave_passwd::setpwent (void)
{
  string msg;
  return setpwent (msg);
}

int
octave_passwd::setpwent (string& msg)
{
#if defined (HAVE_SETPWENT)
  msg = string ();
  ::setpwent ();
  return 0;
#else
  msg = NOT_SUPPORTED ("setpwent");
  return -1;
#endif
}

int
octave_passwd::endpwent (void)
{
  string msg;
  return endpwent (msg);
}

int
octave_passwd::endpwent (string& msg)
{
#if defined (HAVE_ENDPWENT)
  msg = string ();
  ::endpwent ();
  return 0;
#else
  msg = NOT_SUPPORTED ("endpwent");
  return -1;
#endif
}

octave_passwd::octave_passwd (void *p, string& msg)
  : pw_name (), pw_passwd (), pw_uid (0), pw_gid (0), pw_gecos (),
    pw_dir (), pw_shell (), valid (false)
{
#if defined (HAVE_PWD_H)
  msg = string ();

  if (p)
    {
      struct passwd *pw = static_cast<struct passwd *> (p);

      pw_name = pw->pw_name;
      pw_passwd = pw->pw_passwd;
      pw_uid = pw->pw_uid;
      pw_gid = pw->pw_gid;
      pw_gecos = pw->pw_gecos;
      pw_dir = pw->pw_dir;
      pw_shell = pw->pw_shell;

      valid = true;
    }
#else
  msg = NOT_SUPPORTED ("password functions");
#endif
}

void
octave_passwd::gripe_invalid (void) const
{
  (*current_liboctave_error_handler) ("invalid password object");
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
