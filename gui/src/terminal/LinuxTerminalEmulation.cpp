/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2011 John P. Swensen, Jacob Dawid
 * jacob.dawid@googlemail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LinuxTerminalEmulation.h"

LinuxTerminalEmulation::LinuxTerminalEmulation ()
  : TerminalEmulation ()
{
  int fdm, fds;
  openpty (&fdm, &fds, 0, 0, 0);

  dup2 (fds, STDIN_FILENO);
  dup2 (fds, STDOUT_FILENO);
  dup2 (fds, STDERR_FILENO);

  m_pty = new KPtyDevice ();
  m_pty->open (fdm);
  connect (m_pty, SIGNAL(readyRead ()),
           this, SLOT (handleReadyRead ()));
}

LinuxTerminalEmulation::~LinuxTerminalEmulation ()
{
  m_pty->close ();
}

void LinuxTerminalEmulation::processKeyEvent (QKeyEvent *keyEvent)
{
  switch (keyEvent->key ())
    {
      case Qt::Key_PageUp:
      //if (verticalScrollBar ())
      //  verticalScrollBar ()->setValue (verticalScrollBar ()->value () - 10);
      return;
      case Qt::Key_PageDown:
      //if (verticalScrollBar ())
      //  verticalScrollBar ()->setValue (verticalScrollBar ()->value () + 10);
      return;

      case Qt::Key_Up:
      m_pty->write ("\033OA");
      break;

      case Qt::Key_Down:
      m_pty->write ("\033OB");
      break;

      case Qt::Key_Right:
      m_pty->write ("\033OC");
      break;

      case Qt::Key_Left:
      m_pty->write ("\033OF");
      break;

      case Qt::Key_C:
      if (keyEvent->modifiers() & Qt::ControlModifier)
        {
          // TODO: Lookup and implement Control + C.
        }
      else
        {
          m_pty->write (keyEvent->text ().toAscii ());
        }
      break;

      case Qt::Key_V:
      if (keyEvent->modifiers() & Qt::ControlModifier)
        {
          // TODO: Lookup and implement Control + V.
        }
      else
        {
          m_pty->write (keyEvent->text ().toAscii ());
        }
      break;

      case Qt::Key_D:
      if (keyEvent->modifiers() & Qt::ControlModifier)
        {
          // Do not send EOT, because this will crash
          // the program.
        }
      else
        {
          m_pty->write (keyEvent->text ().toAscii ());
        }
      break;

      default:
      m_pty->write (keyEvent->text ().toAscii ());
      break;
    }
  keyEvent->accept ();
}

void
LinuxTerminalEmulation::transmitText (const QString &text)
{
  m_pty->write (text.toLocal8Bit ());
}

void
LinuxTerminalEmulation::handleReadyRead ()
{
  QByteArray data = m_pty->readAll ();

  data.replace("\033[K", "");
  data.replace("\033[9", "");
  data.replace("\033[A", "");
  data.replace("\033[B", "");
  data.replace("\033[C", "");
  data.replace("\033[D", "");
  data.replace("\033[1", "");
  data.replace("\033[H", "");
  data.replace("\033[2J", "");
  int position;
  QTextCursor tc = m_terminal->textCursor ();
  tc.beginEditBlock ();

  // Decode data into cursor actions.
  foreach(QChar character, data)
    {
      unsigned short unicode = character.unicode ();
      switch (unicode)
        {
        case 0: // Null (NUL)
          qDebug ("NUL");
          break;
        case 1: // Start Of Heading (SOH)
          qDebug ("SOH");
          break;
        case 2: // Start Of Text (STX)
          qDebug ("STX");
          break;
        case 3: // End Of Text (ETX)
          qDebug ("ETX");
          break;
        case 4: // End Of Transmission (EOT)
          qDebug ("EOT");
          break;
        case 5: // Enquiry (ENQ)
          qDebug ("ENQ");
          break;
        case 6: // Acknowledgement (ACK)
          qDebug ("ACK");
          break;
        case 7: // Bell (BEL)
          m_terminal->bell ();
          break;
        case 8: // Backspace (BS)
          tc.deletePreviousChar ();
          break;
        case 9: // Horizontal Tab (HT)
          qDebug ("HT");
          break;
        case 10: // Line Feed (LF)
          position = tc.position ();
          tc.movePosition (QTextCursor::EndOfLine);
          tc.insertText ("\n");
          tc.setPosition (position);
          tc.movePosition (QTextCursor::Down);
          break;
        case 11: // Vertical Tab (VT)
          qDebug ("VT");
          break;
        case 12: // Form Feed (FF)
          qDebug ("FF");
          break;
        case 13: // Carriage Return (CR)
          tc.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
          break;
        case 14: // Shift Out (SO)
          qDebug ("SO");
          break;
        case 15: // Shift In (SI)
          qDebug ("SI");
          break;
        case 16: // Data Link Escape (DLE)
          qDebug ("DLE");
          break;
        case 17: // Device Control 1 (DC1, XON)
          qDebug ("DC1");
          break;
        case 18: // Device Control 2 (DC2)
          qDebug ("DC2");
          break;
        case 19: // Device Control 3 (DC3, XOFF)
          qDebug ("DC3");
          break;
        case 20: // Device Control 4 (DC4)
          qDebug ("DC4");
          break;
        case 21: // Negative Acknowledgement (NAK)
          qDebug ("NAK");
          break;
        case 22: // Synchronous Idle (SYN)
          qDebug ("SYN");
          break;
        case 23: // End Of Transmission Block (ETB)
          qDebug ("ETB");
          break;
        case 24: // Cancel (CAN)
          qDebug ("CAN");
          break;
        case 25: // End of Medium (EM)
          qDebug ("EM");
          break;
        case 26: // Substitute (SUB)
          qDebug ("SUB");
          break;
        case 27: // Escape (ESC)
          qDebug ("ESC");
          break;
        case 28: // File Separator (FS)
          qDebug ("FS");
          break;
        case 29: // Group Separator (GS)
          qDebug ("GS");
          break;
        case 30: // Record Separator (RS)
          qDebug ("RS");
          break;
        case 31: // Unit Separator (US)
          qDebug ("US");
          break;
        case 127: // Delete (DEL)
          qDebug ("DEL");
          break;
        default:
          tc.insertText (character);
          break;
        }
    }

  tc.endEditBlock ();
  m_terminal->setTextCursor (tc);
}
