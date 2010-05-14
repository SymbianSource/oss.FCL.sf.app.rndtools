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

#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <hbview.h>
#include <hbmainwindow.h>

#include <qdatetime.h>

class QDir;
class EngineWrapper;
class QStringList;
class HbDataForm;
class HbDataFormModelItem;

/**
  * Settings class that is used for 
  */
class SearchAttributes
{
public:
    QString   mSearchDir;
    QString   mWildCards;
    QString   mTextInFile;
    int       mMinSize;
    int       mMaxSize;
    QDate     mMinDate;
    QDate     mMaxDate;
    bool      mRecurse;
};

/**
 * search results class.
 */
class SearchResults
{
public:
    int         mNumberOfFoundFiles;
    QStringList* mFoundFilesList;
};

class SearchView : public HbView
{
    Q_OBJECT

public:
    SearchView(HbView &mainView, HbMainWindow &mainWindow, EngineWrapper &engineWrapper);
    ~SearchView();
    void open(const QString &path);


private slots:
    void backButtonClicked();
    void startFileSearch();
    void loadAttributes();
    void readFormItems();
    
private:
    void fileSearchResults();

    /* Main Window of folderbrowser */
    HbMainWindow &mMainWindow;
    /* Main View of folderbrowser*/
    HbView &mMainView;
    /* EngineWrapper */
    EngineWrapper &mEngineWrapper;  

    /* Search attributes */
    SearchAttributes mAttributes;    
    /* Search results */
    SearchResults mResults;

    HbDataForm *mForm;
    HbDataFormModelItem *mSearchFileNameItem;
    HbDataFormModelItem *mWildCardItem;
    HbDataFormModelItem *mHasString;
    HbDataFormModelItem *mRecurse;
    HbDataFormModelItem *mMinSize;
    HbDataFormModelItem *mMaxSize;
    HbDataFormModelItem *mMinDate;
    HbDataFormModelItem *mMaxDate;

    QString mPath;
};

#endif // SEARCHVIEW_H
