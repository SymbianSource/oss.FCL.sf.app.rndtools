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



#ifndef __CCREATORPHONEBOOKBASE_H_
#define __CCREATORPHONEBOOKBASE_H_

#include "creator_modulebase.h"

static const TInt KPhonebookFieldLength = 128;
static const TInt KCreateRandomAmountOfGroups = -9999;

class CCreatorModuleBaseParameters;

class CCreatorPhonebookBase : public CBase, public MCreatorModuleBase{
public:

    virtual TInt CreateContactEntryL(CCreatorModuleBaseParameters *aParameters) = 0;    
    virtual TInt CreateGroupEntryL(CCreatorModuleBaseParameters *aParameters) = 0;
    virtual TInt CreateSubscribedContactEntryL(CCreatorModuleBaseParameters *aParameters) = 0;
    virtual TBool AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries); // from MCreatorModuleBase
    virtual TBool IsActive() = 0;
    virtual void CancelOperation() = 0;
    virtual void DeleteAllGroupsL() = 0;
    virtual void DeleteAllGroupsCreatedByCreatorL() = 0;
    
protected:

    virtual void SetDefaultParameters();    
    TInt iNumberOfPhoneNumberFields;
    TInt iNumberOfURLFields;
    TInt iNumberOfEmailAddressFields;
    TInt iContactsInGroup;    
    TBool iDefaultFieldsSelected;
};


#endif /*__CCREATORPHONEBOOKBASE_H_*/
