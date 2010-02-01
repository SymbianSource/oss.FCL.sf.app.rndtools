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
* Description:  Functional implementation of PIM service (for vCalendar, vCard)
*
*/


// INCLUDE FILES
#include "HtiPIMServicePlugin.h"
#include "PIMHandler.h"

#include <HtiDispatcherInterface.h>
#include <HTILogging.h>

#include <utf.h>
#include <calcommon.h>
#include <calsession.h>
#include <calenimporter.h>
#include <calentryview.h>

#include <CVPbkContactLinkArray.h>
#include <CVPbkContactManager.h>
#include <CVPbkContactViewDefinition.h>
#include <CVPbkContactStoreUriArray.h>
#include <CVPbkSortOrder.h>
#include <CVPbkVCardEng.h>
#include <MVPbkContactLinkArray.h>
#include <MVPbkContactOperationBase.h>
#include <MVPbkContactStore.h>
#include <MVPbkContactStoreList.h>
#include <MVPbkContactStoreObserver.h>
#include <MVPbkContactStoreProperties.h>
#include <MVPbkContactView.h>
#include <MVPbkContactViewBase.h>
#include <MVPbkViewContact.h>
#include <TVPbkContactStoreUriPtr.h>
#include <VPbkContactStoreUris.h>

// CONSTANTS
_LIT8( KErrorUnrecognizedCommand, "Unrecognized command" );
_LIT8( KErrorVCardImportFailed, "vCard import failed" );
_LIT8( KErrorVCalendarImportFailed, "vCalendar import failed" );
_LIT8( KErrorMissingVCalendar, "Missing vCalendar object" );
_LIT8( KErrorMissingVCard, "Missing vCard object" );
_LIT8( KErrorInvalidId, "Invalid ID parameter" );
_LIT8( KErrorItemNotFound, "Item not found" );
_LIT8( KErrorFailedDelete, "Failed to delete item" );
_LIT8( KErrorFailedDeleteAll, "Failed to delete all items" );
_LIT8( KErrorIdDeleteNotSupported, "Deleting with ID not supported anymore" );

_LIT8( KErrorMissingText, "Text parameter missing" );
_LIT8( KErrorMissingFilepath, "Filepath parameter missing" );
_LIT8( KErrorNotepadAddMemoFailed, "Notepad add memo failed" );
_LIT8( KErrorNotepadAddMemoFromFileFailed, "Notepad add memo from file failed" );
_LIT8( KErrorNotepadDeleteAllFailed, "Notepad delete all failed" );

_LIT( KHtiNpdHlpExe,       "HtiNpdHlp.exe" );
_LIT( KCmdAddMemo,         "AddMemo" );
_LIT( KCmdAddMemoFromFile, "AddMemoFromFile" );
_LIT( KCmdDeleteAll,       "DeleteAll" );
_LIT( KCmdDelim,           " " );

_LIT( KDefaultAgendaFile, "" ); // A default file is opened if fileName is KNullDesC


// ----------------------------------------------------------------------------
CPIMHandler* CPIMHandler::NewL()
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::NewL" );
    CPIMHandler* self = new (ELeave) CPIMHandler();
    CleanupStack::PushL ( self );
    self->ConstructL();
    CleanupStack::Pop();
    HTI_LOG_FUNC_OUT( "CPIMHandler::Done" );
    return self;
    }

// ----------------------------------------------------------------------------
CPIMHandler::CPIMHandler():iIsBusy( EFalse ), iEntryViewErr( KErrNone )
    {
    }

// ----------------------------------------------------------------------------
CPIMHandler::~CPIMHandler()
    {
    HTI_LOG_TEXT( "CPIMHandler destroy" );
    HTI_LOG_TEXT( "Deleting iEntryView" );
    delete iEntryView;
    HTI_LOG_TEXT( "Deleting iCalSession" );
    delete iCalSession;
    if ( iContactView )
        {
        HTI_LOG_TEXT( "Deleting iContactView" );
        iContactView->RemoveObserver( *this );
        delete iContactView;
        }
    HTI_LOG_TEXT( "Deleting iVCardEngine" );
    delete iVCardEngine;
    HTI_LOG_TEXT( "Deleting iContactManager" );
    delete iContactManager;
    HTI_LOG_TEXT( "Deleting iBuffer" );
    delete iBuffer;
    HTI_LOG_TEXT( "Deleting iWaiter" );
    delete iWaiter;
    }

// ----------------------------------------------------------------------------
void CPIMHandler::ConstructL()
    {
    HTI_LOG_TEXT( "CPIMHandler::ConstructL" );
    iWaiter = new ( ELeave ) CActiveSchedulerWait;

    }

// ----------------------------------------------------------------------------
void CPIMHandler::SetDispatcher( MHtiDispatcher* aDispatcher )
    {
    iDispatcher = aDispatcher;
    }

// ----------------------------------------------------------------------------
void CPIMHandler::ProcessMessageL( const TDesC8& aMessage,
    THtiMessagePriority /*aPriority*/ )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::ProcessMessageL" );

    iIsBusy = ETrue;
    TInt err = KErrNone;

    // Zero legth of aMessage tested already in CHtiPIMServicePlugin.
    // Other sanity checks must be done here.

    iCommand = aMessage.Ptr()[0];
    switch ( iCommand )
        {
        case CHtiPIMServicePlugin::EImportVCard:
            {
            TRAP( err, HandleVCardImportFuncL(
                    aMessage.Right( aMessage.Length() - 1 ) ) );
            break;
            }
        case CHtiPIMServicePlugin::EImportVCalendar:
            {
            TRAP( err, HandleVCalendarImportFuncL(
                    aMessage.Right( aMessage.Length() - 1 ) ) );
            break;
            }
        case CHtiPIMServicePlugin::EDeleteContact:
            {
            TRAP( err, HandleContactDeleteFuncL(
                    aMessage.Right( aMessage.Length() - 1 ) ) );
            break;
            }
        case CHtiPIMServicePlugin::EDeleteCalendar:
            {
            TRAP( err, HandleCalendarDeleteFuncL(
                    aMessage.Right( aMessage.Length() - 1 ) ) );
            break;
            }
        case CHtiPIMServicePlugin::ENotepadAddMemo:
            {
            TRAP( err, HandleNotepadAddMemoFuncL( aMessage.Mid( 1 ) ) );
            break;
            }
        case CHtiPIMServicePlugin::ENotepadAddMemoFromFile:
            {
            TRAP( err, HandleNotepadAddMemoFromFileFuncL( aMessage.Mid( 1 ) ) );
            break;
            }
        case CHtiPIMServicePlugin::ENotepadDeleteAll:
            {
            TRAP( err, HandleNotepadDeleteAllFuncL() );
            break;
            }
        default:
            {
            TRAP( err, SendErrorMessageL(
                    KErrArgument, KErrorUnrecognizedCommand ) );
            break;
            }
        }

    if ( err != KErrNone )
        {
        iIsBusy = EFalse;
        User::Leave( err );
        }

    HTI_LOG_FUNC_OUT( "CPIMHandler::ProcessMessageL: Done" );
    }

// ----------------------------------------------------------------------------
TBool CPIMHandler::IsBusy()
    {
    return iIsBusy;
    }

// ----------------------------------------------------------------------------
void CPIMHandler::HandleVCardImportFuncL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleVCardImportFuncL" );

    if ( aData.Length() == 0 )
        {
        SendErrorMessageL( KErrArgument, KErrorMissingVCard );
        return;
        }

    if ( iBuffer ) // delete if exists (just to be sure)
        {
        delete iBuffer;
        iBuffer = NULL;
        }
    iBuffer = CBufFlat::NewL( aData.Length() );
    iBuffer->ExpandL( 0, aData.Length() );

    HTI_LOG_FORMAT( "Data length = %d", aData.Length() );
    HTI_LOG_FORMAT( "Buffer length = %d", iBuffer->Ptr( 0 ).MaxLength() );
    iBuffer->Ptr( 0 ).Copy( aData );

    if ( iContactManager == NULL )
        {
        CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
        uriArray->AppendL( TVPbkContactStoreUriPtr(
            VPbkContactStoreUris::DefaultCntDbUri() ) );
        iContactManager = CVPbkContactManager::NewL( *uriArray );
        CleanupStack::PopAndDestroy( uriArray );
        }

    if ( iVCardEngine == NULL )
        {
        iVCardEngine = CVPbkVCardEng::NewL( *iContactManager );
        }

    MVPbkContactStoreList& stores = iContactManager->ContactStoresL();
    iContactStore = &stores.At( 0 );
    iContactStore->OpenL( *this );

    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleVCardImportFuncL: Done" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::HandleVCalendarImportFuncL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleVCalendarImportFuncL" );

    if ( aData.Length() == 0 )
        {
        HTI_LOG_TEXT( "CPIMHandler::HandleVCalendarImportFuncL: Error: length of data is zero" )
        SendErrorMessageL( KErrArgument, KErrorMissingVCalendar );
        return;
        }

    if ( iBuffer ) // delete if exists (just to be sure)
        {
        delete iBuffer;
        iBuffer = NULL;
        }
    iBuffer = CBufFlat::NewL( aData.Length() );
    iBuffer->ExpandL( 0, aData.Length() );
    iBuffer->Ptr( 0 ).Copy( aData );
    RBufReadStream readStream;
    readStream.Open( *iBuffer, 0 );
    CleanupClosePushL( readStream );

    if ( iCalSession == NULL )
        {
        HTI_LOG_TEXT( "CPIMHandler: Creating Calendar session" );
        iCalSession = CCalSession::NewL();
        iCalSession->OpenL( KDefaultAgendaFile );
        HTI_LOG_TEXT( "CPIMHandler: Calendar session open" );
        }

    CCalenImporter* importer = CCalenImporter::NewL( *iCalSession );
    CleanupStack::PushL( importer );
    HTI_LOG_TEXT( "CPIMHandler: Calendar importer created" );

    RPointerArray<CCalEntry> entryArray;
    CleanupClosePushL( entryArray );

    TInt err = KErrNone;
    TInt size = 0;
    importer->SetImportMode( ECalenImportModeExtended );
    // First try to import as iCalendar
    TRAP( err, importer->ImportICalendarL( readStream, entryArray ) );
    HTI_LOG_FORMAT( "ImportICalendarL return value %d", err );
    size = entryArray.Count();
    HTI_LOG_FORMAT( "Import ICalendarL imported %d entries", size );
    // If import didn't succeed, try as vCalendar
    if ( err != KErrNone || size == 0 )
        {
        readStream.Close();
        readStream.Open( *iBuffer, 0 ); // reset read stream
        entryArray.ResetAndDestroy(); // avoid double imports
        TRAP( err, importer->ImportVCalendarL( readStream, entryArray ) );
        HTI_LOG_FORMAT( "ImportVCalendarL return value %d", err );
        size = entryArray.Count();
        HTI_LOG_FORMAT( "Import VCalendarL imported %d entries", size );
        }
    TCalLocalUid uniqueId = 0;
    TInt success = 0;
    if ( size > 0 )
        {
        iEntryView = CCalEntryView::NewL( *iCalSession, *this );
        iWaiter->Start();
        if ( iEntryViewErr == KErrNone )
            {
            TRAP( err, iEntryView->StoreL( entryArray, success ) );
            HTI_LOG_FORMAT( "StoreL return value %d", err );
            HTI_LOG_FORMAT( "Successfully stored %d entries", success );
            uniqueId = entryArray[0]->LocalUidL();
            }
        delete iEntryView;
        iEntryView = NULL;
        }
    entryArray.ResetAndDestroy();
    CleanupStack::PopAndDestroy(); // entryArray

    if ( err == KErrNone && success > 0 )
        {
        TBuf8<8> uniqueIdStr;
        uniqueIdStr.Copy( ( TUint8* ) ( &uniqueId ), sizeof( uniqueId ) );
        SendOkMsgL( uniqueIdStr );
        }
    else
        {
        if ( err == KErrNone ) err = KErrGeneral;
        SendErrorMessageL( err, KErrorVCalendarImportFailed );
        }

    CleanupStack::PopAndDestroy( 2 ); // readStream, importer
    delete iCalSession;
    iCalSession = NULL;
    delete iBuffer;
    iBuffer = NULL;
    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleVCalendarImportFuncL: Done" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::HandleContactDeleteFuncL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleContactDeleteFuncL" );

    TInt dataLength = aData.Length();
    if ( dataLength != 0 && dataLength != 4 )
        {
        HTI_LOG_TEXT( "CPIMHandler: Error: wrong length of data" )
        SendErrorMessageL( KErrArgument, KErrorInvalidId );
        return;
        }

    if ( dataLength == 4 )
        {
        SendErrorMessageL( KErrNotSupported, KErrorIdDeleteNotSupported );
        return;
        }

    if ( iContactManager == NULL )
        {
        CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
        uriArray->AppendL( TVPbkContactStoreUriPtr(
            VPbkContactStoreUris::DefaultCntDbUri() ) );
        iContactManager = CVPbkContactManager::NewL( *uriArray );
        CleanupStack::PopAndDestroy( uriArray );
        }

    if ( iContactStore == NULL )
        {
        MVPbkContactStoreList& stores = iContactManager->ContactStoresL();
        iContactStore = &stores.At( 0 );
        }

    iContactStore->OpenL( *this );

    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleContactDeleteFuncL" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::HandleCalendarDeleteFuncL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleVCalendarDeleteFuncL" );

    TInt dataLength = aData.Length();
    if ( dataLength != 0 && dataLength != 4 )
        {
        HTI_LOG_TEXT( "CPIMHandler: Error: wrong length of data" )
        SendErrorMessageL( KErrArgument, KErrorInvalidId );
        return;
        }

    if ( iBuffer ) // delete if exists (just to be sure)
        {
        delete iBuffer;
        iBuffer = NULL;
        }
    if ( aData.Length() > 0 )
        {
        iBuffer = CBufFlat::NewL( aData.Length() );
        iBuffer->ExpandL( 0, aData.Length() );
        iBuffer->Ptr( 0 ).Copy( aData );
        }

    if ( iCalSession == NULL )
        {
        HTI_LOG_TEXT( "CPIMHandler: Creating Calendar session" );
        iCalSession = CCalSession::NewL();
        iCalSession->OpenL( KDefaultAgendaFile );
        HTI_LOG_TEXT( "CPIMHandler: Calendar session open" );
        }

    HTI_LOG_TEXT( "CPIMHandler: Creating entry view" );
    iEntryView = CCalEntryView::NewL( *iCalSession, *this );
    iWaiter->Start();
    if ( iEntryViewErr != KErrNone )
        {
        delete iEntryView;
        iEntryView = NULL;
        delete iCalSession;
        iCalSession = NULL;
        delete iBuffer;
        iBuffer = NULL;
        User::Leave( iEntryViewErr );
        }

    // If iBuffer is NULL, no ID given, delete all calendar entries
    if ( iBuffer == NULL )
        {
        HTI_LOG_TEXT( "CPIMHandler: Deleting all calendar entries" );
        TCalTime minTime;
        TCalTime maxTime;
        minTime.SetTimeUtcL( TCalTime::MinTime() );
        maxTime.SetTimeUtcL( TCalTime::MaxTime() );
        CalCommon::TCalTimeRange timeRange( minTime, maxTime );
        TRAPD( err, iEntryView->DeleteL( timeRange,
                CalCommon::EIncludeAll, *this ) );
        iWaiter->Start();
        if ( err == KErrNone && iEntryViewErr == KErrNone )
            {
            SendOkMsgL( KNullDesC8 );
            }
        else
            {
            SendErrorMessageL( KErrGeneral, KErrorFailedDeleteAll );
            }
        }

    // If id given, delete only calendar entry having that id
    else
        {
        TPtr8 data = iBuffer->Ptr( 0 );
        TCalLocalUid id = data[0] + ( data[1] << 8 )
                             + ( data[2] << 16 )
                             + ( data[3] << 24 );
        HTI_LOG_FORMAT( "CPIMHandler: Deleting one calendar entry %d", id );
        CCalEntry* entryToDelete = NULL;
        TRAPD( err, entryToDelete = iEntryView->FetchL( id ) );

        if ( err || entryToDelete == NULL )
            {
            HTI_LOG_TEXT( "CPIMHandler: Calendar entry not found" );
            SendErrorMessageL( KErrNotFound, KErrorItemNotFound );
            }
        else
            {
            CleanupStack::PushL( entryToDelete );
            TRAP( err, iEntryView->DeleteL( *entryToDelete ) );
            if ( err == KErrNone )
                {
                SendOkMsgL( KNullDesC8 );
                }
            else
                {
                HTI_LOG_TEXT( "CPIMHandler: Error deleting calendar entry" )
                SendErrorMessageL( KErrGeneral, KErrorFailedDelete );
                }
            CleanupStack::PopAndDestroy( entryToDelete );
            }
        }
    delete iEntryView;
    iEntryView = NULL;
    delete iCalSession;
    iCalSession = NULL;
    delete iBuffer;
    iBuffer = NULL;
    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleVCalendarDeleteFuncL" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::SendOkMsgL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::SendOkMsgL: Starting" );

    User::LeaveIfNull( iDispatcher );

    HBufC8* temp = HBufC8::NewL( aData.Length() + 1 );
    TPtr8 response = temp->Des();
    response.Append( ( TChar ) CHtiPIMServicePlugin::EResultOk );
    response.Append( aData );
    User::LeaveIfError( iDispatcher->DispatchOutgoingMessage(
        temp, KPIMServiceUid ) );
    iIsBusy = EFalse;
    HTI_LOG_FUNC_OUT( "CPIMHandler::SendOkMsgL: Done" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::SendErrorMessageL( TInt aError, const TDesC8& aDescription )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::SendErrorMessageL: Starting" );
    User::LeaveIfNull( iDispatcher );
    User::LeaveIfError( iDispatcher->DispatchOutgoingErrorMessage(
        aError, aDescription, KPIMServiceUid ) );
    iIsBusy = EFalse;
    HTI_LOG_FUNC_OUT( "CPIMHandler::SendErrorMessageL: Done" );
    }

// ----------------------------------------------------------------------------
TInt CallNpdHlp( const TDesC& aCmd )
    {
    HTI_LOG_FUNC_IN( "CallNpdHlp" );

    RProcess HtiNpdHlp;
    TInt err = HtiNpdHlp.Create( KHtiNpdHlpExe, aCmd );
    if ( err )
        {
        HTI_LOG_FORMAT( "Could not create HtiNpdHlp.Exe process %d", err );
        return err;
        }

    TRequestStatus status;
    HtiNpdHlp.Logon( status );
    HtiNpdHlp.Resume();
    User::WaitForRequest( status );
    if ( status.Int() != KErrNone )
        {
        HTI_LOG_FORMAT( "status     %d", status.Int() );
        HTI_LOG_FORMAT( "ExitReason %d", HtiNpdHlp.ExitReason() );
        HTI_LOG_FORMAT( "ExitType   %d", HtiNpdHlp.ExitType() );
        HtiNpdHlp.Close();
        return status.Int();
        }

    HtiNpdHlp.Close();

    HTI_LOG_FUNC_OUT( "CallNpdHlp" );
    return KErrNone;
    }

// ----------------------------------------------------------------------------
void CPIMHandler::SendNotepadOkMsgL( CHtiPIMServicePlugin::TCommand aCommand )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::SendNotepadOkMsgL" );
    TBuf8<1> msg;
    msg.Append( aCommand );
    User::LeaveIfError( iDispatcher->DispatchOutgoingMessage(
                        msg.AllocL(), KPIMServiceUid ) );
    iIsBusy = EFalse;
    HTI_LOG_FUNC_OUT( "CPIMHandler::SendNotepadOkMsgL" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::HandleNotepadAddMemoFuncL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleNotepadAddMemoFuncL" );

    if ( aData.Length() < 1 )
        {
        SendErrorMessageL( KErrArgument, KErrorMissingText );
        return;
        }

    // convert text from TDesC8 -> TDesC
    // expecting the input TDesC8 contains UTF-8 data
    HBufC* text = CnvUtfConverter::ConvertToUnicodeFromUtf8L( aData );
    HTI_LOG_TEXT( "CPIMHandler: Conversion to Unicode done" );
    CleanupStack::PushL( text );

    HBufC* cmd = HBufC::NewLC( KCmdAddMemo().Length() + 1 + ( *text ).Length() );
    cmd->Des().Copy( KCmdAddMemo );
    cmd->Des().Append( KCmdDelim );
    cmd->Des().Append( *text );

    TInt err = CallNpdHlp( *cmd );
    if ( err )
        {
        SendErrorMessageL( err, KErrorNotepadAddMemoFailed );
        }
    else
        {
        SendNotepadOkMsgL( CHtiPIMServicePlugin::ENotepadAddMemo );
        }

    CleanupStack::PopAndDestroy( 2 ); // text, cmd

    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleNotepadAddMemoFuncL" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::HandleNotepadAddMemoFromFileFuncL( const TDesC8& aData )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleNotepadAddMemoFromFileFuncL" );

    if ( aData.Length() < 1 )
        {
        SendErrorMessageL( KErrArgument, KErrorMissingFilepath );
        return;
        }

    // convert filename from TDesC8 -> TDesC
    // expecting the input TDesC8 contains UTF-8 data
    HBufC* filename = CnvUtfConverter::ConvertToUnicodeFromUtf8L( aData );
    HTI_LOG_TEXT( "CPIMHandler: Conversion to Unicode done" );
    CleanupStack::PushL( filename );

    HBufC* cmd = HBufC::NewLC( KCmdAddMemoFromFile().Length() + 1 + ( *filename ).Length() );
    cmd->Des().Copy( KCmdAddMemoFromFile );
    cmd->Des().Append( KCmdDelim );
    cmd->Des().Append( *filename );

    TInt err = CallNpdHlp( *cmd );
    if ( err )
        {
        SendErrorMessageL( err, KErrorNotepadAddMemoFromFileFailed );
        }
    else
        {
        SendNotepadOkMsgL( CHtiPIMServicePlugin::ENotepadAddMemoFromFile );
        }

    CleanupStack::PopAndDestroy( 2 ); // filename, cmd

    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleNotepadAddMemoFromFileFuncL" );
    }

// ----------------------------------------------------------------------------
void CPIMHandler::HandleNotepadDeleteAllFuncL()
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleNotepadDeleteAllFuncL" );

    TInt err = CallNpdHlp( KCmdDeleteAll() );
    if ( err )
        {
        SendErrorMessageL( err, KErrorNotepadDeleteAllFailed );
        }
    else
        {
        SendNotepadOkMsgL( CHtiPIMServicePlugin::ENotepadDeleteAll );
        }

    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleNotepadDeleteAllFuncL" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::CreateContactDeleteViewL
// Creates a contact view containing the contacts to be deleted.
// ----------------------------------------------------------------------------
void CPIMHandler::CreateContactDeleteViewL()
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::CreateContactDeleteViewL" );

    if ( iContactView )
        {
        iContactView->RemoveObserver( *this );
        delete iContactView;
        iContactView = NULL;
        }

    CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
    CleanupStack::PushL( viewDef );
    viewDef->SetType( EVPbkContactsView );
    viewDef->SetSharing( EVPbkLocalView );
    viewDef->SetSortPolicy( EVPbkSortedContactView );
    CVPbkSortOrder* sortOrder = CVPbkSortOrder::NewL(
            iContactStore->StoreProperties().SupportedFields() );
    CleanupStack::PushL( sortOrder );
    iContactView = iContactStore->CreateViewLC( *viewDef, *this, *sortOrder );
    CleanupStack::Pop(); // view;
    CleanupStack::PopAndDestroy( 2 ); // sortOrder, viewDef

    HTI_LOG_FUNC_OUT( "CPIMHandler::CreateContactDeleteViewL" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::DeleteContactsInViewL
// Deletes the contacts that are currently in iContactView.
// ----------------------------------------------------------------------------
void CPIMHandler::DeleteContactsInViewL()
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::DeleteContactsInViewL" );

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
        iOp = iContactManager->DeleteContactsL( *contactLinks, *this );
        CleanupStack::PopAndDestroy(); // contactLinks
        }
    else
        {
        // Nothing to delete
        iContactStore->Close( *this );
        SendOkMsgL( KNullDesC8 );
        }

    // We don't need the view anymore
    HTI_LOG_TEXT( "Deleting the contact view" );
    iContactView->RemoveObserver( *this );
    delete iContactView;
    iContactView = NULL;

    HTI_LOG_FUNC_OUT( "CPIMHandler::DeleteContactsInViewL" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::StoreReady
// Called when a contact store is ready to use.
// From MVPbkContactStoreObserver
// ----------------------------------------------------------------------------
void CPIMHandler::StoreReady( MVPbkContactStore& aContactStore )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::StoreReady" );
    if ( iIsBusy && iContactStore == &aContactStore )
        {
        if ( iCommand == CHtiPIMServicePlugin::EImportVCard )
            {
            iReadStream.Open( *iBuffer, 0 );
            HTI_LOG_TEXT( "Starting vCard import" );
            TRAPD( err, iOp = iVCardEngine->ImportVCardL(
                    *iContactStore, iReadStream, *this ) );
            HTI_LOG_FORMAT( "ImpoortVCardL returned %d", err );
            if ( err != KErrNone )
                {
                delete iOp;
                iOp = NULL;
                iReadStream.Close();
                delete iBuffer;
                iBuffer = NULL;
                iContactStore->Close( *this );
                TRAP_IGNORE( SendErrorMessageL( err, KErrorVCardImportFailed ) );
                }
            }
        else if ( iCommand == CHtiPIMServicePlugin::EDeleteContact )
            {
            TRAPD( err, CreateContactDeleteViewL() );
            if ( err != KErrNone )
                {
                iContactStore->Close( *this );
                TRAP_IGNORE( SendErrorMessageL( err, KErrorFailedDelete ) );
                }
            }
        }
    HTI_LOG_FUNC_OUT( "CPIMHandler::StoreReady" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::StoreUnavailable
// Called when a contact store becomes unavailable.
// From MVPbkContactStoreObserver
// ----------------------------------------------------------------------------
void CPIMHandler::StoreUnavailable( MVPbkContactStore& aContactStore,
        TInt aReason )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::StoreUnavailable" );

    if ( iIsBusy && iContactStore == &aContactStore )
        {
        delete iBuffer;
        iBuffer = NULL;
        TRAP_IGNORE( SendErrorMessageL( aReason, KErrorVCardImportFailed ) );
        }

    HTI_LOG_FUNC_OUT( "CPIMHandler::StoreUnavailable" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::HandleStoreEventL
// Called when changes occur in the contact store.
// From MVPbkContactStoreObserver
// ----------------------------------------------------------------------------
void CPIMHandler::HandleStoreEventL( MVPbkContactStore& /*aContactStore*/,
                        TVPbkContactStoreEvent /*aStoreEvent*/ )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::HandleStoreEventL" );
    HTI_LOG_FUNC_OUT( "CPIMHandler::HandleStoreEventL" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::ContactsSaved
// Called when the contact has been successfully commited or copied.
// From MVPbkContactCopyObserver
// ----------------------------------------------------------------------------
void CPIMHandler::ContactsSaved( MVPbkContactOperationBase& aOperation,
        MVPbkContactLinkArray* aResults )
{
    HTI_LOG_FUNC_IN( "CPIMHandler::ContactsSaved" );

    if ( iIsBusy && iOp == &aOperation )
        {
        TInt count = aResults->Count();
        HTI_LOG_FORMAT( "%d contact(s) added", count );
        delete aResults;
        delete iOp;
        iOp = NULL;
        iReadStream.Close();
        delete iBuffer;
        iBuffer = NULL;
        iContactStore->Close( *this );
        TInt entryId = 0; // We can't get the ID, just send zero
        TBuf8<4> idBuf;
        idBuf.Append( ( TUint8* ) &entryId, 4 );
        TRAP_IGNORE( SendOkMsgL( idBuf ) );
        }

    HTI_LOG_FUNC_OUT( "CPIMHandler::ContactsSaved" );
}

// ----------------------------------------------------------------------------
// CPIMHandler::ContactsSavingFailed
// Called when there was en error while saving contact(s).
// From MVPbkContactCopyObserver
// ----------------------------------------------------------------------------
void CPIMHandler::ContactsSavingFailed(
        MVPbkContactOperationBase& aOperation, TInt aError )
{
    HTI_LOG_FUNC_IN( "CPIMHandler::ContactsSavingFailed" );

    if ( iIsBusy && iOp == &aOperation )
        {
        delete iOp;
        iOp = NULL;
        iReadStream.Close();
        delete iBuffer;
        iBuffer = NULL;
        iContactStore->Close( *this );
        TRAP_IGNORE( SendErrorMessageL( aError, KErrorVCardImportFailed ) );
        }

    HTI_LOG_FUNC_OUT( "CPIMHandler::ContactsSavingFailed" );
}

// ----------------------------------------------------------------------------
// CPIMHandler::ContactViewReady
// Called when a view is ready for use.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CPIMHandler::ContactViewReady( MVPbkContactViewBase& aView )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::ContactViewReady" );

    if ( iContactView == &aView && iIsBusy &&
            iCommand == CHtiPIMServicePlugin::EDeleteContact )
        {
        TRAPD( err, DeleteContactsInViewL() );
        if ( err != KErrNone )
            {
            TRAP_IGNORE( SendErrorMessageL( err, KErrorFailedDelete ) );
            }
        }

    HTI_LOG_FUNC_OUT( "CPIMHandler::ContactViewReady" );
    }

// ----------------------------------------------------------------------------
// CHtiSimDirHandlerVPbk::ContactViewUnavailable
// Called when a view is unavailable for a while.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CPIMHandler::ContactViewUnavailable( MVPbkContactViewBase& /*aView*/ )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::ContactViewUnavailable" );
    HTI_LOG_FUNC_OUT( "CPIMHandler::ContactViewUnavailable" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::ContactAddedToView
// Called when a contact has been added to the view.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CPIMHandler::ContactAddedToView( MVPbkContactViewBase& /*aView*/,
        TInt /*aIndex*/, const MVPbkContactLink& /*aContactLink*/ )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::ContactAddedToView" );
    HTI_LOG_FUNC_OUT( "CPIMHandler::ContactAddedToView" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::ContactRemovedFromView
// Called when a contact has been removed from a view.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CPIMHandler::ContactRemovedFromView( MVPbkContactViewBase& /*aView*/,
        TInt /*aIndex*/, const MVPbkContactLink& /*aContactLink*/ )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::ContactRemovedFromView" );
    HTI_LOG_FUNC_OUT( "CPIMHandler::ContactRemovedFromView" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::ContactViewError
// Called when an error occurs in the view.
// From MVPbkContactViewObserver
// ----------------------------------------------------------------------------
void CPIMHandler::ContactViewError( MVPbkContactViewBase& aView,
        TInt aError, TBool aErrorNotified )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::ContactViewError" );
    HTI_LOG_FORMAT( "CPIMHandler::ContactViewError: %d", aError );
    HTI_LOG_FORMAT( "ErrorNotified = %d", aErrorNotified );
    if ( iContactView == &aView )
        {
        iContactView->RemoveObserver( *this );
        delete iContactView;
        iContactView = NULL;
        if ( iIsBusy && iCommand == CHtiPIMServicePlugin::EDeleteContact )
            {
            SendErrorMessageL( aError, KErrorFailedDelete );
            }
        iContactStore->Close( *this );
        }
    aErrorNotified = aErrorNotified;  // avoid compiler warning
    HTI_LOG_FUNC_OUT( "CPIMHandler::ContactViewError" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::StepComplete
// Called when one step of the batch operation is complete.
// From MVPbkBatchOperationObserver
// ----------------------------------------------------------------------------
void CPIMHandler::StepComplete( MVPbkContactOperationBase& /*aOperation*/,
                           TInt /*aStepSize*/ )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::StepComplete" );
    HTI_LOG_FUNC_OUT( "CPIMHandler::StepComplete" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::StepFailed
// Called when one step of the batch operation fails.
// From MVPbkBatchOperationObserver
// ----------------------------------------------------------------------------
TBool CPIMHandler::StepFailed( MVPbkContactOperationBase& aOperation,
                                         TInt /*aStepSize*/, TInt aError )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::StepFailed" );
    HTI_LOG_FORMAT( "Error %d", aError );
    if ( iOp == &aOperation )
        {
        // We are returning EFalse (= do not continue) we can delete
        // the operation. OperationComplete() won't be called.
        delete iOp;
        iOp = NULL;
        iContactStore->Close( *this );
        TRAP_IGNORE( SendErrorMessageL( aError, KErrorFailedDelete ) );
        }
    HTI_LOG_FUNC_OUT( "CPIMHandler::StepFailed" );
    return EFalse; // do not continue
    }

// ----------------------------------------------------------------------------
// CPIMHandler::OperationComplete
// Called when operation is completed.
// From MVPbkBatchOperationObserver
// ----------------------------------------------------------------------------
void CPIMHandler::OperationComplete(
            MVPbkContactOperationBase& aOperation )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::OperationComplete" );
    // Operation is complete -> delete it
    if ( iOp == &aOperation )
        {
        delete iOp;
        iOp = NULL;
        iContactStore->Close( *this );
        if ( iIsBusy && iCommand == CHtiPIMServicePlugin::EDeleteContact )
            {
            // Delete operation has completed
            TRAP_IGNORE( SendOkMsgL( KNullDesC8 ) );
            }
        }
    HTI_LOG_FUNC_OUT( "CPIMHandler::OperationComplete" );
    }


// ----------------------------------------------------------------------------
// CPIMHandler::Progress
// Called during calendar entry view creation and operations.
// Called only if NotifyProgress returns ETrue.
// From MCalProgressCallBack
// ----------------------------------------------------------------------------
void CPIMHandler::Progress( TInt /*aProgress*/ )
    {
    }

// ----------------------------------------------------------------------------
// CPIMHandler::Completed
// Called on completion of calendar entry view creation and operations
// From MCalProgressCallBack
// ----------------------------------------------------------------------------
void CPIMHandler::Completed( TInt aError )
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::Completed" );
    HTI_LOG_FORMAT( "Completed with error code %d", aError );
    iEntryViewErr = aError;
    iWaiter->AsyncStop();
    HTI_LOG_FUNC_OUT( "CPIMHandler::Completed" );
    }

// ----------------------------------------------------------------------------
// CPIMHandler::NotifyProgress
// Returns whether or not progress notification is required
// From MCalProgressCallBack
// ----------------------------------------------------------------------------
TBool CPIMHandler::NotifyProgress()
    {
    HTI_LOG_FUNC_IN( "CPIMHandler::NotifyProgress" );
    HTI_LOG_FUNC_OUT( "CPIMHandler::NotifyProgress" );
    return EFalse; // Don't notify about progress
    }
