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

#include "driveentry.h"

// ---------------------------------------------------------------------------

DriveEntry::DriveEntry(TDriveEntry aDriveEntry)
: mDriveEntry(aDriveEntry)
{
}

// ---------------------------------------------------------------------------

DriveEntry::~DriveEntry()
{
}

// ---------------------------------------------------------------------------

QChar DriveEntry::driveLetter()
{
    return QChar(TUint(mDriveEntry.iLetter));
}

// ---------------------------------------------------------------------------

int DriveEntry::number()
{
    return mDriveEntry.iNumber;
}

// ---------------------------------------------------------------------------

QString DriveEntry::mediaTypeString()
{
    return QString::fromUtf16(mDriveEntry.iMediaTypeDesc.Ptr(), mDriveEntry.iMediaTypeDesc.Length());
}

// ---------------------------------------------------------------------------

QString DriveEntry::attributesString()
{
    return QString::fromUtf16(mDriveEntry.iAttributesDesc.Ptr(), mDriveEntry.iAttributesDesc.Length());
}

// ---------------------------------------------------------------------------

int DriveEntry::iconId()
{
    return mDriveEntry.iIconId;
}

// ---------------------------------------------------------------------------

qint64 DriveEntry::volumeInfoFree()
{
    return mDriveEntry.iVolumeInfo.iFree;
}

// ---------------------------------------------------------------------------

qint64 DriveEntry::volumeInfoSize()
{
    return mDriveEntry.iVolumeInfo.iSize;
}

// ---------------------------------------------------------------------------
