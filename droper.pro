include(json/json.pri)
include(QsKineticScroller/QsKineticScroller.pri)

QT += core gui network

TARGET = Droper
TEMPLATE = app
VERSION = 0.4.0


SOURCES += main.cpp\
    mainwindow.cpp \
    authenticationdialog.cpp \
    dropbox.cpp \
    oauth.cpp \
    userdata.cpp \
    consumerdata.cpp \
    downloaddialog.cpp \
    uploaddialog.cpp

HEADERS += mainwindow.h \
    authenticationdialog.h \
    dropbox.h \
    oauth.h \
    userdata.h \
    consumerdata.h \
    downloaddialog.h \
    uploaddialog.h

FORMS += mainwindow.ui \
    authenticationdialog.ui \
    downloaddialog.ui \
    uploaddialog.ui

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

    # allow uploads of large files
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
    }
