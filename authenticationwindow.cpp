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

//corresponding headers
#include "authenticationwindow.h"
#include "ui_authenticationwindow.h"

//data members
#include <QNetworkAccessManager>
#include <QSettings>
#include "oauth.h"
#include "userdata.h"
#include "dropbox.h"

//implementation-specific
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDesktopWidget>
#include "json.h"

AuthenticationWindow::AuthenticationWindow(
    QNetworkAccessManager* networkAccessManager,
    OAuth* oAuth,
    UserData* userData,
    Dropbox* dropbox,
    QSettings* settings,
    QWidget *parent
    ) :
    QDialog(parent),
    ui(new Ui::AuthenticationWindow)
{
    //member initialization
    this->networkAccessManager = networkAccessManager;
    this->oAuth = oAuth;
    this->userData = userData;
    this->dropbox = dropbox;
    this->settings = settings;

    //GUI initialization
    ui->setupUi(this);
#ifndef Q_OS_SYMBIAN
    //draw at screen's center
    move(
        QApplication::desktop()->availableGeometry().center() -
        this->rect().center()
        );
#endif

    //initial connections
    connect(
        ui->okPushButton,
        SIGNAL(clicked()),
        SLOT(requestTokenAndSecret())
        );
    connect(
        ui->cancelPushButton,
        SIGNAL(clicked()),
        SLOT(close())
        );
}

AuthenticationWindow::~AuthenticationWindow()
{
    delete ui;
}

void AuthenticationWindow::requestTokenAndSecret()
{
    ui->infoLabel->setText("Please Wait...");

    QUrl url = dropbox->apiToUrl(Dropbox::TOKEN);
    url.addQueryItem("email", ui->emailLineEdit->text());
    url.addQueryItem("password", ui->passwordLineEdit->text());

    QNetworkRequest networkRequest(url);

    oAuth->addConsumerKeyQueryItem(&networkRequest);

    networkAccessManager->get( networkRequest );

    connect(
        this->networkAccessManager,
        SIGNAL(finished(QNetworkReply*)),
        this,
        SLOT(handleTokenAndSecret(QNetworkReply*))
        );
}

void AuthenticationWindow::handleTokenAndSecret(QNetworkReply* networkReply)
{
    disconnect(
        this->networkAccessManager,
        SIGNAL(finished(QNetworkReply*)),
        this,
        SLOT(handleTokenAndSecret(QNetworkReply*))
        );

    networkReply->deleteLater();

    if(networkReply->error() != QNetworkReply::NoError)
    {
        if(networkReply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            QMessageBox::critical(
                this,
                "Droper",
                "The provided user information is not valid."
                );
        }
        else
        {
            QMessageBox::information(
                this,
                "Droper",
                "There was an error, try again later."
                );
        }

        ui->infoLabel->setText("Enter Your Dropbox Credentials.");

        return;
    }

    QString jsonData = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = Json::parse(jsonData, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
            this,
            "Droper",
            "There was an error, try again later."
            );

        return;
    }

    userData->token = jsonResult["token"].toString();
    userData->secret = jsonResult["secret"].toString();
    userData->email = ui->emailLineEdit->text().toLower();

    settings->setValue("user/token", userData->token);
    settings->setValue("user/secret", userData->secret);
    settings->setValue("user/email", userData->email);

    emit done();

    hide();
}
