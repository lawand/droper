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
#include "downloaddialog.h"
#include "ui_downloaddialog.h"

//data members
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "oauth.h"
#include "userdata.h"
#include "dropbox.h"

//implementation-specific
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>

DownloadDialog::DownloadDialog(
        QNetworkAccessManager* networkAccessManager,
        OAuth* oAuth,
        UserData* userData,
        Dropbox* dropbox,
        QWidget *parent
        ) :
    QDialog(parent),
    ui(new Ui::DownloadDialog),
    active(false),
    remoteFile(""),
    networkReply(0),
    remoteFileBytes(0)
{
    //member initialization
    this->networkAccessManager = networkAccessManager;
    this->oAuth = oAuth;
    this->userData = userData;
    this->dropbox = dropbox;

    //GUI initialization
    ui->setupUi(this);

    //initial connections
    connect(ui->buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(ui->toggleStartAction, SIGNAL(triggered()), SLOT(toggleStart()));
    connect(
            ui->toggleStartPushButton,
            SIGNAL(clicked()),
            ui->toggleStartAction,
            SLOT(trigger())
            );

#ifdef Q_OS_SYMBIAN
    ui->toggleStartAction->setSoftKeyRole(QAction::PositiveSoftKey);
    ui->toggleStartPushButton->setVisible(false);
    addAction(ui->toggleStartAction);
#endif

    //initial state
    initialize();
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}

void DownloadDialog::setFileAndFolder(
        QVariantMap* fileMap,
        QString localFolder
        )
{
    this->localFolder = localFolder;
    QString localFolderName = localFolder.right(
            (localFolder.length() - localFolder.lastIndexOf("/")) - 1
            );
    if(localFolderName == "")
        localFolderName = "/";

    remoteFile = (*fileMap)["path"].toString();
    QString remoteFileName = remoteFile.right(
            (remoteFile.length() -
             remoteFile.lastIndexOf("/")) - 1
            );
    remoteFileBytes = (*fileMap)["bytes"].toLongLong();
    QString remoteFileSize = (*fileMap)["size"].toString();

    //update variables
    ui->fileLabel->setText(
            QString("%1 (%2)").arg(remoteFileName).arg(remoteFileSize)
                    );
    ui->folderLabel->setText(localFolderName);
    ui->progressBar->setFormat("%p%");
    ui->stateLabel->setText("Ready to start");
    ui->toggleStartPushButton->setEnabled(true);
    ui->toggleStartAction->setEnabled(true);
}

bool DownloadDialog::isActive()
{
    return active;
}

void DownloadDialog::initialize()
{
    //set variables
    ui->fileLabel->setText("---");
    ui->folderLabel->setText("---");
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat("");
    ui->stateLabel->setText("---");
    ui->toggleStartPushButton->setEnabled(false);
    ui->toggleStartAction->setEnabled(false);
    ui->toggleStartPushButton->setText("Start");
    ui->toggleStartAction->setText("Start");
    active = false;
    remoteFile = "";
    localFolder = "";
    remoteFileBytes = 0;
}

void DownloadDialog::reject()
{
    if(!active)
    {
        initialize();
    }

    QDialog::reject();
}

void DownloadDialog::toggleStart()
{
    if(!active)
    {
        QString remoteFileName = remoteFile.right(
                (remoteFile.length() -
                 remoteFile.lastIndexOf("/")) - 1
                );

        file.setFileName(localFolder + "/" + remoteFileName);

        //check whether the file can be opened for writing,
        //opening it in the process
        if(!file.open(QFile::WriteOnly | QFile::Append))
        {
            QMessageBox::warning(
                    this,
                    "Droper",
                    "Failed to open the file for writing."
                    );

            return;
        }

        QUrl url =
                dropbox->apiToUrl(Dropbox::FILES).toString() +
                remoteFile
                ;

        QNetworkRequest networkRequest(url);

        oAuth->signRequest(
                userData,
                "GET",
                &networkRequest
                );

        networkReply = networkAccessManager->get( networkRequest );
        connect(networkReply, SIGNAL(readyRead()), SLOT(handleReadyRead()));
        connect(
                networkReply,
                SIGNAL(downloadProgress(qint64,qint64)),
                SLOT(handleDownloadProgress(qint64,qint64))
                );
        connect(
                networkReply,
                SIGNAL(finished()),
                SLOT(handleFinished())
                );

        //update variables
        ui->stateLabel->setText("Starting...");
        ui->toggleStartPushButton->setText("Cancel");
        ui->toggleStartAction->setText("Cancel");
        active = true;
        downloadTime.start();
    }
    else
    {
        //set variables
        ui->progressBar->setValue(0);
        ui->stateLabel->setText("Ready to start");
        ui->toggleStartPushButton->setText("Start");
        ui->toggleStartAction->setText("Start");
        active = false;
        file.remove();
        networkReply->abort();
        networkReply->deleteLater();
    }
}

void DownloadDialog::handleReadyRead()
{
    file.write(networkReply->readAll());
}

void DownloadDialog::handleDownloadProgress(qint64 received, qint64 total)
{
    //avoid errors
    if (received == 0 || total == 0 || downloadTime.elapsed() == 0)
        return;

    // calculate the download speed
    double speed = received * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    //update variables
    ui->progressBar->setValue((received*100)/total);
    ui->stateLabel->setText(
            QString("Downloading at %1%2").arg(speed, 3, 'f', 1).arg(unit)
            );
}

void DownloadDialog::handleFinished()
{
    //update variables
    file.close();
    networkReply->deleteLater();

    if(networkReply->error() != QNetworkReply::NoError ||
       QFileInfo(file).size() != remoteFileBytes)
    {
        //if the operation was canceled, do nothing
        if(networkReply->error() == QNetworkReply::OperationCanceledError)
            return;

        //notify the user
        QMessageBox::critical(
                this,
                "Droper",
                "File download error!"
                );
        show();

        //update variables
        ui->progressBar->setValue(0);
        ui->stateLabel->setText("Ready to start");
        ui->toggleStartPushButton->setText("Start");
        ui->toggleStartAction->setText("Start");
        active = false;
        file.remove();
    }
    else
    {
        //compute remoteFileName and localFolderName
        QString remoteFileName = remoteFile.right(
                (remoteFile.length() -
                 remoteFile.lastIndexOf("/")) - 1
                );
        QString localFolderName = localFolder.right(
                (localFolder.length() - localFolder.lastIndexOf("/")) - 1
                );
        if(localFolderName == "")
            localFolderName = "/";

        //notify the user
        QMessageBox::information(
                this,
                "Droper",
                QString(
                        "The file '%1' was successfully downloaded to the "
                        "folder '%2'."
                        ).arg(remoteFileName).arg(localFolderName)
                );

        //return to initial state
        initialize();
    }
}
