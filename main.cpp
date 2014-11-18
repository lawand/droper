/****************************************************************************
**
** Copyright 2011, 2012, 2013, 2014 Omar Lawand Dalatieh.
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
#include <QSettings>
#include "mainwindow.h"
#include "common.h"
#include "userdata.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

#ifdef Q_WS_MAEMO_5
    // make it easier to find the config file in maemo
    application.setOrganizationName("droper");
    application.setApplicationName("droper0.5.2");
#else
    application.setOrganizationName("lawand");
    application.setApplicationName("droper0.5.2");
#endif

    // load user data
    QSettings settings;
    settings.beginGroup("user_data");
    if( (settings.childKeys().indexOf("access_token") == -1) ||
        (settings.childKeys().indexOf("access_token_secret") == -1) ||
        (settings.childKeys().indexOf("uid") == -1) )
    {
        settings.remove("user_data");
    }
    else
    {
        Common::userData->token = settings.value("access_token").toString();
        Common::userData->secret = settings.value(
            "access_token_secret"
            ).toString();
        Common::userData->uid = settings.value("uid").toString();
    }

    MainWindow mainWindow;

#if defined(Q_WS_S60)
    mainWindow.showMaximized();
#else
    mainWindow.show();
#endif

    int exitCode = application.exec();

    // save user data
    settings.setValue("access_token", Common::userData->token);
    settings.setValue("access_token_secret", Common::userData->secret);
    settings.setValue("uid", Common::userData->uid);

    return exitCode;
}
