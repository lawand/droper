/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTFLICKGESTURE_P_H
#define QTFLICKGESTURE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qobject.h"
#include "qpointer.h"
#include "qevent.h"
#include "qgesture.h"
#include "qgesturerecognizer.h"
#include "qtscroller.h"

#ifndef QT_NO_GESTURES

class PressDelayHandler;

class QtFlickGesture : public QGesture
{
    Q_OBJECT

public:
    QtFlickGesture(QObject *receiver, Qt::MouseButton button, QObject *parent = 0);
    ~QtFlickGesture();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    QPointer<QObject> receiver;
    QtScroller *receiverScroller;
    Qt::MouseButton button; // NoButton == Touch
    bool macIgnoreWheel;
    // QWidget::mapFromGlobal is very expensive on X11, so we cache the global position of the widget
    QPointer<QWidget> receiverWindow;
    QPoint receiverWindowPos;

    static PressDelayHandler *pressDelayHandler;

    friend class QtFlickGestureRecognizer;
};

class QtFlickGestureRecognizer : public QGestureRecognizer
{
public:
    QtFlickGestureRecognizer(Qt::MouseButton button);

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);

private:
    Qt::MouseButton button; // NoButton == Touch
};

#endif // QT_NO_GESTURES

#endif // QTFLICKGESTURE_P_H
