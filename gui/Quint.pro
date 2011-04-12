#-------------------------------------------------
#
# Project created by QtCreator 2011-04-04T12:17:52
#
#-------------------------------------------------

QT       += core gui webkit xml
UI_DIR = ui-files
MOC_DIR = moc-files
OBJECTS_DIR = object-files
TARGET = Quint
TEMPLATE = app
DEFINES += HAVE_POSIX_OPENPT
INCLUDEPATH += src qcodeedit-2.2.3
DESTDIR = ../Quint/bin
SOURCES +=\
        src/TerminalCharacterDecoder.cpp \
        src/KeyboardTranslator.cpp \
        src/Screen.cpp \
        src/History.cpp \
        src/BlockArray.cpp \
        src/konsole_wcwidth.cpp \
        src/ScreenWindow.cpp \
        src/Emulation.cpp \
        src/Vt102Emulation.cpp \
        src/TerminalDisplay.cpp \
        src/Filter.cpp \
        src/Pty.cpp \
        src/kpty.cpp \
        src/kptyprocess.cpp \
        src/kprocess.cpp \
        src/kptydevice.cpp \
        src/k3process.cpp \
        src/k3processcontroller.cpp \
        src/Session.cpp \
        src/ShellCommand.cpp \
        src/QTerminalWidget.cpp \
        src/MainWindow.cpp \
        src/Quint.cpp \
        src/OctaveLink.cpp \
        src/ProcessInfo.cpp \
    src/OctaveTerminal.cpp \
    src/VariablesDockWidget.cpp \
    src/HistoryDockWidget.cpp \
    src/FilesDockWidget.cpp \
    src/CodeEdit.cpp \
    src/FileEditorMdiSubWindow.cpp \
    src/SyntaxHighlighter.cpp

HEADERS += \
        src/TerminalCharacterDecoder.h \
        src/Character.h \
        src/CharacterColor.h \
        src/KeyboardTranslator.h \
        src/ExtendedDefaultTranslator.h \
        src/Screen.h \
        src/History.h \
        src/BlockArray.h \
        src/konsole_wcwidth.h \
        src/ScreenWindow.h \
        src/Emulation.h \
        src/Vt102Emulation.h \
        src/TerminalDisplay.h \
        src/Filter.h \
        src/LineFont.h \
        src/Pty.h \
        src/kpty.h \
        src/kpty_p.h \
        src/kptyprocess.h \
        src/kprocess.h \
        src/kprocess_p.h \
        src/kptydevice.h \
        src/k3process.h \
        src/k3processcontroller.h \
        src/Session.h \
        src/ShellCommand.h \
        src/QTerminalWidget.h \
    	src/MainWindow.h \
        src/OctaveLink.h \
        src/konsole_export.h \
        src/ProcessInfo.h \
        src/kpty_export.h \
        src/kdecore_export.h \
    src/OctaveTerminal.h \
    src/VariablesDockWidget.h \
    src/HistoryDockWidget.h \
    src/FilesDockWidget.h \
    src/CodeEdit.h \
    src/FileEditorMdiSubWindow.h \
    src/SyntaxHighlighter.h

INCFLAGS = -g3 $$system(mkoctfile -p INCFLAGS)
LFLAGS = $$system(mkoctfile -p LFLAGS) \
         $$system(mkoctfile -p OCTAVE_LIBS) \
         $$system(mkoctfile -p LIBS)
LIBS    += $$LFLAGS -loctave -loctinterp -lreadline -lutil
QMAKE_CXXFLAGS  += $$INCFLAGS
