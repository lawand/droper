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

// corresponding headers
#include "signinpage.h"
#include "ui_signinpage.h"

// member functions
#include <QNetworkReply>
#include "common.h"

// implementation-specific
#include <QDesktopServices>
#include <QMessageBox>
#include "dropbox.h"
#include "oauth.h"
#include "userdata.h"

SignInPage::SignInPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SignInPage)

{
    // private data members initialization
    ui->setupUi(this);
}

SignInPage::~SignInPage()
{
    delete ui;
}

void SignInPage::on_signInPushButton_clicked()
{
    requestOauthRequesttoken();
}

void SignInPage::on_doneSigningInPushButton_clicked()
{
    if(requestUserData.token.isEmpty() || requestUserData.secret.isEmpty())
    {
        QMessageBox::information(
                    this,
                    "Droper",
                    "You don't seem to have signed it yet. "
                    "Sign in and try again."
                    );
    }
    else
    {
        requestOauthAccesstoken();
    }
}

void SignInPage::requestOauthRequesttoken()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_REQUESTTOKEN);
    QNetworkRequest networkRequest(url);
    UserData userData;
    Common::oAuth->signRequestHeader("GET", &networkRequest, &userData);

    emit networkRequestGetNeeded(&networkRequest);
}

void SignInPage::requestOauthAccesstoken()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_ACCESSTOKEN);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest, &requestUserData);

    emit networkRequestGetNeeded(&networkRequest);
}

void SignInPage::handleNetworkReply(QNetworkReply *networkReply)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkReply->url());
    switch(api)
    {
    case Dropbox::OAUTH_REQUESTTOKEN:
        handleOauthRequesttoken(networkReply);
        break;

    case Dropbox::OAUTH_ACCESSTOKEN:
        handleOauthAccesstoken(networkReply);
        break;

    default:
        break;
    }
}

void SignInPage::handleOauthRequesttoken(QNetworkReply *networkReply)
{
    QString reply = networkReply->readAll();

    requestUserData.token = reply.split("&").at(1).split("=").at(1);
    requestUserData.secret = reply.split("&").at(0).split("=").at(1);

    openDropboxInABrowser();
}

void SignInPage::handleOauthAccesstoken(QNetworkReply *networkReply)
{
    QString reply = networkReply->readAll();

    // update user data
    Common::userData->token = reply.split("&").at(1).split("=").at(1);
    Common::userData->secret = reply.split("&").at(0).split("=").at(1);
    Common::userData->uid = reply.split("&").at(2).split("=").at(1);

    emit oauthAccesstokenHandled();
}

void SignInPage::openDropboxInABrowser()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::OAUTH_AUTHORIZE).toString();
    url.addQueryItem("oauth_token", requestUserData.token);

    QDesktopServices::openUrl(url);
}
