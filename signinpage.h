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

#ifndef SIGNINPAGE_H
#define SIGNINPAGE_H

// base class(es)
#include <QWidget>

// member functions
class QNetworkRequest;
class QNetworkReply;

// data members
#include "userdata.h"

namespace Ui {
class SignInPage;
}

class SignInPage : public QWidget
{
    Q_OBJECT

// member functions
public:
    explicit SignInPage(QWidget *parent = 0);
    ~SignInPage();    
private slots:
    // misc
    void openDropboxInABrowser();
    void copySignInUrl();
    void setupActions();
signals:
    // signals
    void networkRequestGetNeeded(QNetworkRequest *request);
    void oauthAccesstokenHandled();
public slots:
    // network slots
    void requestOauthRequesttoken();
    void requestOauthAccesstoken();
    void handleNetworkReply(QNetworkReply *networkReply);
    void handleOauthRequesttoken(QNetworkReply *networkReply);
    void handleOauthAccesstoken(QNetworkReply *networkReply);
private slots:
    // ui slots
    void on_signInPushButton_clicked();
    void on_donePushButton_clicked();
    void on_reportBugsPushButton_clicked();
    // action slots
    void on_copySignInUrlAction_triggered();

// private data members
private:
    Ui::SignInPage *ui;
    UserData requestUserData;
    bool shouldOnlyCopySignInUrl;
};

#endif // SIGNINPAGE_H
