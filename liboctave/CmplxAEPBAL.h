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

#if !defined (octave_ComplexAEPBALANCE_h)
#define octave_ComplexAEPBALANCE_h 1

#if defined (__GNUG__) && ! defined (NO_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma interface
#endif

#include <iostream>
#include <string>

#include "CMatrix.h"

class
ComplexAEPBALANCE
{
public:

  ComplexAEPBALANCE (void) : balanced_mat (), balancing_mat () { }

  ComplexAEPBALANCE (const ComplexMatrix& a, const std::string& balance_job)
    {
      init (a, balance_job); 
    }

  ComplexAEPBALANCE (const ComplexAEPBALANCE& a)
    : balanced_mat (a.balanced_mat), balancing_mat (a.balancing_mat) { }

  ComplexAEPBALANCE& operator = (const ComplexAEPBALANCE& a)
    {
      if (this != &a)
	{
	  balanced_mat = a.balanced_mat;
	  balancing_mat = a.balancing_mat;
	}
      return *this;
    }

  ~ComplexAEPBALANCE (void) { }

  ComplexMatrix balanced_matrix (void) const { return balanced_mat; }

  ComplexMatrix balancing_matrix (void) const { return balancing_mat; }

  friend std::ostream& operator << (std::ostream& os, const ComplexAEPBALANCE& a);

private:

  ComplexMatrix balanced_mat;
  ComplexMatrix balancing_mat;

  int init (const ComplexMatrix& a, const std::string& balance_job);
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
