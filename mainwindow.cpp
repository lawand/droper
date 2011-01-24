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

//data member(s)
#include <QNetworkAccessManager>
#include "oauth.h"
#include "userdata.h"
#include "dropbox.h"

//implementation-specific data type(s)
#include <QNetworkReply>
#include <QDateTime>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include "json.h"

//corresponding header file(s)
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QNetworkAccessManager* networkAccessManager,
                       OAuth* oAuth,
                       UserData* userData,
                       Dropbox* dropbox,
                       QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //member initialization
    this->networkAccessManager = networkAccessManager;
    this->oAuth = oAuth;
    this->userData = userData;
    this->dropbox = dropbox;

    //GUI initialization
    ui->setupUi(this);

    //initial connections
    connect(ui->showAccountInfoAction,
            SIGNAL(triggered()),
            SLOT(requestAccountInformation()));
    connect(ui->exitAction,
            SIGNAL(triggered()),
            SLOT(close()));
    connect(ui->aboutAction,
            SIGNAL(triggered()),
            SLOT(about()));
    connect(ui->aboutQtAction,
            SIGNAL(triggered()),
            qApp,
            SLOT(aboutQt()));

    QTreeWidgetItem* root = new QTreeWidgetItem();
    root->setText(0, "/");
    root->setChildIndicatorPolicy(
            QTreeWidgetItem::ShowIndicator
            );
    ui->filesAndFoldersTreeWidget->addTopLevelItem(root);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::requestAccountInformation()
{
    QString url = dropbox->apiToUrlString(Dropbox::ACCOUNT_INFO);

    QString query = oAuth->consumerKeyParameter() + "&" +
                    oAuth->userTokenParameter(userData) + "&" +
                    oAuth->timestampAndNonceParameters() + "&" +
                    oAuth->signatureMethodParameter();

    QString signatureParameter = oAuth->signatureParameter(
            userData,
            "GET",
            url,
            query
            );

    query += "&" + signatureParameter;

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query ) ) );

    connect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleAccountInformation(QNetworkReply*))
            );
}

void MainWindow::handleAccountInformation(QNetworkReply* networkReply)
{
    disconnect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleAccountInformation(QNetworkReply*))
            );

    networkReply->deleteLater();

    if(networkReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::information(this,
                                 "Error",
                                 "There was an error, try again later."
                                 );

        return;
    }

    //read data from reply
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

    QVariantMap quotaInfo = jsonResult["quota_info"].toMap();

    QString accountInfo = QString("Name: %1 \n"
                                  "Country: %2 \n"
                                  "UID: %3 \n"
                                  "Quota Info: \n"
                                  "  Shared: %4 \n"
                                  "  Quota: %5 \n"
                                  "  Normal: %6 \n")
            .arg(jsonResult["display_name"].toString())
            .arg(jsonResult["country"].toString())
            .arg(jsonResult["uid"].toString())
            .arg(quotaInfo["shared"].toString())
            .arg(quotaInfo["quota"].toString())
            .arg(quotaInfo["normal"].toString());

    QMessageBox::information(this,
                             "Account Information",
                             accountInfo);
}

void MainWindow::requestDirectoryListing(QTreeWidgetItem* item)
{
    ui->statusbar->showMessage("Loading...");

    //find the path of the directory
    QString path = item->text(0);
    QTreeWidgetItem* iterator = item->parent();
    while(iterator != 0)
    {
        path.prepend(iterator->text(0));
        iterator = iterator->parent();
    }

    ui->filesAndFoldersTreeWidget->setCurrentItem(item);

    QString url = dropbox->apiToUrlString(Dropbox::METADATA) + path;

    QString query = oAuth->consumerKeyParameter() + "&" +
                    oAuth->userTokenParameter(userData) + "&" +
                    oAuth->timestampAndNonceParameters() + "&" +
                    oAuth->signatureMethodParameter() + "&" +
                    "list=true";

    QString signatureParameter = oAuth->signatureParameter(
            userData,
            "GET",
            url,
            query
            );

    query = query + "&" + signatureParameter;

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query ) ) );

    connect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleDirectoryListing(QNetworkReply*))
            );
}

void MainWindow::handleDirectoryListing(QNetworkReply* networkReply)
{
    ui->statusbar->clearMessage();

    disconnect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleDirectoryListing(QNetworkReply*))
            );

    networkReply->deleteLater();

    if(networkReply->error() != QNetworkReply::NoError)
    {
        ui->filesAndFoldersTreeWidget->currentItem()->setExpanded(false);

        QMessageBox::information(this,
                                 "Error",
                                 "There was an error, try again later."
                                 );

        return;
    }

    QString dirJson = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = Json::parse(dirJson, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(this,
                                 "Error",
                                 "There was an error, try again later."
                                 );

        return;
    }

    //this removes the child indicator if no children
    ui->filesAndFoldersTreeWidget->currentItem()->setChildIndicatorPolicy(
            QTreeWidgetItem::DontShowIndicatorWhenChildless
            );

    //add folders
    foreach(const QVariant &subDirJson, jsonResult["contents"].toList())
    {
        QVariantMap subDir = subDirJson.toMap();

        if(subDir["is_dir"] == true)
        {
            QTreeWidgetItem *subDirItem;
            subDirItem = new QTreeWidgetItem(
                    ui->filesAndFoldersTreeWidget->currentItem()
                    );

            QString subDirPath = subDir["path"].toString();
            QString name = subDirPath.right(
                    (subDirPath.length() - subDirPath.lastIndexOf("/")) - 1
                     );

            subDirItem->setText(0, name + "/");

            subDirItem->setChildIndicatorPolicy(
                    QTreeWidgetItem::ShowIndicator
                    );
        }
    }

    //add files
    foreach(const QVariant &subDirJson, jsonResult["contents"].toList())
    {
        QVariantMap subDir = subDirJson.toMap();

        if(subDir["is_dir"] != true)
        {
            QTreeWidgetItem *subDirItem;
            subDirItem = new QTreeWidgetItem(
                    ui->filesAndFoldersTreeWidget->currentItem()
                    );

            QString subDirPath = subDir["path"].toString();
            QString name = subDirPath.right(
                    (subDirPath.length() - subDirPath.lastIndexOf("/")) - 1
                     );

            subDirItem->setText(0, name);

            subDirItem->setText(
                    1,
                    subDir["size"].toString()
                    );
        }
    }
}

void MainWindow::requestFile(QTreeWidgetItem* item)
{
    //find the path of the directory that contains the file
    QString path = item->text(0);
    QTreeWidgetItem* iterator = item->parent();
    while(iterator != 0)
    {
        path.prepend(iterator->text(0));
        iterator = iterator->parent();
    }

    ui->filesAndFoldersTreeWidget->setCurrentItem(item);

    QString url = dropbox->apiToUrlString(Dropbox::FILES) + path;

    QString query = oAuth->consumerKeyParameter() + "&" +
                    oAuth->userTokenParameter(userData) + "&" +
                    oAuth->timestampAndNonceParameters() + "&" +
                    oAuth->signatureMethodParameter();

    QString signatureParameter = oAuth->signatureParameter(
            userData,
            "GET",
            url,
            query
            );

    query = query + "&" + signatureParameter;

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query ) ) );

    connect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleFile(QNetworkReply*))
            );
}

void MainWindow::handleFile(QNetworkReply* networkReply)
{
    disconnect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleFile(QNetworkReply*))
            );

    networkReply->deleteLater();

    if(networkReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::information(this,
                                 "Error",
                                 "There was an error, try again later."
                                 );

        return;
    }

    QByteArray fileContents = networkReply->readAll();

    QString path = QFileDialog::getExistingDirectory(this,
                                                     "Select a directory"
                                                     );

    QString fileName = ui->filesAndFoldersTreeWidget->currentItem()->text(0);

    QFile file(path + "/" + fileName);

    file.open(QFile::WriteOnly);

    file.write(fileContents);

    file.close();
}

void MainWindow::about()
{
    QMessageBox qMessageBox(this);

    qMessageBox.setWindowTitle("About");

    qMessageBox.setText(
            "<b>Droper</b> <br>"
            "Copyright 2011 Omar Lawand Dalatieh. <br><br>"

            "Licensed under the GNU GPLv3 license. <br><br>"

            "<a href=\"http://lawand.github.com/droper/\">"
            "http://lawand.github.com/droper/"
            "</a>"
            );

    qMessageBox.exec();
}

void MainWindow::on_filesAndFoldersTreeWidget_itemExpanded(
        QTreeWidgetItem* item
        )
{
    //don't re-request listing if the item's directory is already listed
    if(item->childIndicatorPolicy() == QTreeWidgetItem::ShowIndicator)
        requestDirectoryListing(item);
}

void MainWindow::on_filesAndFoldersTreeWidget_currentItemChanged(
        QTreeWidgetItem* current
        )
{
    QString last = current->text(0).at(current->text(0).length() - 1);
    if( last != "/" ) //if not a directory
        ui->downloadFilePushButton->setEnabled(true);
    else
        ui->downloadFilePushButton->setEnabled(false);
}

void MainWindow::on_downloadFilePushButton_clicked()
{
    requestFile(ui->filesAndFoldersTreeWidget->currentItem());
}
