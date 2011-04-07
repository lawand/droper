QT       += core gui network svg

VERSION = 0.2.2

TARGET = Droper
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    oauth.cpp \
    userdata.cpp \
    authenticationdialog.cpp \
    json.cpp \
    consumerdata.cpp \
    dropbox.cpp \
    downloaddialog.cpp \
    uploaddialog.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    oauth.h \
    userdata.h \
    authenticationdialog.h \
    json.h \
    consumerdata.h \
    dropbox.h \
    downloaddialog.h \
    uploaddialog.h \
    settingsdialog.h

FORMS    += mainwindow.ui \
    authenticationdialog.ui \
    downloaddialog.ui \
    uploaddialog.ui \
    settingsdialog.ui

RESOURCES += \
    main.qrc

RC_FILE = main.rc

symbian {
    TARGET.CAPABILITY = NetworkServices

    ICON = droper.svg

    #for kinetic scrolling
    include(kineticscroller/qtscroller.pri)

    TARGET.EPOCHEAPSIZE =  0x00020000 0x25600000
    TARGET.EPOCSTACKSIZE = 0x00014000
    }
