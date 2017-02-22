/*

Copyright (C) 2014-2017 Torsten

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

// Author: Torsten <ttl@justmail.de>

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#if defined (HAVE_QSCINTILLA)

#include <Qsci/qscilexer.h>

#include "octave-txt-lexer.h"

QString
octave_txt_lexer::description (int style) const
{
  if (style == 0)
    return tr ("Default");
  else
    return QString ();
};

const char*
octave_txt_lexer::language () const
{
  return "Text";
}

const char*
octave_txt_lexer::lexer () const
{
  return "text";
}

#endif
