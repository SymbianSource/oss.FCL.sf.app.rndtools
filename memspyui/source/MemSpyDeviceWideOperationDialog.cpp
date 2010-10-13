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
* Description:
*
*/

#include "MemSpyDeviceWideOperationDialog.h"

// System includes
#include <eikprogi.h>
#include <AknWaitDialog.h>
#include <AknQueryDialog.h>
#include <coemain.h>
#include <eikenv.h>
#include <avkon.hrh>
#include <memspyui.rsg>


CMemSpyDeviceWideOperationDialog::CMemSpyDeviceWideOperationDialog( CMemSpyEngine& aEngine, MMemSpyDeviceWideOperationDialogObserver& aObserver )
:   iEngine( aEngine ), iObserver( aObserver )
    {
    }


CMemSpyDeviceWideOperationDialog::~CMemSpyDeviceWideOperationDialog()
    {
#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::~CMemSpyDeviceWideOperationDialog() - START - iForcedCancel: %d", iForcedCancel );
#endif

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::~CMemSpyDeviceWideOperationDialog() - deleting operation...: 0x%08x", iOperation );
#endif
    delete iOperation;

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::~CMemSpyDeviceWideOperationDialog() - deleting dialog...: 0x%08x", iProgressDialog );
#endif
    iProgressInfo = NULL;
    delete iProgressDialog;

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::~CMemSpyDeviceWideOperationDialog() - sending EDialogDismissed to observer..." );
#endif

    iObserver.DWOperationCompleted();

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::~CMemSpyDeviceWideOperationDialog() - END" );
#endif
    }


void CMemSpyDeviceWideOperationDialog::ExecuteL( CMemSpyDeviceWideOperations::TOperation aOperation )
    {
#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::ExecuteL() - START" );
#endif

    iOperation = CMemSpyDeviceWideOperations::NewL( iEngine, *this, aOperation );

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::ExecuteL() - constructed operation..." );
#endif

    ASSERT( iProgressDialog == NULL );
    iProgressDialog = new( ELeave ) CAknProgressDialog( reinterpret_cast< CEikDialog** >( &iProgressDialog ), ETrue );
    iProgressDialog->PrepareLC( R_MEMSPY_DEVICE_WIDE_OPERATION_PROGRESS_DIALOG );
    iProgressDialog->SetCallback( this );
    iProgressDialog->SetGloballyCapturing( ETrue );
    //
    iProgressInfo = iProgressDialog->GetProgressInfoL();
    iProgressInfo->SetFinalValue( iOperation->TotalOperationSize() );
    //
    iProgressDialog->RunLD();

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::ExecuteL() - END" );
#endif
    }

void CMemSpyDeviceWideOperationDialog::ExecuteLD( CMemSpyEngine& aEngine, MMemSpyDeviceWideOperationDialogObserver& aObserver, CMemSpyDeviceWideOperations::TOperation aOperation )
    {
    CMemSpyDeviceWideOperationDialog* self = new(ELeave) CMemSpyDeviceWideOperationDialog( aEngine, aObserver );
    CleanupStack::PushL( self );
    self->ExecuteL( aOperation );
    CleanupStack::PopAndDestroy( self );
    }


void CMemSpyDeviceWideOperationDialog::Cancel()
    {
#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::Cancel() - START - iOperation: 0x%08x, iForcedCancel: %d", iOperation, iForcedCancel );
#endif

    iForcedCancel = ETrue;
    //
    if  ( iOperation )
        {
        iOperation->Cancel();
        }
    //
    iObserver.DWOperationCancelled();

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::Cancel() - END" );
#endif
    }


void CMemSpyDeviceWideOperationDialog::DialogDismissedL( TInt aButtonId )
    {
#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::DialogDismissedL() - START - aButtonId: %d, iForcedCancel: %d", aButtonId, iForcedCancel );
#else
    (void) aButtonId;
#endif

    Cancel();

#ifdef _DEBUG
    RDebug::Printf("[MemSpy] CMemSpyDeviceWideOperationDialog::DialogDismissedL() - END - aButtonId: %d, iForcedCancel: %d", aButtonId, iForcedCancel );
#endif
    }


void CMemSpyDeviceWideOperationDialog::HandleDeviceWideOperationEvent( TEvent aEvent, TInt aParam1, const TDesC& aParam2 )
    {
#ifdef _DEBUG
    RDebug::Print( _L("[MemSpy] CMemSpyDeviceWideOperationDialog::HandleDeviceWideOperationEvent() - START - aEvent: %d, iProgressDialog: 0x%08x, aParam1: %d, aParam2: %S"), aEvent, iProgressDialog, aParam1, &aParam2 );
#endif

    switch( aEvent )
        {
    case MMemSpyDeviceWideOperationsObserver::EOperationSized:
        break;
    case MMemSpyDeviceWideOperationsObserver::EOperationStarting:
        iObserver.DWOperationStarted();
        break;
    case MMemSpyDeviceWideOperationsObserver::EOperationProgressStart:
        ASSERT( iProgressDialog != NULL );
        SetDialogCaptionL( aParam2 );
        break;
    case MMemSpyDeviceWideOperationsObserver::EOperationProgressEnd:
        ASSERT( iProgressDialog != NULL );
        iProgressInfo->IncrementAndDraw( aParam1 );
        break;
    case MMemSpyDeviceWideOperationsObserver::EOperationCancelled:
        break;
    case MMemSpyDeviceWideOperationsObserver::EOperationCompleting:
        {
        ASSERT( iProgressDialog != NULL );
        const TInt finalValue = iProgressInfo->Info().iFinalValue;
        iProgressInfo->SetAndDraw( finalValue );
        break;
        }
    case MMemSpyDeviceWideOperationsObserver::EOperationCompleted:
        if  ( iProgressDialog )
            {
            iProgressDialog->ProcessFinishedL();
            }
        break;
    default:
        break;
        }

#ifdef _DEBUG
    RDebug::Print( _L("[MemSpy] CMemSpyDeviceWideOperationDialog::HandleDeviceWideOperationEvent() - END - aEvent: %d, aParam1: %d, aParam2: %S"), aEvent, aParam1, &aParam2 );
#endif
    }


void CMemSpyDeviceWideOperationDialog::SetDialogCaptionL( const TDesC& aText )
    {
    if  ( aText.Length() )
        {
        iProgressDialog->SetTextL( aText );
        iProgressDialog->DrawNow();
        }
    }





