/* OctaveGUI - A graphical user interface for Octave
 * Copyright (C) 2011 Jacob Dawid
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

#include "IRCClientImpl.h"

IRCServerMessage::IRCServerMessage (const QString& serverMessage)
{
  int position = 0;
  QString buffer;

  m_nick = "";
  m_user = "";
  m_host = "";

  // A server message starting with a prefix indicates
  // a prefix. A prefix has the format:
  // :nick!user@host
  // followed by a space character.
  if (serverMessage.startsWith(":"))
    {
      position++;
      while ((serverMessage.at (position) != '!') && !serverMessage.at (position).isSpace ())
        {
          buffer.append (serverMessage.at (position));
          position++;
        }
      m_nick = buffer, buffer.clear(), position++;

      // If it belongs to the prefix, it must be concatenanted neatlessly without
      // any spaces.
      if (!serverMessage.at (position - 1).isSpace ())
        {
          while (serverMessage.at (position) != '@')
            {
              buffer.append (serverMessage.at (position));
              position++;
            }
          m_user = buffer, buffer.clear(), position++;
        }

      // If it belongs to the prefix, it must be concatenanted neatlessly without
      // any spaces.
      if (!serverMessage.at (position - 1).isSpace ())
        {
          while (serverMessage.at (position) != ' ')
            {
              buffer.append (serverMessage.at (position));
              position++;
            }
          m_host = buffer, buffer.clear(), position++;
        }
    }

  // The next part is the command. The command can either be numeric
  // or a written command.
  while (!serverMessage.at (position).isSpace ())
    {
      buffer.append (serverMessage.at (position));
      position++;
    }
  m_command = buffer.toUpper(), buffer.clear(), position++;
  m_codeNumber = m_command.toInt(&m_isNumeric);

  // Next: a list of parameters. If any of these parameters
  // starts with a colon, we have to read everything that follows
  // as a single parameter.
  bool readUntilEnd = false;
  while (position < serverMessage.size ())
    {
      if (buffer.isEmpty () && !readUntilEnd && (serverMessage.at (position) == ':'))
        {
          readUntilEnd = true;
        }
      else
        {

          if (readUntilEnd)
            {
              buffer.append (serverMessage.at (position));
            }
          else
            {
              if (serverMessage.at (position) == ' ')
                {
                  m_parameters.append (buffer);
                  buffer.clear ();
                }
            }
        }
      position++;
    }

  if (!buffer.isEmpty ())
    m_parameters.append (buffer);
}

int
IRCServerMessage::numericValue ()
{
  if (m_isNumeric)
    return m_codeNumber;
  return -1;
}

QString
IRCServerMessage::parameter (int index)
{
  if (index >= 0 && index < m_parameters.size ())
    return m_parameters.at (index);
  return "";
}


IRCChannelProxy::IRCChannelProxy (IRCClientInterface *clientInterface, const QString& channelName, QObject *parent)
  : IRCChannelProxyInterface (clientInterface, channelName, parent),
    m_clientInterface (clientInterface)
{
  m_channelName = channelName;
}

QTextDocument *
IRCChannelProxy::conversationModel ()
{
  return &m_conversationModel;
}

QStringListModel *
IRCChannelProxy::userListModel ()
{
  return &m_userListModel;
}

QString
IRCChannelProxy::channelName ()
{
  return m_channelName;
}

void
IRCChannelProxy::sendMessage (const QString& message)
{
  QStringList arguments;
  arguments << m_channelName;
  arguments << message;
  m_clientInterface->sendIRCCommand (IRCCommand::PrivateMessage, arguments);
}

void
IRCChannelProxy::sendJoinRequest ()
{
  m_clientInterface->sendIRCCommand (IRCCommand::Join, QStringList (m_channelName));
}


void
IRCChannelProxy::leave (const QString& reason)
{
  Q_UNUSED (reason);
}

IRCClientImpl::IRCClientImpl (QObject *parent)
  : IRCClientInterface (parent)
{
  connect (&m_tcpSocket, SIGNAL (connected ()), this, SLOT (handleConnected ()));
  connect (&m_tcpSocket, SIGNAL (disconnected ()), this, SLOT (handleDisconnected ()));
  connect (&m_tcpSocket, SIGNAL (readyRead ()), this, SLOT (handleReadyRead ()));
}

IRCClientImpl::~IRCClientImpl ()
{
  foreach (IRCChannelProxyInterface *ircChannelProxy, m_channels)
    {
      delete ircChannelProxy;
    }
}

void
IRCClientImpl::connectToHost (const QHostAddress& host, int port, const QString& initialNick)
{
  m_host = host;
  m_nickname = initialNick;
  m_tcpSocket.connectToHost(host, port);
}

void
IRCClientImpl::disconnect ()
{
  m_tcpSocket.disconnect ();
}

void
IRCClientImpl::reconnect ()
{
  disconnect ();
  connectToHost (m_host, m_port, m_nickname);
}

bool
IRCClientImpl::isConnected ()
{
  return m_connected;
}

const QHostAddress&
IRCClientImpl::host()
{
  return m_host;
}

int
IRCClientImpl::port()
{
  return m_port;
}

IRCChannelProxyInterface *
IRCClientImpl::ircChannelProxy (const QString &channel)
{
  if (!m_channels.contains (channel))
      m_channels[channel] = new IRCChannelProxy(this, channel);
  return m_channels[channel];
}

void
IRCClientImpl::sendNicknameChangeRequest (const QString &nickname)
{
  sendIRCCommand (IRCCommand::Nick, QStringList (nickname));
}

void
IRCClientImpl::sendPrivateMessage (const QString &recipient, const QString &message)
{
  QStringList arguments;
  arguments << recipient;
  arguments << message;
  sendIRCCommand (IRCCommand::PrivateMessage, arguments);
}

const QString&
IRCClientImpl::nickname ()
{
  return m_nickname;
}

void
IRCClientImpl::handleConnected ()
{
  m_connected = true;
  QStringList arguments;
  arguments << "na" << "0" << "0" << "na";
  sendIRCCommand (IRCCommand::User, arguments);
  sendNicknameChangeRequest (m_nickname);
  emit connected (m_host.toString ());
}

void
IRCClientImpl::handleDisconnected ()
{
  m_connected = false;
  emit disconnected ();
}

void
IRCClientImpl::handleReadyRead ()
{
  QByteArray line;
  do
    {
      line = m_tcpSocket.readLine();
      if (line.size ())
        handleIncomingLine(QString(line));
      else
        break;
    }
  while (true);
}

void
IRCClientImpl::handleNicknameChanged (const QString &oldNick, const QString &newNick)
{
  emit nicknameChanged (oldNick, newNick);
}

void
IRCClientImpl::handleUserJoined (const QString &nick, const QString &channel)
{
  emit userJoined (nick, channel);
}

void
IRCClientImpl::handleUserQuit (const QString &nick, const QString &reason)
{
  emit userQuit (nick, reason);
}

void
IRCClientImpl::handleIncomingLine (const QString &line)
{
  //emit debugMessage (QString (">>>recv: \"%1\"").arg (line));
  if (m_connected && !line.isEmpty())
    {
      IRCServerMessage ircServerMessage(line);
      if (ircServerMessage.isNumeric () == true)
        {
          switch (ircServerMessage.numericValue ())
            {
              case IRCReply::Welcome:
                emit loggedIn (nickname ());
                break;
              case IRCError::NicknameInUse:
              case IRCError::NickCollision:
                emit debugMessage ("FIXME: Received nickname in use reply.");
                break;
              case IRCError::PasswordMismatch:
                emit debugMessage ("FIXME: Received password mismatch reply.");
                break;
              case IRCReply::MessageOfTheDayStart:
              case IRCReply::MessageOfTheDay:
              case IRCReply::MessageOfTheDayEnd:
              case IRCError::NoMessageOfTheDay:
                break;
              case IRCReply::NoTopic:
              case IRCReply::Topic:
                break;
              case IRCReply::NameReply:
                emit debugMessage (QString ("LINKME: (NameReply) \'%1\'").arg (ircServerMessage.parameter (3)));
                //m_nickList = event->getParam (3).split (QRegExp ("\\s+"), QString::SkipEmptyParts);
                break;
            }
        }
      else
        {
          QString command = ircServerMessage.command ();
          if (command == IRCCommand::Nick)
            {
              handleNicknameChanged (ircServerMessage.parameter (0), ircServerMessage.parameter (1));
            }
          else if (command == IRCCommand::Quit)
            {
              handleUserQuit (ircServerMessage.nick (), ircServerMessage.parameter (0));
            }
          else if (command == IRCCommand::Join)
            {
              handleUserJoined(ircServerMessage.nick (), ircServerMessage.parameter (0));
            }
          else if (command == IRCCommand::Part)
            {
              emit debugMessage ("WRITEME: Received part.");
              //emit part (ircEvent.getNick ().toStdString ().c_str (),
              //           ircEvent.getParam (0).toStdString ().c_str (),
              //           ircEvent.getParam (1).toStdString ().c_str ());
            }
          else if (command == IRCCommand::Mode)
            {
              emit debugMessage ("WRITEME: Received mode.");
              //emit mode (&ircEvent);
            }
          else if (command == IRCCommand::Topic)
            {
              emit debugMessage ("WRITEME: Received topic.");
              //emit topic (ircEvent.getNick ().toStdString ().c_str (),
              //            ircEvent.getParam (0).toStdString ().c_str (),
              //            ircEvent.getParam (1).toStdString ().c_str ());
            }
          else if (command == IRCCommand::Kick)
            {
              emit debugMessage ("WRITEME: Received kick.");
              //emit kick (ircEvent.getNick ().toStdString ().c_str (),
              //           ircEvent.getParam (0).toStdString ().c_str (),
              //           ircEvent.getParam (1).toStdString ().c_str (),
              //           ircEvent.getParam (2).toStdString ().c_str ());
            }
          else if (command == IRCCommand::Invite)
            {
              emit debugMessage ("WRITEME: Received invite.");
              //emit invite (ircEvent.getNick ().toStdString ().c_str (),
              //             ircEvent.getParam (1).toStdString ().c_str ());
            }
          else if (command == IRCCommand::PrivateMessage)
            {
              emit message (ircServerMessage.parameter (0), ircServerMessage.nick (), ircServerMessage.parameter (1));
            }
          else if (command == IRCCommand::Notice)
            {
              emit notification (ircServerMessage.nick ().toStdString ().c_str (),
                                 ircServerMessage.parameter (1).toStdString ().c_str ());
            }
          else if (command == IRCCommand::Ping)
            {
              sendIRCCommand (IRCCommand::Pong, QStringList (m_nickname));
            }
          else if (command == IRCCommand::Error)
            {
              emit error (ircServerMessage.parameter (0));
            }
          else
            {
              emit debugMessage (QString("FIXME: Received unknown reply: %1").arg(command));
              // not recognized.
            }
        }
    }
}

void
IRCClientImpl::sendLine (const QString &line)
{
  //emit debugMessage (QString (">>>send: \"%1\"").arg (line));
  if (m_connected)
    m_tcpSocket.write ((line + "\r\n").toStdString ().c_str ());
}

void
IRCClientImpl::sendIRCCommand (const QString &command, const QStringList &arguments)
{
  QString line = command;
  for (int i = 0; i < arguments.size (); i++)
    {
      bool applyColon = false;
      // Usually all parameters are separated by spaces.
      // The last parameter of the message may contain spaces, it is usually used
      // to transmit messages. In order to parse it correctly, if needs to be prefixed
      // with a colon, so the server knows to ignore all forthcoming spaces and has to treat
      // all remaining characters as a single parameter. If we detect any whitespace in the
      // last argument, prefix it with a colon:
      if ((i == arguments.size () - 1) && arguments.at (i).contains (QRegExp("\\s")))
        applyColon = true;
      line += QString (" %1%2").arg (applyColon ? ":" : "").arg (arguments.at (i));
    }
  sendLine (line);
}
