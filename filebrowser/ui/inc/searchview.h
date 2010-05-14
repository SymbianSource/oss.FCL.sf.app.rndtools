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

#include <HbView>
#include <HbMainWindow>

#include <QDateTime>

class QDir;
class QStringList;

class EngineWrapper;
class HbDataForm;
class HbDataFormModelItem;
class HbProgressDialog;

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
    int mNumberOfFoundFiles;
    QStringList *mFoundFilesList;
};

class SearchView : public HbView
{
    Q_OBJECT

public:
    explicit SearchView(EngineWrapper &engineWrapper);
    ~SearchView();
    void open(const QString &path);

signals:
    void finished(bool ok);

private slots:
    void accept();
    void reject();

private:
    void initDataForm();
    void createToolbar();
    void loadAttributes();
    void readFormItems();

    void startFileSearch();
    void fileSearchResults();

private:
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

    HbProgressDialog *mProgressDialog;
};

#endif // SEARCHVIEW_H
