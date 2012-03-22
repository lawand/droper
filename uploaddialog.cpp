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

//corresponding headers
#include "uploaddialog.h"
#include "ui_uploaddialog.h"

//data members
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "dropbox.h"
#include "oauth.h"
#include "userdata.h"

//implementation-specific
#include <QMessageBox>

UploadDialog::UploadDialog(
    QNetworkAccessManager *networkAccessManager,
    Dropbox *dropbox,
    OAuth *oAuth,
    UserData *userData,
    QWidget *parent
    ) :
    QDialog(parent),
    ui(new Ui::UploadDialog),
    startStopRestartAction(new QAction("Start", this)),
    closeAction(new QAction("Close", this)),
    overwrite(true)
{
    //shared data members initialization
    this->networkAccessManager = networkAccessManager;
    this->dropbox = dropbox;
    this->oAuth = oAuth;
    this->userData = userData;

    //private data members initialization
    ui->setupUi(this);
    startStopRestartAction->setSoftKeyRole(QAction::PositiveSoftKey);
    this->addAction(startStopRestartAction);
    connect(
        startStopRestartAction,
        SIGNAL(triggered()),
        SLOT(startStopRestart())
        );
    closeAction->setSoftKeyRole(QAction::NegativeSoftKey);
    this->addAction(closeAction);
    connect(closeAction, SIGNAL(triggered()), SLOT(close()));

    //initialize the state
    setState(UploadDialog::INITIAL);
}

UploadDialog::~UploadDialog()
{
    delete ui;
}

void UploadDialog::setFileAndFolderInformation(
    QString filePath,
    QString fileSize,
    int fileBytes,
    QString folderPath,
    bool overwrite
    )
{
    //file information
    this->filePath = filePath;
    fileName = this->filePath.right(
        (this->filePath.length() - this->filePath.lastIndexOf("/")) - 1
        );
    this->fileSize = fileSize;
    this->fileBytes = fileBytes;

    //folder information
    this->folderPath = folderPath;
    folderName = this->folderPath.right(
        (this->folderPath.length() - this->folderPath.lastIndexOf("/")) - 1
        );
    if(folderName.isEmpty())
        folderName = "/ (root)";

    //overwrite
    this->overwrite = overwrite;

    //update the state
    setState(UploadDialog::READY_TO_START);
}

void UploadDialog::startStopRestart()
{
    switch(state)
    {
    case READY_TO_START:
        setState(UploadDialog::UPLOADING);
        break;

    case UPLOADING:
        networkReply->abort();
        break;

    case NOT_FINISHED:
        setState(UploadDialog::UPLOADING);
        break;

    default:
        break;
    }
}

void UploadDialog::setState(UploadDialog::State state)
{
    this->state = state;

    switch(this->state)
    {
    case INITIAL:
        ui->fileNameAndSizeLabel->setText("---");
        ui->folderNameLabel->setText("---");
        ui->progressBar->setValue(-1);
        ui->progressBar->setFormat("%p%");
        startStopRestartAction->setVisible(false);
        break;

    case READY_TO_START:
        ui->fileNameAndSizeLabel->setText(
            QString("%1 (%2)").arg(fileName).arg(fileSize)
            );
        ui->folderNameLabel->setText(folderName);
        ui->progressBar->setValue(0);
        ui->progressBar->setFormat("Ready To Start");
        startStopRestartAction->setText("Start");
        startStopRestartAction->setVisible(true);
        break;

    case UPLOADING:
        {
            ui->progressBar->setValue(0);
            ui->progressBar->setFormat("%p% Uploading");
            startStopRestartAction->setText("Stop");
            startStopRestartAction->setVisible(true);
            uploadTime.start();

            //prepare the local file
            file = new QFile(filePath);

            //check whether the file can be opened for reading,
            //opening it in the process
            if(!file->open(QFile::ReadOnly))
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    "Failed to open the file for reading."
                    );

                setState(UploadDialog::NOT_FINISHED);

                return;
            }

            //send the content of the local file
            QUrl url;
            if(folderPath != "/")
            {
                url = dropbox->apiToUrl(
                    Dropbox::FILESPUT
                    ).toString() + folderPath + "/" + fileName;
            }
            else
            {
                url = dropbox->apiToUrl(
                    Dropbox::FILESPUT
                    ).toString() + folderPath + fileName;
            }
            if(overwrite == true)
                url.addQueryItem("overwrite", "true");
            else
                url.addQueryItem("overwrite", "false");
            QNetworkRequest networkRequest(url);
            oAuth->signRequestHeader("PUT", &networkRequest, userData);
            networkRequest.setRawHeader(
                "Content-Length",
                QString("%1").arg(fileBytes).toAscii()
                );
            networkReply = networkAccessManager->put(
                networkRequest,
                file
                );
            connect(
                networkReply,
                SIGNAL(uploadProgress(qint64, qint64)),
                SLOT(handleUploadProgress(qint64, qint64))
                );
        }

        break;

    case FINISHED:
        ui->progressBar->setFormat("Finished");
        startStopRestartAction->setVisible(false);
        delete file;
        emit itemUploadedToDirectory(folderPath);
        break;

    case NOT_FINISHED:
        ui->progressBar->setFormat("Not Finished");
        startStopRestartAction->setText("Restart");
        startStopRestartAction->setVisible(true);
        delete file;
        break;

    default:
        break;
    }
}

bool UploadDialog::isUploading()
{
    if(state == UploadDialog::UPLOADING)
        return true;
    else
        return false;
}

void UploadDialog::handleUploadProgress(qint64 sent, qint64 total)
{
    //avoid errors
    if (sent == 0 || total == 0 || uploadTime.elapsed() == 0)
        return;

    //calculate upload speed
    double speed = sent * 1000.0 / uploadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bit/s";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "Kibit/s";
    } else {
        speed /= 1024*1024;
        unit = "Mibit/s";
    }

    //update progress bar
    ui->progressBar->setFormat(
        QString("%p% Upl. at %1%2").arg(speed, 0, 'f', 1).arg(unit)
        );
    ui->progressBar->setValue( (sent*100) / total );
}

void UploadDialog::handleNetworkReply(QNetworkReply *networkReply)
{
    if(networkReply != this->networkReply)
        return;

    if(networkReply->error() != QNetworkReply::NoError)
        setState(UploadDialog::NOT_FINISHED);
    else
        setState(UploadDialog::FINISHED);
}
