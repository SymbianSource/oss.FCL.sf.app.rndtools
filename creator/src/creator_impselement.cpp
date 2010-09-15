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


#ifdef __PRESENCE

#include "creator_impselement.h"
#include "creator_traces.h"
#include "creator_imps.h"

using namespace creatorimps;

/*
 * 
 */
CCreatorImpsServerElement* CCreatorImpsServerElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext )
    {
    CCreatorImpsServerElement* self = new (ELeave) CCreatorImpsServerElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop(self);
    return self;
    }
/*
 * 
 */
CCreatorImpsServerElement::CCreatorImpsServerElement(CCreatorEngine* aEngine) 
: 
CCreatorScriptElement(aEngine)
    {
    iIsCommandElement = ETrue;
    }

void CCreatorImpsServerElement::AsyncExecuteCommandL()
    {
    const CCreatorScriptAttribute* amountAttr = FindAttributeByName(KAmount);
    TInt entryAmount = 1;    
    if( amountAttr )
        {
        entryAmount = ConvertStrToIntL(amountAttr->Value());
        }
    // Get 'fields' element 
    CCreatorScriptElement* fieldsElement = FindSubElement(KFields);
    if( fieldsElement && fieldsElement->SubElements().Count() > 0 )
        {
        // Get sub-elements
        const RPointerArray<CCreatorScriptElement>& fields = fieldsElement->SubElements();        
        // Create note entries, the amount of entries is defined by entryAmount:
        if( iLoopIndex < entryAmount )
            {            
            CIMPSParameters* param = new (ELeave) CIMPSParameters;
            CleanupStack::PushL(param);
            
            for( TInt i = 0; i < fields.Count(); ++i )
                {
                CCreatorScriptElement* field = fields[i];
                TPtrC elemName = field->Name();
                TPtrC elemContent = field->Content();
                const CCreatorScriptAttribute* randomAttr = fields[i]->FindAttributeByName(KRandomLength);
                TBool useRandom = EFalse;
                if( randomAttr || elemContent.Length() == 0 )
                    {
                    useRandom = ETrue;
                    }
                
                if( elemName == KName )
                    {
                    if( useRandom )
                        {
                        param->SetRandomNameL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerName, elemContent);
                        }                    
                    }
                if( elemName == KUrl )
                    {
                    if( useRandom )
                        {
                        param->SetRandomUrlL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerURL, elemContent);
                        }
                    }
                if( elemName == KUsername )
                    {
                    if( useRandom )
                        {
                        param->SetRandomUsernameL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerUsername, elemContent);
                        }
                    }
                if( elemName == KPassword )
                    {
                    if( useRandom )
                        {
                        param->SetRandomPasswordL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerPassword, elemContent);
                        }
                    }
                if( elemName == KConnectionMethodName )
                    {
                    if( useRandom || CompareIgnoreCase(elemContent, creatorimps::KDefault) == 0 )
                        {
                        param->SetDefaultAccessPointL();
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerAccessPointName, elemContent);
                        }
                    }
                }
            iEngine->AppendToCommandArrayL(ECmdCreateMiscEntryIMPSServers, param);
            CleanupStack::Pop(); // param
            StartNextLoop();
            }
        else
            {
            AsyncCommandFinished();
            }
        }
    else
        {
        iEngine->AppendToCommandArrayL(ECmdCreateMiscEntryIMPSServers, 0, entryAmount);
        AsyncCommandFinished();
        }
    }

void CCreatorImpsServerElement::ExecuteCommandL()
    {
    const CCreatorScriptAttribute* amountAttr = FindAttributeByName(KAmount);
    TInt entryAmount = 1;    
    if( amountAttr )
        {
        entryAmount = ConvertStrToIntL(amountAttr->Value());
        }
    // Get 'fields' element 
    CCreatorScriptElement* fieldsElement = FindSubElement(KFields);
    if( fieldsElement && fieldsElement->SubElements().Count() > 0 )
        {
        // Get sub-elements
        const RPointerArray<CCreatorScriptElement>& fields = fieldsElement->SubElements();        
        // Create note entries, the amount of entries is defined by entryAmount:
        for( TInt cI = 0; cI < entryAmount; ++cI )
            {            
            CIMPSParameters* param = new (ELeave) CIMPSParameters;
            CleanupStack::PushL(param);
            
            for( TInt i = 0; i < fields.Count(); ++i )
                {
                CCreatorScriptElement* field = fields[i];
                TPtrC elemName = field->Name();
                TPtrC elemContent = field->Content();
                const CCreatorScriptAttribute* randomAttr = fields[i]->FindAttributeByName(KRandomLength);
                TBool useRandom = EFalse;
                if( randomAttr || elemContent.Length() == 0 )
                    {
                    useRandom = ETrue;
                    }
                
                if( elemName == KName )
                    {
                    if( useRandom )
                        {
                        param->SetRandomNameL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerName, elemContent);
                        }                    
                    }
                if( elemName == KUrl )
                    {
                    if( useRandom )
                        {
                        param->SetRandomUrlL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerURL, elemContent);
                        }
                    }
                if( elemName == KUsername )
                    {
                    if( useRandom )
                        {
                        param->SetRandomUsernameL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerUsername, elemContent);
                        }
                    }
                if( elemName == KPassword )
                    {
                    if( useRandom )
                        {
                        param->SetRandomPasswordL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerPassword, elemContent);
                        }
                    }
                if( elemName == KConnectionMethodName )
                    {
                    if( useRandom || CompareIgnoreCase(elemContent, creatorimps::KDefault) == 0 )
                        {
                        param->SetDefaultAccessPointL();
                        }
                    else
                        {
                        SetContentToTextParamL(param->iServerAccessPointName, elemContent);
                        }
                    }
                }
            iEngine->AppendToCommandArrayL(ECmdCreateMiscEntryIMPSServers, param);
            CleanupStack::Pop(); // param
            }
        }
    else
    	{
    	iEngine->AppendToCommandArrayL(ECmdCreateMiscEntryIMPSServers, 0, entryAmount);
    	}
    }

#endif // __PRESENCE
