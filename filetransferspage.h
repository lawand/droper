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

#ifndef FILETRANSFERSPAGE_H
#define FILETRANSFERSPAGE_H

// base class(es)
#include <QWidget>

// data members
#include <QFile>
#include <QTime>
class QNetworkReply;

// member functions
#include <QVariantMap>
class QNetworkRequest;

namespace Ui {
class FileTransfersPage;
}

class FileTransfersPage : public QWidget
{
    Q_OBJECT
    
// enumerations
public:
    enum State
    {
        INITIAL,
        TRANSFERRING,
        FINISHED,
        NOT_FINISHED
    };

// member functions
public:
    explicit FileTransfersPage(QWidget *parent = 0);
    ~FileTransfersPage();
    // misc
public:
    bool isDownloading();
    bool isUploading();
    void setDownloadFileAndFolderInformation(
        QVariantMap fileInfo,
        QString folderPath
        );
    void setUploadFileAndFolderInformation(
        QString filePath,
        QString fileSize,
        int fileBytes,
        QString folderPath,
        bool overwrite
        );
    void setDownloadState(State state);
    void setUploadState(State state);
    void setDownloadNetworkReply(QNetworkReply *networkReply);
    void setUploadNetworkReply(QNetworkReply *networkReply);
private:
    void setupActions();
signals:
    // signals
    void networkRequestGetNeeded(QNetworkRequest *request);
    void networkRequestPutNeeded(QNetworkRequest *request, QIODevice *data);
    void backRequested();
    void fileUploaded(QVariantMap fileMetadata);
public slots:
    // network slots
    void handleDownloadReadyRead();
    void handleDownloadProgress(qint64 received, qint64 total);
    void handleUploadProgress(qint64 sent, qint64 total);
public slots:
    void handleNetworkReply(QNetworkReply *networkReply);
    // action slots
private slots:
    void on_downloadStartStopRestartAction_triggered();
    void on_uploadStartStopRestartAction_triggered();
    void on_backAction_triggered();

// private data members
private:
    Ui::FileTransfersPage *ui;
    bool ovewriteWhenUploading;
    State downloadState;
    State uploadState;
    QTime downloadTime;
    QTime uploadTime;
    QFile downloadFile;
    QFile *uploadFile;
    QString downloadFileName;
    QString uploadFileName;
    QString downloadFilePath;
    QString uploadFilePath;
    QString downloadFileSize;
    QString uploadFileSize;
    int uploadFileBytes;
    QString downloadFolderPath;
    QString uploadFolderPath;
    QString downloadFolderName;
    QString uploadFolderName;
    QNetworkReply *downloadNetworkReply;
    QNetworkReply *uploadNetworkReply;
};

#endif // FILETRANSFERSPAGE_H
