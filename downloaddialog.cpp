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
    remotePathAndFileName(""),
    networkReply(0)
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

    //initial state
    initialize();
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}

bool DownloadDialog::setFile(QVariantMap* fileMap)
{
    //can't set a new file while actively downloading another
    if(active)
        return false;

    remotePathAndFileName = (*fileMap)["path"].toString();
    QString fileName = remotePathAndFileName.right(
            (remotePathAndFileName.length() -
             remotePathAndFileName.lastIndexOf("/")) - 1
            );

    ui->fileNameAndSizeLabel->setText(
            QString("%1 (%2)").arg(fileName).arg((*fileMap)["size"].toString())
                    );
    ui->localPathLineEdit->setEnabled(true);
    ui->browsePushButton->setEnabled(true);
    ui->progressBar->setFormat("%p%");

    //report success
    return true;
}

void DownloadDialog::initialize()
{
    //set variables
    ui->fileNameAndSizeLabel->setText("No Active Downloads");
    ui->localPathLineEdit->setEnabled(false);
    ui->localPathLineEdit->setText("");
    ui->browsePushButton->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat("");
    ui->toggleStartPushButton->setEnabled(false);
    ui->toggleStartPushButton->setText("Start");
    ui->speedLabel->clear();
    active = false;
    remotePathAndFileName = "";
    networkReply = 0;
}

void DownloadDialog::reject()
{
    if(!active)
    {
        initialize();
    }

    QDialog::reject();
}

void DownloadDialog::on_browsePushButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(
            this,
            "Select a directory",
            QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)
            );

    QString fileName = remotePathAndFileName.right(
            (remotePathAndFileName.length() -
             remotePathAndFileName.lastIndexOf("/")) - 1
            );

    if(QFile(directory + "/" + fileName).exists())
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
        ui->localPathLineEdit->setText(directory);
    }
}

void DownloadDialog::on_localPathLineEdit_textChanged(QString text)
{
    ui->toggleStartPushButton->setEnabled(
            !text.isEmpty()
            );
}

void DownloadDialog::on_toggleStartPushButton_clicked()
{
    if(!active)
    {
        QString fileName = remotePathAndFileName.right(
                (remotePathAndFileName.length() -
                 remotePathAndFileName.lastIndexOf("/")) - 1
                );

        QString localPath = ui->localPathLineEdit->text();

        localFile.setFileName(localPath + "/" + fileName);

        //check whether the file can be opened for writing,
        //opening it in the process
        if(!localFile.open(QFile::WriteOnly | QFile::Append))
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
                remotePathAndFileName
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
        downloadTime.start();

        ui->progressBar->setFormat("%p% (Starting...)");

        ui->browsePushButton->setEnabled(false);

        ui->toggleStartPushButton->setText("Cancel");

        active = true;
    }
    else
    {
        //set variables
        ui->browsePushButton->setEnabled(true);
        ui->progressBar->setValue(0);
        ui->progressBar->setFormat("%p%");
        ui->toggleStartPushButton->setText("Start");
        ui->speedLabel->clear();
        active = false;
        localFile.remove();
        networkReply->abort();
        networkReply->deleteLater();
    }
}

void DownloadDialog::handleReadyRead()
{
    localFile.write(networkReply->readAll());
}

void DownloadDialog::handleDownloadProgress(qint64 received, qint64 total)
{
    //avoid arithmetic errors (division by zero)
    if (received == 0)
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

    ui->speedLabel->setText(
            QString::fromLatin1("%1 %2").arg(speed, 3, 'f', 1).arg(unit)
            );

    ui->progressBar->setFormat("%p% (Downloading)");

    ui->progressBar->setValue((received*100)/total);
}

void DownloadDialog::handleFinished()
{
    if(networkReply->error() != QNetworkReply::NoError)
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

        //set variables
        ui->browsePushButton->setEnabled(true);
        ui->progressBar->setValue(0);
        ui->progressBar->setFormat("%p%");
        ui->toggleStartPushButton->setText("Start");
        ui->speedLabel->clear();
        active = false;
        localFile.remove();
        networkReply->deleteLater();
    }
    else
    {
        //notify the user
        QMessageBox::information(
                this,
                "Droper",
                "File download is done."
                );

        //set variables
        localFile.close();
        networkReply->deleteLater();

        //return to initial state
        initialize();
    }
}
