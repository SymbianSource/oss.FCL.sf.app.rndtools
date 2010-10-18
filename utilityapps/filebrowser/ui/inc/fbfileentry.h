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

#ifndef FILEEENTRY_H
#define FILEEENTRY_H

#include "FBFileUtils.h"
#include <QString>
#include <QDate>
#include <QStringList>

/**
 * class that is used for communicating between Symbian and Qt code.
 */
class FbFileEntry
{
public:
    /**
     * Constructor
     */
    FbFileEntry(TFileEntry &);
    
    /**
     * Destructor
     */
    ~FbFileEntry();
    
    /* Functions that are called from UI */
public: 
    QString fullName();
    QString path();

    int attributes();
    QString attributesString();
    bool isArchive() const { return mFileEntry.iEntry.IsArchive(); }
    bool isHidden() const { return mFileEntry.iEntry.IsHidden(); }
    bool isReadOnly() const { return mFileEntry.iEntry.IsReadOnly(); }
    bool isSystem() const { return mFileEntry.iEntry.IsSystem(); }

    const QString archiveTextShort() const  { return QString("A"); }
    const QString hiddenTextShort() const  { return QString("H"); }
    const QString readOnlyTextShort() const  { return QString("R"); }
    const QString systemTextShort() const  { return QString("S"); }
    const QString noAttributeTextShort() const  { return QString("-"); }

    const QString archiveText() const  { return QString("Archive"); }
    const QString hiddenText() const  { return QString("Hidden"); }
    const QString readOnlyText() const  { return QString("Read Only"); }
    const QString systemText() const  { return QString("System"); }
    const QString noAttributeText() const  { return QString("None"); }

    int size();
    QString sizeString();

//    QDateTime modified();
    QString modifiedDateTimeString(const QString aDateTimeFormat);
    QString modifiedString();
//    int Uid(int id);
    bool isDir();
    QString name();

    int dirEntries();
    QString dirEntriesString();

    int iconId();

private:
    TFileEntry mFileEntry;
    };

#endif //FBFILEEENTRY_H
