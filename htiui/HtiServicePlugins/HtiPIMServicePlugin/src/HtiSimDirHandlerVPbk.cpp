/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of SIM card contacts handling using the new
*                Virtual Phonebook API
*
*/


// INCLUDE FILES
#include "HtiSimDirHandlerVPbk.h"
#include "HtiPIMServicePlugin.h"

#include <CVPbkContactManager.h>
#include <CVPbkContactLinkArray.h>
#include <CVPbkContactStoreUriArray.h>
#include <CVPbkContactViewDefinition.h>
#include <CVPbkSortOrder.h>
#include <MVPbkContactFieldTextData.h>
#include <MVPbkContactLink.h>
#include <MVPbkContactOperationBase.h>
#include <MVPbkContactStore.h>
#include <MVPbkContactStoreInfo.h>
#include <MVPbkContactStoreList.h>
#include <MVPbkContactStoreProperties.h>
#include <MVPbkContactView.h>
#include <MVPbkContactViewBase.h>
#include <MVPbkFieldType.h>
#include <MVPbkStoreContact.h>
#include <MVPbkStoreContactField.h>
#include <MVPbkContactStoreObserver.h>
#include <TVPbkContactStoreUriPtr.h>
#include <TVPbkFieldVersitProperty.h>
#include <VPbkContactStoreUris.h>
#include <VPbkContactView.hrh>
#include <VPbkFieldType.hrh>
#include <VPbkEng.rsg>

#include <HtiDispatcherInterface.h>
#include <HTILogging.h>

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS
const TInt KSimInfoResponseLength = 12;

// MACROS

// LOCAL CONSTANTS AND MACROS
_LIT8( KErrorUnrecognizedCommand, "Unrecognized command" );
_LIT8( KErrorInvalidParameters, "Invalid command parameters" );
_LIT8( KErrorImportFailed, "Contact import failed" );
_LIT8( KErrorDeleteFailed, "Failed to delete contact" );
_LIT8( KErrorSimCardInfoFailed, "Failed to get SIM card info" );
_LIT8( KErrorSimStoreOpenFailed, "Failed to open SIM contact store" );
_LIT8( KErrorSimStoreUnavailable, "SIM contact store unavailable" );

_LIT8( KErrorContactOperationFailed, "SIM contact operation failed" );

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CHtiSimDirHandlerVPbk* CHtiSimDirHandlerVPbk::NewL()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::NewL" );
    CHtiSimDirHandlerVPbk* self = new (ELeave) CHtiSimDirHandlerVPbk();
    CleanupStack::PushL ( self );
    self->ConstructL();
    CleanupStack::Pop();
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::NewL" );
    return self;
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::CHtiSimDirHandlerVPbk
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------------------------
CHtiSimDirHandlerVPbk::CHtiSimDirHandlerVPbk():iIsBusy( EFalse ),
                                               iIsStoreOpen( EFalse )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::CHtiSimDirHandlerVPbk" );

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::CHtiSimDirHandlerVPbk" );
    }


// -----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::~CHtiSimDirHandlerVPbk
// Destructor.
// -----------------------------------------------------------------------------
CHtiSimDirHandlerVPbk::~CHtiSimDirHandlerVPbk()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::~CHtiSimDirHandlerVPbk" );

    if ( iContactView )
        {
        HTI_LOG_TEXT( "Deleting iContactView" );
        iContactView->RemoveObserver( *this );
        delete iContactView;
        }
    if ( iSimStore )
        {
        HTI_LOG_TEXT( "Closing iSimStore" );
        iSimStore->Close( *this );
        }
    if ( iContactManager )
        {
        HTI_LOG_TEXT( "Deleting iContactManager" );
        delete iContactManager;
        }
    if ( iMessage )
        {
        HTI_LOG_TEXT( "Deleting iMessage" );
        delete iMessage;
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::~CHtiSimDirHandlerVPbk" );
    }


// -----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ConstructL()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ConstructL" );

    CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
    uriArray->AppendL( TVPbkContactStoreUriPtr(
        VPbkContactStoreUris::SimGlobalAdnUri() ) );

    iContactManager = CVPbkContactManager::NewL( *uriArray );
    CleanupStack::PopAndDestroy( uriArray );

    MVPbkContactStoreList& storeList = iContactManager->ContactStoresL();
    iSimStore = storeList.Find( TVPbkContactStoreUriPtr(
        VPbkContactStoreUris::SimGlobalAdnUri() ) );

    if ( !iSimStore )
        {
        HTI_LOG_TEXT( "SIM ADN store not found - leaving" );
        User::Leave( KErrNotFound );
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ConstructL" );
    }


// -----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::SetDispatcher
// Sets the dispatcher pointer.
// -----------------------------------------------------------------------------

void CHtiSimDirHandlerVPbk::SetDispatcher( MHtiDispatcher* aDispatcher )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::SetDispatcher" );
    iDispatcher = aDispatcher;
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::SetDispatcher" );
    }


// -----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ProcessMessageL
// Parses the received message and calls handler functions.
// -----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ProcessMessageL( const TDesC8& aMessage,
    THtiMessagePriority /*aPriority*/ )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ProcessMessageL" );

    if ( iIsBusy )
        {
        HTI_LOG_TEXT( "HtiSimDirHandlerVPbk is busy - leaving" );
        User::Leave( KErrInUse );
        }

    // Will be set to EFalse in the SendOkMsgL or SendErrorMessageL methods.
    iIsBusy = ETrue;

    delete iMessage;
    iMessage = NULL;

    // Zero legth of aMessage tested already in CHtiPIMServicePlugin.
    // Other sanity checks must be done here.

    // Trap the AllocL to be able to set iIsBusy false before leaving.
    TRAPD( err, iMessage = aMessage.Right( aMessage.Length() - 1 ).AllocL() );
    if ( err != KErrNone )
        {
        iIsBusy = EFalse;
        User::Leave( err );
        }

    iCommand = aMessage.Ptr()[0];

    // Do basic validity checking for message
    TBool isParamsOk = ETrue;
    switch ( iCommand )
        {
        case CHtiPIMServicePlugin::ESimCardInfo:
            {
            if ( iMessage->Length() != 0 )
                {
                isParamsOk = EFalse;
                }
            break;
            }
        case CHtiPIMServicePlugin::EImportSimContact:
            {
            if ( !CheckImportMsg() )
                {
                isParamsOk = EFalse;
                }
            break;
            }
        case CHtiPIMServicePlugin::EDeleteSimContact:
            {
            if ( iMessage->Length() != 4 && iMessage->Length() != 0 )
                {
                isParamsOk = EFalse;
                }
            break;
            }
        default:
            {
            SendErrorMessageL( KErrArgument, KErrorUnrecognizedCommand );
            return;
            }
        }

    if ( !isParamsOk )
        {
        SendErrorMessageL( KErrArgument, KErrorInvalidParameters );
        }

    else
        {
        if ( iIsStoreOpen )
            {
            StoreReady( *iSimStore );
            }
        else
            {
            // Start async open operation.
            // StoreReady callback will be called when opening is complete.
            TRAPD( err, iSimStore->OpenL( *this ) );
            if ( err != KErrNone )
                {
                HTI_LOG_FORMAT(
                        "Leave from CHtiSimDirHandler::OpenL() %d", err );
                SendErrorMessageL( err, KErrorSimStoreOpenFailed );
                }
            }
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ProcessMessageL" );
    }


// -----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::IsBusy
// -----------------------------------------------------------------------------
//
TBool CHtiSimDirHandlerVPbk::IsBusy()
    {
    return iIsBusy;
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::OpenComplete
// Called when the opening process is complete.
// From MVPbkContactStoreListObserver
// ----------------------------------------------------------------------------
/*
void CHtiSimDirHandlerVPbk::OpenComplete()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::OpenComplete" );

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::OpenComplete" );
    }
*/

// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::StoreReady
// Called when a contact store is ready to use.
// From MVPbkContactStoreObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::StoreReady( MVPbkContactStore& aContactStore )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::StoreReady" );

    if ( !iIsBusy || iSimStore != &aContactStore )
        {
        return;
        }

    iIsStoreOpen = ETrue;

    if ( iCommand == CHtiPIMServicePlugin::ESimCardInfo )
        {
        TRAPD( err, HandleSimCardInfoL() );
        if ( err != KErrNone )
            {
            TRAP_IGNORE( SendErrorMessageL( err, KErrorSimCardInfoFailed ) );
            }
        }

    else if ( iCommand == CHtiPIMServicePlugin::EImportSimContact )
        {
        TRAPD( err, HandleSimContactImportL() );
        if ( err != KErrNone )
            {
            TRAP_IGNORE( SendErrorMessageL( err, KErrorImportFailed ) );
            }
        }

    else if ( iCommand == CHtiPIMServicePlugin::EDeleteSimContact )
        {
        TRAPD( err, HandleSimContactDeleteL() );
        if ( err != KErrNone )
            {
            TRAP_IGNORE( SendErrorMessageL( err, KErrorDeleteFailed ) );
            }
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::StoreReady" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::StoreUnavailable
// Called when a contact store becomes unavailable.
// From MVPbkContactStoreObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::StoreUnavailable( MVPbkContactStore& aContactStore,
                                              TInt aReason )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::StoreUnavailable" );

    if ( iIsBusy && iSimStore == &aContactStore )
        {
        TRAP_IGNORE( SendErrorMessageL( aReason, KErrorSimStoreUnavailable ) );
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::StoreUnavailable" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::HandleStoreEventL
// Called when changes occur in the contact store.
// From MVPbkContactStoreObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::HandleStoreEventL( MVPbkContactStore& aContactStore,
                        TVPbkContactStoreEvent aStoreEvent )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::HandleStoreEventL" );

    if ( iIsBusy && iSimStore == &aContactStore &&
         aStoreEvent.iEventType == TVPbkContactStoreEvent::EContactAdded )
        {
        HTI_LOG_TEXT( "Contact added" );
        TInt entryId = 0;
        TBuf8<4> idBuf;
        idBuf.Append( ( TUint8* ) &entryId, 4 );
        TRAP_IGNORE( SendOkMsgL( idBuf ) );
        }

    else if ( iIsBusy && iSimStore == &aContactStore &&
              aStoreEvent.iEventType == TVPbkContactStoreEvent::EContactDeleted )
        {
        HTI_LOG_TEXT( "Contact deleted" );
        if ( iMessage->Length() > 0 )  // This is a single deletion
            {
            TRAP_IGNORE( SendOkMsgL( KNullDesC8 ) );
            }
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::HandleStoreEventL" );
    }

// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactOperationCompleted
// Called when a contact operation has succesfully completed.
// From MVPbkContactObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ContactOperationCompleted( TContactOpResult /*aResult*/ )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ContactOperationCompleted" );

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ContactOperationCompleted" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactOperationFailed
// Called when a contact operation has failed.
// From MVPbkContactObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ContactOperationFailed( TContactOp aOpCode,
                                                    TInt aErrorCode,
                                                    TBool aErrorNotified )
    {

    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ContactOperationFailed" );
    HTI_LOG_FORMAT( "aOpCode: %d", aOpCode );
    HTI_LOG_FORMAT( "aErrorCode: %d", aErrorCode );
    HTI_LOG_FORMAT( "aErrorNotified: %d", aErrorNotified );
    TRAP_IGNORE( SendErrorMessageL( aErrorCode, KErrorContactOperationFailed ) );
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ContactOperationFailed" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactViewReady
// Called when a view is ready for use.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ContactViewReady( MVPbkContactViewBase& aView )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ContactViewReady" );

    if ( iContactView == &aView && iIsBusy &&
            iCommand == CHtiPIMServicePlugin::EDeleteSimContact )
        {
        TRAPD( err, DeleteContactsInViewL() );
        if ( err != KErrNone )
            {
            TRAP_IGNORE( SendErrorMessageL( err, KErrorDeleteFailed ) );
            }
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ContactViewReady" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactViewUnavailable
// Called when a view is unavailable for a while.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ContactViewUnavailable( MVPbkContactViewBase& /*aView*/ )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ContactViewUnavailable" );

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ContactViewUnavailable" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactAddedToView
// Called when a contact has been added to the view.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ContactAddedToView( MVPbkContactViewBase& /*aView*/,
        TInt /*aIndex*/, const MVPbkContactLink& /*aContactLink*/ )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ContactAddedToView" );

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ContactAddedToView" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactRemovedFromView
// Called when a contact has been removed from a view.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ContactRemovedFromView( MVPbkContactViewBase& /*aView*/,
        TInt /*aIndex*/, const MVPbkContactLink& /*aContactLink*/ )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ContactRemovedFromView" );

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ContactRemovedFromView" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactViewError
// Called when an error occurs in the view.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::ContactViewError( MVPbkContactViewBase& aView,
                                              TInt aError,
                                              TBool aErrorNotified )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::ContactViewError" );
    HTI_LOG_FORMAT( "CHtiSimDirHandlerVPbk::ContactViewError: %d", aError );
    HTI_LOG_FORMAT( "ErrorNotified = %d", aErrorNotified );
    if ( iContactView == &aView )
        {
        iContactView->RemoveObserver( *this );
        if ( iIsBusy && iCommand == CHtiPIMServicePlugin::EDeleteSimContact )
            {
            SendErrorMessageL( aError, KErrorDeleteFailed );
            }
        }
    aErrorNotified = aErrorNotified;  // avoid compiler warning
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::ContactViewError" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::StepComplete
// Called when one step of the batch operation is complete.
// From MVPbkBatchOperationObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::StepComplete( MVPbkContactOperationBase& /*aOperation*/,
                           TInt aStepSize )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::StepComplete" );
    HTI_LOG_FORMAT( "Step size = %d", aStepSize );
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::StepComplete" );
    aStepSize = aStepSize; // avoid compiler warning
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::StepFailed
// Called when one step of the batch operation fails.
// From MVPbkBatchOperationObserver
// ----------------------------------------------------------------------------
TBool CHtiSimDirHandlerVPbk::StepFailed( MVPbkContactOperationBase& aOperation,
                                         TInt aStepSize, TInt aError )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::StepFailed" );
    HTI_LOG_FORMAT( "Error %d", aError );
    if ( iCurrentOperation == &aOperation )
        {
        // We are returning EFalse (= do not continue) we can delete
        // the operation. OperationComplete() won't be called.
        delete iCurrentOperation;
        iCurrentOperation = NULL;
        TRAP_IGNORE( SendErrorMessageL( aError, KErrorDeleteFailed ) );
        }
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::StepFailed" );
    aStepSize = aStepSize; // avoid compiler warning
    return EFalse; // do not continue
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::OperationComplete
// Called when operation is completed.
// From MVPbkBatchOperationObserver
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::OperationComplete(
            MVPbkContactOperationBase& aOperation )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::OperationComplete" );
    // Operation is complete -> delete it
    if ( iCurrentOperation == &aOperation )
        {
        delete iCurrentOperation;
        iCurrentOperation = NULL;
        if ( iIsBusy && iCommand == CHtiPIMServicePlugin::EDeleteSimContact )
            {
            // Delete operation has completed
            TRAP_IGNORE( SendOkMsgL( KNullDesC8 ) );
            }
        }
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::OperationComplete" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::HandleSimCardInfoL
// Gets information about the SIM card.
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::HandleSimCardInfoL()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::HandleSimCardInfoL" );

    // Get entry counts
    const MVPbkContactStoreInfo& info = iSimStore->StoreInfo();
    TInt maxEntries = info.MaxNumberOfContactsL();
    TInt currentEntries = info.NumberOfContactsL();
    HTI_LOG_FORMAT( "Current entries = %d", currentEntries );
    HTI_LOG_FORMAT( "Max entries = %d", maxEntries );

    // Create new entry object to get field informations
    MVPbkStoreContact* entry = iSimStore->CreateNewContactLC();
    TVPbkFieldVersitProperty prop;
    TVPbkFieldTypeParameters param;

    // Resolve field types
    prop.SetName( EVPbkVersitNameN );
    const MVPbkFieldType* nameFieldType =
        iContactManager->FieldTypes().FindMatch( prop, 0 );

    const MVPbkFieldType* secNameFieldType =
        iContactManager->FieldTypes().Find( R_VPBK_FIELD_TYPE_SECONDNAME );

    param.Reset();
    param.Add( EVPbkVersitParamCELL );
    prop.SetName( EVPbkVersitNameTEL );
    prop.SetParameters( param );
    const MVPbkFieldType* numberFieldType =
        iContactManager->FieldTypes().FindMatch( prop, 0 );

    param.Reset();
    param.Add( EVPbkVersitParamINTERNET );
    prop.SetName( EVPbkVersitNameEMAIL );
    prop.SetParameters( param );
    const MVPbkFieldType* mailFieldType =
        iContactManager->FieldTypes().FindMatch( prop, 0 );

    // Get max field counts
    TInt maxNumbers = entry->MaxNumberOfFieldL( *numberFieldType );
    TInt maxSecondNames = entry->MaxNumberOfFieldL( *secNameFieldType );
    TInt maxEmails = entry->MaxNumberOfFieldL( *mailFieldType );

    HTI_LOG_FORMAT( "Max numbers = %d", maxNumbers );
    HTI_LOG_FORMAT( "Max second names = %d", maxSecondNames );
    HTI_LOG_FORMAT( "Max emails = %d", maxEmails );

    // Create field objects to get field data max lengths and verify that
    // fields can be created. It is assumed that all SIM cards support name
    // and number fields but e-mail and second name fields are TRAP:ed as
    // creating them would cause a leave if not supported by the SIM card.
    // It was noticed that with a SIM card not supporting the second name
    // field the MaxNumberOfFieldL method for that field returns 1 but then
    // the CreateFieldLC leaves with KErrNotSupported.
    MVPbkStoreContactField* nameField = entry->CreateFieldLC( *nameFieldType );
    MVPbkContactFieldTextData& nameFieldData =
        MVPbkContactFieldTextData::Cast( nameField->FieldData() );
    TInt maxNameLength = nameFieldData.MaxLength();
    CleanupStack::PopAndDestroy(); // nameField

    MVPbkStoreContactField* numberField = entry->CreateFieldLC( *numberFieldType );
    MVPbkContactFieldTextData& numberFieldData =
        MVPbkContactFieldTextData::Cast( numberField->FieldData() );
    TInt maxNumberLength = numberFieldData.MaxLength();
    CleanupStack::PopAndDestroy(); // numberField

    MVPbkStoreContactField* mailField = NULL;
    TInt maxMailLength = 0;
    if ( maxEmails > 0 )
        {
        TRAPD( err, mailField = entry->CreateFieldLC( *mailFieldType );
            CleanupStack::Pop(); ); // Popping inside the TRAP
        if ( err != KErrNone )
            {
            maxEmails = 0;
            }
        else
            {
            MVPbkContactFieldTextData& mailFieldData =
                MVPbkContactFieldTextData::Cast( mailField->FieldData() );
            maxMailLength = mailFieldData.MaxLength();
            delete mailField;
            mailField = NULL;
            }
        }

    MVPbkStoreContactField* secNameField = NULL;
    TInt maxSecNameLength = 0;
    if ( maxSecondNames > 0 )
        {
        TRAPD( err, secNameField = entry->CreateFieldLC( *secNameFieldType );
            CleanupStack::Pop(); ); // Popping inside the TRAP
        if ( err != KErrNone )
            {
            maxSecondNames = 0;
            }
        else
            {
            MVPbkContactFieldTextData& secNameFieldData =
                MVPbkContactFieldTextData::Cast( secNameField->FieldData() );
            maxSecNameLength = secNameFieldData.MaxLength();
            delete secNameField;
            secNameField = NULL;
            }
        }

    HTI_LOG_FORMAT( "Max name length = %d", maxNameLength );
    HTI_LOG_FORMAT( "Max 2nd name length = %d", maxSecNameLength );
    HTI_LOG_FORMAT( "Max number length = %d", maxNumberLength );
    HTI_LOG_FORMAT( "Max mail length = %d", maxMailLength );

    CleanupStack::PopAndDestroy(); // entry

    // Create and send response message
    TBuf8<KSimInfoResponseLength> reply;
    reply.Append( maxSecondNames );
    reply.Append( maxNumbers - 1 ); // max num of additional numbers, so -1
    reply.Append( maxEmails );
    reply.Append( maxNameLength );
    reply.Append( maxNumberLength );
    reply.Append( maxSecNameLength );
    reply.Append( maxNumberLength ); // additional number uses same field type
    reply.Append( maxMailLength );
    reply.Append( ( TUint8* ) ( &maxEntries ), 2 );
    reply.Append( ( TUint8* ) ( &currentEntries ), 2 );

    SendOkMsgL( reply );

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::HandleSimCardInfoL" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::HandleSimContactImportL
// Imports the contact to SIM card.
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::HandleSimContactImportL()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::HandleSimContactImportL" );

    MVPbkStoreContact* newEntry = iSimStore->CreateNewContactLC();

    TInt offset = 0;
    TInt fieldCount = iMessage->Des()[offset];
    offset++;

    for ( TInt i = 0; i < fieldCount; i++ )
        {
        HTI_LOG_FORMAT( "Processing field %d", i + 1 );

        MVPbkStoreContactField* field = NULL;
        TVPbkFieldVersitProperty prop;
        TVPbkFieldTypeParameters param;
        const MVPbkFieldType* fieldType = NULL;
        TContactFieldType type = ( TContactFieldType ) iMessage->Des()[offset];
        offset++;
        switch ( type )
            {
            case ENameField:
                prop.SetName( EVPbkVersitNameN );
                fieldType = iContactManager->FieldTypes().FindMatch( prop, 0 );
                break;
            case ESecondNameField:
                fieldType = iContactManager->FieldTypes().Find(
                    R_VPBK_FIELD_TYPE_SECONDNAME );
                break;
            case EPhoneNumberField:
                param.Add( EVPbkVersitParamCELL );
                prop.SetName( EVPbkVersitNameTEL );
                prop.SetParameters( param );
                fieldType = iContactManager->FieldTypes().FindMatch( prop, 0 );
                break;
            case EEMailField:
                param.Add( EVPbkVersitParamINTERNET );
                prop.SetName( EVPbkVersitNameEMAIL );
                prop.SetParameters( param );
                fieldType = iContactManager->FieldTypes().FindMatch( prop, 0 );
                break;
            case EAdditNumberField:
                param.Add( EVPbkVersitParamCELL );
                param.Add( EVPbkVersitParamHOME );
                prop.SetName( EVPbkVersitNameTEL );
                prop.SetParameters( param );
                fieldType = iContactManager->FieldTypes().FindMatch( prop, 0 );
                break;
            default:
                HTI_LOG_FORMAT( "Unknown field type %d", type );
                User::Leave( KErrArgument );
                break;
            }

        if ( fieldType == NULL )
            {
            User::Leave( KErrArgument );
            }

        HTI_LOG_FORMAT( "Field type res id %d", fieldType->FieldTypeResId() );

        field = newEntry->CreateFieldLC( *fieldType );

        TInt fieldLength = iMessage->Des()[offset];
        offset++;
        HBufC* fieldData = HBufC::NewLC( fieldLength );
        fieldData->Des().Copy( iMessage->Mid( offset, fieldLength ) );
        HTI_LOG_FORMAT( "Field created - adding data: %S", fieldData );

        MVPbkContactFieldTextData& targetData =
            MVPbkContactFieldTextData::Cast( field->FieldData() );
        targetData.SetTextL( *fieldData );

        CleanupStack::PopAndDestroy(); // fieldData

        HTI_LOG_TEXT( "Data set - Adding the field" );
        newEntry->AddFieldL( field );
        CleanupStack::Pop(); // field

        offset += fieldLength;
        }

    HTI_LOG_TEXT( "Entry created - Commiting" );
    // Starts the async commit operation. Takes ownership of newEntry.
    newEntry->CommitL( *this );
    CleanupStack::Pop(); // newEntry

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::HandleSimContactImportL" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::HandleSimContactDeleteL
// Creates a contact view containing the contacts to be deleted.
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::HandleSimContactDeleteL()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::HandleSimContactDeleteL" );

    if ( iContactView )
        {
        iContactView->RemoveObserver( *this );
        delete iContactView;
        iContactView = NULL;
        }

    // Delete one entry based on ID
    if ( iMessage->Length() == 4 )
        {
        TPtr8 msgPtr = iMessage->Des();
        TInt id = msgPtr[0] + ( msgPtr[1] << 8 )
                            + ( msgPtr[2] << 16 )
                            + ( msgPtr[3] << 24 );
        HTI_LOG_FORMAT( "Delete with id %d", id );

        User::Leave( KErrNotSupported );
        }

    // Delete all
    else
        {
        CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
        CleanupStack::PushL( viewDef );
        viewDef->SetType( EVPbkContactsView );
        viewDef->SetSharing( EVPbkLocalView );
        viewDef->SetSortPolicy( EVPbkUnsortedContactView ); // in SIM index order
        CVPbkSortOrder* sortOrder = CVPbkSortOrder::NewL(
                iSimStore->StoreProperties().SupportedFields() );
        CleanupStack::PushL( sortOrder );
        iContactView = iSimStore->CreateViewLC( *viewDef, *this, *sortOrder );
        CleanupStack::Pop(); // view;
        CleanupStack::PopAndDestroy( 2 ); // sortOrder, viewDef
        }

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::HandleSimContactDeleteL" );
    }

// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::DeleteContactsInViewL
// Deletes the contacts that are currently in iContactView.
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::DeleteContactsInViewL()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::DeleteContactsInViewL" );

    TInt cntCount( iContactView->ContactCountL() );
    HTI_LOG_FORMAT( "Contact count in view = %d", cntCount );
    if ( cntCount > 0 )
        {
        CVPbkContactLinkArray* contactLinks = CVPbkContactLinkArray::NewLC();
        for ( TInt i = 0; i < cntCount; ++i )
            {
            MVPbkContactLink* link =
                iContactView->ContactAtL( i ).CreateLinkLC();
            contactLinks->AppendL( link );
            CleanupStack::Pop(); // link
            }
        // Following DeleteContactsL will result in calls to StepComplete
        // and finally OperationComplete (StepFailed if error occurs)
        iCurrentOperation = iContactManager->DeleteContactsL(
                *contactLinks, *this );
        CleanupStack::PopAndDestroy(); // contactLinks
        }
    else
        {
        // Nothing to delete
        SendOkMsgL( KNullDesC8 );
        }

    // We don't need the view anymore
    HTI_LOG_TEXT( "Deleting the contact view" );
    iContactView->RemoveObserver( *this );
    delete iContactView;
    iContactView = NULL;

    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::DeleteContactsInViewL" );
    }

// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::CheckImportMsg
// Validates the syntax of import contact message.
// ----------------------------------------------------------------------------
TBool CHtiSimDirHandlerVPbk::CheckImportMsg()
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::CheckImportMsg" );
    // Import command syntax:
    //   amount of fields     (1 byte)   __
    //   type of field        (1 byte)     |
    //   length of data field (1 byte)     | repeated <amount of fields> times
    //   field data           (variable) __|

    TInt length = iMessage->Length();
    if ( length < 4 ) // min length 4 bytes
        {
        HTI_LOG_FORMAT( "Message too short %d", length );
        return EFalse;
        }

    TInt offset = 0;
    TInt fieldCount = iMessage->Des()[offset];
    HTI_LOG_FORMAT( "Fields %d", fieldCount );
    if ( fieldCount < 1 ) // must be at least one field
        {
        return EFalse;
        }

    offset++;
    TInt fieldsFound = 0;
    while ( offset < length )
        {
        fieldsFound++;
        TInt fieldType = iMessage->Des()[offset];
        HTI_LOG_FORMAT( "Field type %d", fieldType );
        if ( fieldType < ENameField || fieldType > EAdditNumberField )
            {
            return EFalse; // invalid field type
            }
        offset++; // the type of field byte
        if ( offset >= length )
            {
            return EFalse;
            }
        TInt fieldLength = iMessage->Des()[offset];
        HTI_LOG_FORMAT( "Field length %d", fieldLength );
        if ( fieldLength < 1 )
            {
            return EFalse; // Field data can not be empty
            }
        offset++; // advance over the length of data byte
        offset += fieldLength; // and the field data
        }

    if ( offset == length && fieldsFound == fieldCount )
        {
        HTI_LOG_TEXT( "Message OK" );
        return ETrue;
        }

    return EFalse;
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::SendOkMsgL
// Helper function for sending response messages.
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::SendOkMsgL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::SendOkMsgL" );
    iIsBusy = EFalse; // Done with the current request
    User::LeaveIfNull( iDispatcher );
    HBufC8* temp = HBufC8::NewL( aData.Length() + 1 );
    TPtr8 response = temp->Des();
    response.Append( ( TChar ) CHtiPIMServicePlugin::EResultOk );
    response.Append( aData );
    User::LeaveIfError( iDispatcher->DispatchOutgoingMessage(
        temp, KPIMServiceUid ) );
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::SendOkMsgL" );
    }


// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::SendErrorMessageL
// Helper function for sending error response messages.
// ----------------------------------------------------------------------------
void CHtiSimDirHandlerVPbk::SendErrorMessageL( TInt aError, const TDesC8& aDescription )
    {
    HTI_LOG_FUNC_IN( "CHtiSimDirHandlerVPbk::SendErrorMessageL" );
    iIsBusy = EFalse; // Done with the current request
    User::LeaveIfNull( iDispatcher );
    User::LeaveIfError( iDispatcher->DispatchOutgoingErrorMessage(
        aError, aDescription, KPIMServiceUid ) );
    HTI_LOG_FUNC_OUT( "CHtiSimDirHandlerVPbk::SendErrorMessageL" );
    }


// End of file
