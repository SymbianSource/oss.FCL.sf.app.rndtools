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

#include "MemSpyViewKernelObjects.h"

// System includes
#include <eikspane.h>
#include <aknnavide.h> 
#include <akntabgrp.h>
#include <eikclbd.h>
#include <AknQueryDialog.h>
#include <aknnotewrappers.h>
#include <apgwgnam.h>
#include <aknmessagequerydialog.h>

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperkernelcontainers.h>
#include <memspy/engine/memspyenginehelperserver.h>
#include <memspy/engine/memspyenginehelpercondvar.h>

// User includes
#include "MemSpyUiUtils.h"
#include "MemSpyViewKernel.h"
#include "MemSpyContainerObserver.h"
#include "MemSpyViewKernelContainers.h"

const TInt KMaxInfoLength = 128;

CMemSpyViewKernelObjects::CMemSpyViewKernelObjects( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, TMemSpyDriverContainerType aObjectType )
:   CMemSpyViewBase( aEngine, aObserver ),
    iObjectType( aObjectType ),
    iCurrItemIndex( KErrNotFound )
    {
    }


CMemSpyViewKernelObjects::~CMemSpyViewKernelObjects()
    {
    if ( iNavContainer && iNavDecorator )
        {
        iNavContainer->Pop( iNavDecorator );
        delete iNavDecorator;
        }
    delete iModel;
    delete iItems;
    delete iObjectList;
    }


void CMemSpyViewKernelObjects::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Kernel Objects" );
    SetTitleL( KTitle );
    iItems = new(ELeave) CDesCArrayFlat(5);
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    CreateTabsL();
    }


CEikListBox* CMemSpyViewKernelObjects::ConstructListBoxL()
    {
    delete iListBox;
    iListBox = NULL;
    CAknSingleStyleListBox* listbox = new (ELeave) CAknSingleStyleListBox();
    iListBox = listbox;
    listbox->ConstructL( this, EAknListBoxSelectionList | EAknListBoxLoopScrolling );
    listbox->SetContainerWindowL( *this );
    listbox->CreateScrollBarFrameL( ETrue );
    SetListBoxModelL();
    listbox->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
    listbox->SetListBoxObserver( this );
    listbox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
    listbox->SetObserver( this );
    return listbox;
    }


void CMemSpyViewKernelObjects::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewKernelObjects::ViewType() const
    {
    return EMemSpyViewTypeKernelObjects;
    }


CMemSpyViewBase* CMemSpyViewKernelObjects::PrepareParentViewL()
    {
    if ( iNavContainer && iNavDecorator )
        {
        iNavContainer->Pop( iNavDecorator );
        delete iNavDecorator;
        iNavDecorator = NULL;
        }
    CMemSpyViewBase* parent = new(ELeave) CMemSpyViewKernelContainers( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewKernelObjects::PrepareChildViewL()
    {
    CMemSpyViewBase* child = NULL;
    if ( iListBox && 
         iListBox->Model()->NumberOfItems() && 
         iListBox->CurrentItemIndex() > KErrNotFound )
        {
        DetailsL();
        }
    return child;
    }


void CMemSpyViewKernelObjects::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if  ( aResourceId == R_MEMSPY_MENUPANE )
       {
       if ( iObjectType > EMemSpyDriverContainerTypeProcess )
           {
           // Display Terminate, Switch to, End and Panic
           // commands only for Threads and processes
           aMenuPane->SetItemDimmed( EMemSpyCmdKernelObjects, ETrue );
           }
        }
    }


void CMemSpyViewKernelObjects::SetListBoxModelL()
    {
    _LIT( KLineFormatSpec, "\t%S" );
    
    // Take ownership of new model
    CMemSpyEngineHelperKernelContainers& kernelContainerManager = iEngine.HelperKernelContainers();
    CMemSpyEngineGenericKernelObjectContainer* model = kernelContainerManager.ObjectsAllL();
    
    delete iModel;
    iModel = model;
    
    iItems->Reset();
    delete iObjectList;
    iObjectList = NULL;
    iObjectList = kernelContainerManager.ObjectsForSpecificContainerL( iObjectType );
    
    // TODO: CMemSpyEngineGenericKernelObjectList could inherit MDesCArray like
    // CMemSpyEngineGenericKernelObjectContainer does. iObjectList could then be passed
    // to listbox model:
    // listbox->Model()->SetItemTextArray( iObjectList );
    
    TName item;
    TName name;
    for ( TInt i = 0; i < iObjectList->Count(); i++ )
        {
        name.Copy( iObjectList->At( i ).iName );
        item.Format( KLineFormatSpec, &name );
        iItems->AppendL( item );
        }
    
    // Set up list box
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( iItems );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


TBool CMemSpyViewKernelObjects::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
        case EMemSpyCmdKernelObjectTerminate:
            {
            TRAPD( err, OnCmdTerminateL() );
            if ( err )
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote( ETrue );
                note->ExecuteLD( _L("Cannot terminate task") );
                }
            break;
            }
        case EMemSpyCmdKernelObjectSwitchTo:
            {
            TRAPD( err, OnCmdSwitchToL() );
            if ( err )
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote( ETrue );
                note->ExecuteLD( _L("Cannot bring to foreground") );
                }
            break;
            }
        case EMemSpyCmdKernelObjectEnd:
            {
            TRAPD( err, OnCmdEndL() );
            if ( err )
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote( ETrue );
                note->ExecuteLD( _L("Cannot end task") );
                }
            break;
            }
        case EMemSpyCmdKernelObjectPanic:
            {
            TRAPD( err, OnCmdPanicL() );
            if ( err )
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote( ETrue );
                note->ExecuteLD( _L("Cannot panic task") );
                }
            break;
            }
        default:
            {
            handled = CMemSpyViewBase::HandleCommandL( aCommand );
            break;        
            }
        }
    //
    return handled;
    }


TKeyResponse CMemSpyViewKernelObjects::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse resp = EKeyWasNotConsumed;
    if  ( iTabs && ( aKeyEvent.iScanCode == EStdKeyRightArrow || aKeyEvent.iScanCode == EStdKeyLeftArrow ) )
        {
        resp = iTabs->OfferKeyEventL( aKeyEvent, aType );
        }
    else
        {
        resp = CMemSpyViewBase::OfferKeyEventL( aKeyEvent, aType );    
        }
    return resp;
    }


void CMemSpyViewKernelObjects::TabChangedL( TInt /*aIndex*/ )
    {
    iObjectType = (TMemSpyDriverContainerType)iTabs->ActiveTabId();
    RefreshL();
    }


void CMemSpyViewKernelObjects::OnCmdTerminateL()
    {
    switch ( iObjectType )
        {
        case EMemSpyDriverContainerTypeThread:
            {
            TBool doTerminate = ETrue;
            
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TThreadId id( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            
            // Try to find the thread in question...
            CMemSpyProcess* process = NULL;
            CMemSpyThread* thread = NULL; 
            User::LeaveIfError( container.ProcessAndThreadByThreadId( id, process, thread ) );

            if ( thread )
                {
                thread->Open();
                //
                if  ( thread->IsSystemPermanent() || thread->IsSystemCritical() )
                    {
                    CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
                    doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
                    }
                //
                if  ( doTerminate )
                    {
                    thread->TerminateL();
                    }                
                }
            RefreshL();
            break;
            }
        case EMemSpyDriverContainerTypeProcess:
            {
            TBool doTerminate = ETrue;
            // Obtain the process that corresponds to the selected item
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TProcessId id( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            CMemSpyProcess& process = container.ProcessByIdL( id );
            process.Open();

            if  ( process.IsSystemPermanent() || process.IsSystemCritical() )
                {
                CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
                doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
                }
            //
            if  ( doTerminate )
                {
                process.TerminateL();
                RefreshL();
                }
            break;
            }
        default:
            {
            // Programming error
            __ASSERT_ALWAYS( EFalse, User::Panic( _L("MemSpy-View"), 0 ) );
            }
        }
    }


void CMemSpyViewKernelObjects::OnCmdSwitchToL()
    {
    TInt wgCount;
    RWsSession wsSession;
    User::LeaveIfError( wsSession.Connect() );
    CleanupClosePushL( wsSession );
    User::LeaveIfError( wgCount = wsSession.NumWindowGroups() );
    RArray<RWsSession::TWindowGroupChainInfo> wgArray;
    CleanupClosePushL( wgArray );
    User::LeaveIfError( wsSession.WindowGroupList( &wgArray ) );
    TApaTask task( wsSession );
    TBool brought( EFalse );
    TInt wgId( KErrNotFound );
    TThreadId threadId;
    switch ( iObjectType )
        {
        case EMemSpyDriverContainerTypeThread:
            {
            TThreadId currentThreadId( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            
            // loop trough all window groups and see if a thread id matches
            while( !brought && wgCount-- )
                {
                wgId = wgArray[wgCount].iId;
                User::LeaveIfError( wsSession.GetWindowGroupClientThreadId( wgId, threadId ) );
                if ( threadId == currentThreadId )
                    {
                    CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC( wsSession, wgId );
                    task.SetWgId( wgId );
                    if ( !wgName->Hidden() && task.Exists() )
                        {
                        task.BringToForeground();
                        brought = ETrue;                        
                        }
                    CleanupStack::PopAndDestroy( wgName );
                    }
                }                
            break;
            }
        case EMemSpyDriverContainerTypeProcess:
            {
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TProcessId id( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            CMemSpyProcess& process = container.ProcessByIdL( id );
            
            // loop trough threads in a process
            for ( TInt i = 0; i < process.MdcaCount(); i++ )
                {
                TInt wgCountLocal = wgCount;
                
                // loop trough all window groups and see if a thread id matches
                while( !brought && wgCountLocal-- )
                    {
                    wgId = wgArray[wgCountLocal].iId;
                    User::LeaveIfError( wsSession.GetWindowGroupClientThreadId( wgId, threadId ) );
                    if ( threadId == process.At( i ).Id() )
                        {
                        CApaWindowGroupName* wgName = CApaWindowGroupName::NewLC( wsSession, wgId );
                        task.SetWgId( wgId );
                        if ( !wgName->Hidden() && task.Exists() )
                            {
                            task.BringToForeground();
                            brought = ETrue;                        
                            }
                        CleanupStack::PopAndDestroy( wgName );
                        }
                    }
                }

            break;
            }
        default:
            {
            // Programming error
            __ASSERT_ALWAYS( EFalse, User::Panic( _L("MemSpy-View"), 0 ) );
            }
        }
    if ( !brought )
        {
        // Error handling in HandleCommandL
        User::Leave( KErrGeneral );
        }
    CleanupStack::PopAndDestroy( 2 ); //wgArray,wsSession
    }


void CMemSpyViewKernelObjects::OnCmdEndL()
    {
    switch ( iObjectType )
        {
        case EMemSpyDriverContainerTypeThread:
            {
            TBool doTerminate = ETrue;
            
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TThreadId id( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            
            // Try to find the thread in question...
            CMemSpyProcess* process = NULL;
            CMemSpyThread* thread = NULL; 
            User::LeaveIfError( container.ProcessAndThreadByThreadId( id, process, thread ) );

            if ( thread )
                {
                thread->Open();
                //
                if  ( thread->IsSystemPermanent() || thread->IsSystemCritical() )
                    {
                    CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
                    doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
                    }
                //
                if  ( doTerminate )
                    {
                    thread->KillL();
                    }                
                }
            RefreshL();
            break;
            }
        case EMemSpyDriverContainerTypeProcess:
            {
            TBool doTerminate = ETrue;
            // Obtain the process that corresponds to the selected item
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TProcessId id( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            CMemSpyProcess& process = container.ProcessByIdL( id );
            process.Open();

            if  ( process.IsSystemPermanent() || process.IsSystemCritical() )
                {
                CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
                doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
                }
            if  ( doTerminate )
                {
                process.KillL();
                RefreshL();
                }

            break;
            }
        default:
            {
            // Programming error
            __ASSERT_ALWAYS( EFalse, User::Panic( _L("MemSpy-View"), 0 ) );
            }
        }    
    }


void CMemSpyViewKernelObjects::OnCmdPanicL()
    {
    switch ( iObjectType )
        {
        case EMemSpyDriverContainerTypeThread:
            {
            TBool doTerminate = ETrue;
            
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TThreadId id( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            
            // Try to find the thread in question...
            CMemSpyProcess* process = NULL;
            CMemSpyThread* thread = NULL; 
            User::LeaveIfError( container.ProcessAndThreadByThreadId( id, process, thread ) );

            if ( thread )
                {
                thread->Open();
                //
                if  ( thread->IsSystemPermanent() || thread->IsSystemCritical() )
                    {
                    CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
                    doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
                    }
                //
                if  ( doTerminate )
                    {
                    thread->PanicL();
                    }                
                }
            RefreshL();
            break;
            }
        case EMemSpyDriverContainerTypeProcess:
            {
            TBool doTerminate = ETrue;
            // Obtain the process that corresponds to the selected item
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TProcessId id( iObjectList->At( iListBox->CurrentItemIndex() ).iId );
            CMemSpyProcess& process = container.ProcessByIdL( id );
            process.Open();

            if  ( process.IsSystemPermanent() || process.IsSystemCritical() )
                {
                CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
                doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
                }
            if  ( doTerminate )
                {
                process.PanicL();
                RefreshL();
                }
            break;
            }
        default:
            {
            // Programming error
            __ASSERT_ALWAYS( EFalse, User::Panic( _L("MemSpy-View"), 0 ) );
            }
        }
    }


void CMemSpyViewKernelObjects::CreateTabsL()
    {
    CEikStatusPane* statusPane = static_cast<CAknAppUi*> ( iEikonEnv->EikAppUi() )->StatusPane();
    TUid uid;
    uid.iUid = EEikStatusPaneUidNavi;
    iNavDecorator = ((CAknNavigationControlContainer*)(statusPane->ControlL(uid)))->Top();

    if ( !iNavDecorator )
        {
        iNavContainer = (CAknNavigationControlContainer*)statusPane->ControlL(uid);
        iNavDecorator = iNavContainer->CreateTabGroupL();

        iTabs = ( CAknTabGroup* )iNavDecorator->DecoratedControl();
        iTabs->SetTabFixedWidthL( KTabWidthWithThreeLongTabs );
    
        for ( TInt i = 0; i < iModel->Count(); i++ )
            {
            const CMemSpyEngineGenericKernelObjectList& item = iModel->At( i );
            iTabs->AddTabL( item.Type(), item.TypeAsString( item.Type() ) );
            }
        iTabs->SetActiveTabById( iObjectType );
        iTabs->SetObserver( this );
        iNavContainer->PushL( *iNavDecorator );
        }
    }


void CMemSpyViewKernelObjects::DetailsL()
    {
    HBufC* messageBuf = HBufC::NewLC( 40960 );
    TPtr messagePtr = messageBuf->Des();

    const TMemSpyDriverHandleInfoGeneric& selectedObject = iObjectList->At( iListBox->CurrentItemIndex() );

    // General attributes:
    TFullName name;
    name.Copy( selectedObject.iNameDetail );
    AppendFormatString( messagePtr, _L("Name: %S\n"), &name );
    name.Zero();
    name.Copy( selectedObject.iName );
    AppendFormatString( messagePtr, _L("FullName: %S\n"), &name );
    name.Zero();
    AppendFormatString( messagePtr, _L("AccessCount: %d\n"), selectedObject.iAccessCount );
    AppendFormatString( messagePtr, _L("UniqueID: %d\n"), selectedObject.iUniqueID );
    AppendFormatString( messagePtr, _L("Protection: %u\n"), selectedObject.iProtection );
    AppendFormatString( messagePtr, _L("OwnrAddr: 0x%08X\n"), selectedObject.iAddressOfKernelOwner );
    AppendFormatString( messagePtr, _L("KernelAddr: 0x%08X"), selectedObject.iHandle );
    
    // Object type specific attributes:
    switch ( selectedObject.iType )
        {
        case EMemSpyDriverContainerTypeUnknown:
            {
            break;
            }
        case EMemSpyDriverContainerTypeThread:
            {
            AppendFormatString( messagePtr, _L("\nOwnrPrAddr: 0x%08X\n"), selectedObject.iAddressOfOwningProcess );
            AppendFormatString( messagePtr, _L("ThreadID: %d\n"), selectedObject.iId );
            AppendFormatString( messagePtr, _L("Priority: %d\n"), selectedObject.iPriority );
            TFullName procName;
            procName.Copy( selectedObject.iNameOfOwner );
            AppendFormatString( messagePtr, _L("Proc: %S"), &procName );
            procName.Zero();
            break;
            }
        case EMemSpyDriverContainerTypeProcess:
            {
            AppendFormatString( messagePtr, _L("\nOwnrPrAddr: 0x%08X\n"), selectedObject.iAddressOfOwningProcess );
            AppendFormatString( messagePtr, _L("CreatorId: %u\n"), selectedObject.iCreatorId );
            AppendFormatString( messagePtr, _L("Attributes: 0x%08X\n"), selectedObject.iAttributes );
            AppendFormatString( messagePtr, _L("StckChnk: 0x%08X\n"), selectedObject.iAddressOfDataBssStackChunk );
            AppendFormatString( messagePtr, _L("ProcessID: %d\n"), selectedObject.iId );
            AppendFormatString( messagePtr, _L("Priority: %d\n"), selectedObject.iPriority );
            AppendFormatString( messagePtr, _L("SecurityZone: %d"), selectedObject.iSecurityZone );
            CMemSpyEngineObjectContainer& container = iEngine.Container();
            TProcessId id( selectedObject.iId );
            TRAP_IGNORE(
                CMemSpyProcess& process = container.ProcessByIdL( id );
                AppendFormatString( messagePtr, _L("\nSID: 0x%08X\n"), process.SID() );
                AppendFormatString( messagePtr, _L("VID: 0x%08X\n"), process.VID() );
                AppendFormatString( messagePtr, _L("UID1: 0x%08X\n"), process.UIDs()[0].iUid  );
                AppendFormatString( messagePtr, _L("UID2: 0x%08X\n"), process.UIDs()[1].iUid  );
                AppendFormatString( messagePtr, _L("UID3: 0x%08X\n"), process.UIDs()[2].iUid );
                AppendFormatString( messagePtr, _L("Caps: 0x%08X%08X"), process.Capabilities().iCaps[0], process.Capabilities().iCaps[1]);
                );
            break;
            }
        case EMemSpyDriverContainerTypeChunk:
            {
            AppendFormatString( messagePtr, _L("\nOwnrPrAddr: 0x%08X\n"), selectedObject.iAddressOfOwningProcess );
            AppendFormatString( messagePtr, _L("Size: %d\n"), selectedObject.iSize );
            AppendFormatString( messagePtr, _L("MaxSize: %d\n"), selectedObject.iMaxSize );
            AppendFormatString( messagePtr, _L("Bottom: %d\n"), selectedObject.iBottom );
            AppendFormatString( messagePtr, _L("Top: %d\n"), selectedObject.iTop );
            AppendFormatString( messagePtr, _L("Attr: 0x%08X\n"), selectedObject.iAttributes );
            AppendFormatString( messagePtr, _L("Start: %d\n"), selectedObject.iStartPos );
            AppendFormatString( messagePtr, _L("CntrlID: %u\n"), selectedObject.iControllingOwner );
            AppendFormatString( messagePtr, _L("Restrictions: %u\n"), selectedObject.iRestrictions );
            AppendFormatString( messagePtr, _L("MapAttr: %u\n"), selectedObject.iMapAttr );
            AppendFormatString( messagePtr, _L("Type: %u\n"), selectedObject.iChunkType );
            TFullName procName;
            procName.Copy( selectedObject.iNameOfOwner );
            AppendFormatString( messagePtr, _L("Proc: %S"), &procName );
            procName.Zero();
            // OwnerAddr iAddressOfKernelOwner already listed
            // ProcAddr iAddressOfOwningProcess already listed
            break;
            }
        case EMemSpyDriverContainerTypeLibrary:
            {
            AppendFormatString( messagePtr, _L("\nMapCount: %d\n"), selectedObject.iMapCount );
            AppendFormatString( messagePtr, _L("State: %u\n"), selectedObject.iState );
            AppendFormatString( messagePtr, _L("CodeSeg: 0x%08X"), selectedObject.iAddressOfCodeSeg );
            break;
            }
        case EMemSpyDriverContainerTypeSemaphore:
            {
            AppendFormatString( messagePtr, _L("\nCount: %d\n"), selectedObject.iCount );
            AppendFormatString( messagePtr, _L("Resetting: %u"), selectedObject.iResetting );
            break;
            }
        case EMemSpyDriverContainerTypeMutex:
            {
            AppendFormatString( messagePtr, _L("\nHoldCount: %d\n"), selectedObject.iCount );
            AppendFormatString( messagePtr, _L("WaitCount: %d\n"), selectedObject.iWaitCount );
            AppendFormatString( messagePtr, _L("Resetting: %u\n"), selectedObject.iResetting );
            AppendFormatString( messagePtr, _L("Order: %u"), selectedObject.iOrder );
            break;
            }
        case EMemSpyDriverContainerTypeTimer:
            {
            AppendFormatString( messagePtr, _L("\nState: %d\n"), selectedObject.iTimerState );
            AppendFormatString( messagePtr, _L("Type: %d"), selectedObject.iTimerType );
            break;
            }
        case EMemSpyDriverContainerTypeServer:
            {
            // Owner == ThrdAddr
            AppendFormatString( messagePtr, _L("\nThrdAddr: 0x%08X\n"), selectedObject.iAddressOfOwningThread );
            TFullName thrName;
            thrName.Copy( selectedObject.iNameOfOwner );
            AppendFormatString( messagePtr, _L("Thr: %S\n"), &thrName );
            thrName.Zero();
            AppendFormatString( messagePtr, _L("Type: %d\n"), selectedObject.iSessionType );
            TBuf8<55>buf;
            RArray<TMemSpyDriverServerSessionInfo> sessions;
            CleanupClosePushL( sessions );
            iEngine.HelperServer().GetServerSessionsL( selectedObject, sessions );
            const TInt count = sessions.Count();
            for ( TInt i = 0; i < count; i++ )
                {
                const TMemSpyDriverServerSessionInfo& session = sessions[ i ];
                AppendFormatString( messagePtr, _L("SessAddr: 0x%08X\n"), session.iAddress );
                TFullName sessName;
                sessName.Copy( session.iName );
                AppendFormatString( messagePtr, _L("Sess: %S\n"), &sessName );
                }
            CleanupStack::PopAndDestroy( &sessions );
            break;
            }
        case EMemSpyDriverContainerTypeSession:
            {
            // Server == SvrAddr
            AppendFormatString( messagePtr, _L("\nServer: 0x%08X\n"), selectedObject.iAddressOfServer );
            TFullName srvName;
            srvName.Copy( selectedObject.iName );
            AppendFormatString( messagePtr, _L("Srv: %S\n"), &srvName );
            srvName.Zero();
            // Session: not supported
            AppendFormatString( messagePtr, _L("AccCount: %u\n"), selectedObject.iTotalAccessCount );
            AppendFormatString( messagePtr, _L("SesType: %u\n"), selectedObject.iSessionType );
            AppendFormatString( messagePtr, _L("SvrType: %u\n"), selectedObject.iSvrSessionType );
            AppendFormatString( messagePtr, _L("MsgCount: %d\n"),  selectedObject.iMsgCount );
            AppendFormatString( messagePtr, _L("MsgLimit: %d"), selectedObject.iMsgLimit );
            break;
            }
        case EMemSpyDriverContainerTypeLogicalDevice:
            {
            AppendFormatString( messagePtr, 
                                _L("\nVersion: %d.%d.%d\n"), 
                                selectedObject.iVersion.iMajor,
                                selectedObject.iVersion.iMinor,
                                selectedObject.iVersion.iBuild );
            AppendFormatString( messagePtr, _L("ParseMask: 0x%08X\n"), selectedObject.iParseMask );
            AppendFormatString( messagePtr, _L("UnitsMask: 0x%08X\n"), selectedObject.iUnitsMask );
            AppendFormatString( messagePtr, _L("Open channels: %d"), selectedObject.iOpenChannels );
            break;
            }
        case EMemSpyDriverContainerTypePhysicalDevice:
            {
            AppendFormatString( messagePtr, 
                                _L("\nVersion: %d.%d.%d\n"), 
                                selectedObject.iVersion.iMajor,
                                selectedObject.iVersion.iMinor,
                                selectedObject.iVersion.iBuild );
            AppendFormatString( messagePtr, _L("UnitsMask: 0x%08X\n"), selectedObject.iUnitsMask );
            AppendFormatString( messagePtr, _L("CodeSeg: 0x%08X"), selectedObject.iAddressOfCodeSeg );
            break;
            }
        case EMemSpyDriverContainerTypeLogicalChannel:
            {
            // No other details
            break;
            }
        case EMemSpyDriverContainerTypeChangeNotifier:
            {
            AppendFormatString( messagePtr, _L("\nChanges: %u\n"), selectedObject.iChanges );
            // Thread == ThrdAddr
            AppendFormatString( messagePtr, _L("ThrdAddr: 0x%08X\n"), selectedObject.iAddressOfOwningThread );
            TFullName thrName;
            thrName.Copy( selectedObject.iNameOfOwner );
            AppendFormatString( messagePtr, _L("Thr: %S"), &thrName );
            thrName.Zero();
            break;
            }
        case EMemSpyDriverContainerTypeUndertaker:
            {
            // Thread == ThrdAddr
            AppendFormatString( messagePtr, _L("\nThrdAddr: 0x%08X\n"), selectedObject.iAddressOfOwningThread );
            TFullName thrName;
            thrName.Copy( selectedObject.iNameOfOwner );
            AppendFormatString( messagePtr, _L("Thr: %S"), &thrName );
            thrName.Zero();
            break;
            }
        case EMemSpyDriverContainerTypeMsgQueue:
            {
            // No other details
            break;
            }
        case EMemSpyDriverContainerTypePropertyRef:
            {
            /*
            Not listing details here, as propertyRef is not listed in TaskMgr.
            Following propertyRef attributes are available at engine side. 
            
            IsReady
            Type
            Category
            Key
            RefCount
            ThreadId
            CreatorSID
            */
            break;
            }
        case EMemSpyDriverContainerTypeCondVar:
            {
            AppendFormatString( messagePtr, _L("\nResetting: %u\n"), selectedObject.iResetting );
            // Using iAddressOfOwningThread for mutex
            AppendFormatString( messagePtr, _L("Mutex: 0x%08X\n"), selectedObject.iAddressOfOwningThread );
            // Using iNameOfOwner for mutex
            TFullName mtxName;
            mtxName.Copy( selectedObject.iNameOfOwner );
            AppendFormatString( messagePtr, _L("Mtx: %S\n"), &mtxName );
            mtxName.Zero();
            AppendFormatString( messagePtr, _L("WaitCount: %d\n"), selectedObject.iWaitCount );
            
            RArray<TMemSpyDriverCondVarSuspendedThreadInfo> threads;
            CleanupClosePushL( threads );
            iEngine.HelperCondVar().GetCondVarSuspendedThreadsL( selectedObject, threads );
            const TInt count = threads.Count();
            for ( TInt i = 0; i < count; i++ )
                {
                const TMemSpyDriverCondVarSuspendedThreadInfo& thr = threads[ i ];
                AppendFormatString( messagePtr, _L("SuspThrdAddr: 0x%08X\n"), thr.iAddress );
                TFullName thrName;
                thrName.Copy( thr.iName );
                AppendFormatString( messagePtr, _L("Thr: %S\n"), &thrName );
                }
            CleanupStack::PopAndDestroy( &threads );
            break;
            }
        default:
            {
            // Programming error
            __ASSERT_ALWAYS( EFalse, User::Panic( _L("MemSpy-View"), 0 ) );
            }
        }
    
    CAknMessageQueryDialog* dialog = new(ELeave) CAknMessageQueryDialog( CAknQueryDialog::ENoTone );
    dialog->PrepareLC( R_MEMSPY_KERNEL_OBJECT_DETAILS_DIALOG );
    TFileName headerText;
    headerText.Copy( selectedObject.iName );
    dialog->SetHeaderTextL( headerText );
    dialog->SetMessageTextL(messagePtr);
    dialog->RunLD();
    CleanupStack::PopAndDestroy( messageBuf );
    }


void CMemSpyViewKernelObjects::AppendFormatString( TPtr& aPtr, TRefByValue<const TDesC> aFmt, ... )
    {
    TBuf<KMaxInfoLength> infoString;
    VA_LIST list;
    VA_START ( list, aFmt );
    infoString.FormatList( aFmt, list );
    aPtr.Append( infoString );
    }

