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
#include "uploaddialog.h"
#include "ui_uploaddialog.h"

//data members
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QFile>
#include "oauth.h"
#include "userdata.h"
#include "dropbox.h"

//implementation-specific
#include <QFileInfo>
#include <QMessageBox>

UploadDialog::UploadDialog(
    QNetworkAccessManager* networkAccessManager,
    OAuth* oAuth,
    UserData* userData,
    Dropbox* dropbox,
    QWidget *parent
    ) :
    QDialog(parent),
    ui(new Ui::UploadDialog),
    networkReply(0),
    active(false)
{
    //member initialization
    this->networkAccessManager = networkAccessManager;
    this->oAuth = oAuth;
    this->userData = userData;
    this->dropbox = dropbox;

    //GUI initialization
    ui->setupUi(this);

    //initial connections
    connect(
        ui->buttonBox,
        SIGNAL(rejected()),
        SLOT(reject())
        );
    connect(
        ui->toggleStartPushButton,
        SIGNAL(clicked()),
        ui->toggleStartAction,
        SLOT(trigger())
        );
    connect(ui->toggleStartAction, SIGNAL(triggered()), SLOT(toggleStart()));

#ifdef Q_OS_SYMBIAN
    ui->toggleStartAction->setSoftKeyRole(QAction::PositiveSoftKey);
    ui->toggleStartPushButton->setVisible(false);
    addAction(ui->toggleStartAction);
#endif

    //initial state
    initialize();
}

UploadDialog::~UploadDialog()
{
    delete ui;
}

void UploadDialog::setFileAndFolder(
    QString localFile,
    QString remoteFolder
    )
{
    this->localFile = localFile;
    this->remoteFolder = remoteFolder;

    QFileInfo fileInfo(this->localFile);
    int bytes = fileInfo.size();

    qreal size = (qreal)bytes;
    QString sizeUnit;
    if (size < 1024) {
        sizeUnit = "bytes";
    } else if (size < 1024*1024) {
        size /= 1024;
        sizeUnit = "kB";
    } else if (size < 1024*1024*1024){
        size /= 1024*1024;
        sizeUnit = "MB";
    } else {
        size /= 1024*1024*1024;
        sizeUnit = "GB";
    }
    QString sizeString = QString("%1%2")
        .arg(size, 0, 'f', 1)
        .arg(sizeUnit)
        ;

    QString remoteFolderName = remoteFolder.right(
        (remoteFolder.length()-remoteFolder.lastIndexOf("/"))-1
        );
    if(remoteFolderName == "")  //the case of the remoteFolder being "/"
        remoteFolderName = "Dropbox (root)";

    //update variables
    ui->fileNameAndSizeLabel->setText(
        QString("%1 (%2)").arg(fileInfo.fileName()).arg(sizeString)
        );
    ui->folderLabel->setText(remoteFolderName);
    ui->progressBar->setFormat("%p%");
    ui->stateLabel->setText("Ready to start");
    ui->toggleStartPushButton->setEnabled(true);
    ui->toggleStartAction->setEnabled(true);

    resize(sizeHint());
}

bool UploadDialog::isActive()
{
    return active;
}

void UploadDialog::initialize()
{
    //update variables
    ui->fileNameAndSizeLabel->setText("---");
    ui->folderLabel->setText("---");
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat("");
    ui->stateLabel->setText("---");
    ui->toggleStartPushButton->setEnabled(false);
    ui->toggleStartAction->setEnabled(false);
    ui->toggleStartPushButton->setText("Start");
    ui->toggleStartAction->setText("Start");
    networkReply = 0;
    active = false;
}

void UploadDialog::reject()
{
    if(!active)
    {
        initialize();
    }

    QDialog::reject();
}

void UploadDialog::toggleStart()
{
    if(active == false)
    {
        //compute localFileName
            QString localFileName = localFile.right(
                (localFile.length()-localFile.lastIndexOf("/"))-1
                );

        //prepare binary data
            multipartform = new QByteArray();
            QString crlf("\r\n");
            QString boundaryStr(
                "---------------------------109074266748897678777839994"
                );
            QString boundary="--"+boundaryStr+crlf;
            multipartform->append(boundary.toAscii());
            multipartform->append(
                QString("Content-Disposition: form-data; name=\"file\"; "
                    "filename=\"" + localFileName.toUtf8() + "\"" + crlf
                    ).toAscii()
                );
            multipartform->append(
                QString("Content-Type: text/plain" + crlf + crlf).toAscii()
                );
            file = new QFile(localFile);
            if(file->open(QIODevice::ReadOnly) == false)
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    "Can't open the file for reading!"
                    );

                return;
            }
            multipartform->append(file->readAll());
            file->close();
            multipartform->append(
                QString(crlf + "--" + boundaryStr + "--" + crlf).toAscii()
                );

        //prepare request
            QUrl url =
                dropbox->apiToUrl(Dropbox::FILES).toString() +
                remoteFolder
                ;
            url.addQueryItem("file", localFileName);

            QNetworkRequest networkRequest(url);

            networkRequest.setHeader(
                QNetworkRequest::ContentTypeHeader,
                "multipart/form-data; boundary=" + boundaryStr
                );

            oAuth->signRequest(
                userData,
                "POST",
                &networkRequest
                );

        //send request
            networkReply = networkAccessManager->post(
                networkRequest, *multipartform
                );

        //update variables
            ui->stateLabel->setText("Starting...");
            ui->toggleStartPushButton->setText("Cancel");
            ui->toggleStartAction->setText("Cancel");
            ui->toggleStartPushButton->setEnabled(false);
            ui->toggleStartAction->setEnabled(false);
            active = true;
            uploadTime.start();

        //establish connections
            connect(
                networkReply,
                SIGNAL(uploadProgress(qint64,qint64)),
                SLOT(handleUploadProgress(qint64,qint64))
                );
            connect(
                networkReply,
                SIGNAL(finished()),
                SLOT(handleFinished())
                );
    }
    else
    {
        //update variables
        ui->progressBar->setValue(0);
        ui->stateLabel->setText("Ready to start");
        ui->toggleStartPushButton->setText("Start");
        ui->toggleStartAction->setText("Start");
        if(networkReply != 0)
        {
            networkReply->abort();
            networkReply->deleteLater();
        }
        active = false;
    }
}

void UploadDialog::handleUploadProgress(qint64 sent, qint64 total)
{
    //avoid errors
    if(sent == 0 || total == 0 || uploadTime.elapsed() == 0)
        return;

    // calculate the download speed
    double speed = sent * 1000.0 / uploadTime.elapsed();
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
    ui->progressBar->setValue((sent*100)/total);
    ui->stateLabel->setText(
        QString("Uploading at %1%2").arg(speed, 3, 'f', 1).arg(unit)
        );
    ui->toggleStartPushButton->setEnabled(true);
    ui->toggleStartAction->setEnabled(true);
}

void UploadDialog::handleFinished()
{
    //delete temporary data used during upload
    delete multipartform;
    delete file;

    if(networkReply->error() != QNetworkReply::NoError)
    {
        //if the operation was canceled, do nothing
        if(networkReply->error() == QNetworkReply::OperationCanceledError)
            return;

        //notify the user
        QMessageBox::critical(
            this,
            "Droper",
            "File upload error!"
            );
        show();

        //update variables
        ui->progressBar->setValue(0);
        ui->stateLabel->setText("Ready to start");
        ui->toggleStartPushButton->setText("Start");
        ui->toggleStartAction->setText("Start");
        ui->toggleStartPushButton->setEnabled(true);
        ui->toggleStartAction->setEnabled(true);
        networkReply->deleteLater();
        active = false;
    }
    else
    {
        //compute remoteFolderName and localFileName
        QString remoteFolderName = remoteFolder.right(
            (remoteFolder.length()-remoteFolder.lastIndexOf("/"))-1
            );
        if(remoteFolderName == "")  //the case of the remoteFolder being "/"
            remoteFolderName = "Dropbox (root)";
        QString localFileName = localFile.right(
            (localFile.length()-localFile.lastIndexOf("/"))-1
            );

        //notify the user
        QMessageBox::information(
            this,
            "Droper",
            QString(
                "The file '%1' was successfully uploaded to the "
                "folder '%2'."
                ).arg(localFileName).arg(remoteFolderName)
            );

        //notify the main window
        emit done(remoteFolder);

        //discard reply
        networkReply->deleteLater();

        //return to initial state
        initialize();
    }
}
