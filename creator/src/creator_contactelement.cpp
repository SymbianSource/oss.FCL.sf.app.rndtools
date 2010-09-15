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


#include "creator_contactelement.h"
#include "creator_traces.h"
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
#include "creator_virtualphonebook.h"
#endif
#include "creator_factory.h"
#include "creator_contactsetcache.h"
#include <xml/documentparameters.h>

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
#include "VPbkEng.rsg"
#endif

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
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
FieldMapping fieldMappingTbl[] = {
       {TPtrC(KFirstname), R_VPBK_FIELD_TYPE_FIRSTNAME, EDataTypeText},
       {TPtrC(KLastname), R_VPBK_FIELD_TYPE_LASTNAME, EDataTypeText},
       {TPtrC(KFirstnameReading), R_VPBK_FIELD_TYPE_FIRSTNAMEREADING, EDataTypeText},
       {TPtrC(KLastnameReading), R_VPBK_FIELD_TYPE_LASTNAMEREADING, EDataTypeText},
       {TPtrC(KPrefix), R_VPBK_FIELD_TYPE_PREFIX, EDataTypeText},
       {TPtrC(KSuffix), R_VPBK_FIELD_TYPE_SUFFIX, EDataTypeText},
       {TPtrC(KSecondname), R_VPBK_FIELD_TYPE_SECONDNAME, EDataTypeText},
       {TPtrC(KLandphoneHome), R_VPBK_FIELD_TYPE_LANDPHONEHOME, EDataTypeText},
       
       {TPtrC(KMobilephoneHome), R_VPBK_FIELD_TYPE_MOBILEPHONEHOME, EDataTypeText},
       {TPtrC(KVideonumberHome), R_VPBK_FIELD_TYPE_VIDEONUMBERHOME, EDataTypeText},
       {TPtrC(KFaxnumberHome), R_VPBK_FIELD_TYPE_FAXNUMBERHOME, EDataTypeText},
       {TPtrC(KVoipHome), R_VPBK_FIELD_TYPE_VOIPHOME, EDataTypeText},
       {TPtrC(KEmailHome), R_VPBK_FIELD_TYPE_EMAILHOME, EDataTypeText},
       {TPtrC(KUrlHome), R_VPBK_FIELD_TYPE_URLHOME, EDataTypeText},
       {TPtrC(KAddrlabelHome), R_VPBK_FIELD_TYPE_ADDRLABELHOME, EDataTypeText},
       {TPtrC(KAddrpoHome), R_VPBK_FIELD_TYPE_ADDRPOHOME, EDataTypeText},
       
       {TPtrC(KAddrextHome), R_VPBK_FIELD_TYPE_ADDREXTHOME, EDataTypeText},
       {TPtrC(KAddrstreetHome), R_VPBK_FIELD_TYPE_ADDRSTREETHOME, EDataTypeText},
       {TPtrC(KAddrlocalHome), R_VPBK_FIELD_TYPE_ADDRLOCALHOME, EDataTypeText},
       {TPtrC(KAddrregionHome), R_VPBK_FIELD_TYPE_ADDRREGIONHOME, EDataTypeText},
       {TPtrC(KAddrpostcodeHome), R_VPBK_FIELD_TYPE_ADDRPOSTCODEHOME, EDataTypeText},
       {TPtrC(KAddrcountryHome), R_VPBK_FIELD_TYPE_ADDRCOUNTRYHOME, EDataTypeText},
       {TPtrC(KJobtitle), R_VPBK_FIELD_TYPE_JOBTITLE, EDataTypeText},
       {TPtrC(KCompanyname), R_VPBK_FIELD_TYPE_COMPANYNAME, EDataTypeText},
       
       {TPtrC(KLandphoneWork), R_VPBK_FIELD_TYPE_LANDPHONEWORK, EDataTypeText},
       {TPtrC(KMobilephoneWork), R_VPBK_FIELD_TYPE_MOBILEPHONEWORK, EDataTypeText},
       {TPtrC(KVideonumberWork), R_VPBK_FIELD_TYPE_VIDEONUMBERWORK, EDataTypeText},
       {TPtrC(KFaxnumberWork), R_VPBK_FIELD_TYPE_FAXNUMBERWORK, EDataTypeText},
       {TPtrC(KVoipWork), R_VPBK_FIELD_TYPE_VOIPWORK, EDataTypeText},
       {TPtrC(KEmailWork), R_VPBK_FIELD_TYPE_EMAILWORK, EDataTypeText},
       {TPtrC(KUrlWork), R_VPBK_FIELD_TYPE_URLWORK, EDataTypeText},
       {TPtrC(KAddrlabelWork), R_VPBK_FIELD_TYPE_ADDRLABELWORK, EDataTypeText},
       
       {TPtrC(KAddrpoWork), R_VPBK_FIELD_TYPE_ADDRPOWORK, EDataTypeText},
       {TPtrC(KAddrextWork), R_VPBK_FIELD_TYPE_ADDREXTWORK, EDataTypeText},
       {TPtrC(KAddrstreetWork), R_VPBK_FIELD_TYPE_ADDRSTREETWORK, EDataTypeText},
       {TPtrC(KAddrlocalWork), R_VPBK_FIELD_TYPE_ADDRLOCALWORK, EDataTypeText},
       {TPtrC(KAddrregionWork), R_VPBK_FIELD_TYPE_ADDRREGIONWORK, EDataTypeText},
       {TPtrC(KAddrpostcodeWork), R_VPBK_FIELD_TYPE_ADDRPOSTCODEWORK, EDataTypeText},
       {TPtrC(KAddrcountryWork), R_VPBK_FIELD_TYPE_ADDRCOUNTRYWORK, EDataTypeText},
       {TPtrC(KLandphoneGen), R_VPBK_FIELD_TYPE_LANDPHONEGEN, EDataTypeText},
       
       {TPtrC(KMobilephoneGen), R_VPBK_FIELD_TYPE_MOBILEPHONEGEN, EDataTypeText},
       {TPtrC(KVideonumberGen), R_VPBK_FIELD_TYPE_VIDEONUMBERGEN, EDataTypeText},
       {TPtrC(KFaxnumberGen), R_VPBK_FIELD_TYPE_FAXNUMBERGEN, EDataTypeText},
       {TPtrC(KVoipGen), R_VPBK_FIELD_TYPE_VOIPGEN, EDataTypeText},
       {TPtrC(KPoc), R_VPBK_FIELD_TYPE_POC, EDataTypeText},
       {TPtrC(KSwis), R_VPBK_FIELD_TYPE_SWIS, EDataTypeText},
       {TPtrC(KSip), R_VPBK_FIELD_TYPE_SIP, EDataTypeText},
       {TPtrC(KEmailGen), R_VPBK_FIELD_TYPE_EMAILGEN, EDataTypeText},
       
       {TPtrC(KUrlGen), R_VPBK_FIELD_TYPE_URLGEN, EDataTypeText},
       {TPtrC(KAddrlabelGen), R_VPBK_FIELD_TYPE_ADDRLABELGEN, EDataTypeText},
       {TPtrC(KAddrpoGen), R_VPBK_FIELD_TYPE_ADDRPOGEN, EDataTypeText},
       {TPtrC(KAddrextGen), R_VPBK_FIELD_TYPE_ADDREXTGEN, EDataTypeText},
       {TPtrC(KAddrstreetGen), R_VPBK_FIELD_TYPE_ADDRSTREETGEN, EDataTypeText},
       {TPtrC(KAddrlocalGen), R_VPBK_FIELD_TYPE_ADDRLOCALGEN, EDataTypeText},
       {TPtrC(KAddrregionGen), R_VPBK_FIELD_TYPE_ADDRREGIONGEN, EDataTypeText},
       {TPtrC(KAddrpostcodeGen), R_VPBK_FIELD_TYPE_ADDRPOSTCODEGEN, EDataTypeText},
       
       {TPtrC(KAddrcountryGen), R_VPBK_FIELD_TYPE_ADDRCOUNTRYGEN, EDataTypeText},
       {TPtrC(KPagerNumber), R_VPBK_FIELD_TYPE_PAGERNUMBER, EDataTypeText},
       {TPtrC(KDtmfString), R_VPBK_FIELD_TYPE_DTMFSTRING, EDataTypeText},
       {TPtrC(KWvAddress), R_VPBK_FIELD_TYPE_WVADDRESS, EDataTypeText},
       {TPtrC(KDate), R_VPBK_FIELD_TYPE_DATE, EDataTypeDateTime},
       {TPtrC(KNote), R_VPBK_FIELD_TYPE_NOTE, EDataTypeText},
       {TPtrC(KThumbnailPath), R_VPBK_FIELD_TYPE_THUMBNAILPIC, EDataTypeText},
       {TPtrC(KThumbnailId), R_VPBK_FIELD_TYPE_THUMBNAILPIC, EDataTypeText},
       {TPtrC(KRingTone), R_VPBK_FIELD_TYPE_RINGTONE, EDataTypeText},
       {TPtrC(KRingToneId), R_VPBK_FIELD_TYPE_RINGTONE, EDataTypeText},
       
       {TPtrC(KCallerobjImg), R_VPBK_FIELD_TYPE_CALLEROBJIMG, EDataTypeText},
       {TPtrC(KCallerobjText), R_VPBK_FIELD_TYPE_CALLEROBJTEXT, EDataTypeText},
       {TPtrC(KMiddlename), R_VPBK_FIELD_TYPE_MIDDLENAME, EDataTypeText},
       {TPtrC(KDepartment), R_VPBK_FIELD_TYPE_DEPARTMENT, EDataTypeText},
       {TPtrC(KAsstname), R_VPBK_FIELD_TYPE_ASSTNAME, EDataTypeText},
       {TPtrC(KSpouse), R_VPBK_FIELD_TYPE_SPOUSE, EDataTypeText},
       {TPtrC(KChildren), R_VPBK_FIELD_TYPE_CHILDREN, EDataTypeText},
       {TPtrC(KAsstphone), R_VPBK_FIELD_TYPE_ASSTPHONE, EDataTypeText},
       
       {TPtrC(KCarphone), R_VPBK_FIELD_TYPE_CARPHONE, EDataTypeText},
       {TPtrC(KAnniversary), R_VPBK_FIELD_TYPE_ANNIVERSARY, EDataTypeDateTime},
       {TPtrC(KSyncclass), R_VPBK_FIELD_TYPE_SYNCCLASS, EDataTypeText},
       {TPtrC(KLocPrivacy), R_VPBK_FIELD_TYPE_LOCPRIVACY, EDataTypeText},
       {TPtrC(KGenlabel), R_VPBK_FIELD_TYPE_GENLABEL, EDataTypeText}
};
#endif 

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

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
void CCreatorContactElement::AsyncExecuteCommandL()
    {    
    LOGSTRING("Creator: CCreatorContactElement::AsyncExecuteCommandL");
    // Find out how many contacts should we create:
    const CCreatorScriptAttribute* contactAmountAttr = this->FindAttributeByName(KAmount);    
    TInt contactAmount = 1;    
    if( contactAmountAttr )
        {
        contactAmount = ConvertStrToIntL(contactAmountAttr->Value());
        }
    CCreatorScriptElement* fieldsElement = FindSubElement(KFields);
    TInt fieldMappingTblSize = sizeof(fieldMappingTbl)/sizeof(FieldMapping);
    if( iLoopIndex < contactAmount )
        {
        CVirtualPhonebookParameters* param = (CVirtualPhonebookParameters*) TCreatorFactory::CreatePhoneBookParametersL();
        CleanupStack::PushL(param);
        // Loop all the field elements
        if( fieldsElement && fieldsElement->SubElements().Count() > 0 )
            {
            
            const RPointerArray<CCreatorScriptElement>& fieldList = fieldsElement->SubElements();
            
            if( fieldList.Count() == 0 )
                {
                
                }
            for( TInt i = 0; i < fieldList.Count(); ++i )
                {
                TPtrC fieldName = fieldList[i]->Name();
                const CCreatorScriptAttribute* amountAttr = fieldList[i]->FindAttributeByName(KAmount);
                const CCreatorScriptAttribute* rndLenAttr = fieldList[i]->FindAttributeByName(KRandomLength);
                const CCreatorScriptAttribute* increaseAttr = fieldList[i]->FindAttributeByName(KIncrease);
                TBool increase( EFalse );
                if ( increaseAttr )
                    {
                    increase = ConvertStrToBooleanL( increaseAttr->Value() );
                    }
                CCreatorContactField* field = 0;
                for( TInt j = 0; j < fieldMappingTblSize; ++j )
                    {
                    const FieldMapping& mapping = fieldMappingTbl[j];
                    if( fieldName == mapping.iElementName )
                        {
                        TInt rndLen = 0;
                        TPtrC content = fieldList[i]->Content();
                                                                        
                        MCreatorRandomDataField::TRandomLengthType randomLenType = MCreatorRandomDataField::ERandomLengthUndefined;
                    
                        if( content == TPtrC(KEmpty) || content == TPtrC(KNullDesC) )
                            {
                            if( rndLenAttr == 0 )
                                {
                                randomLenType = MCreatorRandomDataField::ERandomLengthDefault;
                                }
                            else
                                {
                                randomLenType = ResolveRandomDataTypeL(*rndLenAttr, rndLen);
                                }
                            }
                        else
                            {
                            // Special handling for file id content:
                            if( fieldName == KThumbnailId )
                                {
                                // Thumbnail data is copied to the contact entry, so we can use temporary file:
                                CCreatorEngine::TTestDataPath id = (CCreatorEngine::TTestDataPath) iEngine->GetAttachmentIdL(content);
                                content.Set(iEngine->TestDataPathL(id));
                                }
                            else if( fieldName == KRingToneId )
                                {
                                // Contact entry contains just a link to the sound file, so we must 
                                // copy the temporary file to a permanent location:
                                CCreatorEngine::TTestDataPath id = (CCreatorEngine::TTestDataPath) iEngine->GetAttachmentIdL(content);
                                TPtrC fullTargetPath(iEngine->CreateSoundFileL(id));                            
                                content.Set(fullTargetPath);
                                }
                            }
                        TInt amountValue = 1;
                        if( amountAttr )
                            {                        
                            amountValue = ConvertStrToIntL(amountAttr->Value());
                            }
                        for( TInt k = 0; k < amountValue; ++k )
                            {
                            if ( increase &&
                                 ( fieldName.FindF( _L("number") ) > KErrNotFound ||
                                 fieldName.FindF( _L("phone") ) > KErrNotFound ) )
                                {
                                // increase phonenumber for each copy
                                HBufC* incNum = HBufC::NewLC( content.Length() + 3 );
                                if ( amountValue > 1 )
                                    {
                                    // amount defined in number field level
                                    IncreasePhoneNumL( content, k, incNum );
                                    }
                                else
                                    {
                                    // amount defined in contact field level
                                    IncreasePhoneNumL( content, iLoopIndex, incNum );
                                    }
                                field = CCreatorContactField::NewL( mapping.iFieldCode, *incNum );
                                CleanupStack::PushL( field );
                                field->SetRandomParametersL( randomLenType, rndLen );
                                param->iContactFields.AppendL( field );
                                CleanupStack::Pop( field );
                                CleanupStack::PopAndDestroy( incNum );
                                }
                            else
                                {
                                field = CCreatorContactField::NewL(mapping.iFieldCode, content);
                                CleanupStack::PushL(field);
                                field->SetRandomParametersL(randomLenType, rndLen);
                                param->iContactFields.AppendL(field);
                                CleanupStack::Pop(field);                                
                                }
                            }
                        break;
                        }
                    }
                }
            } 
        else
            {
            // No fields specified, so add all fields with random content:
            for( TInt i = 0; i < fieldMappingTblSize; ++i )
                {
                if( fieldMappingTbl[i].iElementName == KThumbnailId || 
                    fieldMappingTbl[i].iElementName == KRingToneId )
                    {
                    // Skip thumbnail and ringtone IDs, since they are duplicates with thumbnailpath and ringtonepath
                    continue;
                    }
                CCreatorContactField* field = 0;
                if(fieldMappingTbl[i].iDataType == EDataTypeText )
                    {
                    field = CCreatorContactField::NewL(fieldMappingTbl[i].iFieldCode, KNullDesC);
                    }
                else if( fieldMappingTbl[i].iDataType == EDataTypeBinary )
                    {
                    field = CCreatorContactField::NewL(fieldMappingTbl[i].iFieldCode, KNullDesC8);
                    }
                else if( fieldMappingTbl[i].iDataType == EDataTypeDateTime )
                    {                       
                    TDateTime dateTime(Time::NullTTime().DateTime());
                    field = CCreatorContactField::NewL(fieldMappingTbl[i].iFieldCode, dateTime);
                    }
                if( field )
                    {
                    CleanupStack::PushL(field);
                    field->SetRandomParametersL(MCreatorRandomDataField::ERandomLengthDefault, 0);
                    param->iContactFields.AppendL(field);
                    CleanupStack::Pop(field);
                    }
                }
            }
        iEngine->AppendToCommandArrayL(ECmdCreatePhoneBookEntryContacts, param);
        iParameters.AppendL(param); // Save the pointer to the parametes. 
        CleanupStack::Pop(param);
        StartNextLoop();
        }
    else
        {
        // stop loop and signal end of the executing command
        AsyncCommandFinished();
        }
    }

void CCreatorContactElement::ExecuteCommandL()
    {    
    LOGSTRING("Creator: CCreatorContactElement::ExecuteCommandL");
    // Find out how many contacts should we create:
    const CCreatorScriptAttribute* contactAmountAttr = this->FindAttributeByName(KAmount);    
    TInt contactAmount = 1;    
    if( contactAmountAttr )
        {
        contactAmount = ConvertStrToIntL(contactAmountAttr->Value());
        }
    CCreatorScriptElement* fieldsElement = FindSubElement(KFields);
    TInt fieldMappingTblSize = sizeof(fieldMappingTbl)/sizeof(FieldMapping);
    for( TInt cI = 0; cI < contactAmount; ++cI )
        {
        CVirtualPhonebookParameters* param = (CVirtualPhonebookParameters*) TCreatorFactory::CreatePhoneBookParametersL();
        CleanupStack::PushL(param);
        // Loop all the field elements
        if( fieldsElement && fieldsElement->SubElements().Count() > 0 )
            {
            
            const RPointerArray<CCreatorScriptElement>& fieldList = fieldsElement->SubElements();
            
            if( fieldList.Count() == 0 )
                {
                
                }
            for( TInt i = 0; i < fieldList.Count(); ++i )
                {
                TPtrC fieldName = fieldList[i]->Name();
                const CCreatorScriptAttribute* amountAttr = fieldList[i]->FindAttributeByName(KAmount);
                const CCreatorScriptAttribute* rndLenAttr = fieldList[i]->FindAttributeByName(KRandomLength);
                const CCreatorScriptAttribute* increaseAttr = fieldList[i]->FindAttributeByName(KIncrease);
                TBool increase( EFalse );
                if ( increaseAttr )
                    {
                    increase = ConvertStrToBooleanL( increaseAttr->Value() );
                    }
                CCreatorContactField* field = 0;
                for( TInt j = 0; j < fieldMappingTblSize; ++j )
                    {
                    const FieldMapping& mapping = fieldMappingTbl[j];
                    if( fieldName == mapping.iElementName )
                        {
                        TInt rndLen = 0;
                        TPtrC content = fieldList[i]->Content();
                                                                        
                        MCreatorRandomDataField::TRandomLengthType randomLenType = MCreatorRandomDataField::ERandomLengthUndefined;
                    
                        if( content == TPtrC(KEmpty) || content == TPtrC(KNullDesC) )
                            {
                            if( rndLenAttr == 0 )
                                {
                                randomLenType = MCreatorRandomDataField::ERandomLengthDefault;
                                }
                            else
                                {
                                randomLenType = ResolveRandomDataTypeL(*rndLenAttr, rndLen);
                                }
                            }
                        else
                            {
                            // Special handling for file id content:
                            if( fieldName == KThumbnailId )
                                {
                                // Thumbnail data is copied to the contact entry, so we can use temporary file:
                                CCreatorEngine::TTestDataPath id = (CCreatorEngine::TTestDataPath) iEngine->GetAttachmentIdL(content);
                                content.Set(iEngine->TestDataPathL(id));
                                }
                            else if( fieldName == KRingToneId )
                            	{
                            	// Contact entry contains just a link to the sound file, so we must 
                            	// copy the temporary file to a permanent location:
                            	CCreatorEngine::TTestDataPath id = (CCreatorEngine::TTestDataPath) iEngine->GetAttachmentIdL(content);
                            	TPtrC fullTargetPath(iEngine->CreateSoundFileL(id));                           	
                            	content.Set(fullTargetPath);
                            	}
                            }
                        TInt amountValue = 1;
                        if( amountAttr )
                            {                        
                            amountValue = ConvertStrToIntL(amountAttr->Value());
                            }
                        for( TInt k = 0; k < amountValue; ++k )
                            {
                            if ( increase &&
                                 ( fieldName.FindF( _L("number") ) > KErrNotFound ||
                                 fieldName.FindF( _L("phone") ) > KErrNotFound ) )
                                {
                                // increase phonenumber for each copy
                                HBufC* incNum = HBufC::NewLC( content.Length() + 3 );
                                if ( amountValue > 1 )
                                    {
                                    // amount defined in number field level
                                    IncreasePhoneNumL( content, k, incNum );
                                    }
                                else
                                    {
                                    // amount defined in contact field level
                                    IncreasePhoneNumL( content, cI, incNum );
                                    }
                                field = CCreatorContactField::NewL( mapping.iFieldCode, *incNum );
                                CleanupStack::PushL( field );
                                field->SetRandomParametersL( randomLenType, rndLen );
                                param->iContactFields.AppendL( field );
                                CleanupStack::Pop( field );
                                CleanupStack::PopAndDestroy( incNum );
                                }
                            else
                                {
                                field = CCreatorContactField::NewL(mapping.iFieldCode, content);
                                CleanupStack::PushL(field);
                                field->SetRandomParametersL(randomLenType, rndLen);
                                param->iContactFields.AppendL(field);
                                CleanupStack::Pop(field);                                
                                }
                            }
                        break;
                        }
                    }
                }
            } 
        else
        	{
        	// No fields specified, so add all fields with random content:
        	for( TInt i = 0; i < fieldMappingTblSize; ++i )
        		{
        		if( fieldMappingTbl[i].iElementName == KThumbnailId || 
        			fieldMappingTbl[i].iElementName == KRingToneId )
        			{
        			// Skip thumbnail and ringtone IDs, since they are duplicates with thumbnailpath and ringtonepath
        			continue;
        			}
        		CCreatorContactField* field = 0;
        		if(fieldMappingTbl[i].iDataType == EDataTypeText )
        			{
        			field = CCreatorContactField::NewL(fieldMappingTbl[i].iFieldCode, KNullDesC);
        			}
        		else if( fieldMappingTbl[i].iDataType == EDataTypeBinary )
        			{
        			field = CCreatorContactField::NewL(fieldMappingTbl[i].iFieldCode, KNullDesC8);
        			}
        		else if( fieldMappingTbl[i].iDataType == EDataTypeDateTime )
        			{                    	
        			TDateTime dateTime(Time::NullTTime().DateTime());
        			field = CCreatorContactField::NewL(fieldMappingTbl[i].iFieldCode, dateTime);
        			}
        		if( field )
        			{
        			CleanupStack::PushL(field);
        			field->SetRandomParametersL(MCreatorRandomDataField::ERandomLengthDefault, 0);
        			param->iContactFields.AppendL(field);
        			CleanupStack::Pop(field);
        			}
        		}
        	}
        iEngine->AppendToCommandArrayL(ECmdCreatePhoneBookEntryContacts, param);
        iParameters.AppendL(param); // Save the pointer to the parametes. 
        CleanupStack::Pop(param);
        }
    }
#else
void CCreatorContactElement::ExecuteCommandL()
    {}
#endif

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

void CCreatorContactSetElement::AsyncExecuteCommand()
    { 
    AsyncCommandFinished();
    }

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

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ )
void CCreatorContactGroupElement::AsyncExecuteCommandL()
    {
    LOGSTRING("Creator: CCreatorContactGroupElement::AsyncExecuteCommandL");
    // Get attributes (amount and name)
    const CCreatorScriptAttribute* groupAmountAttr = this->FindAttributeByName(KAmount);        
    const CCreatorScriptAttribute* groupNameAttr = this->FindAttributeByName(KName);
    // How many groups to create:
    TInt groupAmount = 1;        
    if( groupAmountAttr )
        {
        groupAmount = ConvertStrToIntL(groupAmountAttr->Value());
        }
    
    if( iLoopIndex <  groupAmount )
        {        
        CCreatorScriptElement* membersElement = FindSubElement(KMembers);
        
        if( membersElement )
            {    
            CVirtualPhonebookParameters* param = (CVirtualPhonebookParameters*) TCreatorFactory::CreatePhoneBookParametersL();            
            CleanupStack::PushL(param);
            
            // Links to contact-sets:
            const RPointerArray<CCreatorScriptElement>& linkList = membersElement->SubElements();
            if( linkList.Count() > 0 )
                {
                for( TInt i = 0; i < linkList.Count(); ++i )
                    {
                    AppendContactSetReferenceL(*linkList[i], param->iLinkIds);
                    }                
                }
            
            if( groupNameAttr )
                {
                param->iGroupName->Des().Copy( groupNameAttr->Value() );
                }
            else
                {
                param->iGroupName->Des().Copy( iEngine->RandomString(CCreatorEngine::EGroupName) );
                }                
            iEngine->AppendToCommandArrayL(ECmdCreatePhoneBookEntryGroups, param);
            CleanupStack::Pop(param);
            }
        StartNextLoop();
        }
    else
        {
        // stop loop and signal end of the executing command
        AsyncCommandFinished();        
        }
    }

void CCreatorContactGroupElement::ExecuteCommandL()
    {
    LOGSTRING("Creator: CCreatorContactGroupElement::ExecuteCommandL");
    // Get attributes (amount and name)
    const CCreatorScriptAttribute* groupAmountAttr = this->FindAttributeByName(KAmount);        
    const CCreatorScriptAttribute* groupNameAttr = this->FindAttributeByName(KName);
    // How many groups to create:
    TInt groupAmount = 1;        
    if( groupAmountAttr )
        {
        groupAmount = ConvertStrToIntL(groupAmountAttr->Value());
        }
    
    for( TInt i = 0; i < groupAmount; ++i )
        {        
        CCreatorScriptElement* membersElement = FindSubElement(KMembers);
        
        if( membersElement )
            {    
            CVirtualPhonebookParameters* param = (CVirtualPhonebookParameters*) TCreatorFactory::CreatePhoneBookParametersL();            
            CleanupStack::PushL(param);
            
            // Links to contact-sets:
            const RPointerArray<CCreatorScriptElement>& linkList = membersElement->SubElements();
            if( linkList.Count() > 0 )
                {
                for( TInt i = 0; i < linkList.Count(); ++i )
                    {
                    AppendContactSetReferenceL(*linkList[i], param->iLinkIds);
                    }                
                }
            
            if( groupNameAttr )
                {
                param->iGroupName->Des().Copy( groupNameAttr->Value() );
                }
            else
                {
                param->iGroupName->Des().Copy( iEngine->RandomString(CCreatorEngine::EGroupName) );
                }                
            iEngine->AppendToCommandArrayL(ECmdCreatePhoneBookEntryGroups, param);
            CleanupStack::Pop(param);
            }
        }
    }
#else
void CCreatorContactGroupElement::ExecuteCommandL()
    {}
#endif

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
