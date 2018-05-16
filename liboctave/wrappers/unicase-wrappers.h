/*

Copyright (C) 2018 Markus Mützel

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_unicase_wrappers_h)
#define octave_unicase_wrappers_h 1

#if defined __cplusplus
extern "C" {
#endif

extern uint8_t *
octave_u8_tolower_wrapper (const uint8_t *s, size_t n,
                           const char *iso639_language,
                           uint8_t *resultbuf, size_t *lengthp);

extern uint8_t *
octave_u8_toupper_wrapper (const uint8_t *s, size_t n,
                           const char *iso639_language,
                           uint8_t *resultbuf, size_t *lengthp);

#if defined __cplusplus
}
#endif

#endif
