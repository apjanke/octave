/*

Copyright (C) 2002 John W. Eaton

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

#if !defined (octave_base_dae_h)
#define octave_base_dae_h 1

#include "base-de.h"

class
base_diff_alg_eqn : public base_diff_eqn
{
public:

  base_diff_alg_eqn (void)
    : base_diff_eqn (), xdot () { }

  base_diff_alg_eqn (const ColumnVector& xx, double tt)
    : base_diff_eqn (xx, tt), xdot (xx.length (), 0.0) { }

  base_diff_alg_eqn (const ColumnVector& xx, const ColumnVector& xxdot,
		     double tt)
    : base_diff_eqn (xx, tt), xdot (xxdot) { }

  base_diff_alg_eqn (const base_diff_alg_eqn& a)
    : base_diff_eqn (a), xdot (a.xdot) { }

  virtual ~base_diff_alg_eqn (void) { }

  base_diff_alg_eqn& operator = (const base_diff_alg_eqn& a)
    {
      if (this != &a)
	{
	  base_diff_eqn::operator = (a);
	  xdot = a.xdot;
	}
      return *this;
    }

  void initialize (const ColumnVector& x0, double t0)
    {
      base_diff_eqn::initialize (x0, t0);
      xdot.resize (x0.length (), 0.0);
      force_restart ();
    }

  void initialize (const ColumnVector& x0, const ColumnVector& xdot0,
		   double t0)
    {
      base_diff_eqn::initialize (x0, t0);
      xdot = xdot0;
      force_restart ();
    }

  ColumnVector state_derivative (void) { return xdot; }

protected:

  ColumnVector xdot;
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
