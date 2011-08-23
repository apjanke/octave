/*

   This file is part of the KDE libraries
   Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>
   Copyright (C) 2010 KDE e.V. <kde-ev-board@kde.org>
     Author Adriaan de Groot <groot@kde.org>

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

#include "kptydevice.h"
#include "kpty_p.h"
#define i18n

#include <QtCore/QSocketNotifier>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define PTY_BYTES_AVAILABLE TIOCINQ

//////////////////
// private data //
//////////////////

// Lifted from Qt. I don't think they would mind. ;)
// Re-lift again from Qt whenever a proper replacement for pthread_once appears
static void
qt_ignore_sigpipe ()
{
  static QBasicAtomicInt atom = Q_BASIC_ATOMIC_INITIALIZER (0);
  if (atom.testAndSetRelaxed (0, 1))
    {
      struct sigaction noaction;
      memset (&noaction, 0, sizeof (noaction));
      noaction.sa_handler = SIG_IGN;
      sigaction (SIGPIPE, &noaction, 0);
    }
}

#define NO_INTR(ret,func) do { ret = func; } while (ret < 0 && errno == EINTR)

bool
KPtyDevicePrivate::_k_canRead ()
{
  Q_Q (KPtyDevice);
  qint64 readBytes = 0;


  int available;
  if (!::ioctl (q->masterFd (), PTY_BYTES_AVAILABLE, (char *) &available))
    {
      char *ptr = readBuffer.reserve (available);
	// Useless block braces except in Solaris
	{
	  NO_INTR (readBytes, read (q->masterFd (), ptr, available));
	}
      if (readBytes < 0)
	{
	  readBuffer.unreserve (available);
	  q->setErrorString (i18n ("Error reading from PTY"));
	  return false;
	}
      readBuffer.unreserve (available - readBytes);	// *should* be a no-op
    }

  if (!readBytes)
    {
      readNotifier->setEnabled (false);
      return false;
    }
  else
    {
      if (!emittedReadyRead)
	{
	  emittedReadyRead = true;
	  emit q->readyRead ();
	  emittedReadyRead = false;
	}
      return true;
    }
}

bool
KPtyDevicePrivate::_k_canWrite ()
{
  Q_Q (KPtyDevice);

  writeNotifier->setEnabled (false);
  if (writeBuffer.isEmpty ())
    return false;

  qt_ignore_sigpipe ();
  int wroteBytes;
  NO_INTR (wroteBytes,
	   write (q->masterFd (),
		  writeBuffer.readPointer (), writeBuffer.readSize ()));
  if (wroteBytes < 0)
    {
      q->setErrorString (i18n ("Error writing to PTY"));
      return false;
    }
  writeBuffer.free (wroteBytes);

  if (!emittedBytesWritten)
    {
      emittedBytesWritten = true;
      emit q->bytesWritten (wroteBytes);
      emittedBytesWritten = false;
    }

  if (!writeBuffer.isEmpty ())
    writeNotifier->setEnabled (true);
  return true;
}

#ifndef timeradd
// Lifted from GLIBC
#define timeradd(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
        if ((result)->tv_usec >= 1000000) { \
            ++(result)->tv_sec; \
            (result)->tv_usec -= 1000000; \
        } \
    } while (0)
#define timersub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_usec += 1000000; \
        } \
    } while (0)
#endif

bool
KPtyDevicePrivate::doWait (int msecs, bool reading)
{
  Q_Q (KPtyDevice);
  struct timeval tv, *tvp;

  if (msecs < 0)
    tvp = 0;
  else
    {
      tv.tv_sec = msecs / 1000;
      tv.tv_usec = (msecs % 1000) * 1000;
      tvp = &tv;
    }

  while (reading ? readNotifier->isEnabled () : !writeBuffer.isEmpty ())
    {
      fd_set rfds;
      fd_set wfds;

      FD_ZERO (&rfds);
      FD_ZERO (&wfds);

      if (readNotifier->isEnabled ())
	FD_SET (q->masterFd (), &rfds);
      if (!writeBuffer.isEmpty ())
	FD_SET (q->masterFd (), &wfds);

      switch (select (q->masterFd () + 1, &rfds, &wfds, 0, tvp))
	{
	case -1:
	  if (errno == EINTR)
	    break;
	  return false;
	case 0:
	  q->setErrorString (i18n ("PTY operation timed out"));
	  return false;
	default:
	  if (FD_ISSET (q->masterFd (), &rfds))
	    {
	      bool canRead = _k_canRead ();
	      if (reading && canRead)
		return true;
	    }
	  if (FD_ISSET (q->masterFd (), &wfds))
	    {
	      bool canWrite = _k_canWrite ();
	      if (!reading)
		return canWrite;
	    }
	  break;
	}
    }
  return false;
}

void
KPtyDevicePrivate::finishOpen (QIODevice::OpenMode mode)
{
  Q_Q (KPtyDevice);

  q->QIODevice::open (mode);
  fcntl (q->masterFd (), F_SETFL, O_NONBLOCK);
  readBuffer.clear ();
  readNotifier =
    new QSocketNotifier (q->masterFd (), QSocketNotifier::Read, q);
  writeNotifier =
    new QSocketNotifier (q->masterFd (), QSocketNotifier::Write, q);
  QObject::connect (readNotifier, SIGNAL (activated (int)), q,
		    SLOT (_k_canRead ()));
  QObject::connect (writeNotifier, SIGNAL (activated (int)), q,
		    SLOT (_k_canWrite ()));
  readNotifier->setEnabled (true);
}

KPtyDevice::KPtyDevice (QObject * parent):
QIODevice (parent), KPty (new KPtyDevicePrivate (this))
{
}

KPtyDevice::~KPtyDevice ()
{
  close ();
}

bool
KPtyDevice::open (OpenMode mode)
{
  Q_D (KPtyDevice);

  if (masterFd () >= 0)
    return true;

  if (!KPty::open ())
    {
      setErrorString (i18n ("Error opening PTY"));
      return false;
    }

  d->finishOpen (mode);

  return true;
}

bool
KPtyDevice::open (int fd, OpenMode mode)
{
  Q_D (KPtyDevice);

  if (!KPty::open (fd))
    {
      setErrorString (i18n ("Error opening PTY"));
      return false;
    }

  d->finishOpen (mode);

  return true;
}

void
KPtyDevice::close ()
{
  Q_D (KPtyDevice);

  if (masterFd () < 0)
    return;

  delete d->readNotifier;
  delete d->writeNotifier;

  QIODevice::close ();

  KPty::close ();
}

bool
KPtyDevice::canReadLine () const
{
  Q_D (const KPtyDevice);
  return QIODevice::canReadLine () || d->readBuffer.canReadLine ();
}

bool
KPtyDevice::atEnd () const
{
  Q_D (const KPtyDevice);
  return QIODevice::atEnd () && d->readBuffer.isEmpty ();
}

qint64
KPtyDevice::bytesAvailable () const
{
  Q_D (const KPtyDevice);
  return QIODevice::bytesAvailable () + d->readBuffer.size ();
}

qint64
KPtyDevice::bytesToWrite () const
{
  Q_D (const KPtyDevice);
  return d->writeBuffer.size ();
}

// protected
qint64
KPtyDevice::readData (char *data, qint64 maxlen)
{
  Q_D (KPtyDevice);
  return d->readBuffer.read (data, (int) qMin < qint64 > (maxlen, KMAXINT));
}

// protected
qint64
KPtyDevice::readLineData (char *data, qint64 maxlen)
{
  Q_D (KPtyDevice);
  return d->readBuffer.readLine (data,
				 (int) qMin < qint64 > (maxlen, KMAXINT));
}

// protected
qint64
KPtyDevice::writeData (const char *data, qint64 len)
{
  Q_D (KPtyDevice);
  Q_ASSERT (len <= KMAXINT);

  d->writeBuffer.write (data, len);
  d->writeNotifier->setEnabled (true);
  return len;
}
