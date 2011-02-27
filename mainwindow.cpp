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
#include "mainwindow.h"
#include "ui_mainwindow.h"

//data members
#include <QNetworkAccessManager>
#include <QSettings>
#include "oauth.h"
#include "userdata.h"
#include "dropbox.h"

//member functions
#include <QListWidgetItem>

//implementation-specific
#include <QNetworkReply>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QResource>
#include "filetransferdialog.h"
#include "json.h"

MainWindow::MainWindow(QNetworkAccessManager* networkAccessManager,
                       OAuth* oAuth,
                       UserData* userData,
                       Dropbox* dropbox,
                       QSettings* settings,
                       QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentDirectory("/"),
    fileTransferDialog(networkAccessManager,
                       oAuth,
                       userData,
                       dropbox)
{
    //member initialization
    this->networkAccessManager = networkAccessManager;
    this->oAuth = oAuth;
    this->userData = userData;
    this->dropbox = dropbox;
    this->settings = settings;

    //GUI initialization
    ui->setupUi(this);

    //initial connections
    connect(ui->showAccountInfoAction,
            SIGNAL(triggered()),
            SLOT(requestAccountInformation()));
    connect(ui->showDownloadAction,
            SIGNAL(triggered()),
            &fileTransferDialog,
            SLOT(show()));
    connect(ui->forgetAuthenticationAction,
            SIGNAL(triggered()),
            SLOT(forgetAuthentication()));
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

    //set icon size
    ui->filesAndFoldersListWidget->setIconSize(QSize(32, 32));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleNetworkReply(QNetworkReply* networkReply)
{
    Dropbox::Api api = dropbox->urlToApi(networkReply->url());

    //files APIs are handled by the fileTransferDialog
    if(api == Dropbox::FILES)
        return;

    ui->statusbar->clearMessage();

    const int MAX_RETRIES = 20;
    static int retryCount = 0;
    if(networkReply->error() != QNetworkReply::NoError)
    {
        if(retryCount < MAX_RETRIES)
        {
            QUrl url = networkReply->url();
            oAuth->updateRequest(userData, "GET", &url);
            networkAccessManager->get(QNetworkRequest( url ));
            ui->statusbar->showMessage(
                    QString("Retring...%1").arg(retryCount)
                    );
            retryCount++;
        }
        else
        {
            QMessageBox::information(this,
                                     "Error",
                                     "There was an error, try again later."
                                     );
        }

        return;
    }
    retryCount = 0;

    switch(api)
    {
    case Dropbox::ACCOUNT_INFO:
        handleAccountInformation(networkReply);
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

    networkReply->deleteLater();
}

void MainWindow::requestAccountInformation()
{
    QUrl url = dropbox->apiToUrl(Dropbox::ACCOUNT_INFO);

    QPair<QString,QString> temp;

    temp = oAuth->consumerKeyQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->userTokenQueryItem(userData);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->timestampQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->nonceQueryItem(temp.second.toLongLong());
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureMethodQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureQueryItem(
            userData,
            "GET",
            &url
            );
    url.addQueryItem(temp.first, temp.second);

    networkAccessManager->get( QNetworkRequest( url ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleAccountInformation(QNetworkReply* networkReply)
{
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
    QUrl url = dropbox->apiToUrl(Dropbox::METADATA).toString() + path;

    QPair<QString,QString> temp;

    temp = oAuth->consumerKeyQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->userTokenQueryItem(userData);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->timestampQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->nonceQueryItem(temp.second.toLongLong());
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureMethodQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("list"), QString("true"));
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureQueryItem(
            userData,
            "GET",
            &url
            );
    url.addQueryItem(temp.first, temp.second);

    networkAccessManager->get( QNetworkRequest( url ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleDirectoryListing(QNetworkReply* networkReply)
{
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
        ui->filesAndFoldersListWidget->clear();

        //update currentDirectory and ui->currentFolderLineEdit
        currentDirectory = dropbox->metaDataPathFromUrl(networkReply->url());
        QString currentFolder = currentDirectory.right(
                (currentDirectory.length()-currentDirectory.lastIndexOf("/"))-1
                 );
        if(!currentFolder.isEmpty())
            ui->currentFolderLabel->setText(currentFolder);
        else
            ui->currentFolderLabel->setText("Dropbox");

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
            QListWidgetItem* subDirItem = new QListWidgetItem(
                    ui->filesAndFoldersListWidget
                    );

            QString subDirPath = subDir["path"].toString();
            QString subDirName = subDirPath.right(
                    (subDirPath.length() - subDirPath.lastIndexOf("/")) - 1
                     );

            subDirItem->setText(subDirName);

            QResource iconResource(
                    QString(":/icons/%1")
                    .arg(subDir["icon"].toString())
                    + "48.gif"
                    );

            if(iconResource.isValid())
                subDirItem->setIcon(QIcon(iconResource.fileName()));
            else
                subDirItem->setIcon(QIcon(":/icons/folder48.gif"));

            subDirItem->setData(Qt::UserRole, subDir);
        }
    }

    //add files
    foreach(const QVariant &subDirJson, jsonResult["contents"].toList())
    {
        QVariantMap subDir = subDirJson.toMap();

        if(subDir["is_dir"] == false)
        {
            QListWidgetItem* subDirItem = new QListWidgetItem(
                    ui->filesAndFoldersListWidget
                    );

            QString subDirPath = subDir["path"].toString();
            QString subDirName = subDirPath.right(
                    (subDirPath.length() - subDirPath.lastIndexOf("/")) - 1
                     );

            QString size =  subDir["size"].toString();

            subDirItem->setText(subDirName + "\n" + "  " + size);

            QResource iconResource(
                    QString(":/icons/%1")
                    .arg(subDir["icon"].toString())
                    + "48.gif"
                    );

            if(iconResource.isValid())
                subDirItem->setIcon(QIcon(iconResource.fileName()));
            else
                subDirItem->setIcon(QIcon(":/icons/page_white48.gif"));

            subDirItem->setData(Qt::UserRole, subDir);
        }
    }
}

void MainWindow::requestCopying(QString source, QString destination)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_COPY);

    QPair<QString,QString> temp;

    temp = oAuth->consumerKeyQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->userTokenQueryItem(userData);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->timestampQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->nonceQueryItem(temp.second.toLongLong());
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureMethodQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("from_path"), source);
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("to_path"), destination);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureQueryItem(
            userData,
            "GET",
            &url
            );
    url.addQueryItem(temp.first, temp.second);

    networkAccessManager->get( QNetworkRequest( url ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleCopying(QNetworkReply* networkReply)
{
    refreshCurrentDirectory();
}

void MainWindow::requestMoving(QString source, QString destination)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_MOVE);

    QPair<QString,QString> temp;

    temp = oAuth->consumerKeyQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->userTokenQueryItem(userData);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->timestampQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->nonceQueryItem(temp.second.toLongLong());
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureMethodQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("from_path"), source);
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("to_path"), destination);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureQueryItem(
            userData,
            "GET",
            &url
            );
    url.addQueryItem(temp.first, temp.second);

    networkAccessManager->get( QNetworkRequest( url ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleMoving(QNetworkReply* networkReply)
{
    refreshCurrentDirectory();

    clipboard.clear();
}

void MainWindow::requestDeleting(QString path)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_DELETE);

    QPair<QString,QString> temp;

    temp = oAuth->consumerKeyQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->userTokenQueryItem(userData);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->timestampQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->nonceQueryItem(temp.second.toLongLong());
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureMethodQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("path"), path);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureQueryItem(
            userData,
            "GET",
            &url
            );
    url.addQueryItem(temp.first, temp.second);

    networkAccessManager->get( QNetworkRequest( url ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleDeleting(QNetworkReply* networkReply)
{
    refreshCurrentDirectory();
}

void MainWindow::requestFolderCreation(QString path)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_CREATEFOLDER);

    QPair<QString,QString> temp;

    temp = oAuth->consumerKeyQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->userTokenQueryItem(userData);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->timestampQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->nonceQueryItem(temp.second.toLongLong());
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureMethodQueryItem();
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("path"), path);
    url.addQueryItem(temp.first, temp.second);

    temp = oAuth->signatureQueryItem(
            userData,
            "GET",
            &url
            );
    url.addQueryItem(temp.first, temp.second);

    networkAccessManager->get( QNetworkRequest( url ) );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleFolderCreation(QNetworkReply* networkReply)
{
    refreshCurrentDirectory();
}

void MainWindow::about()
{
    QMessageBox messageBox;

    messageBox.setWindowTitle("About");

    messageBox.setText(
            "<b>Droper v0.1.1</b> - "
            "<a href=\"http://www.dropbox.com/\">Dropbox</a> Client."
            );

    messageBox.setInformativeText(
            "Copyright 2011 Omar Lawand Dalatieh. <br><br>"

            "Licensed under the GNU GPLv3 license. <br><br>"

            "<a href=\"http://lawand.github.com/droper/\">"
            "http://lawand.github.com/droper/"
            "</a>"
            );

    QIcon droper(":/droper.svg");
    messageBox.setIconPixmap(droper.pixmap(QSize(32, 32)));

    messageBox.exec();
}

void MainWindow::forgetAuthentication()
{
    settings->clear();
}

void MainWindow::refreshCurrentDirectory()
{
    requestDirectoryListing(currentDirectory);
}

void MainWindow::on_filesAndFoldersListWidget_itemDoubleClicked(
        QListWidgetItem* item
        )
{
    QVariantMap map = item->data(Qt::UserRole).toMap();

    if(map["is_dir"] == true)   //if the item is a directory
        //navigate to that sub directory
        requestDirectoryListing(
                map["path"].toString()
                );
    else    //download the file
    {
        if(fileTransferDialog.setFile(&map) != true)
        {
            QMessageBox::information(this,
                                     "Download Active",
                                     "There already is a file being "
                                     "downloaded.");
        }
        else
        {
            fileTransferDialog.show();
        }
    }
}

void MainWindow::on_upPushButton_clicked()
{
    //generate new directory
        QStringList parts = currentDirectory.split("/");
        parts.removeLast();
        QString newDirectory = parts.join("/");

        //handle root
        if(newDirectory.isEmpty())
            newDirectory = "/";

    requestDirectoryListing(newDirectory);
}

void MainWindow::on_refreshPushButton_clicked()
{
    refreshCurrentDirectory();
}

void MainWindow::on_cutPushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    //mark the operation as a cut operation
    shouldPreserveClipboardContents = false;

    //fill the clipboard
    QListWidgetItem* currentItem =
            ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
            currentItem->data(Qt::UserRole).toMap();
    clipboard = map["path"].toString();
}

void MainWindow::on_copyPushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    //mark the operation as a copy operation
    shouldPreserveClipboardContents = true;

    //fill the clipboard
    QListWidgetItem* currentItem =
            ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
            currentItem->data(Qt::UserRole).toMap();
    clipboard = map["path"].toString();
}

void MainWindow::on_pastePushButton_clicked()
{
    //if the clipboard is empty, do nothing
    if(clipboard.isEmpty())
        return;

    //get file or folder name
    QString name = clipboard.right(
            (clipboard.length() - clipboard.lastIndexOf("/")) - 1
            );

    if(shouldPreserveClipboardContents) //if this is a cut operation
    {
        if(currentDirectory == "/")
        {
            requestCopying(clipboard, currentDirectory + name);
        }
        else
        {
            requestCopying(clipboard, currentDirectory + "/" + name);
        }
    }
    else
    {
        if(currentDirectory == "/")
        {
            requestMoving(clipboard, currentDirectory + name);
        }
        else
        {
            requestMoving(clipboard, currentDirectory + "/" + name);
        }
    }
}

void MainWindow::on_renamePushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    QListWidgetItem* currentItem =
            ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
            currentItem->data(Qt::UserRole).toMap();
    QString path = map["path"].toString();
    QString oldName = path.right(
            (path.length() - path.lastIndexOf("/")) - 1
            );

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
    if(currentDirectory == "/")
    {
        requestMoving(
                currentDirectory + oldName,
                currentDirectory + newName
                );
    }
    else
    {
        requestMoving(
                currentDirectory + "/" + oldName,
                currentDirectory + "/" + newName
                );
    }
}

void MainWindow::on_deletePushButton_clicked()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    QMessageBox::StandardButton response = QMessageBox::question(
            this,
            "Are you sure?",
            "Are you sure you want to delete the file/folder?",
            QMessageBox::No|QMessageBox::Yes,
            QMessageBox::No
            );

    if(response == QMessageBox::Yes)
    {
        QListWidgetItem* currentItem =
                ui->filesAndFoldersListWidget->currentItem();
        QVariantMap map =
                currentItem->data(Qt::UserRole).toMap();
        QString path = map["path"].toString();

        requestDeleting(path);
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

    if(currentDirectory == "/")
    {
        requestFolderCreation(currentDirectory + folderName);
    }
    else
    {
        requestFolderCreation(currentDirectory + "/" + folderName);
    }
}
