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
#include "mainwindow.h"
#include "ui_mainwindow.h"

//data member(s)
#include <QNetworkAccessManager>
#include "oauth.h"
#include "userdata.h"
#include "dropbox.h"

//member-function(s)-related forward declaration(s)
#include <QTreeWidgetItem>

//implementation-specific data type(s)
#include <QNetworkReply>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include "json.h"

MainWindow::MainWindow(QNetworkAccessManager* networkAccessManager,
                       OAuth* oAuth,
                       UserData* userData,
                       Dropbox* dropbox,
                       QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentDirectory("/")
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

    //initial directory listing
    requestDirectoryListing(currentDirectory);

    //connect the networkAccessManager with the handler
    connect(this->networkAccessManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(handleNetworkReply(QNetworkReply*))
            );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleNetworkReply(QNetworkReply* networkReply)
{
    ui->statusbar->clearMessage();

    if(networkReply->error() != QNetworkReply::NoError)
    {
        QMessageBox::information(this,
                                 "Error",
                                 "There was an error, try again later."
                                 );

        return;
    }

    Dropbox::Api api = dropbox->urlStringToApi(networkReply->url().toString());

    switch(api)
    {
    case Dropbox::ACCOUNT_INFO:
        handleAccountInformation(networkReply);
        break;

    case Dropbox::FILES:
        handleFile(networkReply);
        break;

    case Dropbox::METADATA:
        handleDirectoryListing(networkReply);
        break;

    case Dropbox::FILEOPS_CREATEFOLDER:
        handleFolderCreation(networkReply);
        break;

    case Dropbox::FILEOPS_COPY:
        handleCopying(networkReply);
        break;

    case Dropbox::FILEOPS_MOVE:
        handleMoving(networkReply);
        break;

    case Dropbox::FILEOPS_DELETE:
        handleDeleting(networkReply);
        break;
    }
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

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleAccountInformation(QNetworkReply* networkReply)
{
    networkReply->deleteLater();

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

void MainWindow::requestDirectoryListing(QString path)
{
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

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleDirectoryListing(QNetworkReply* networkReply)
{
    networkReply->deleteLater();

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

    //prepare to change current directory
        ui->filesAndFoldersTreeWidget->clear();

        //update currentDirectory and ui->currentDirectoryLineEdit
        QString urlString = networkReply->url().toString(QUrl::RemoveQuery);
        currentDirectory = dropbox->extractMetaDataPath(urlString);
        ui->currentDirectoryLineEdit->setText(currentDirectory);

        if(currentDirectory == "/")
            ui->upPushButton->setEnabled(false);
        else
            ui->upPushButton->setEnabled(true);

    //add folders
    foreach(const QVariant &subDirJson, jsonResult["contents"].toList())
    {
        QVariantMap subDir = subDirJson.toMap();

        if(subDir["is_dir"] == true)
        {
            QTreeWidgetItem* subDirItem = new QTreeWidgetItem(
                    ui->filesAndFoldersTreeWidget
                    );

            QString subDirPath = subDir["path"].toString();
            QString name = subDirPath.right(
                    (subDirPath.length() - subDirPath.lastIndexOf("/")) - 1
                     );

            subDirItem->setText(0, name + "/");
        }
    }

    //add files
    foreach(const QVariant &subDirJson, jsonResult["contents"].toList())
    {
        QVariantMap subDir = subDirJson.toMap();

        if(subDir["is_dir"] == false)
        {
            QTreeWidgetItem* subDirItem = new QTreeWidgetItem(
                    ui->filesAndFoldersTreeWidget
                    );

            QString subDirPath = subDir["path"].toString();
            QString name = subDirPath.right(
                    (subDirPath.length() - subDirPath.lastIndexOf("/")) - 1
                     );

            subDirItem->setText(0, name);

            QString size =  subDir["size"].toString();

            subDirItem->setText(1, size);
        }
    }
}

void MainWindow::requestFile(QString path)
{
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

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleFile(QNetworkReply* networkReply)
{
    networkReply->deleteLater();

    QByteArray fileContents = networkReply->readAll();

    QString fileSystemPath = QFileDialog::getExistingDirectory(this,
                                                     "Select a directory"
                                                     );

    QString urlString = networkReply->url().toString(QUrl::RemoveQuery);
    QString path = dropbox->extractFilePath(urlString);

    QString fileName = path.right(
            (path.length() - path.lastIndexOf("/")) - 1
             );

    QFile file(fileSystemPath + "/" + fileName);

    file.open(QFile::WriteOnly);

    file.write(fileContents);

    file.close();
}

void MainWindow::requestCopying(QString source, QString destination)
{
    QString url = dropbox->apiToUrlString(Dropbox::FILEOPS_COPY);

    QString query = oAuth->consumerKeyParameter() + "&" +
                    oAuth->userTokenParameter(userData) + "&" +
                    oAuth->timestampAndNonceParameters() + "&" +
                    oAuth->signatureMethodParameter() + "&" +
                    QString("root=%1").arg("dropbox") + "&" +
                    QString("from_path=%1").arg(source) + "&" +
                    QString("to_path=%1").arg(destination);

    QString signatureParameter = oAuth->signatureParameter(
            userData,
            "GET",
            url,
            query
            );

    query = query + "&" + signatureParameter;

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleCopying(QNetworkReply* networkReply)
{
    networkReply->deleteLater();

    refreshCurrentDirectory();
}

void MainWindow::requestMoving(QString source, QString destination)
{
    QString url = dropbox->apiToUrlString(Dropbox::FILEOPS_MOVE);

    QString query = oAuth->consumerKeyParameter() + "&" +
                    oAuth->userTokenParameter(userData) + "&" +
                    oAuth->timestampAndNonceParameters() + "&" +
                    oAuth->signatureMethodParameter() + "&" +
                    QString("root=%1").arg("dropbox") + "&" +
                    QString("from_path=%1").arg(source) + "&" +
                    QString("to_path=%1").arg(destination);

    QString signatureParameter = oAuth->signatureParameter(
            userData,
            "GET",
            url,
            query
            );

    query = query + "&" + signatureParameter;

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleMoving(QNetworkReply* networkReply)
{
    networkReply->deleteLater();

    refreshCurrentDirectory();
}

void MainWindow::requestDeleting(QString path)
{
    QString url = dropbox->apiToUrlString(Dropbox::FILEOPS_DELETE);

    QString query = oAuth->consumerKeyParameter() + "&" +
                    oAuth->userTokenParameter(userData) + "&" +
                    oAuth->timestampAndNonceParameters() + "&" +
                    oAuth->signatureMethodParameter() + "&" +
                    QString("path=%1").arg(path) + "&" +
                    QString("root=%1").arg("dropbox");

    QString signatureParameter = oAuth->signatureParameter(
            userData,
            "GET",
            url,
            query
            );

    query = query + "&" + signatureParameter;

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleDeleting(QNetworkReply* networkReply)
{
    networkReply->deleteLater();

    refreshCurrentDirectory();
}

void MainWindow::requestFolderCreation(QString path)
{
    QString url = dropbox->apiToUrlString(Dropbox::FILEOPS_CREATEFOLDER);

    QString query = oAuth->consumerKeyParameter() + "&" +
                    oAuth->userTokenParameter(userData) + "&" +
                    oAuth->timestampAndNonceParameters() + "&" +
                    oAuth->signatureMethodParameter() + "&" +
                    QString("path=%1").arg(path) + "&" +
                    QString("root=%1").arg("dropbox");

    QString signatureParameter = oAuth->signatureParameter(
            userData,
            "GET",
            url,
            query
            );

    query = query + "&" + signatureParameter;

    networkAccessManager->get( QNetworkRequest( QUrl(url+"?"+query) ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleFolderCreation(QNetworkReply* networkReply)
{
    networkReply->deleteLater();

    refreshCurrentDirectory();
}

void MainWindow::about()
{
    QMessageBox qMessageBox(this);

    qMessageBox.setWindowTitle("About");

    qMessageBox.setText(
            "<b>Droper</b> - Dropbox Client.<br>"
            "Copyright 2011 Omar Lawand Dalatieh. <br><br>"

            "Licensed under the GNU GPLv3 license. <br><br>"

            "<a href=\"http://lawand.github.com/droper/\">"
            "http://lawand.github.com/droper/"
            "</a>"
            );

    qMessageBox.exec();
}

void MainWindow::refreshCurrentDirectory()
{
    requestDirectoryListing(currentDirectory);
}

void MainWindow::on_filesAndFoldersTreeWidget_itemDoubleClicked(
        QTreeWidgetItem* item
        )
{
    QString lastCharacter = item->text(0).at(item->text(0).length() - 1);

    //if the item is a directory
    if(lastCharacter == "/")
        //navigate to that sub directory
        requestDirectoryListing(currentDirectory + item->text(0));
    else
        //download the file
        requestFile(currentDirectory + item->text(0));
}

void MainWindow::on_upPushButton_clicked()
{
    //generate new directory
        QStringList parts = currentDirectory.split("/");
        //remove last part knowing that there are two empty parts, one at the
        //beginning and one at the end
        parts.removeAt(parts.length() - 2);
        QString newDirectory = parts.join("/");

    requestDirectoryListing(newDirectory);
}

void MainWindow::on_refreshPushButton_clicked()
{
    refreshCurrentDirectory();
}

void MainWindow::on_cutPushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersTreeWidget->selectedItems().isEmpty() )
        return;

    //mark the operation as a cut operation
    shouldPreserveClipboardContents = false;

    //fill the clipboard
    clipboard = currentDirectory +
                ui->filesAndFoldersTreeWidget->currentItem()->text(0);
}

void MainWindow::on_copyPushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersTreeWidget->selectedItems().isEmpty() )
        return;

    //mark the operation as a copy operation
    shouldPreserveClipboardContents = true;

    //fill the clipboard
    clipboard = currentDirectory +
                ui->filesAndFoldersTreeWidget->currentItem()->text(0);
}

void MainWindow::on_pastePushButton_clicked()
{
    //if the clipboard is empty, do nothing
    if(clipboard.isEmpty())
        return;

    //perform the paste operation
        QString fileName = clipboard.right(
                (clipboard.length() - clipboard.lastIndexOf("/")) - 1
                 );

        //check whether this is a cut or copy operation
        if(shouldPreserveClipboardContents)
        {
            requestCopying(clipboard, currentDirectory + fileName);
        }
        else
        {
            requestMoving(clipboard, currentDirectory + fileName);

            clipboard.clear();
        }
}

void MainWindow::on_renamePushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersTreeWidget->selectedItems().isEmpty() )
        return;

    QString oldName = ui->filesAndFoldersTreeWidget->currentItem()->text(0);

    //check to know whether this is a directory or a file
    bool isDir;
    if(oldName.at(oldName.length() - 1) == '/')
        isDir = true;
    else
        isDir = false;

    if(isDir)
    {
        //this removes the last character which should be '/'
        oldName.chop(1);
    }

    QString newName = QInputDialog::getText(
            this,
            "Rename",
            "Enter a new name:",
            QLineEdit::Normal,
            oldName
            );

    //if no new value was entered, do nothing
    if(newName.isEmpty())
        return;

    //if the new name is the same as the old name, do nothing
    if(newName == oldName)
        return;

    //perform the rename operation
    if(isDir)
    {
        requestMoving(
                currentDirectory + oldName + "/",
                currentDirectory + newName + "/"
                );
    }
    else
    {
        requestMoving(
                currentDirectory + oldName,
                currentDirectory + newName
                );
    }
}

void MainWindow::on_deletePushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersTreeWidget->selectedItems().isEmpty() )
        return;

    //perform the delete operation
        QMessageBox::StandardButton response = QMessageBox::question(
                this,
                "Are you sure?",
                "Are you sure you want to delete the file/folder?",
                QMessageBox::No|QMessageBox::Yes,
                QMessageBox::No
                );

        if(response == QMessageBox::Yes)
        {
            requestDeleting(
                    currentDirectory +
                    ui->filesAndFoldersTreeWidget->currentItem()->text(0)
                    );
        }
}

void MainWindow::on_createFolderPushButton_clicked()
{
    QString folderName = QInputDialog::getText(this,
                          "Create Folder",
                          "Enter the folder's name:"
                          );

    //if no folderName was entered, do nothing
    if( folderName.isEmpty() )
        return;

    //perform the folder creation operation
        requestFolderCreation(currentDirectory + folderName);
}
