/*

Copyright (C) 2015 John W. Eaton

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

#include "oct-hdf5.h"

// Programming Note: This file exists so that we can hide the use
// of macros and C-style casts in a C warnings about using old-style
// casts in C++.

// Please do NOT eliminate this file and move code from here to
// oct-hdf5-types.cc

#if defined (HAVE_HDF5)

const octave_hdf5_id octave_H5E_DEFAULT = H5E_DEFAULT;
const octave_hdf5_id octave_H5P_DEFAULT = H5P_DEFAULT;
const octave_hdf5_id octave_H5S_ALL = H5S_ALL;

#endif
