/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPROCESS_P_H
#define KPROCESS_P_H

#include "kprocess.h"

class KProcessPrivate {
    Q_DECLARE_PUBLIC(KProcess)
protected:
    KProcessPrivate() :
        openMode(QIODevice::ReadWrite)
    {
    }
    void writeAll(const QByteArray &buf, int fd);
    void forwardStd(KProcess::ProcessChannel good, int fd);
    void _k_forwardStdout();
    void _k_forwardStderr();

    QString prog;
    QStringList args;
    KProcess::OutputChannelMode outputChannelMode;
    QIODevice::OpenMode openMode;

    KProcess *q_ptr;
};


#endif
