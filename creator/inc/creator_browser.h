/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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




#ifndef __CREATORBROWSER_H__
#define __CREATORBROWSER_H__

#include "creator_model.h"
#include "creator_modulebase.h"

#include <e32base.h>
#include <favouritesitem.h>
#include <favouritesdb.h>
#include <favouritesfile.h>



class CCreatorEngine;
class CBrowserParameters;


class CCreatorBrowser : public CBase, public MCreatorModuleBase
    {
public: 
    static CCreatorBrowser* NewL(CCreatorEngine* aEngine);
    static CCreatorBrowser* NewLC(CCreatorEngine* aEngine);
    ~CCreatorBrowser();

private:
    CCreatorBrowser();
    void ConstructL(CCreatorEngine* aEngine); // from MCreatorModuleBase

public:
    TBool AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries); // from MCreatorModuleBase
    TInt CreateBookmarkEntryL(CBrowserParameters *aParameters);    
    TInt CreateBookmarkFolderEntryL(CBrowserParameters *aParameters);
    TInt CreateSavedDeckEntryL(CBrowserParameters *aParameters); 
    TInt CreateSavedDeckFolderEntryL(CBrowserParameters *aParameters);
    
    void DeleteAllL();
    void DeleteAllCreatedByCreatorL();
    
    void DeleteAllBookmarksL();
    void DeleteAllBookmarksCreatedByCreatorL();
    void DeleteAllBookmarkFoldersL();
    void DeleteAllBookmarkFoldersCreatedByCreatorL();
    void DeleteAllSavedPagesL();
    void DeleteAllSavedPagesCreatedByCreatorL();
    void DeleteAllSavedPageFoldersL();
    void DeleteAllSavedPageFoldersCreatedByCreatorL();

private:
    void DeleteAllItemsL( const TDesC& aDbName, CFavouritesItem::TType aTypeFilter, TBool aOnlyCreatedWithCreator, RArray<TInt>& aEntryIds, const TUid aDictUid );
    


private:
    CBrowserParameters* iParameters;
    RArray<TInt> iBmEntryIds;
    RArray<TInt> iBmFEntryIds;
    RArray<TInt> iSpEntryIds;
    RArray<TInt> iSpFEntryIds;
public:
    };


class CBrowserParameters : public CCreatorModuleBaseParameters
    {
public: 
    HBufC*              iBookmarkName;
    HBufC*              iBookmarkAddress;
    TFavouritesWapAp    iBookmarkAccessPoint;
    HBufC*              iBookmarkUsername;
    HBufC*              iBookmarkPassword;    

    HBufC*              iBookmarkFolderName;
    
    HBufC*              iSavedDeckLinkName;
    HBufC*              iSavedDeckLocalAddress;
            
    HBufC*              iSavedDeckFolderName;

public:
    CBrowserParameters();
    ~CBrowserParameters();
    };



#endif // __CREATORBROWSER_H__
