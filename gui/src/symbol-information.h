/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2011 Jacob Dawid (jacob.dawid@googlemail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYMBOLINFORMATION_H
#define SYMBOLINFORMATION_H

#include <QString>
#include <QHash>

// Octave includes
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_URL
#include "octave/config.h"
#include "octave/cmd-edit.h"
#include "octave/error.h"
#include "octave/file-io.h"
#include "octave/input.h"
#include "octave/lex.h"
#include "octave/load-path.h"
#include "octave/octave.h"
#include "octave/oct-hist.h"
#include "octave/oct-map.h"
#include "octave/oct-obj.h"
#include "octave/ops.h"
#include "octave/ov.h"
#include "octave/ov-usr-fcn.h"
#include "octave/symtab.h"
#include "octave/pt.h"
#include "octave/pt-eval.h"
#include "octave/config.h"
#include "octave/Range.h"
#include "octave/toplev.h"
#include "octave/procstream.h"
#include "octave/sighandlers.h"
#include "octave/debug.h"
#include "octave/sysdep.h"
#include "octave/ov.h"
#include "octave/unwind-prot.h"
#include "octave/utils.h"
#include "octave/variables.h"

/**
  * \struct symbol_information
  * \brief Meta-information over a symbol-table entry.
  * \author Jacob Dawid
  * This struct is used to store meta information over a symbol entry.
  * It reduces memory consumption, since it only stores relevant data
  * about a symbol-table entry that will be used in the model for the
  * graphical user interface.
  */
typedef struct symbol_information
{
  enum Scope
  {
    local       = 0,
    global      = 1,
    persistent  = 2,
    hidden      = 3
  };

  QString _symbol;
  QString _type;
  QString _value;
  Scope   _scope;

  /** Hashes the symbol information for quickly comparing it. */
  int
  hash () const
  {
    return qHash (_symbol) + qHash (_type) + qHash (_value) + (int)_scope;
  }

  /** Compares two symbol information objects. */
  bool
  equals (const symbol_information& other) const
  {
    if (hash () == other.hash ())
      {
        return _symbol == other._symbol
            && _type   == other._type
            && _value  == other._value
            && _scope  == other._scope;
      }
  }

  /** Extracts meta information from a given symbol record. */
  bool
  from_symbol_record (const symbol_table::symbol_record& symbol_record)
  {
    if (symbol_record.is_local () && !symbol_record.is_global () && !symbol_record.is_hidden ())
      _scope = local;
    else if (symbol_record.is_global ())
      _scope = global;
    else if (symbol_record.is_persistent ())
      _scope = persistent;
    else if (symbol_record.is_hidden ())
      _scope = hidden;

    _symbol = QString (symbol_record.name ().c_str ());
    _type   = QString (symbol_record.varval ().type_name ().c_str ());
    octave_value ov = symbol_record.varval ();

    // For every type, convert to a human readable string.
    if (ov.is_sq_string ())
      _value = QString ("\'%1\'").arg (ov.string_value ().c_str ());
    else if (ov.is_dq_string ())
      _value = QString ("\"%1\"").arg (ov.string_value ().c_str ());
    else if (ov.is_real_scalar ())
      _value = QString ("%1").arg (ov.scalar_value ());
    else if (ov.is_complex_scalar ())
      _value = QString ("%1 + %2i").arg (ov.scalar_value ())
                                   .arg (ov.complex_value ().imag ());
    else if (ov.is_range ())
      _value =  QString ("%1 : %2 : %3").arg (ov.range_value ().base ())
                                        .arg (ov.range_value ().inc ())
                                        .arg (ov.range_value ().limit ());
    else if (ov.is_matrix_type())
      _value = QString ("%1x%2").arg (ov.rows ())
                                .arg (ov.columns ());
    else if (ov.is_cell())
      _value = QString ("%1x%2").arg (ov.rows ())
                                .arg (ov.columns ());
    else if (ov.is_bool_type () && !ov.is_matrix_type())
      _value = ov.bool_value () ? "true" : "false";
    else
      _value = QString ("<Type not recognized>");

    _value.replace("\n", "\\n");
    return true;
  }
} symbol_information;

#endif // SYMBOLINFORMATION_H
