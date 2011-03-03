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
    filetransferdialog.cpp

HEADERS  += mainwindow.h \
    oauth.h \
    userdata.h \
    authenticationdialog.h \
    json.h \
    consumerdata.h \
    dropbox.h \
    filetransferdialog.h

FORMS    += mainwindow.ui \
    authenticationdialog.ui \
    filetransferdialog.ui

RESOURCES += \
    main.qrc

RC_FILE = main.rc

symbian {
    TARGET.CAPABILITY = NetworkServices

    TARGET.UID3 = 0x2003AD26

    ICON = droper.svg

    vendorinfo = \
    "%{\"Lawand\"}" \
    ":\"Lawand\""
    my_deployment.pkg_prerules = vendorinfo
    DEPLOYMENT += my_deployment

    DEPLOYMENT.installer_header=0x2002CCCF
    }
