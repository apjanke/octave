/*

Copyright (C) 2016 John W. Eaton

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

#if ! defined (octave_glob_wrappers_h)
#define octave_glob_wrappers_h 1

#if defined __cplusplus
extern "C" {
#endif

extern void *octave_create_glob_info_struct (void);

// Does not call globfree.
extern void octave_destroy_glob_info_struct (void *glob_info);

// We don't need the error function pointer that the system glob
// function allows.
extern int
octave_glob_wrapper (const char *pattern, int flags, void *glob_info);

extern int octave_glob_num_matches (void *glob_info);

extern char **octave_glob_match_list (void *glob_info);

extern void octave_globfree_wrapper (void *glob_info);

extern int octave_glob_nosort_wrapper (void);
  
extern int
octave_fnmatch_wrapper (const char *pattern, const char *name, int flags);

extern int octave_fnm_nomatch_wrapper (void);

extern int octave_fnm_pathname_wrapper (void);

extern int octave_fnm_noescape_wrapper (void);

extern int octave_fnm_period_wrapper (void);

#if defined __cplusplus
}
#endif

#endif

