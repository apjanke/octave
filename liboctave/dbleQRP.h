//                                  -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994 John W. Eaton

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
Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#if !defined (octave_QRP_h)
#define octave_QRP_h 1

#if defined (__GNUG__)
#pragma interface
#endif

class ostream;

#include "dbleQR.h"

extern "C++" {

class QRP : public QR
{
public:

  QRP (void) {}

  QRP (const Matrix& A, QR::type qr_type = QR::std);

  QRP (const QRP& a);

  QRP& operator = (const QRP& a);

  Matrix P (void) const;

  friend ostream&  operator << (ostream& os, const QRP& a);

private:

  Matrix p;
};

inline QRP::QRP (const QRP& a) : QR (a)
{
  p = a.p;
}

inline QRP& QRP::operator = (const QRP& a)
{
  QR::operator = (a);
  p = a.p;
  return *this;
}

inline Matrix QRP::P (void) const
{
  return p;
}

} // extern "C++"

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
