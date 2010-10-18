/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/

#ifndef FBPROPERTIESDIALOG_H_
#define FBPROPERTIESDIALOG_H_

#include <HbDialog>
#include <QVector>
#include <QPair>

// forward declarations
class HbMarqueeItem;
class QString;
class QGraphicsItem;
class QGraphicsGridLayout;

class FbPropertiesDialog : public HbDialog
{
    Q_OBJECT

public:
    explicit FbPropertiesDialog(QGraphicsItem *parent = 0);
    virtual ~FbPropertiesDialog();

    void setTitle(const QString &);
    void setProperties(const QVector<QPair<QString, QString> > &);

private:
    void createHeading();
    void createContentWidget();
    void createToolBar();
private:
    HbMarqueeItem *mTitle;
    QGraphicsGridLayout *mGridLayout;
};

#endif // FBPROPERTIESDIALOG_H_
