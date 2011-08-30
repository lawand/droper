/****************************************************************************
**
** Copyright 2011 Omar Lawand Dalatieh.
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
class Dropbox;
class OAuth;

//member functions
class QNetworkReply;
class QNetworkRequest;

namespace Ui {
    class AuthenticationDialog;
}

class AuthenticationDialog : public QDialog
{
    Q_OBJECT

//shared data members
public:
    QNetworkAccessManager *networkAccessManager;
    Dropbox *dropbox;
    OAuth *oAuth;

//member functions
public:
    explicit AuthenticationDialog(
        QNetworkAccessManager *networkAccessManager,
        Dropbox *dropbox,
        OAuth *oAuth,
        QWidget *parent = 0
        );
    ~AuthenticationDialog();
    void switchToSignIn();
    void switchToSignUp();
private slots:
    void on_buttonBox_accepted();
    void requestSignIn(QString email, QString password);
    void requestSignUp(
        QString email,
        QString first_name,
        QString last_name,
        QString password
        );
    void requestNetworkRequest(QNetworkRequest *networkRequest);
public slots:
    void handleNetworkReply(QNetworkReply *networkReply);
private slots:
    void handleSignIn(QNetworkReply *networkReply);
    void handleSignUp(QNetworkReply *networkReply);
    void clearLineEdits();

//private data members
private:
    Ui::AuthenticationDialog *ui;
    QWidget *tempPage;
};

#endif // AUTHENTICATIONDIALOG_H
