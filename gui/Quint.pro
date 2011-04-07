#-------------------------------------------------
#
# Project created by QtCreator 2011-04-04T12:17:52
#
#-------------------------------------------------

QT       += core gui webkit
UI_DIR = ui-files
MOC_DIR = moc-files
OBJECTS_DIR = object-files
TARGET = Quint
TEMPLATE = app
DEFINES += HAVE_POSIX_OPENPT
INCLUDEPATH += src
DESTDIR = bin
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
        src/k3process.cpp \
        src/k3processcontroller.cpp \
        src/Session.cpp \
        src/ShellCommand.cpp \
        src/QTerminalWidget.cpp \
        src/TerminalMdiSubWindow.cpp \
        src/MainWindow.cpp \
        src/Quint.cpp

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
        src/k3process.h \
        src/k3processcontroller.h \
        src/Session.h \
        src/ShellCommand.h \
        src/QTerminalWidget.h \
        src/TerminalMdiSubWindow.h \
    	src/MainWindow.h

