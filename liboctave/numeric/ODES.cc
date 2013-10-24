/*

Copyright (C) 2002-2013 John W. Eaton

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

#include "ODES.h"
#include "lo-error.h"

void
ODES::initialize (const ColumnVector& xx, double tt)
{
  base_diff_eqn::initialize (xx, tt);
  xdot = ColumnVector (xx.length (), 0.0);
}

void
ODES::initialize (const ColumnVector& xx, double tt,
                  const ColumnVector& xtheta)
{
  base_diff_eqn::initialize (xx, tt);
  xdot = ColumnVector (xx.length (), 0.0);
  theta = xtheta;
}
