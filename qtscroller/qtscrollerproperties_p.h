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

#ifndef QTSCROLLERPROPERTIES_P_H
#define QTSCROLLERPROPERTIES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QPointF>
#include <QEasingCurve>
#include "qtscrollerproperties.h"

class QtScrollerPropertiesPrivate
{
public:
    static QtScrollerPropertiesPrivate *defaults();

    bool operator==(const QtScrollerPropertiesPrivate &) const;

    qreal mousePressEventDelay;
    qreal dragStartDistance;
    qreal dragVelocitySmoothingFactor;
    qreal axisLockThreshold;
    QEasingCurve scrollingCurve;
    qreal decelerationFactor;
    qreal minimumVelocity;
    qreal maximumVelocity;
    qreal maximumClickThroughVelocity;
    qreal acceleratingFlickMaximumTime;
    qreal acceleratingFlickSpeedupFactor;
    qreal snapPositionRatio;
    qreal snapTime;
    qreal overshootDragResistanceFactor;
    qreal overshootDragDistanceFactor;
    qreal overshootScrollDistanceFactor;
    qreal overshootScrollTime;

    QtScrollerProperties::OvershootPolicy hOvershootPolicy;
    QtScrollerProperties::OvershootPolicy vOvershootPolicy;
    QtScrollerProperties::FrameRates frameRate;
};

#endif // QTSCROLLERPROPERTIES_P_H

