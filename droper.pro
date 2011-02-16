QT       += core gui network script svg

TARGET = droper
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    oauth.cpp \
    userdata.cpp \
    authenticationdialog.cpp \
    json.cpp \
    consumerdata.cpp \
    dropbox.cpp

HEADERS  += mainwindow.h \
    oauth.h \
    userdata.h \
    authenticationdialog.h \
    json.h \
    consumerdata.h \
    dropbox.h

FORMS    += mainwindow.ui \
    authenticationdialog.ui

RESOURCES += \
    main.qrc

RC_FILE = main.rc
