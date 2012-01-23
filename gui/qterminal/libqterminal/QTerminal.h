/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)
    
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
						    

#ifndef Q_TERMINAL
#define Q_TERMINAL

#include <QtGui>
#include "Session.h"
#include "TerminalDisplay.h"

using namespace Konsole;

class QTerminal : public QWidget
{
    Q_OBJECT
public:
    QTerminal(QWidget *parent = 0);
    ~QTerminal();

    void startShellProgram();
    
    void setTerminalFont(QFont &font); 

    void setEnvironment(const QStringList& environment);

    void setShellProgram(const QString &progname);

    void setWorkingDirectory(const QString& dir);

    void setArgs(QStringList &args);

    void setTextCodec(QTextCodec *codec);

    void setSize(int h, int v);

    void setHistorySize(int lines);

    void setFlowControlEnabled(bool enabled);

    bool flowControlEnabled(void);

    /**
     * Sets whether the flow control warning box should be shown
     * when the flow control stop key (Ctrl+S) is pressed.
     */
    void setFlowControlWarningEnabled(bool enabled);


    void setReadOnly(bool);
            
signals:
    void finished();

public slots:
    void copyClipboard();
    void pasteClipboard();
        
protected: 
    virtual void resizeEvent(QResizeEvent *);
    void *getTerminalDisplay();
    
protected slots:
    void sessionFinished();        
    
private:
    void init();
    TerminalDisplay *m_terminalDisplay;
    Session *m_session;
};

#endif // Q_TERMINAL
