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
#include "mainwindow.h"
#include "ui_mainwindow.h"

// member functions
#include <QNetworkRequest>
#include <QNetworkReply>

// implementation-specific
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>
#include <QFileDialog>
#include "common.h"
#include "dropbox.h"
#include "filetransferspage.h"
#include "userdata.h"
#include "util.h"
#include "json.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // private data members initialization
    ui->setupUi(this);

    // signal/slot connections
    connect(
        Common::networkAccessManager,
        SIGNAL(finished(QNetworkReply*)),
        SLOT(handleNetworkReply(QNetworkReply*))
        );
    connect(
        ui->signInPage,
        SIGNAL(networkRequestGetNeeded(QNetworkRequest*)),
        SLOT(getNetworkRequest(QNetworkRequest*))
        );
    connect(
        ui->signInPage,
        SIGNAL(oauthAccesstokenHandled()),
        SLOT(attemptSignIn())
        );
    connect(
        ui->navigationPage,
        SIGNAL(networkRequestGetNeeded(QNetworkRequest*)),
        SLOT(getNetworkRequest(QNetworkRequest*))
        );
    connect(
        ui->navigationPage,
        SIGNAL(accountInfoRequested()),
        SLOT(switchToAccountInfo())
        );
    connect(
        ui->navigationPage,
        SIGNAL(downloadRequested(QVariantMap)),
        SLOT(download(QVariantMap))
        );
    connect(
        ui->navigationPage,
        SIGNAL(uploadRequested(QString)),
        SLOT(upload(QString))
        );
    connect(
        ui->navigationPage,
        SIGNAL(switchToFileTransfersRequested()),
        SLOT(switchToFileTransfers())
        );
    connect(
        ui->accountInfoPage,
        SIGNAL(networkRequestGetNeeded(QNetworkRequest*)),
        SLOT(getNetworkRequest(QNetworkRequest*))
        );
    connect(
        ui->accountInfoPage,
        SIGNAL(backRequested()),
        SLOT(switchToNavigation())
        );
    connect(
        ui->accountInfoPage,
        SIGNAL(signOutRequested()),
        SLOT(signOut())
        );
    connect(
        ui->fileTransfersPage,
        SIGNAL(networkRequestGetNeeded(QNetworkRequest*)),
        SLOT(getNetworkRequest(QNetworkRequest*))
        );
    connect(
        ui->fileTransfersPage,
        SIGNAL(networkRequestPutNeeded(QNetworkRequest*,QIODevice*)),
        SLOT(putNetworkRequest(QNetworkRequest*,QIODevice*))
        );
    connect(
        ui->fileTransfersPage,
        SIGNAL(backRequested()),
        SLOT(switchToNavigation())
        );
    connect(
        ui->fileTransfersPage,
        SIGNAL(fileUploaded(QVariantMap)),
        ui->navigationPage,
        SLOT(addItemToFilesAndFoldersListWidget(QVariantMap))
        );

    // initial sign in attempt
    attemptSignIn();

    // actions setup
    setupActions();
    ui->menubar->addActions(this->actions());

#ifdef Q_OS_SYMBIAN
    // optimize layout for mobile phones
    if(!Util::s60v3())
    {
        ui->centralwidget->layout()->setMargin(0);
    }
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getNetworkRequest(QNetworkRequest *networkRequest)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkRequest->url());

    switch(api)
    {
    case Dropbox::FILES:
    case Dropbox::FILESPUT:
        break;

    default:
        // show animation
        previousPage = ui->stackedWidget->currentWidget();
        setCurrentPage(ui->loadingPage);
        break;
    }

    QNetworkReply *networkReply = Common::networkAccessManager->get(
        *networkRequest
        );

    switch(api)
    {
    case Dropbox::FILES:
        ui->fileTransfersPage->setDownloadNetworkReply(networkReply);
        break;

    default:
        break;
    }
}

void MainWindow::putNetworkRequest(
    QNetworkRequest *networkRequest,
    QIODevice *data
    )
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkRequest->url());

    switch(api)
    {
    case Dropbox::FILES:
    case Dropbox::FILESPUT:
        break;

    default:
        // show animation
        previousPage = ui->stackedWidget->currentWidget();
        setCurrentPage(ui->loadingPage);
        break;
    }

    QNetworkReply *networkReply = Common::networkAccessManager->put(
        *networkRequest,
        data
        );

    switch(api)
    {
    case Dropbox::FILESPUT:
        ui->fileTransfersPage->setUploadNetworkReply(networkReply);
        break;

    default:
        break;
    }
}

void MainWindow::handleNetworkReply(QNetworkReply *networkReply)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkReply->url());
    switch(api)
    {
    case Dropbox::FILES:
    case Dropbox::FILESPUT:
        ui->fileTransfersPage->handleNetworkReply(networkReply);
        break;

    default:
        // stop animation
        setCurrentPage(previousPage);
        break;
    }

    networkReply->deleteLater();

    if(networkReply->error() != QNetworkReply::NoError &&
       networkReply->error() != QNetworkReply::OperationCanceledError)
    {
        QString replyData = networkReply->readAll();
        QVariantMap jsonResult = QtJson::Json::parse(replyData).toMap();

        if(jsonResult.contains("error"))
        {
            if(jsonResult["error"].type() != QVariant::Map)
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    jsonResult["error"].toString()
                    );
                return;
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    QString(
                        jsonResult["error"].toMap().keys().first() +
                        ": " +
                        jsonResult["error"].toMap()
                                .values().first().toString()
                        )
                    );
                return;
            }
        }
        else
        {
            if(!replyData.isEmpty())
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    replyData
                );
                return;
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    networkReply->errorString()
                    );
                return;
            }
        }
    }

    switch(api)
    {
    case Dropbox::OAUTH_REQUESTTOKEN:
    case Dropbox::OAUTH_ACCESSTOKEN:
        ui->signInPage->handleNetworkReply(networkReply);
        break;

    case Dropbox::METADATA:
    case Dropbox::FILEOPS_MOVE:
    case Dropbox::FILEOPS_COPY:
    case Dropbox::FILEOPS_DELETE:
    case Dropbox::FILEOPS_CREATEFOLDER:
    case Dropbox::SHARES:
        ui->navigationPage->handleNetworkReply(networkReply);
        break;

    case Dropbox::ACCOUNT_INFO:
        ui->accountInfoPage->handleNetworkReply(networkReply);
        break;

    default:
        break;
    }
}

void MainWindow::setupActions()
{
    QMenu *helpMenu = new QMenu(this);
    helpMenu->addAction(ui->aboutAction);
    helpMenu->addAction(ui->aboutQtAction);
    QAction *helpMenuAction = new QAction("Help", this);
    helpMenuAction->setMenu(helpMenu);
    this->addAction(helpMenuAction);
}

void MainWindow::attemptSignIn()
{
    if( !Common::userData->token.isEmpty() &&
        !Common::userData->secret.isEmpty() &&
        !Common::userData->uid.isEmpty() )
    {
        // sign in
        setCurrentPage(ui->navigationPage);
        ui->navigationPage->requestMetadata("/");
    }
}

void MainWindow::signOut()
{
    // remove old user data
    QSettings settings;
    settings.remove("user");
    Common::userData->token.clear();
    Common::userData->secret.clear();

    // return to the authentication page
    setCurrentPage(ui->signInPage);
}

void MainWindow::download(QVariantMap fileInfo)
{
    if(fileInfo["is_dir"].toBool() == true)
    {
        QMessageBox::information(
            this,
            "Droper",
            "This is a folder and it's currently only possible to "
            "download files."
            );

        return;
    }

    if(ui->fileTransfersPage->isDownloading())
    {
        QMessageBox::information(
            this,
            "Droper",
            "Another file is being downloaded."
            );

        return;
    }

    // initial download path
    QSettings settings;
    QString initialPath = settings.value(
        "config/last_download_path",
        QDesktopServices::storageLocation(
            QDesktopServices::DesktopLocation
            )
        ).toString();

    // prepare folder
    QString folderPath;
    while(true)
    {
        folderPath.clear();

        folderPath = QFileDialog::getExistingDirectory(
            0,
            "Droper",
            initialPath
            );

        // if no directory selected, do nothing
        if(folderPath.isEmpty())
        {
            return;
        }

        QString filePath = fileInfo["path"].toString();
        QString fileName = filePath.right(
            (filePath.length() -
             filePath.lastIndexOf("/")) - 1
            );
        if(QFile(folderPath + "/" + fileName).exists())
        {
            QMessageBox::information(
                this,
                "Droper",
                QString(
                    "This directory already has a file "
                    "named '%1'. Choose another one."
                    ).arg(fileName)
                );
        }
        else
        {
            break;
        }
    }

    settings.setValue("config/last_download_path", folderPath);

    ui->fileTransfersPage->setDownloadFileAndFolderInformation(
        fileInfo,
        folderPath
        );

    // start download
    ui->fileTransfersPage->setDownloadState(FileTransfersPage::TRANSFERRING);

    switchToFileTransfers();
}

void MainWindow::upload(QString directory)
{
    if(ui->fileTransfersPage->isUploading())
    {
        QMessageBox::information(
            this,
            "Droper",
            "Another file is being uploaded."
            );

        return;
    }

    // initial upload path
    QSettings settings;
    QString initialPath = settings.value(
        "config/last_upload_path",
        QDesktopServices::storageLocation(
            QDesktopServices::DesktopLocation
            )
        ).toString();

    // prepare file
    bool ok = false;
    QString filePath;
    while(!ok)
    {
        filePath.clear();

        filePath = QFileDialog::getOpenFileName(
            0,
            "Droper",
            initialPath
            );

        // if no file selected, do nothing
        if(filePath.isEmpty())
        {
            return;
        }

        QString fileName = filePath.right(
            (filePath.length() - filePath.lastIndexOf("/")) - 1
            );

        // these symbols aren't allowed by Dropbox
        QRegExp disallowedSymbols("[/:?*<>\"|]");
        if(fileName.contains(disallowedSymbols) ||
            fileName.contains("\\") || fileName == "." || fileName == ".."
            )
        {
            QMessageBox::information(
                this,
                "Droper",
                "The following characters aren't allowed by Dropbox: \n"
                "\\ / : ? * < > \" | \n"
                "And you can't name a file or folder . or .."
                );
        }
        else
        {
            ok = true;
        }
    }

    // compute fileSize
    QFileInfo fileInfo(filePath);
    int fileBytes = fileInfo.size();

    // make sure the file size is smaller than the upload file limit
    if(fileBytes > 150000000)
    {
        QMessageBox::critical(
            this,
            "Droper",
            "The file size is larger than the limit, upload it using the "
            " Dropbox desktop client."
            );

        return;
    }

    // make sure the current directory is loaded
    QVariantMap metadataOfCurrentDirectory =
        ui->navigationPage->getMetadataOfCurrentDirectory();
    if(metadataOfCurrentDirectory.isEmpty())
    {
        QMessageBox::critical(
            this,
            "Droper",
            "The current directory must be loaded before uploading files. "
            "Refresh or open another directory and try again."
            );

        return;
    }

    // make sure there isn't any folder that has the same name that the
    // file to be uploaded has and if there was a file with the same name,
    // tell the user that this upload modifies that file and that the
    // revision history can be viewed at dropbox.com
    bool overwrite = true;
    foreach(const QVariant &itemJson,
            metadataOfCurrentDirectory["contents"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();
        QString itemPath = itemMap["path"].toString();
        QString itemName = itemPath.right(
            (itemPath.length() - itemPath.lastIndexOf("/")) - 1
            );
        QString fileName = filePath.right(
            (filePath.length() - filePath.lastIndexOf("/")) - 1
            );
        if(itemName.toLower() == fileName.toLower())
        {
            if(itemMap["is_dir"].toBool() == true)
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    "There is a folder with the same name in the current "
                    "directory."
                    );

                return;
            }
            else
            {
                QMessageBox::StandardButton result = QMessageBox::question(
                    this,
                    "Droper",
                    "There is a file with the same name in the current "
                    "directory. Do you want to rename the file you are "
                    "trying to upload? (Otherwise the remote file will be "
                    "overwritten, but you can restore it later on "
                    "dropbox.com)",
                    QMessageBox::Yes|QMessageBox::No,
                    QMessageBox::Yes
                    );
                if(result == QMessageBox::Yes)
                    overwrite = false;
            }
        }
    }

    settings.setValue("config/last_upload_path", filePath);

    ui->fileTransfersPage->setUploadFileAndFolderInformation(
        filePath,
        Util::bytesToString(fileBytes),
        fileBytes,
        directory,
        overwrite
        );

    ui->fileTransfersPage->setUploadState(FileTransfersPage::TRANSFERRING);

    switchToFileTransfers();
}

void MainWindow::switchToNavigation()
{
    setCurrentPage(ui->navigationPage);
}

void MainWindow::switchToAccountInfo()
{
    setCurrentPage(ui->accountInfoPage);
    ui->accountInfoPage->requestAccountInfo();
}

void MainWindow::switchToFileTransfers()
{
    setCurrentPage(ui->fileTransfersPage);
}

void MainWindow::setCurrentPage(QWidget *page)
{
    ui->menubar->clear();
    ui->menubar->addActions(page->actions());
    ui->menubar->addActions(this->actions());

    ui->stackedWidget->setCurrentWidget(page);

    // I don't know why this is necessary for softkey actions
    if(Util::s60v3())
    {
        if(page == ui->navigationPage)
            ui->navigationPage->focusFilesAndFoldersListWidget();
    }
}

void MainWindow::on_aboutAction_triggered()
{
    QMessageBox messageBox(this);

    messageBox.setWindowTitle("Droper");

    messageBox.setText(
        "<b>Droper v0.5.1</b> - "
        "<a href=\"http://www.dropbox.com/\">Dropbox</a> Client"
        "<br>"
        "(unstable testing version)"
        );

    messageBox.setInformativeText(
        "Copyright 2011, 2012 Omar Lawand Dalatieh <br><br>"

        "Licensed under the GNU GPLv3 license <br><br>"

        "<a href=\"http://coinonedge.com/droper/\">"
        "http://coinonedge.com/droper/"
        "</a>"
        );

    QIcon droper(":/droper.svg");
    messageBox.setIconPixmap(droper.pixmap(QSize(32, 32)));

    messageBox.exec();
}

void MainWindow::on_aboutQtAction_triggered()
{
    qApp->aboutQt();
}
