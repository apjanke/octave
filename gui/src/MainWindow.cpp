/* Quint - A graphical user interface for Octave
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

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QtWebKit/QWebView>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      isRunning(true) {
    resize(1000, 600);
    constructWindow();
    establishOctaveLink();
}

MainWindow::~MainWindow() {
}

void MainWindow::constructWindow() {
    m_octaveTerminal = new OctaveTerminal(this);
    m_variablesDockWidget = new VariablesDockWidget(this);
    m_historyDockWidget = new HistoryDockWidget(this);
    setWindowTitle("Quint");
    setCentralWidget(m_octaveTerminal);

    addDockWidget(Qt::LeftDockWidgetArea, m_variablesDockWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_historyDockWidget);
}

void MainWindow::establishOctaveLink() {
    //QMetaObject::invokeMethod(this, "setStatus", Q_ARG(QString, QString("Establishing Octave link..")));
    m_octaveLink = new OctaveLink();
    pthread_create(&m_octaveThread, 0, MainWindow::octaveMainWrapper, this);
    pthread_create(&m_octaveCallbackThread, 0, MainWindow::octaveCallback, this);
    command_editor::add_event_hook(server_rl_event_hook_function);

    int fdm, fds;
    if(openpty(&fdm, &fds, 0, 0, 0) < 0) {
        assert(0);
    }
    dup2 (fds, 0);
    dup2 (fds, 1);
    dup2 (fds, 2);
    m_octaveTerminal->openTeletype(fdm);
}

void* MainWindow::octaveMainWrapper(void *widget) {
    //MainWindow *mainWindow = (MainWindow*)ptr;

    int argc = 3;
    const char* argv[] = {"octave", "--interactive", "--line-editing"};
    octave_main(argc, (char**)argv,1);
    switch_to_buffer(create_buffer(get_input_from_stdin()));
    main_loop();
    clean_up_and_exit(0);
    return 0;
}

void* MainWindow::octaveCallback(void *widget) {
    MainWindow *mainWindow = (MainWindow*)widget;

    while(mainWindow->isRunning) {

    // Get a full variable list.
    std::vector<OctaveLink::VariableMetaData> variables = oct_octave_server.variableInfoList();
    if(variables.size()) {
        // TODO: Update variables view.
    }

    // Check whether any requested variables have been returned.
    std::vector<OctaveLink::RequestedVariable> reqVars = oct_octave_server.requestedVariables();
    for(std::vector<OctaveLink::RequestedVariable>::iterator it = reqVars.begin();
        it != reqVars.end(); it++ ) {
        // TODO: Process requested variables.
    }

    // Collect history list.
    string_vector historyList = oct_octave_server.getHistoryList();
    if(historyList.length()) {
        mainWindow->m_historyDockWidget->updateHistory(historyList);
    }

    // Put a marker in each buffer at the proper location.
    int status = 0;
    std::vector<OctaveLink::BreakPoint> breakPoints = oct_octave_server.breakPointList(status);
    if(status==0) {
        //MEditor::GetInstance()->process_breakpoint_list (bps);
    }

    // Find out if a breakpoint is hit
    static bool lineNumber = -1;
    bool hitBreakPoint = oct_octave_server.isBreakpointReached(status);
    if((status==0) && hitBreakPoint) {
        std::vector<OctaveLink::BreakPoint> hit_breakpoint = oct_octave_server.reachedBreakpoint();

        if(hit_breakpoint.size() > 0 && (hit_breakpoint[0].lineNumber != lineNumber)) {
            //MEditor::GetInstance()->remove_hit_breakpoint_marker ();
            //MEditor::GetInstance()->add_breakpoint_marker(hit_breakpoint[0], BP_MARKER_TYPE_HIT);
            lineNumber = hit_breakpoint[0].lineNumber;
        }
    }
    else if((status==0) && lineNumber>0) {
        //MEditor::GetInstance()->remove_hit_breakpoint_marker ();
        lineNumber = -1;
    }

        usleep(100000);
    }

    return 0;
}
