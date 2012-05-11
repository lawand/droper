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
#include "downloaddialog.h"
#include "ui_downloaddialog.h"

//data members
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "dropbox.h"
#include "oauth.h"
#include "userdata.h"

//implementation-specific
#include <QMessageBox>
#include "util.h"

DownloadDialog::DownloadDialog(
    QNetworkAccessManager *networkAccessManager,
    Dropbox *dropbox,
    OAuth *oAuth,
    UserData *userData,
    QWidget *parent
    ) :
    QDialog(parent),
    ui(new Ui::DownloadDialog),
    startStopRestartAction(new QAction("Start", this)),
    closeAction(new QAction("Close", this))
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
    setState(DownloadDialog::INITIAL);
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}

void DownloadDialog::setFileAndFolderInformation(
    QVariantMap fileInfo,
    QString folderPath
    )
{
    //file information
    filePath = fileInfo["path"].toString();
    fileName = filePath.right(
        (filePath.length() - filePath.lastIndexOf("/")) - 1
        );
    fileSize = fileInfo["size"].toString();

    //folder information
    this->folderPath = folderPath;
    folderName = this->folderPath.right(
        (this->folderPath.length() - this->folderPath.lastIndexOf("/")) - 1
        );
    if(folderName.isEmpty())
        folderName = "/ (root)";

    //update the state
    setState(DownloadDialog::READY_TO_START);
}

void DownloadDialog::startStopRestart()
{
    switch(state)
    {
    case READY_TO_START:
        setState(DownloadDialog::DOWNLOADING);
        break;

    case DOWNLOADING:
        networkReply->abort();
        break;

    case NOT_FINISHED:
        setState(DownloadDialog::DOWNLOADING);
        break;

    default:
        break;
    }
}

void DownloadDialog::setState(DownloadDialog::State state)
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

    case DOWNLOADING:
        ui->progressBar->setValue(0);
        ui->progressBar->setFormat("%p% Downloading");
        startStopRestartAction->setText("Stop");
        startStopRestartAction->setVisible(true);
        downloadTime.start();

        //prepare the local file
        file.setFileName(folderPath + "/" + fileName);
        //check whether the file can be opened for writing,
        //opening it in the process
        if(!file.open(QFile::WriteOnly))
        {
            QMessageBox::critical(
                this,
                "Droper",
                "Failed to open the file for writing."
                );

            setState(DownloadDialog::NOT_FINISHED);

            return;
        }

        //request the content of the remote file
        {
            QUrl url = dropbox->apiToUrl(Dropbox::FILES).toString() + filePath;
            QNetworkRequest networkRequest(url);
            oAuth->signRequestHeader("GET", &networkRequest, userData);
            networkReply = networkAccessManager->get(networkRequest);
            connect(
                networkReply,
                SIGNAL(readyRead()),
                SLOT(handleReadyRead())
                );
            connect(
                networkReply,
                SIGNAL(downloadProgress(qint64, qint64)),
                SLOT(handleDownloadProgress(qint64, qint64))
                );
        }

        break;

    case FINISHED:
        ui->progressBar->setFormat("Finished");
        startStopRestartAction->setVisible(false);
        file.close();
        break;

    case NOT_FINISHED:
        ui->progressBar->setFormat("Not Finished");
        startStopRestartAction->setText("Restart");
        startStopRestartAction->setVisible(true);
        file.remove();
        break;

    default:
        break;
    }
}

bool DownloadDialog::isDownloading()
{
    if(state == DownloadDialog::DOWNLOADING)
        return true;
    else
        return false;
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

    //update progress bar
    ui->progressBar->setFormat(
        QString("%p% Down. at %1").arg(
            Util::bytesToString(
                double(received * 1000.0) / downloadTime.elapsed()
                )
            )
        );
    ui->progressBar->setValue( (received*100) / total );
}

void DownloadDialog::handleNetworkReply(QNetworkReply *networkReply)
{
    if(this->networkReply != networkReply)
        return;

    if(networkReply->error() != QNetworkReply::NoError)
        setState(DownloadDialog::NOT_FINISHED);
    else
        setState(DownloadDialog::FINISHED);
}
