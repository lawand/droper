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
#include <QNetworkReply>
#include <QListWidgetItem>

//implementation-specific
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QResource>
#include <QDesktopWidget>
#include "downloaddialog.h"
#include "json.h"

MainWindow::MainWindow(
        QNetworkAccessManager* networkAccessManager,
        OAuth* oAuth,
        UserData* userData,
        Dropbox* dropbox,
        QSettings* settings,
        QWidget *parent
        ) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentDirectory("/"),
    downloadDialog(
            networkAccessManager,
            oAuth,
            userData,
            dropbox,
            this
            )
{
    //member initialization
    this->networkAccessManager = networkAccessManager;
    this->oAuth = oAuth;
    this->userData = userData;
    this->dropbox = dropbox;
    this->settings = settings;

    //GUI initialization
    ui->setupUi(this);
    showMaximized();

    //initial connections
    connect(
            ui->showAccountInfoAction,
            SIGNAL(triggered()),
            SLOT(requestAccountInformation())
            );
    connect(
            ui->showActiveDownloadAction,
            SIGNAL(triggered()),
             &downloadDialog,
             SLOT(show())
             );
    connect(
            ui->forgetAuthenticationAction,
            SIGNAL(triggered()),
            SLOT(forgetAuthentication())
            );
    connect( ui->cutAction, SIGNAL(triggered()), SLOT(cut()) );
    connect( ui->copyAction, SIGNAL(triggered()), SLOT(copy()) );
    connect( ui->pasteAction, SIGNAL(triggered()), SLOT(paste()) );
    connect( ui->renameAction, SIGNAL(triggered()), SLOT(rename()) );
    connect( ui->deleteAction, SIGNAL(triggered()), SLOT(del()) );
    connect(
            ui->createFolderAction,
            SIGNAL(triggered()),
            SLOT(createFolder())
            );
    connect( ui->upAction, SIGNAL(triggered()), SLOT(up()) );
    connect( ui->refreshAction, SIGNAL(triggered()), SLOT(refresh()) );
    connect( ui->aboutAction, SIGNAL(triggered()), SLOT(about()) );
    connect( ui->aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()) );
    connect( ui->downloadAction, SIGNAL(triggered()), SLOT(download()) );
    connect(
            ui->propertiesAction,
            SIGNAL(triggered()),
            SLOT(showProperties())
            );

    //initial directory listing
    requestDirectoryListing(currentDirectory);

    //connect the networkAccessManager with the handler
    connect(
            this->networkAccessManager,
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

    //files APIs are handled by the downloadDialog
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
            QNetworkRequest networkRequest(url);
            oAuth->signRequest(userData, "GET", &networkRequest);
            networkAccessManager->get(networkRequest);

            ui->statusbar->showMessage(
                    QString("Retring...%1").arg(retryCount)
                    );
            retryCount++;
        }
        else
        {
            QMessageBox::information(
                    this,
                    "Droper",
                    "There was an error, try again later."
                    );

            //reset for next time
            retryCount = 0;
        }

        return;
    }
    //reset for next time
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

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(
            userData,
            "GET",
            &networkRequest
            );

    networkAccessManager->get( networkRequest );

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
        QMessageBox::information(
                this,
                "Droper",
                "There was an error, try again later."
                );

        return;
    }

    QVariantMap quotaInfo = jsonResult["quota_info"].toMap();

    qreal normalFiles = quotaInfo["normal"].toReal();
    QString normalFilesUnit;
    if (normalFiles < 1024) {
        normalFilesUnit = "bytes";
    } else if (normalFiles < 1024*1024) {
        normalFiles /= 1024;
        normalFilesUnit = "kB";
    } else if (normalFiles < 1024*1024*1024){
        normalFiles /= 1024*1024;
        normalFilesUnit = "MB";
    } else {
        normalFiles /= 1024*1024*1024;
        normalFilesUnit = "GB";
    }
    QString normalFilesString = QString("%1%2")
                                .arg(normalFiles, 0, 'f', 1)
                                .arg(normalFilesUnit)
                                ;

    qreal sharedFiles = quotaInfo["shared"].toReal();
    QString sharedFilesUnit;
    if (sharedFiles < 1024) {
        sharedFilesUnit = "bytes";
    } else if (sharedFiles < 1024*1024) {
        sharedFiles /= 1024;
        sharedFilesUnit = "kB";
    } else if (sharedFiles < 1024*1024*1024){
        sharedFiles /= 1024*1024;
        sharedFilesUnit = "MB";
    } else {
        sharedFiles /= 1024*1024*1024;
        sharedFilesUnit = "GB";
    }
    QString sharedFilesString = QString("%1%2")
                                .arg(sharedFiles, 0, 'f', 1)
                                .arg(sharedFilesUnit)
                                ;

    qreal quota = quotaInfo["quota"].toReal();
    QString quotaUnit;
    if (quota < 1024) {
        quotaUnit = "bytes";
    } else if (quota < 1024*1024) {
        quota /= 1024;
        quotaUnit = "kB";
    } else if (quota < 1024*1024*1024){
        quota /= 1024*1024;
        quotaUnit = "MB";
    } else {
        quota /= 1024*1024*1024;
        quotaUnit = "GB";
    }
    QString quotaString = QString("%1%2")
                                .arg(quota, 0, 'f', 1)
                                .arg(quotaUnit)
                                ;

    qreal total = quotaInfo["normal"].toReal() +
                quotaInfo["shared"].toReal() +
                quotaInfo["quota"].toReal();
    QString totalUnit;
    if (total < 1024) {
        totalUnit = "bytes";
    } else if (total < 1024*1024) {
        total /= 1024;
        totalUnit = "kB";
    } else if (total < 1024*1024*1024){
        total /= 1024*1024;
        totalUnit = "MB";
    } else {
        total /= 1024*1024*1024;
        totalUnit = "GB";
    }
    QString totalString = QString("%1%2")
                                .arg(total, 0, 'f', 1)
                                .arg(totalUnit)
                                ;


    QMessageBox messageBox(this);

    messageBox.setWindowTitle("Droper");

    messageBox.setText(
            QString("Showing account information for:\n%1").arg(userData->email)
            );

    messageBox.setInformativeText(
            QString(
                        "Name: %1 \n"
                        "Country: %2 \n"
                        "UID: %3 \n"
                        "\n"
                        "Total Space: %7 \n"
                        "Space Left: %4 \n"
                        "Regular files: %5 \n"
                        "Shared files: %6"
                        )
                        .arg(jsonResult["display_name"].toString())
                        .arg(jsonResult["country"].toString())
                        .arg(jsonResult["uid"].toString())
                        .arg(quotaString)
                        .arg(normalFilesString)
                        .arg(sharedFilesString)
                        .arg(totalString)
            );

    messageBox.exec();
}

void MainWindow::requestDirectoryListing(QString path)
{
    QUrl url = dropbox->apiToUrl(Dropbox::METADATA).toString() + path;

    QPair<QString,QString> temp;
    temp = qMakePair(QString("list"), QString("true"));
    url.addQueryItem(temp.first, temp.second);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(userData, "GET", &networkRequest);

    networkAccessManager->get( networkRequest );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleDirectoryListing(QNetworkReply* networkReply)
{
    QString dirJson = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = Json::parse(dirJson, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
                this,
                "Droper",
                "There was an error, try again later."
                );

        return;
    }

    //prepare to change current directory
        ui->filesAndFoldersListWidget->clear();

        //set current directory's icon
            QResource iconResource(
                    QString(":/icons/%1")
                    .arg(jsonResult["icon"].toString())
                    + "48.png"
                    );
            if(iconResource.isValid())
                ui->currentFolderToolButton->setIcon(
                        QIcon(iconResource.fileName())
                        );
            else
                ui->currentFolderToolButton->setIcon(
                        QIcon(":/icons/folder48.png")
                        );

        //update currentDirectory and ui->currentFolderLabel
        currentDirectory = dropbox->metaDataPathFromUrl(networkReply->url());
        QString currentFolder = currentDirectory.right(
                (currentDirectory.length()-currentDirectory.lastIndexOf("/"))-1
                 );
        if(!currentFolder.isEmpty())
            ui->currentFolderLabel->setText(currentFolder);
        else
            ui->currentFolderLabel->setText("Dropbox");

        if(currentDirectory != "/")
        {
            QListWidgetItem* upItem = new QListWidgetItem(
                    ui->filesAndFoldersListWidget
                    );

            upItem->setText("Up");
            upItem->setIcon(QIcon(":/icons/up48.png"));
        }

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
                    + "48.png"
                    );

            if(iconResource.isValid())
                subDirItem->setIcon(QIcon(iconResource.fileName()));
            else
                subDirItem->setIcon(QIcon(":/icons/folder48.png"));

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
                    + "48.png"
                    );

            if(iconResource.isValid())
                subDirItem->setIcon(QIcon(iconResource.fileName()));
            else
                subDirItem->setIcon(QIcon(":/icons/page_white48.png"));

            subDirItem->setData(Qt::UserRole, subDir);
        }
    }
}

void MainWindow::requestCopying(QString source, QString destination)
{
    //avoid errors
    if(source == destination)
        return;

    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_COPY);

    QPair<QString,QString> temp;

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("from_path"), source);
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("to_path"), destination);
    url.addQueryItem(temp.first, temp.second);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(
            userData,
            "GET",
            &networkRequest
            );

    networkAccessManager->get( networkRequest );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleCopying(QNetworkReply* networkReply)
{
    refreshCurrentDirectory();
}

void MainWindow::requestMoving(QString source, QString destination)
{
    //avoid errors
    if(source == destination)
        return;

    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_MOVE);

    QPair<QString,QString> temp;

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("from_path"), source);
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("to_path"), destination);
    url.addQueryItem(temp.first, temp.second);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(
            userData,
            "GET",
            &networkRequest
            );

    networkAccessManager->get( networkRequest );

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

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("path"), path);
    url.addQueryItem(temp.first, temp.second);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(
            userData,
            "GET",
            &networkRequest
            );

    networkAccessManager->get( networkRequest );

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

    temp = qMakePair(QString("root"), QString("dropbox"));
    url.addQueryItem(temp.first, temp.second);

    temp = qMakePair(QString("path"), path);
    url.addQueryItem(temp.first, temp.second);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(
            userData,
            "GET",
            &networkRequest
            );

    networkAccessManager->get( networkRequest );

    ui->statusbar->showMessage("Loading...");
}

void MainWindow::handleFolderCreation(QNetworkReply* networkReply)
{
    refreshCurrentDirectory();
}

void MainWindow::about()
{
    QMessageBox messageBox(this);

    messageBox.setWindowTitle("Droper");

    messageBox.setText(
            "<b>Droper v0.2.2</b> - "
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
    if(item->data(Qt::UserRole).isNull()) //if this is the Up item
    {
        //navigate to the parent directory
        ui->upAction->trigger();

        return;
    }

    QVariantMap map = item->data(Qt::UserRole).toMap();

    if(map["is_dir"] == true)   //if the item is a directory
    {
        //navigate to a sub directory
        requestDirectoryListing(
                map["path"].toString()
                );
    }
}

void MainWindow::up()
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

void MainWindow::refresh()
{
    refreshCurrentDirectory();
}

void MainWindow::cut()
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

void MainWindow::copy()
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

void MainWindow::paste()
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

void MainWindow::rename()
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

void MainWindow::del()
{
    //if no item is selected, do nothing
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    QMessageBox::StandardButton response = QMessageBox::question(
            this,
            "Droper",
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

void MainWindow::download()
{
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    //fill the clipboard
    QListWidgetItem* currentItem =
            ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
            currentItem->data(Qt::UserRole).toMap();

    if(downloadDialog.setFile(&map) != true)
    {
        QMessageBox::information(
                this,
                "Droper",
                "There already is a file being "
                "downloaded."
                );
    }
    else
    {
        downloadDialog.show();
    }
}

void MainWindow::createFolder()
{
    QString folderName = QInputDialog::getText(
            this,
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

void MainWindow::showProperties()
{
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    //get raw info
    QListWidgetItem* currentItem =
            ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
            currentItem->data(Qt::UserRole).toMap();

    //size
    QString size = map["size"].toString();
    if(!size.endsWith("bytes"))
    {
        QString bytes = map["bytes"].toString();
        size += QString(" (%1 bytes)").arg(bytes);
    }

    //path and name
    QString path = map["path"].toString();
    QString name = path.right(
            (path.length() - path.lastIndexOf("/")) - 1
            );

    //modified date and time
    QString modifiedString = map["modified"].toString();
    modifiedString.chop(6);     //chop() removes the time zone
    QDateTime modifiedTimeDate = QDateTime::fromString(
            modifiedString,
            "ddd, dd MMM yyyy HH:mm:ss"
            );
    modifiedTimeDate.setTimeSpec(Qt::UTC);
    QDateTime current = QDateTime::currentDateTimeUtc();
    int secs = modifiedTimeDate.secsTo(current);
    int mins = secs/60;
    int hours = mins/60;
    int days = hours/24;
    int months = days/30;
    int years = months/12;
    QString friendlyModifiedString;
    if(secs < 60)
    {
        friendlyModifiedString = QString(
                "about %1 second(s) ago"
                ).arg(secs);
    }
    else if(mins < 60)
    {
        friendlyModifiedString = QString(
                "about %1 minute(s) ago"
                ).arg(mins);
    }
    else if(hours < 24)
    {
        friendlyModifiedString = QString(
                "about %1 hour(s) ago"
                ).arg(hours);
    }
    else if(days < 30)
    {
        friendlyModifiedString = QString(
                "about %1 day(s) ago"
                ).arg(days);
    }
    else if(months < 12)
    {
        friendlyModifiedString = QString(
                "about %1 month(s) ago"
                ).arg(months);
    }
    else
    {
        friendlyModifiedString = QString(
                "about %1 year(s) ago"
                ).arg(years);
    }
    modifiedString += QString(" (%1)").arg(friendlyModifiedString);

    //show results
    QMessageBox messageBox(this);
    messageBox.setWindowTitle("Droper");
    messageBox.setText(
            QString(
                    "Showing details for file : %1"
                    ).arg(name)
            );
    messageBox.setInformativeText(
            QString(
                    "Size: %1\n"
                    "Path: %2 \n"
                    "Modified (in UTC): %3"
                    ).arg(size).arg(path).arg(modifiedString)
            );
    messageBox.exec();
}

void MainWindow::on_filesAndFoldersListWidget_customContextMenuRequested(
        QPoint point
        )
{
    if(! ui->filesAndFoldersListWidget->selectedItems().isEmpty())
    {
        if(ui->filesAndFoldersListWidget->currentItem()
            ->data(Qt::UserRole).isNull()
            ) //if this is the Up item
        {
            //don't show a menu
            return;
        }

        QMenu menu(this);
        menu.addAction(ui->cutAction);
        menu.addAction(ui->copyAction);
        menu.addAction(ui->renameAction);
        menu.addAction(ui->deleteAction);

        QVariantMap map = ui->filesAndFoldersListWidget->currentItem()
                          ->data(Qt::UserRole).toMap();
        if(map["is_dir"] != true)   //if the item is not a directory
        {
            menu.addAction(ui->propertiesAction);
            menu.addAction(ui->downloadAction);
        }

        menu.exec(ui->filesAndFoldersListWidget->mapToGlobal(point));
    }
}

void MainWindow::on_currentFolderToolButton_pressed()
{
    QMenu menu(this);
    menu.addAction(ui->pasteAction);
    menu.addAction(ui->createFolderAction);
    menu.addAction(ui->refreshAction);
    menu.exec(
            this->mapToGlobal(
                    ui->currentFolderToolButton->geometry().center()
                    )
            );

    ui->currentFolderToolButton->setDown(false);
}
