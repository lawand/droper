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
#include "authenticationdialog.h"
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

    QNetworkAccessManager networkAccessManager;
    ConsumerData consumerData;
    OAuth oAuth(&consumerData);
    UserData userData("", "", "");
    Dropbox dropbox;

    QSettings settings("lawand", "droper");

    if(settings.value("user/token").isNull() ||
       settings.value("user/secret").isNull() ||
       settings.value("user/email").isNull())
    {
        AuthenticationDialog authenticationDialog(
                &networkAccessManager,
                &oAuth,
                &userData,
                &dropbox
                );

        if(authenticationDialog.exec() != QDialog::Accepted)
        {
            return 0;
        }
        else
        {
            settings.setValue("user/token", userData.token);
            settings.setValue("user/secret", userData.secret);
            settings.setValue("user/email", userData.email);
        }
    }
    else
    {
        userData.token = settings.value("user/token").toString();
        userData.secret = settings.value("user/secret").toString();
        userData.email = settings.value("user/email").toString();
    }

    MainWindow mainWindow(
            &networkAccessManager,
            &oAuth,
            &userData,
            &dropbox,
            &settings
            );
    mainWindow.show();

    return application.exec();
}
