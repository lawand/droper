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
#include "filetransferspage.h"
#include "ui_filetransferspage.h"

// data members
#include <QNetworkReply>

// implementation-specific
#include <QMessageBox>
#include <QNetworkRequest>
#include <QMenu>
#include "common.h"
#include "dropbox.h"
#include "oauth.h"
#include "util.h"
#include "json.h"

FileTransfersPage::FileTransfersPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileTransfersPage)
{
    // private data members initialization
    ui->setupUi(this);

    // actions setup
    setupActions();

    // initialize the state
    setDownloadState(INITIAL);
    setUploadState(INITIAL);
}

FileTransfersPage::~FileTransfersPage()
{
    delete ui;
}

bool FileTransfersPage::isDownloading()
{
    if(downloadState == TRANSFERRING)
        return true;
    else
        return false;
}

bool FileTransfersPage::isUploading()
{
    if(uploadState == TRANSFERRING)
        return true;
    else
        return false;
}

void FileTransfersPage::setDownloadFileAndFolderInformation(
    QVariantMap fileInfo,
    QString folderPath
    )
{
    // file information
    downloadFilePath = fileInfo["path"].toString();
    downloadFileName = downloadFilePath.right(
        (downloadFilePath.length() - downloadFilePath.lastIndexOf("/")) - 1
        );
    downloadFileSize = fileInfo["size"].toString();

    // folder information
    downloadFolderPath = folderPath;
    downloadFolderName = downloadFolderPath.right(
        (downloadFolderPath.length() - downloadFolderPath.lastIndexOf("/")) - 1
        );
    if(downloadFolderName.isEmpty())
        downloadFolderName = "/ (root)";
}

void FileTransfersPage::setUploadFileAndFolderInformation(
    QString filePath,
    QString fileSize,
    int fileBytes,
    QString folderPath,
    bool overwrite
    )
{
    // file information
    uploadFilePath = filePath;
    uploadFileName = uploadFilePath.right(
        (uploadFilePath.length() - uploadFilePath.lastIndexOf("/")) - 1
        );
    uploadFileSize = fileSize;
    uploadFileBytes = fileBytes;

    // folder information
    uploadFolderPath = folderPath;
    uploadFolderName = uploadFolderPath.right(
        (uploadFolderPath.length() - uploadFolderPath.lastIndexOf("/")) - 1
        );
    if(uploadFolderName.isEmpty())
        uploadFolderName = "/ (root)";

    // overwrite
    ovewriteWhenUploading = overwrite;
}

void FileTransfersPage::setDownloadState(FileTransfersPage::State state)
{
    downloadState = state;

    switch(downloadState)
    {
    case INITIAL:
        ui->downloadInfoLabel->setText("---");
        ui->downloadProgressBar->setValue(0);
        ui->downloadProgressBar->setFormat("No Active Download");
        ui->downloadStartStopRestartAction->setVisible(false);
        if(!Util::s60v3())
        {
            ui->downloadStartStopRestartToolButton->setVisible(false);
        }
        break;

    case TRANSFERRING:
        ui->downloadInfoLabel->setText(
            QString("%1 (%2) to %3")
                .arg(downloadFileName)
                .arg(downloadFileSize)
                .arg(downloadFolderName)
            );
        ui->downloadProgressBar->setValue(0);
        ui->downloadProgressBar->setFormat("%p% Downloading");
        ui->downloadStartStopRestartAction->setText("Stop Download");
        ui->downloadStartStopRestartAction->setVisible(true);
        if(!Util::s60v3())
        {
            ui->downloadStartStopRestartToolButton->setVisible(true);
        }
        downloadTime.start();

        // prepare the local file
        downloadFile.setFileName(downloadFolderPath + "/" + downloadFileName);
        // check whether the file can be opened for writing,
        // opening it in the process
        if(!downloadFile.open(QFile::WriteOnly))
        {
            QMessageBox::critical(
                this,
                "Droper",
                "Failed to open the file for writing."
                );

            setDownloadState(NOT_FINISHED);

            return;
        }

        // request the content of the remote file
        {
            QUrl url = Common::dropbox->apiToUrl(
                Dropbox::FILES
                ).toString() + downloadFilePath;
            QNetworkRequest networkRequest(url);
            Common::oAuth->signRequestHeader("GET", &networkRequest);
            emit networkRequestGetNeeded(&networkRequest);
        }

        break;

    case FINISHED:
        ui->downloadProgressBar->setFormat("Finished");
        ui->downloadStartStopRestartAction->setText("---");
        ui->downloadStartStopRestartAction->setVisible(false);
        if(!Util::s60v3())
        {
            ui->downloadStartStopRestartToolButton->setVisible(false);
        }
        downloadFile.close();
        break;

    case NOT_FINISHED:
        ui->downloadProgressBar->setFormat("Not Finished");
        ui->downloadStartStopRestartAction->setText("Restart Download");
        ui->downloadStartStopRestartAction->setVisible(true);
        if(!Util::s60v3())
        {
            ui->downloadStartStopRestartToolButton->setVisible(true);
        }
        downloadFile.remove();
        break;

    default:
        break;
    }
}

void FileTransfersPage::setUploadState(FileTransfersPage::State state)
{
    uploadState = state;

    switch(uploadState)
    {
    case INITIAL:
        ui->uploadInfoLabel->setText("---");
        ui->uploadProgressBar->setValue(0);
        ui->uploadProgressBar->setFormat("No Active Upload");
        ui->uploadStartStopRestartAction->setVisible(false);
        if(!Util::s60v3())
        {
            ui->uploadStartStopRestartToolButton->setVisible(false);
        }
        break;

    case TRANSFERRING:
        {
            ui->uploadInfoLabel->setText(
                QString("%1 (%2) to %3")
                    .arg(uploadFileName)
                    .arg(uploadFileSize)
                    .arg(uploadFolderName)
                );
            ui->uploadProgressBar->setValue(0);
            ui->uploadProgressBar->setFormat("%p% Uploading");
            ui->uploadStartStopRestartAction->setText("Stop Upload");
            ui->uploadStartStopRestartAction->setVisible(true);
            if(!Util::s60v3())
            {
                ui->uploadStartStopRestartToolButton->setVisible(true);
            }
            uploadTime.start();

            // prepare the local file
            uploadFile = new QFile(uploadFilePath);

            // check whether the file can be opened for reading,
            // opening it in the process
            if(!uploadFile->open(QFile::ReadOnly))
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    "Failed to open the file for reading."
                    );

                setUploadState(NOT_FINISHED);

                return;
            }

            // send the content of the local file
            QUrl url;
            if(uploadFolderPath != "/")
            {
                url = Common::dropbox->apiToUrl(
                    Dropbox::FILESPUT
                    ).toString() + uploadFolderPath + "/" + uploadFileName;
            }
            else
            {
                url = Common::dropbox->apiToUrl(
                    Dropbox::FILESPUT
                    ).toString() + uploadFolderPath + uploadFileName;
            }
            if(ovewriteWhenUploading == true)
                url.addQueryItem("overwrite", "true");
            else
                url.addQueryItem("overwrite", "false");
            QNetworkRequest networkRequest(url);
            Common::oAuth->signRequestHeader("PUT", &networkRequest);
            networkRequest.setRawHeader(
                "Content-Length",
                QString("%1").arg(uploadFileBytes).toAscii()
                );
            emit networkRequestPutNeeded(&networkRequest, uploadFile);
        }

        break;

    case FINISHED:
        ui->uploadProgressBar->setFormat("Finished");
        ui->uploadStartStopRestartAction->setText("---");
        ui->uploadStartStopRestartAction->setVisible(false);
        if(!Util::s60v3())
        {
            ui->uploadStartStopRestartToolButton->setVisible(false);
        }
        delete uploadFile;
        break;

    case NOT_FINISHED:
        ui->uploadProgressBar->setFormat("Not Finished");
        ui->uploadStartStopRestartAction->setText("Restart Upload");
        ui->uploadStartStopRestartAction->setVisible(true);
        if(!Util::s60v3())
        {
            ui->uploadStartStopRestartToolButton->setVisible(true);
        }
        delete uploadFile;
        break;

    default:
        break;
    }
}

void FileTransfersPage::setDownloadNetworkReply(QNetworkReply *networkReply)
{
    downloadNetworkReply = networkReply;

    connect(
        downloadNetworkReply,
        SIGNAL(readyRead()),
        SLOT(handleDownloadReadyRead())
        );
    connect(
        downloadNetworkReply,
        SIGNAL(downloadProgress(qint64, qint64)),
        SLOT(handleDownloadProgress(qint64, qint64))
        );
}

void FileTransfersPage::setUploadNetworkReply(QNetworkReply *networkReply)
{
    uploadNetworkReply = networkReply;

    connect(
        uploadNetworkReply,
        SIGNAL(uploadProgress(qint64, qint64)),
        SLOT(handleUploadProgress(qint64, qint64))
        );
}

void FileTransfersPage::setupActions()
{
    if(Util::s60v3())
    {
        ui->downloadStartStopRestartToolButton->setVisible(false);
        ui->uploadStartStopRestartToolButton->setVisible(false);

        this->addAction(ui->downloadStartStopRestartAction);
        this->addAction(ui->uploadStartStopRestartAction);
    }
    else
    {
        ui->downloadStartStopRestartToolButton->setDefaultAction(
            ui->downloadStartStopRestartAction
            );
        ui->uploadStartStopRestartToolButton->setDefaultAction(
            ui->uploadStartStopRestartAction
            );
    }

    this->addAction(ui->backAction);
}

void FileTransfersPage::handleDownloadReadyRead()
{
    downloadFile.write(downloadNetworkReply->readAll());
}

void FileTransfersPage::handleDownloadProgress(qint64 received, qint64 total)
{
    // avoid errors
    if (received == 0 || total == 0 || downloadTime.elapsed() == 0)
        return;

    // update progress bar
    ui->downloadProgressBar->setFormat(
        QString("%p% Down. at %1").arg(
            Util::bytesToString(
                double(received * 1000.0) / downloadTime.elapsed()
                )
            )
        );
    ui->downloadProgressBar->setValue( (received*100) / total );
}

void FileTransfersPage::handleUploadProgress(qint64 sent, qint64 total)
{
    // avoid errors
    if (sent == 0 || total == 0 || uploadTime.elapsed() == 0)
        return;

    // update progress bar
    ui->uploadProgressBar->setFormat(
        QString("%p% Upl. at %1").arg(
            Util::bytesToString(double(sent * 1000.0) / uploadTime.elapsed())
            )
        );
    ui->uploadProgressBar->setValue( (sent*100) / total );
}

void FileTransfersPage::handleNetworkReply(QNetworkReply *networkReply)
{
    if(networkReply == downloadNetworkReply)
    {
        if(networkReply->error() != QNetworkReply::NoError)
            setDownloadState(NOT_FINISHED);
        else
            setDownloadState(FINISHED);
    }
    else if(networkReply == uploadNetworkReply)
    {
        if(networkReply->error() != QNetworkReply::NoError)
        {
            setUploadState(NOT_FINISHED);
        }
        else
        {
            setUploadState(FINISHED);
            QVariantMap metadataMap = QtJson::Json::parse(
                networkReply->readAll()
                ).toMap();
            emit fileUploaded(metadataMap);
        }
    }
}

void FileTransfersPage::on_downloadStartStopRestartAction_triggered()
{
    switch(downloadState)
    {
    case TRANSFERRING:
        downloadNetworkReply->abort();
        break;

    case NOT_FINISHED:
        setDownloadState(TRANSFERRING);
        break;

    default:
        break;
    }
}

void FileTransfersPage::on_uploadStartStopRestartAction_triggered()
{
    switch(uploadState)
    {
    case TRANSFERRING:
        uploadNetworkReply->abort();
        break;

    case NOT_FINISHED:
        setUploadState(TRANSFERRING);
        break;

    default:
        break;
    }
}

void FileTransfersPage::on_backAction_triggered()
{
    emit backRequested();
}
