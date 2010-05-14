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

#include "engine.h"
#include "enginewrapper.h"
#include "creator_phonebookbase.h"
#include "creator_traces.h"

TBool CCreatorPhonebookBase::AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries)
    {
    LOGSTRING("Creator: CCreatorPhonebook::AskDataFromUserL");
    
    if( aCommand == ECmdDeleteContacts )
        {
        return iEngine->GetEngineWrapper()->YesNoQueryDialog(_L("Delete all contacts?") );
        }
    if( aCommand == ECmdDeleteCreatorContacts )
        {
        return iEngine->GetEngineWrapper()->YesNoQueryDialog(_L("Delete all contacts created with Creator?") ); 
        }
    if( aCommand == ECmdDeleteContactGroups )
        {
        return iEngine->GetEngineWrapper()->YesNoQueryDialog(_L("Delete all contact groups?") );
        }
    if( aCommand == ECmdDeleteCreatorContactGroups )
        {
        return iEngine->GetEngineWrapper()->YesNoQueryDialog(_L("Delete all contact groups created with Creator?") ); 
        }    
    // display queries
    if (iEngine->GetEngineWrapper()->EntriesQueryDialog(aNumberOfEntries, _L("How many entries to create?"))) // ask number of entries to create
        {
        if (aCommand == ECmdCreatePhoneBookEntryContacts)
            {
            TInt index = 0;

            //CAknListQueryDialog* dlg1 = new(ELeave) CAknListQueryDialog(&index);
            if (iEngine->GetEngineWrapper()->ListQueryDialog(_L("Fields in contact"), R_CONTACT_CREATION_TYPE_QUERY, index))
                {
                if (index == 0) //first item
                    {
                    iDefaultFieldsSelected = ETrue;
                    return ETrue;    
                    }
                else  // detailed mode selected
                    {
                    iDefaultFieldsSelected = EFalse;
                    if (iEngine->GetEngineWrapper()->EntriesQueryDialog(iNumberOfPhoneNumberFields, _L("Amount of phone number fields in one contact?"), ETrue))
                        {
                        if (iEngine->GetEngineWrapper()->EntriesQueryDialog(iNumberOfURLFields, _L("Amount of URL fields in one contact?"), ETrue))
                            {
                            if (iEngine->GetEngineWrapper()->EntriesQueryDialog(iNumberOfEmailAddressFields, _L("Amount of email fields in one contact?"), ETrue))
                                {                                
                                return ETrue;
                                }
                            else
                                return EFalse;
                            }
                            else
                                return EFalse;
                        }
                    else
                        return EFalse;
                    }
                }
            else
                return EFalse;
            }
        else if (aCommand == ECmdCreatePhoneBookEntryGroups)
            {
            TInt index = 0;
            //CAknListQueryDialog* dlg1 = new(ELeave) CAknListQueryDialog(&index);
            if (iEngine->GetEngineWrapper()->ListQueryDialog(_L("Fields in contact"), R_GROUP_CREATION_TYPE_QUERY, index))
                {
                if (index == 0) //first item
                    {
                    iContactsInGroup = KCreateRandomAmountOfGroups;
                    return ETrue;
                    }
                else
                    {
                    if (iEngine->GetEngineWrapper()->EntriesQueryDialog(iContactsInGroup, _L("Amount of contacts in one group?"), ETrue))
                        {
                        return ETrue;
                        }
                    else
                        return EFalse;
                    }
                }
            else
                return EFalse;
            }
        else 
            return ETrue; 
        }
    else
        return EFalse;
    }


void CCreatorPhonebookBase::SetDefaultParameters()
    {
    iNumberOfPhoneNumberFields = 1;
    iNumberOfURLFields = 0;
    iNumberOfEmailAddressFields = 0;
    iContactsInGroup = 50;
    iDefaultFieldsSelected = EFalse;
    }
