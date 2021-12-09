
TEMPLATE = lib
TARGET = KeyboardHook
DEPENDPATH += .
INCLUDEPATH += .
DEFINES += SHARED_LIB


# Input
HEADERS += KeyboardHook.h \
                  KeyboardHookMsg.h
SOURCES += KeyboardHook.cpp
CONFIG -= qt
