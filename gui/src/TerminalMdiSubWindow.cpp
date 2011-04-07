#include "TerminalMdiSubWindow.h"

TerminalMdiSubWindow::TerminalMdiSubWindow()
    : QMdiSubWindow(),
      m_terminalWidget(0) {
    setWindowTitle("Terminal Session");
    resize(800, 400);
    launchTerminal();
}

void TerminalMdiSubWindow::launchTerminal() {
    delete m_terminalWidget;
    m_terminalWidget = new QTerminalWidget(0, this);
    m_terminalWidget->setScrollBarPosition(QTerminalWidget::ScrollBarRight);
    setWidget(m_terminalWidget);

    QString programName = "octave";
    m_terminalWidget->setShellProgram(programName);
    m_terminalWidget->startShellProgram();
    setFocus();

    connect(m_terminalWidget, SIGNAL(finished()), this, SLOT(launchTerminal()));
}
