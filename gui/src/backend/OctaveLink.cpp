/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2011 John P. Swensen, Jacob Dawid (jacob.dawid@googlemail.com)
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

#include "OctaveLink.h"

OctaveLink OctaveLink::m_singleton;

OctaveLink::OctaveLink ():QObject ()
{
  m_symbolTableSemaphore = new QSemaphore (1);
  m_historyModel = new QStringListModel (this);
}

OctaveLink::~OctaveLink ()
{
}

QString
OctaveLink::octaveValueAsQString (OctaveValue octaveValue)
{
  // Convert single qouted string.
  if (octaveValue.is_sq_string ())
    {
      return QString ("\'%1\'").arg (octaveValue.string_value ().c_str ());

      // Convert double qouted string.
    }
  else if (octaveValue.is_dq_string ())
    {
      return QString ("\"%1\"").arg (octaveValue.string_value ().c_str ());

      // Convert real scalar.
    }
  else if (octaveValue.is_real_scalar ())
    {
      return QString ("%1").arg (octaveValue.scalar_value ());

      // Convert complex scalar.
    }
  else if (octaveValue.is_complex_scalar ())
    {
      return QString ("%1 + %2i").arg (octaveValue.scalar_value ()).
	arg (octaveValue.complex_value ().imag ());

      // Convert range.
    }
  else if (octaveValue.is_range ())
    {
      return QString ("%1 : %2 : %3").arg (octaveValue.range_value ().
					   base ()).arg (octaveValue.
							 range_value ().
							 inc ()).
	arg (octaveValue.range_value ().limit ());

      // Convert real matrix.
    }
  else if (octaveValue.is_real_matrix ())
    {
      return QString ("%1x%2 matrix")
          .arg (octaveValue.matrix_value ().rows ())
          .arg (octaveValue.matrix_value ().cols ());

      // Convert complex matrix.
    }
  else if (octaveValue.is_complex_matrix ())
    {
    return QString ("%1x%2 complex matrix")
        .arg (octaveValue.matrix_value ().rows ())
        .arg (octaveValue.matrix_value ().cols ());

      // If everything else does not fit, we could not recognize the type.
    }
  else
    {
      return QString ("<Type not recognized>");
    }
}

void
OctaveLink::launchOctave ()
{
  // Create both threads.
  m_octaveMainThread = new OctaveMainThread (this);
  m_octaveCallbackThread = new OctaveCallbackThread (this);

  // Launch the second as soon as the first ist ready.
  connect (m_octaveMainThread, SIGNAL(ready()), m_octaveCallbackThread, SLOT(start()));

  // Start the first one.
  m_octaveMainThread->start ();
}

void
OctaveLink::terminateOctave ()
{
  m_octaveCallbackThread->halt();
  m_octaveCallbackThread->wait ();

  m_octaveMainThread->terminate ();
  //m_octaveMainThread->wait();
}

QList < SymbolRecord > OctaveLink::symbolTable ()
{
  m_symbolTableBuffer.clear ();
  std::list < SymbolRecord > allVariables = symbol_table::all_variables ();
  std::list < SymbolRecord >::iterator iterator;
  for (iterator = allVariables.begin (); iterator != allVariables.end ();
       iterator++)
    {
      SymbolRecord s = iterator->dup (symbol_table::global_scope ());
      m_symbolTableBuffer.append (s);
    }
  return m_symbolTableBuffer;
}

void
OctaveLink::triggerUpdateHistoryModel ()
{
  // Determine the client's (our) history length and the one of the server.
  int clientHistoryLength = m_historyModel->rowCount ();
  int serverHistoryLength = command_history::length ();

  // If were behind the server, iterate through all new entries and add them to our history.
  if (clientHistoryLength < serverHistoryLength)
    {
      for (int i = clientHistoryLength; i < serverHistoryLength; i++)
        {
          m_historyModel->insertRow (0);
          m_historyModel->setData (m_historyModel->index (0), QString (command_history::get_entry (i).c_str ()));
        }
    }
}

QStringListModel *
OctaveLink::historyModel ()
{
  return m_historyModel;
}
