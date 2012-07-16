include(qt-json/qt-json.pri)
include(qtscroller/qtscroller.pri)

QT += core gui network

TARGET = Droper
TEMPLATE = app
VERSION = 0.5.0


SOURCES += main.cpp\
    mainwindow.cpp \
    dropbox.cpp \
    oauth.cpp \
    userdata.cpp \
    consumerdata.cpp \
    util.cpp \
    signinpage.cpp \
    loadingpage.cpp \
    common.cpp \
    navigationpage.cpp \
    accountinfopage.cpp \
    filetransferspage.cpp \
    customwidgets.cpp

HEADERS += mainwindow.h \
    dropbox.h \
    oauth.h \
    userdata.h \
    consumerdata.h \
    util.h \
    signinpage.h \
    loadingpage.h \
    common.h \
    navigationpage.h \
    accountinfopage.h \
    filetransferspage.h \
    customwidgets.h

FORMS += mainwindow.ui \
    signinpage.ui \
    loadingpage.ui \
    navigationpage.ui \
    accountinfopage.ui \
    filetransferspage.ui

RESOURCES += \
    main.qrc

symbian {
    TARGET.UID3 = 0xA89FD852
    TARGET.CAPABILITY = NetworkServices
    ICON = droper.svg

    # platform dependencies
        # remove default platform dependencies
        default_deployment.pkg_prerules -= pkg_platform_dependencies

        # setup custom platform dependencies
        pkg_custom_platform_dependencies = \
        "[0x102032BE],0,0,0,{\"S60ProductID\"}" \    # S60 3rd Edition FP1
        "[0x102752AE],0,0,0,{\"S60ProductID\"}" \    # S60 3rd Edition FP2
        "[0x1028315F],0,0,0,{\"S60ProductID\"}" \    # Symbian^1
        "[0x20022E6D],0,0,0,{\"S60ProductID\"}"      # Symbian^3

        # add custom platform dependencies
        custom_platform_dependencies_deployment.pkg_prerules += pkg_custom_platform_dependencies
        DEPLOYMENT += custom_platform_dependencies_deployment
    }
