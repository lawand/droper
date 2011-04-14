/****************************************************************************
**
** Copyright 2011 Omar Lawand Dalatieh.
** Contact: see the README file.
**
** This file is part of Droper.
**
** Droper is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Droper is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Droper.  If not, see <http://www.gnu.org/licenses/>.
**
** For more information, check out the GNU General Public license found
** in the COPYING file and the README file.
**
****************************************************************************/

#include <QApplication>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QMessageBox>
#include <QSslSocket>
#include "authenticationwindow.h"
#include "consumerdata.h"
#include "dropbox.h"
#include "mainwindow.h"
#include "oauth.h"
#include "userdata.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    //check for OpenSSL
    if( !QSslSocket::supportsSsl() )
    {
        QMessageBox::critical(
                0,
                "Droper",
                "OpenSSL not detected, install it and "
                "try again."
                );

        return -1;
    }

#ifdef Q_OS_SYMBIAN
    //check for Qt 4.6.x
    if( !QString(qVersion()).startsWith("4.6") )
    {
        QMessageBox::critical(
                0,
                "Droper",
                QString(
                        "This device contains Qt %1, which is unsupported. \n"
                        "It's highly recommended that you install one of "
                        "Qt 4.6.x since they are the only supported versions."
                        ).arg(qVersion())
                );
    }
#endif

    QNetworkAccessManager networkAccessManager;
    ConsumerData consumerData;
    OAuth oAuth(&consumerData);
    UserData userData("", "", "");
    Dropbox dropbox;

    QSettings settings("lawand", "droper");

    AuthenticationWindow authenticationWindow(
            &networkAccessManager,
            &oAuth,
            &userData,
            &dropbox,
            &settings
            );

    MainWindow mainWindow(
            &networkAccessManager,
            &oAuth,
            &userData,
            &dropbox,
            &settings
            );

    if(settings.value("user/token").isNull() ||
       settings.value("user/secret").isNull() ||
       settings.value("user/email").isNull())
    {
#ifdef Q_OS_SYMBIAN
        authenticationWindow.showFullScreen();
#else
        authenticationWindow.show();
#endif

        QObject::connect(
                &authenticationWindow,
                SIGNAL(done()),
                &mainWindow,
                SLOT(setup())
                );
    }
    else
    {
        mainWindow.setup();
    }

    return application.exec();
}
