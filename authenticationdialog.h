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

#ifndef AUTHENTICATIONDIALOG_H
#define AUTHENTICATIONDIALOG_H

//base class
#include <QDialog>

//data members
class QNetworkAccessManager;
class OAuth;
class UserData;
class Dropbox;

//member functions
class QNetworkReply;

namespace Ui {
    class AuthenticationDialog;
}

class AuthenticationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthenticationDialog(QNetworkAccessManager* networkAccessManager,
                                  OAuth* oAuth,
                                  UserData* userData,
                                  Dropbox* dropbox,
                                  QWidget *parent = 0);
    ~AuthenticationDialog();

public: //shared objects
    QNetworkAccessManager* networkAccessManager;
    OAuth* oAuth;
    UserData* userData;
    Dropbox* dropbox;

private:
    Ui::AuthenticationDialog *ui;

private slots:
    void requestTokenAndSecret();
    void handleTokenAndSecret(QNetworkReply* networkReply);
};

#endif // AUTHENTICATIONDIALOG_H
