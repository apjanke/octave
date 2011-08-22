/*
    This file is part of Konsole, KDE's terminal emulator. 
    
    Copyright 2007-2008 by Robert Knight <robertknight@gmail.com>
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef PTY_H
#define PTY_H

// Qt
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QList>
#include <QtCore/QSize>

// KDE
#include "kptyprocess.h"

/**
 * The Pty class is used to start the terminal process, 
 * send data to it, receive data from it and manipulate 
 * various properties of the pseudo-teletype interface
 * used to communicate with the process.
 *
 * To use this class, construct an instance and connect
 * to the sendData slot and receivedData signal to
 * send data to or receive data from the process.
 *
 * To start the terminal process, call the start() method
 * with the program name and appropriate arguments. 
 */
class Pty:public KPtyProcess
{
Q_OBJECT public:

    /** 
     * Constructs a new Pty.
     * 
     * Connect to the sendData() slot and receivedData() signal to prepare
     * for sending and receiving data from the terminal process.
     *
     * To start the terminal process, call the run() method with the 
     * name of the program to start and appropriate arguments.
     */
  explicit Pty (QObject * parent = 0);

    /** 
     * Construct a process using an open pty master.
     * See KPtyProcess::KPtyProcess()
     */
  explicit Pty (int ptyMasterFd, QObject * parent = 0);

   ~Pty ();

    /**
     * Starts the terminal process.  
     *
     * Returns 0 if the process was started successfully or non-zero
     * otherwise.
     *
     * @param program Path to the program to start
     * @param arguments Arguments to pass to the program being started
     * @param environment A list of key=value pairs which will be added
     * to the environment for the new process.  At the very least this
     * should include an assignment for the TERM environment variable.
     */
  int start (const QString & program,
             const QStringList & arguments);

  public slots:
    /** 
     * Sends data to the process currently controlling the 
     * teletype ( whose id is returned by foregroundProcessGroup() )
     *
     */
  void sendData (const QByteArray& data);

    signals:
    /**
     * Emitted when a new block of data is received from
     * the teletype.
     *
     */
  void receivedData (const QByteArray& data);

protected:
  void setupChildProcess ();

private slots:
  // called when data is received from the terminal process
  void dataReceived ();

private:
  void init ();
  bool _xonXoff;
};

#endif // PTY_H
