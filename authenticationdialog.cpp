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

//corresponding headers
#include "authenticationdialog.h"
#include "ui_authenticationdialog.h"

//data members
#include <QNetworkAccessManager>
#include "dropbox.h"
#include "oauth.h"

//member functions
#include <QNetworkReply>
#include <QNetworkRequest>

//implementation-specific
#include <QMessageBox>
#include <QNetworkRequest>
#include <QSettings>
#include <QMovie>
#include "json.h"

AuthenticationDialog::AuthenticationDialog(
    QNetworkAccessManager *networkAccessManager,
    Dropbox *dropbox,
    OAuth *oAuth,
    QWidget *parent
    ) :
    QDialog(parent),
    ui(new Ui::AuthenticationDialog)
{
    //shared data members initialization
    this->networkAccessManager = networkAccessManager;
    this->dropbox = dropbox;
    this->oAuth = oAuth;

    //private data members initialization
    ui->setupUi(this);

    //QObject connections
    connect(this, SIGNAL(accepted()), SLOT(clearLineEdits()));
    connect(this, SIGNAL(rejected()), SLOT(clearLineEdits()));

    //QMovie initialization
    QMovie *loading = new QMovie(
                ":/resources/animations/loading.gif",
                QByteArray(),
                this
                );
    ui->loadingLabel->setMovie(loading);
    loading->start();
}

AuthenticationDialog::~AuthenticationDialog()
{
    delete ui;
}

void AuthenticationDialog::switchToSignIn()
{
    ui->stackedWidget->setCurrentWidget(ui->signInPage);
}

void AuthenticationDialog::switchToSignUp()
{
    ui->stackedWidget->setCurrentWidget(ui->signUpPage);
}

void AuthenticationDialog::on_buttonBox_accepted()
{
    if(ui->stackedWidget->currentWidget() == ui->signInPage)
    {
        requestSignIn(
            ui->signInEmailLineEdit->text(),
            ui->signInPasswordLineEdit->text()
            );

    }
    else if(ui->stackedWidget->currentWidget() == ui->signUpPage)
    {
        if(ui->signUpPasswordLineEdit->text()
                != ui->signUpRetypedPasswordLineEdit->text())
        {
            QMessageBox::critical(
                this,
                "Droper",
                "Error: Password mismatch."
                );

            return;
        }

        if(ui->signUpPasswordLineEdit->text().isEmpty())
        {
            QMessageBox::critical(
                this,
                "Droper",
                "Error: The password field can't be empty."
                );

            return;
        }

        requestSignUp(
            ui->signUpEmailLineEdit->text(),
            ui->signUpFirstNameLineEdit->text(),
            ui->signUpLastNameLineEdit->text(),
            ui->signUpPasswordLineEdit->text()
            );
    }
}

void AuthenticationDialog::requestSignIn(QString email, QString password)
{
    QUrl url = dropbox->apiToUrl(Dropbox::TOKEN);
    url.addQueryItem("email", email);
    url.addQueryItem("password", password);

    QNetworkRequest networkRequest(url);

    oAuth->addConsumerKeyQueryItem(&networkRequest);

    requestNetworkRequest( &networkRequest );
}

void AuthenticationDialog::requestSignUp(
    QString email,
    QString first_name,
    QString last_name,
    QString password
    )
{
    QUrl url = dropbox->apiToUrl(Dropbox::ACCOUNT);
    url.addQueryItem("email", email);
    url.addQueryItem("first_name", first_name);
    url.addQueryItem("last_name", last_name);
    url.addQueryItem("password", password);

    QNetworkRequest networkRequest(url);

    oAuth->addConsumerKeyQueryItem(&networkRequest);

    requestNetworkRequest( &networkRequest );
}

void AuthenticationDialog::requestNetworkRequest(
    QNetworkRequest *networkRequest
    )
{
    networkAccessManager->get(*networkRequest);

    //show the loading animation
    tempPage = ui->stackedWidget->currentWidget();
    ui->stackedWidget->setCurrentWidget(ui->loadingPage);
    ui->buttonBox->setEnabled(false);
}

void AuthenticationDialog::handleNetworkReply(QNetworkReply *networkReply)
{
    //stop the loading animation
    ui->stackedWidget->setCurrentWidget(tempPage);
    ui->buttonBox->setEnabled(true);

    if(networkReply->error() != QNetworkReply::NoError)
        return;

    Dropbox::Api api = dropbox->urlToApi(networkReply->url());
    switch(api)
    {
        case Dropbox::TOKEN:
            handleSignIn(networkReply);
            break;

        case Dropbox::ACCOUNT:
            handleSignUp(networkReply);
            break;

        default:
            break;
    }
}

void AuthenticationDialog::handleSignIn(QNetworkReply *networkReply)
{
    QString jsonData = networkReply->readAll();

    QVariantMap jsonResult = Json::parse(jsonData).toMap();

    QSettings settings;

    settings.setValue("user/token", jsonResult["token"].toString());
    settings.setValue("user/secret", jsonResult["secret"].toString());

    accept();
}

void AuthenticationDialog::handleSignUp(QNetworkReply *networkReply)
{
    QString data = networkReply->readAll();

    //The string "OK" indicates that the new account was created successfully
    if(data != "OK")
    {
        QMessageBox::critical(
            this,
            "Droper",
            "Error: Account creation failed."
            );

        return;
    }
    else
    {
        QMessageBox::information(
            this,
            "Droper",
            "The new account was succefully created. You will now be signed in."
            );

        //at this point, signing in is possible

        ui->stackedWidget->setCurrentWidget(ui->signInPage);

        ui->signInEmailLineEdit->setText( ui->signUpEmailLineEdit->text() );
        ui->signInPasswordLineEdit->setText( ui->signUpPasswordLineEdit->text() );

        requestSignIn(
            ui->signInEmailLineEdit->text(),
            ui->signInPasswordLineEdit->text()
            );
    }
}

void AuthenticationDialog::clearLineEdits()
{
    ui->signInEmailLineEdit->clear();
    ui->signInPasswordLineEdit->clear();
    ui->signUpFirstNameLineEdit->clear();
    ui->signUpLastNameLineEdit->clear();
    ui->signUpEmailLineEdit->clear();
    ui->signUpPasswordLineEdit->clear();
}
