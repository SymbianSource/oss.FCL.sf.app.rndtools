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



#include "creator_contactelement.h"
#include "creator_traces.h"
#include "creator_factory.h"
#include "creator_contactsetcache.h"
#include <xml/documentparameters.h>

using namespace Xml;
using namespace creatorcontact;

/**
 * Contact field element mappings:
 */
enum TContactFieldDataType
    {
    EDataTypeText,
    EDataTypeBinary,
    EDataTypeDateTime    
    };
    
class FieldMapping
{
public:
    TPtrC iElementName;
    TInt  iFieldCode;
    TContactFieldDataType iDataType;
};

CCreatorContactElementBase* CCreatorContactElementBase::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactElementBase::NewL");
    CCreatorContactElementBase* self = new (ELeave) CCreatorContactElementBase(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactElementBase::CCreatorContactElementBase(CCreatorEngine* aEngine) : CCreatorScriptElement(aEngine)
    { }

CCreatorContactElement* CCreatorContactElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactElement::NewL");
    CCreatorContactElement* self = new (ELeave) CCreatorContactElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactElement::CCreatorContactElement(CCreatorEngine* aEngine) 
: 
CCreatorContactElementBase(aEngine)
    { 
    iIsCommandElement = ETrue;
    }


void CCreatorContactElement::ExecuteCommandL()
    {}

CCreatorContactSetElement* CCreatorContactSetElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactSetElement::NewL");
    CCreatorContactSetElement* self = new (ELeave) CCreatorContactSetElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactSetElement::CCreatorContactSetElement(CCreatorEngine* aEngine) 
: 
CCreatorContactElementBase(aEngine)
    { }

void CCreatorContactSetElement::ExecuteCommandL()
    { }

void CCreatorContactSetElement::AddToCacheL()
    {    
    LOGSTRING("Creator: CCreatorContactSetElement::AddToCacheL");
    const CCreatorScriptAttribute* linkIdAttr = this->FindAttributeByName(KId);
    const CCreatorScriptAttribute* existingElemsAttr = this->FindAttributeByName(KExistingContacts);
    TInt numOfExistingContacts = 0;
    if( existingElemsAttr )
        {
        numOfExistingContacts = ConvertStrToIntL(existingElemsAttr->Value());
        }
    if( linkIdAttr )
        {
        TInt linkId = ConvertStrToIntL(linkIdAttr->Value());
        if( linkId > 0 )
            {    
            CCreatorContactSet* newSet = CCreatorContactSet::NewL(linkId, numOfExistingContacts);
            CleanupStack::PushL(newSet);
            ContactLinkCache::Instance()->AppendL(newSet);
            CleanupStack::Pop(newSet);
                 
            // Mark sub-elements (i.e. contacts) to this contact-set:
            for( TInt i = 0; i < iSubElements.Count(); ++i )
                {
                for( TInt j = 0; j < iSubElements[i]->CommandParameters().Count(); ++j)
                    {
                    CCreatorModuleBaseParameters* params = iSubElements[i]->CommandParameters()[j];
                    if( params )
                        {
                        params->SetScriptLinkId(linkId);
                        }
                    }
                }
            }
        }
    }

CCreatorContactGroupElement* CCreatorContactGroupElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactGroupElement::NewL");
    CCreatorContactGroupElement* self = new (ELeave) CCreatorContactGroupElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactGroupElement::CCreatorContactGroupElement(CCreatorEngine* aEngine)
: 
CCreatorContactElementBase(aEngine)
    { 
    iIsCommandElement = ETrue;
    }


void CCreatorContactGroupElement::ExecuteCommandL()
    {}

CCreatorContactFieldElement* CCreatorContactFieldElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactFieldElement::NewL");
    CCreatorContactFieldElement* self = new (ELeave) CCreatorContactFieldElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }

CCreatorContactFieldElement::CCreatorContactFieldElement(CCreatorEngine* aEngine) 
: 
CCreatorScriptElement(aEngine)
    { }
