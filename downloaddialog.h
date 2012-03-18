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

#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

//base class
#include <QDialog>

//data members
#include <QFile>
#include <QTime>
class QNetworkAccessManager;
class QNetworkReply;
class Dropbox;
class OAuth;
class UserData;

//member functions
#include <QVariantMap>

namespace Ui {
    class DownloadDialog;
}

class DownloadDialog : public QDialog
{
    Q_OBJECT

//shared data members
public:
    QNetworkAccessManager *networkAccessManager;
    Dropbox *dropbox;
    OAuth *oAuth;
    UserData *userData;

//enumerations
private:
    enum State
    {
        INITIAL,
        READY_TO_START,
        DOWNLOADING,
        FINISHED,
        NOT_FINISHED
    };

//member functions
public:
    explicit DownloadDialog(
        QNetworkAccessManager *networkAccessManager,
        Dropbox *dropbox,
        OAuth *oAuth,
        UserData *userData,
        QWidget *parent = 0
        );
    ~DownloadDialog();
    bool isDownloading();
    void setFileAndFolderInformation(
        QVariantMap fileInfo,
        QString folderPath
        );
private slots:
    void startStopRestart();
    void setState(DownloadDialog::State state);
    void handleReadyRead();
    void handleDownloadProgress(qint64 received, qint64 total);
public slots:
    void handleNetworkReply(QNetworkReply *networkReply);

//private data members
private:
    Ui::DownloadDialog *ui;
    State state;
    QAction *startStopRestartAction;
    QAction *closeAction;
    QFile file;
    QString filePath;
    QString fileName;
    QString fileSize;
    QString folderPath;
    QString folderName;
    QNetworkReply *networkReply;
    QTime downloadTime;
};

#endif // DOWNLOADDIALOG_H
