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




#ifndef __CREATORPHONEBOOK_H__
#define __CREATORPHONEBOOK_H__

#include "creator_model.h"
#include "creator_phonebookbase.h"


#include <e32base.h>
#include <cntdef.h>
#include <cntdb.h>
#include <cntitem.h>
#include <cntfldst.h>
#include <cntview.h>
#include <cpbkcontactengine.h>
#include <cpbkcontactitem.h> 
#include <cpbkfieldinfo.h> 
#include <cpbkfieldsinfo.h> 
#include <pbkfields.hrh>



class CCreatorEngine;
class CCreatorModuleBaseParameters;
class CPhonebookParameters;


class CCreatorPhonebook : public CCreatorPhonebookBase 
    {
public: 
    static CCreatorPhonebook* NewL(CCreatorEngine* aEngine);
    static CCreatorPhonebook* NewLC(CCreatorEngine* aEngine);
    ~CCreatorPhonebook();

private:
    CCreatorPhonebook();
    void ConstructL(CCreatorEngine* aEngine); // from MCreatorModuleBase

public:

    TInt CreateContactEntryL(CCreatorModuleBaseParameters *aParameters);    
    TInt CreateGroupEntryL(CCreatorModuleBaseParameters *aParameters);
    TInt CreateSubscribedContactEntryL(CCreatorModuleBaseParameters *aParameters);
    TBool IsActive(){ return EFalse; };
    void CancelOperation() {};
    void DeleteAllL();
    void DeleteAllCreatedByCreatorL();
    void DeleteAllGroupsL();
    void DeleteAllGroupsCreatedByCreatorL();


private:
    void CompactPbkDatabase(TBool aCompressImmediately=EFalse);
    void AddFieldToContactItemL(CPbkContactItem& aItem, TPbkFieldId aFieldId, const TDesC& aFieldText);
    void DeleteAllItemsL( TUid aStoreUid );
    void DeleteItemsCreatedWithCreatorL( TUid aStoreUid );


private:
    CPbkContactEngine* iContactDBEngine;
    TInt iOpCounter;
    CPhonebookParameters* iParameters;
    RArray<TInt> iContactIds;
    RArray<TInt> iGroupIds;        // TContactItemId ( = TInt32 ) CContactItem::Id()
    RArray<TInt> iEntriesToDelete; // TContactItemId ( = TInt32 ) CContactItem::Id()
  
public:
    };


class CPhonebookParameters : public CCreatorModuleBaseParameters
    {
public:
	void ParseL(CCommandParser* parser, TParseParams /*aCase = 0*/);
    HBufC*  iContactFirstName;
    HBufC*  iContactLastName;
    HBufC*  iContactCompanyName;
    HBufC*  iContactJobTitle;
    HBufC*  iContactPostalAddress;
    HBufC*  iWvUserId;
    HBufC*  iRingingtone;
    HBufC*  iThumbnailImage;
    HBufC*  iContactPhoneNumberGeneral;     TInt iNumberOfPhoneNumberFields;
    HBufC*  iContactURL;                    TInt iNumberOfURLFields;
    HBufC*  iContactEmailAddress;           TInt iNumberOfEmailAddressFields;

    HBufC*  iGroupName;
    TInt    iContactsInGroup;


public:
    CPhonebookParameters();
    ~CPhonebookParameters();
    };



#endif // __CREATORPHONEBOOK_H__
