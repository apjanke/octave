/* This file is part of the KDE libraries

    Copyright (C) 2003,2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef kpty_p_h
#define kpty_p_h

#include "kpty.h"

#if defined(Q_OS_MAC)
#define HAVE_UTIL_H
#define HAVE_UTMPX
#define _UTMPX_COMPAT
#define HAVE_PTSNAME
#define HAVE_SYS_TIME_H
#else
#define HAVE_PTY_H
#endif

#define HAVE_OPENPTY

#include <QtCore/QByteArray>

struct KPtyPrivate
{
  Q_DECLARE_PUBLIC (KPty) KPtyPrivate (KPty * parent);
  virtual ~ KPtyPrivate ();
#ifndef HAVE_OPENPTY
  bool chownpty (bool grant);
#endif

  int masterFd;
  int slaveFd;
  bool ownMaster:1;

  QByteArray ttyName;

  KPty *q_ptr;
};

#endif
