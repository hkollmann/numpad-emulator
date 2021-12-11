
TEMPLATE = app
TARGET = Numpad_1.7.1
DEPENDPATH += . Buttons
INCLUDEPATH += . Buttons


QT += core
QT += gui
QT += widgets


# Input
HEADERS += AboutWindow.h \
           Numpad.h \
           NumpadManager.h \
           SettingsDialog.h \
           Buttons/AbstractBigButton.h \
           Buttons/BigHorizontalButton.h \
           Buttons/BigVerticalButton.h \
           Buttons/Button.h \
           KeyboardHookMsg.h \
    msgbox.h \    
    ShowHideKey.h \
    helpwindow.h \
    dndnumpad.h \
    Buttons/confbutton.h \
    btninfo.h \
    Buttons/sourcebutton.h \
    allbtnwidget.h

           
SOURCES += AboutWindow.cpp \
           main.cpp \
           Numpad.cpp \
           NumpadManager.cpp \
           SettingsDialog.cpp \
           Buttons/AbstractBigButton.cpp \
           Buttons/BigHorizontalButton.cpp \
           Buttons/BigVerticalButton.cpp \
           Buttons/Button.cpp \          
    msgbox.cpp \   
    helpwindow.cpp \
    dndnumpad.cpp \
    Buttons/confbutton.cpp \
    Buttons/sourcebutton.cpp \
    allbtnwidget.cpp


RESOURCES += resourse.qrc
RC_FILE = numpad.rc


