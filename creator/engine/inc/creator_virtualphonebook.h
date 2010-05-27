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





#ifndef __CREATORVIRTUALPHONEBOOK_H__
#define __CREATORVIRTUALPHONEBOOK_H__

#include "engine.h"
#include "creator_phonebookbase.h"
#include "creator_randomdatafield.h"

#include <e32base.h>
#include <cntdef.h>
#include <cntdb.h>
#include <cntitem.h>
#include <cntfldst.h>
#include <cntview.h>
#include <cvpbkcontactmanager.h>
#include <pbkfields.hrh>
#include <cpbk2imagemanager.h>
#include <mvpbkcontactstorelistobserver.h>
#include <mvpbkbatchoperationobserver.h>
#include <mvpbkcontactfindobserver.h>
#include <mvpbkcontactobserver.h>
#include <mpbk2imageoperationobservers.h>
#include <vpbkeng.rsg>
#include <cvpbkcontactlinkarray.h>
#include <MVPbkContactViewObserver.h>

class CCreatorEngine;
class CCreatorModuleBaseParameters;

class MVPbkStoreContact;
class MVPbkContactStore;
class CAsyncWaiter;
class CContactDatabase;
class MVPbkContactLinkArray;
class CVirtualPhonebookParameters;


class CCreatorVirtualPhonebook : public CCreatorPhonebookBase,  
	public MVPbkContactStoreListObserver, public MVPbkBatchOperationObserver,
	public MVPbkContactFindObserver, public MVPbkContactObserver, public MPbk2ImageSetObserver,
	public MVPbkSingleContactOperationObserver, public MVPbkContactViewObserver
    {
public: 
    static CCreatorVirtualPhonebook* NewL(CCreatorEngine* aEngine);
    static CCreatorVirtualPhonebook* NewLC(CCreatorEngine* aEngine);
    ~CCreatorVirtualPhonebook();

private:
    CCreatorVirtualPhonebook();
    void ConstructL(CCreatorEngine* aEngine); // from MCreatorModuleBase

public:  
    TInt CreateContactEntryL(CCreatorModuleBaseParameters *aParameters);    
    TInt CreateGroupEntryL(CCreatorModuleBaseParameters *aParameters);
    TInt CreateSubscribedContactEntryL(CCreatorModuleBaseParameters *aParameters); 

    void AddFieldToContactItemL(MVPbkStoreContact& aItem, TPbkFieldId aFieldId, const TDesC& aFieldText);
    void AddFieldToContactItemL(MVPbkStoreContact& aItem, TPbkFieldId aFieldId, const TDesC8& aFieldText);
    void AddFieldToContactItemL(MVPbkStoreContact& aItem, TPbkFieldId aFieldId, const TTime& aFieldText);
    
    void AddImageToContactL(MVPbkStoreContact& aContact, TInt aFieldResId, const TDesC& aFileName);
    TBool IsActive();
    void CancelOperation();
    void DeleteAllL();
    void DeleteAllCreatedByCreatorL();
    void DeleteAllGroupsL();
    void DeleteAllGroupsCreatedByCreatorL();
    
private:
    void CompactPbkDatabaseL(TBool aCompressImmediately=EFalse);    
    void AddFieldToParamsL(TInt aFieldType, const TDesC& aData);
    void AddFieldToParamsL(TInt aFieldType, const TDesC8& aData);
    void AddFieldToParamsL(TInt aFieldType, const TTime& aData);    
    void InitializeContactParamsL(/*CCreatorModuleBaseParameters* aParameters*/);
    TBool IsContactGroupL(const MVPbkContactLink& aLink);
    void StoreLinksForDeleteL( MVPbkContactLinkArray& aLinks, TUid aStoreUid );
    void DeleteContactsL( MVPbkContactLinkArray* aContacts, TBool aGroup );
    void DeleteItemsCreatedWithCreatorL( TUid aStoreUid );
    void DoDeleteItemsCreatedWithCreatorL( TUid aStoreUid, CDictionaryFileStore* aStore );
    TBool HasOtherThanGroupsL( MVPbkContactLinkArray* aContacts );

public: // MVPbkSingleContactOperationObserver
    
    void VPbkSingleContactOperationComplete(
                    MVPbkContactOperationBase& aOperation,
                    MVPbkStoreContact* aContact );
    void VPbkSingleContactOperationFailed(
                    MVPbkContactOperationBase& aOperation, 
                    TInt aError );

private: // MVPbkContactViewObserver
    
    void ContactViewReady( MVPbkContactViewBase& aView );
    void ContactViewUnavailable(
            MVPbkContactViewBase& aView );
    void ContactAddedToView(
            MVPbkContactViewBase& aView, 
            TInt aIndex, 
            const MVPbkContactLink& aContactLink );
    void ContactRemovedFromView(
            MVPbkContactViewBase& aView, 
            TInt aIndex, 
            const MVPbkContactLink& aContactLink );
    void ContactViewError(
            MVPbkContactViewBase& aView, 
            TInt aError, 
            TBool aErrorNotified );

private:
	CVPbkContactManager* iContactManager;
    TInt iOpCounter;

    CVirtualPhonebookParameters* iParameters;
    
    static TInt iPhoneNumberFields[];
    static TInt iUrlFields[];
    static TInt iEmailFields[];
    CVPbkContactLinkArray* iContactLinkArray;
    CVPbkContactLinkArray* iContactsToDelete;
    CVPbkContactLinkArray* iContactGroupsToDelete;
    RPointerArray<MVPbkContactLinkArray> iPreviousDeleteLinks;   
    
private:
	//new variables
	/// Ref: the target of the copy
	MVPbkContactStore* iStore;
    
    CAsyncWaiter* iWaiter;	
    
    /// Own: Contact database for this store
    CContactDatabase* iContactDb;    
    
    //Contacts found in contacts db.
    MVPbkContactLinkArray* iContactResults;
    // Contact groups that are found in the store. These are used in filtering
    // the groups from the find results.
    MVPbkContactLinkArray* iContactGroupsInStore;
    
    // Phonebook asynchronous operation
    MVPbkContactOperationBase* iOperation;
    
    // Flag to indicate whether it is required to notify
    // iEngine after operation is complete. This happens
    // when user has cancelled the operation during 
    // iOperation exists.
    TBool iCancelCbRequested;

public:
	//Observer implentation from MVPbkContactStoreListObserver
	void OpenComplete();

	//observer implentation from MVPbkContactStoreObserver

	void StoreReady(MVPbkContactStore& aContactStore);

	void StoreUnavailable(MVPbkContactStore& aContactStore, 
	        TInt aReason);

	void HandleStoreEventL(
	        MVPbkContactStore& aContactStore, 
	        TVPbkContactStoreEvent aStoreEvent);

	 TAny* ContactStoreObserverExtension(TUid aExtensionUid); 
	        	
	

    
    
public:
	//Observer implementation for MVPbkBatchOperationObserver

	void StepComplete( 
	        MVPbkContactOperationBase& aOperation,
	        TInt aStepSize );
	
	TBool StepFailed(
	        MVPbkContactOperationBase& aOperation,
	        TInt aStepSize, TInt aError );
	
	void OperationComplete( 
	        MVPbkContactOperationBase& aOperation );
	
	TAny* BatchOperationObserverExtension( 
	        TUid aExtensionUid ) ;

	
	//--------------------------------------------
	//Observer implementation for MVPbkContactFindObserver
	public: // Interface

	        void FindCompleteL( MVPbkContactLinkArray* aResults );

	        void FindFailed( TInt aError );

	        TAny* ContactFindObserverExtension( 
	                TUid aExtensionUid );	
	
	
	
   	//--------------------------------------------
   	//Observer implementation for MVPbkContactObserver

	public:
		void ContactOperationCompleted(TContactOpResult aResult);
	
		void ContactOperationFailed
			(TContactOp aOpCode, TInt aErrorCode, TBool aErrorNotified);
	
	    TAny* ContactObserverExtension(TUid aExtensionUid) ;
	     //~MVPbkContactObserver();	        
	        
	//--------------------------------------------
	// Implementation of MPbk2ImageSetObserver:
	public:
	    void Pbk2ImageSetComplete(MPbk2ImageOperation& aOperation);
	    void Pbk2ImageSetFailed(MPbk2ImageOperation& aOperation, TInt aError );	        
    };

class CCreatorContactFieldImpl;
class CCreatorContactField : public CBase, public MCreatorRandomDataField
    {
public:    
    static CCreatorContactField* NewL(TInt aFieldType, const TDesC& aData);
    static CCreatorContactField* NewL(TInt aFieldType, const TDesC8& aData);
    static CCreatorContactField* NewL(TInt aFieldType, const TTime& aData);
    
    static CCreatorContactField* NewL(TInt aFieldType, TRandomLengthType aRandomLenType, TInt aRandomLen);
    
    ~CCreatorContactField();    
    TInt FieldType();
    virtual void AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk);
    virtual void SetRandomParametersL(  MCreatorRandomDataField::TRandomLengthType aRandomLenType = MCreatorRandomDataField::ERandomLengthDefault, 
                                TInt aRandomLen = 0 );
    virtual void ParseL(CCommandParser* parser);
    
private:    
    CCreatorContactField();
    void ConstructL(TInt aFieldType, const TDesC& aData);
    void ConstructL(TInt aFieldType, const TDesC8& aData);
    void ConstructL(TInt aFieldType, const TTime& aData);
    CCreatorContactFieldImpl* pImpl;    
    };
    
    
/**
 * Virtual phonebook parameters
 */
    

class CVirtualPhonebookParameters : public CCreatorModuleBaseParameters
    {
public:
	void ParseL(CCommandParser* parser, TParseParams /*aCase = 0*/);
	
	TInt CVirtualPhonebookParameters::ScriptLinkId() const;
	void CVirtualPhonebookParameters::SetScriptLinkId(TInt aLinkId);
	RPointerArray<CCreatorContactField> iContactFields;

    HBufC*  iGroupName;
    TInt    iContactsInGroup;
    TInt iNumberOfPhoneNumberFields;
    TInt iNumberOfURLFields;
    TInt iNumberOfEmailAddressFields;
    TInt iContactSetPtr;
    RArray<TLinkIdParam> iLinkIds; // For contactgroup. Stores the linked contact ids.
    
public:
    CVirtualPhonebookParameters();
    ~CVirtualPhonebookParameters();
    
private:
    TInt iLinkId; // For contact. Stores the contact id
    };



#endif // __CREATORVIRTUALPHONEBOOK_H__
