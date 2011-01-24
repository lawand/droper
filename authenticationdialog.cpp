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

//corresponding header file(s)
#include "authenticationdialog.h"
#include "ui_authenticationdialog.h"

//data member(s)
#include <QNetworkAccessManager>
#include "oauth.h"
#include "userdata.h"
#include "dropbox.h"

//implementation-specific data type(s)
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include "json.h"

AuthenticationDialog::AuthenticationDialog(
        QNetworkAccessManager* networkAccessManager,
        OAuth* oAuth,
        UserData* userData,
        Dropbox* dropbox,
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthenticationDialog)
{
    //member initialization
    this->networkAccessManager = networkAccessManager;
    this->oAuth = oAuth;
    this->userData = userData;
    this->dropbox = dropbox;

    //GUI initialization
    ui->setupUi(this);

    //initial connections
    connect(ui->okPushButton,
            SIGNAL(clicked()),
            SLOT(requestTokenAndSecret())
            );

    //some initial values for data members
    ui->pleaseWaitLabel->setVisible(false);
}

AuthenticationDialog::~AuthenticationDialog()
{
    delete ui;
}

void AuthenticationDialog::requestTokenAndSecret()
{
    ui->pleaseWaitLabel->setVisible(true);

    QString url = dropbox->apiToUrlString(Dropbox::TOKEN);

    QString query = oAuth->consumerKeyParameter() + "&" +
                    "email=" + ui->emailLineEdit->text() + "&" +
                    "password=" + ui->passwordLineEdit->text();

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    connect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleTokenAndSecret(QNetworkReply*))
            );
}

void AuthenticationDialog::handleTokenAndSecret(QNetworkReply* networkReply)
{
    disconnect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleTokenAndSecret(QNetworkReply*))
            );

    networkReply->deleteLater();

    ui->pleaseWaitLabel->setVisible(false);

    if(networkReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::information(this,
                                 "Error",
                                 "There was an error, try again later."
                                 );

        return;
    }

    QString jsonData = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = Json::parse(jsonData, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(this,
                                 "Error",
                                 "There was an error, try again later."
                                 );

        return;
    }

    userData->token = jsonResult["token"].toString();
    userData->secret = jsonResult["secret"].toString();
    userData->email = ui->emailLineEdit->text();

    accept();
}

void AuthenticationDialog::on_helpToolButton_clicked()
{
    QMessageBox::information(this,
                             "Help",
                             "You should enter your Dropbox account's email "
                             "and password. If you don't have a Dropbox "
                             "account, visit "
                             "<a href=\"http://www.dropbox.com/\">"
                             "http://www.dropbox.com/</a> to create one. <br>"
                             );
}
