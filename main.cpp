/****************************************************************************
**
** Copyright 2011, 2012 Omar Lawand Dalatieh.
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
#include "consumerdata.h"
#include "dropbox.h"
#include "mainwindow.h"
#include "oauth.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setOrganizationName("lawand");
    application.setApplicationName("droper0.4.5");

//shared objects
    QNetworkAccessManager networkAccessManager;
    ConsumerData consumerData;
    Dropbox dropbox(1);
    OAuth oAuth(&consumerData);

    MainWindow mainWindow(
        &networkAccessManager,
        &dropbox,
        &oAuth
        );

    mainWindow.showMaximized();

    return application.exec();
}
