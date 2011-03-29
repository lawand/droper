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
#include "authenticationdialog.h"
#include "ui_authenticationdialog.h"

//data members
#include <QNetworkAccessManager>
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

AuthenticationDialog::AuthenticationDialog(
        QNetworkAccessManager* networkAccessManager,
        OAuth* oAuth,
        UserData* userData,
        Dropbox* dropbox,
        QWidget *parent
        ) :
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
    //draw at screen's center
    move(
            QApplication::desktop()->availableGeometry().center() -
            this->rect().center()
            );

    //initial connections
    connect(
            ui->buttonBox,
            SIGNAL(accepted()),
            SLOT(requestTokenAndSecret())
            );
    connect(
            ui->buttonBox,
            SIGNAL(rejected()),
            SLOT(reject())
            );

    //some initial values for data members
    ui->infoLabel->setVisible(false);
}

AuthenticationDialog::~AuthenticationDialog()
{
    delete ui;
}

void AuthenticationDialog::requestTokenAndSecret()
{
    ui->infoLabel->setText("Please Wait...");
    ui->infoLabel->setVisible(true);

    QUrl url = dropbox->apiToUrl(Dropbox::TOKEN);

    QPair<QString,QString> temp;

    temp = oAuth->consumerKeyQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("email"), ui->emailLineEdit->text());
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("password"), ui->passwordLineEdit->text());
    url.addQueryItem(temp.first, temp.second);

    networkAccessManager->get( QNetworkRequest( url ) );

    connect(
            this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleTokenAndSecret(QNetworkReply*))
            );
}

void AuthenticationDialog::handleTokenAndSecret(QNetworkReply* networkReply)
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

        ui->infoLabel->setVisible(false);

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

    accept();
}
