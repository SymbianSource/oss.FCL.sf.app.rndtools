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

#ifndef DRIVEENTRY_H
#define DRIVEENTRY_H

#include "FBFileUtils.h"
#include <QString>
#include <QDate>


/**
 * class that is used for communicating between Symbian and Qt code.
 */
class DriveEntry {

public:
    
    /**
     * Constructor
     */
    DriveEntry(TDriveEntry aDriveEntry);
    
    /**
     * Destructor
     */
    ~DriveEntry();
    
    /* Functions that are called from UI */
public: 
    QChar driveLetter();
    int number();
    QString mediaTypeString();
    QString attributesString();
    int iconId();
    qint64 volumeInfoFree();
    qint64 volumeInfoSize();

private:
    TDriveEntry mDriveEntry;
    };

#endif //DRIVEENTRY_H
