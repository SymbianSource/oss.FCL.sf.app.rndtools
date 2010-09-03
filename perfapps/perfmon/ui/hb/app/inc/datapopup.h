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

#ifndef DATAPOPUP_H
#define DATAPOPUP_H

#include <HbDeviceDialog>
#include <QFont>

class EngineWrapper;

class DataPopup : public HbDeviceDialog
{
    Q_OBJECT

public:
    DataPopup(EngineWrapper &engine);

public slots:
    void show();
    void hide();
    void updateSamples();
    void updateSettings();
    void updateVisibility(bool foreground);

signals:
    void clicked();

private slots:
    void triggerAction(QVariantMap data);

private:
    QVariantMap collectParams() const;
    void updateData();

private:
    EngineWrapper &mEngine;

    bool mPopupCreated;

    bool mPopupVisible;
};

#endif // DATAPOPUP_H
