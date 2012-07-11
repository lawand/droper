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
#include "accountinfopage.h"
#include "ui_accountinfopage.h"

// member functions
#include <QNetworkReply>

// implementation-specific
#include <QNetworkRequest>
#include <QMessageBox>
#include <QClipboard>
#include <QMenu>
#include "common.h"
#include "dropbox.h"
#include "oauth.h"
#include "util.h"
#include "json.h"

AccountInfoPage::AccountInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AccountInfoPage)
{
    // private data members initialization
    ui->setupUi(this);

    // actions setup
    setupActions();
}

AccountInfoPage::~AccountInfoPage()
{
    delete ui;
}

void AccountInfoPage::setupActions()
{
    this->addAction(ui->referralLinkAction);
    this->addAction(ui->signOutAction);
    this->addAction(ui->backAction);
}

void AccountInfoPage::requestAccountInfo()
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::ACCOUNT_INFO);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    emit networkRequestGetNeeded(&networkRequest);
}

void AccountInfoPage::handleNetworkReply(QNetworkReply *networkReply)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkReply->url());
    switch(api)
    {
    case Dropbox::ACCOUNT_INFO:
        handleAccountInfo(networkReply);
        break;

    default:
        break;
    }
}

void AccountInfoPage::handleAccountInfo(QNetworkReply *networkReply)
{
    QString jsonData = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(jsonData, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
            this,
            "Droper",
            "JSON parsing failed."
            );

        return;
    }

    QVariantMap quotaInfo = jsonResult["quota_info"].toMap();
    qreal normalFiles = quotaInfo["normal"].toReal();
    qreal sharedFiles = quotaInfo["shared"].toReal();
    qreal used = quotaInfo["normal"].toReal() + quotaInfo["shared"].toReal();
    qreal quota = quotaInfo["quota"].toReal();

    ui->emailLabel->setText(jsonResult["email"].toString());

    ui->spaceProgressBar->setValue(
        (quotaInfo["normal"].toReal()+quotaInfo["shared"].toReal())
        *
        100
        /
        quotaInfo["quota"].toReal()
        );

    QString space = QString("%1 out of %2")
        .arg(Util::bytesToString(used))
        .arg(Util::bytesToString(quota));
    ui->spaceProgressBar->setFormat(space);

    QString spaceDetails = QString(
        "Normal Files: %1 / Shared Files: %2"
        )
            .arg(Util::bytesToString(normalFiles))
            .arg(Util::bytesToString(sharedFiles));
    ui->spaceDetailsLabel->setText(spaceDetails);

    referralLink = jsonResult["referral_link"].toString();
}

void AccountInfoPage::on_referralLinkAction_triggered()
{
    if(referralLink.isEmpty())
    {
        QMessageBox::information(
            this,
            "Droper",
            "Referral link doesn't seem to have been retrived."
            );
        return;
    }

    QApplication::clipboard()->setText(referralLink);

    QMessageBox::information(
        this,
        "Droper",
        QString("The referral link %1 was copied to clipboard.").arg(
            referralLink
            )
        );
}

void AccountInfoPage::on_backAction_triggered()
{
    emit backRequested();
}

void AccountInfoPage::on_signOutAction_triggered()
{
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        "Droper",
        "Are you sure you want to sign out?",
        QMessageBox::Yes|QMessageBox::No,
        QMessageBox::No
        );
    if(result != QMessageBox::Yes)
    {
        return;
    }

    emit signOutRequested();
}
