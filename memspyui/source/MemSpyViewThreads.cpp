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

#include "MemSpyViewThreads.h"

// System includes
#include <AknQueryDialog.h>
#include <memspyui.rsg>

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperprocess.h>

// User includes
#include "MemSpyContainerObserver.h"
#include "MemSpyViewProcesses.h"
#include "MemSpyViewThreadInfoItemList.h"




CMemSpyViewThreads::CMemSpyViewThreads( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, CMemSpyProcess& aProcess )
:   CMemSpyViewBase( aEngine, aObserver ), iParentProcess( aProcess )
    {
    iParentProcess.Open();
    }


CMemSpyViewThreads::~CMemSpyViewThreads()
    {
    iParentProcess.Close();
    if  ( iCurrentThread )
        {
        iCurrentThread->Close();
        }
    }

 
void CMemSpyViewThreads::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Threads" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    //
    if  ( aSelectionRune )
        {
        CMemSpyThread* selectedItem = reinterpret_cast< CMemSpyThread* >( aSelectionRune );
        const TInt index = iParentProcess.ThreadIndexById( selectedItem->Id() );
        if  ( index >= 0 && index < iListBox->Model()->NumberOfItems() )
            {
            iListBox->SetCurrentItemIndex( index );
            HandleListBoxItemSelectedL( index );
            }
        }
    else if ( iParentProcess.Count() > 0 )
        {
        iListBox->SetCurrentItemIndex( 0 );
        HandleListBoxItemSelectedL( 0 );
        }
    }


const CMemSpyProcess& CMemSpyViewThreads::Process() const
    {
    return iParentProcess;
    }


CMemSpyThread& CMemSpyViewThreads::CurrentThread()
    {
    __ASSERT_ALWAYS( iCurrentThread != NULL, User::Invariant() );
    return *iCurrentThread;
    }


void CMemSpyViewThreads::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewThreads::ViewType() const
    {
    return EMemSpyViewTypeThreads;
    }


CMemSpyViewBase* CMemSpyViewThreads::PrepareParentViewL()
    {
    CMemSpyViewProcesses* parent = new(ELeave) CMemSpyViewProcesses( iEngine, iObserver, iParentProcess );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), &iParentProcess );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewThreads::PrepareChildViewL()
    {
    CMemSpyViewThreadInfoItemList* child = new(ELeave) CMemSpyViewThreadInfoItemList( iEngine, iObserver, *iCurrentThread );
    CleanupStack::PushL( child );
    child->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( child );
    return child;
    }


void CMemSpyViewThreads::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if  ( aResourceId == R_MEMSPY_MENUPANE )
        {
        CMemSpyThread& thread = CurrentThread();
        aMenuPane->SetItemDimmed( EMemSpyCmdThread, thread.IsDead() );
        }
    }


TBool CMemSpyViewThreads::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
    case EMemSpyCmdThreadEndKill:
        OnCmdEndKillL();
		break;
    case EMemSpyCmdThreadEndTerminate:
        OnCmdEndTerminateL();
		break;
    case EMemSpyCmdThreadEndPanic:
        OnCmdEndPanicL();
		break;
	case EMemSpyCmdThreadSetPriorityAbsoluteVeryLow:
	case EMemSpyCmdThreadSetPriorityAbsoluteLowNormal:
	case EMemSpyCmdThreadSetPriorityAbsoluteLow:
	case EMemSpyCmdThreadSetPriorityAbsoluteBackgroundNormal:
	case EMemSpyCmdThreadSetPriorityAbsoluteBackground:
	case EMemSpyCmdThreadSetPriorityAbsoluteForegroundNormal:
	case EMemSpyCmdThreadSetPriorityAbsoluteForeground:
	case EMemSpyCmdThreadSetPriorityAbsoluteHighNormal:
	case EMemSpyCmdThreadSetPriorityAbsoluteHigh:
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime1:
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime2:
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime3:
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime4:
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime5:
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime6:
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime7: 
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime8:
        OnCmdSetPriorityL( aCommand );
        break;

    case EMemSpyCmdThreadInfoHandles:
        OnCmdInfoHandlesL();
        break;

    default:
        handled = CMemSpyViewBase::HandleCommandL( aCommand );
        break;
        }
    //
    return handled;
    }


void CMemSpyViewThreads::OnCmdEndKillL()
    {
    TBool doTerminate = ETrue;
    CMemSpyThread& thread = CurrentThread();
    //
    if  ( thread.IsSystemPermanent() )
        {
        CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
        doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
        }
    //
    if  ( doTerminate )
        {
        thread.KillL();
        RefreshL();
        }
    }


void CMemSpyViewThreads::OnCmdEndTerminateL()
    {
    TBool doTerminate = ETrue;
    CMemSpyThread& thread = CurrentThread();
    //
    if  ( thread.IsSystemPermanent() || thread.IsSystemCritical() )
        {
        CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
        doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
        }
    //
    if  ( doTerminate )
        {
        thread.TerminateL();
        RefreshL();
        }
    }


void CMemSpyViewThreads::OnCmdEndPanicL()
    {
    TBool doTerminate = ETrue;
    CMemSpyThread& thread = CurrentThread();
    //
    if  ( thread.IsSystemPermanent() || thread.IsSystemCritical() )
        {
        CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
        doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
        }
    //
    if  ( doTerminate )
        {
        thread.PanicL();
        RefreshL();
        }
    }


void CMemSpyViewThreads::OnCmdSetPriorityL( TInt aCommand )
    {
    TThreadPriority pri = EPriorityAbsoluteBackground;
    switch( aCommand )
        {
	case EMemSpyCmdThreadSetPriorityAbsoluteVeryLow:
        pri = EPriorityAbsoluteVeryLow;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteLowNormal:
        pri = EPriorityAbsoluteLowNormal;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteLow:
        pri = EPriorityAbsoluteLow;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteBackgroundNormal:
        pri = EPriorityAbsoluteBackgroundNormal;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteBackground:
        pri = EPriorityAbsoluteBackground;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteForegroundNormal:
        pri = EPriorityAbsoluteForegroundNormal;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteForeground:
        pri = EPriorityAbsoluteForeground;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteHighNormal:
        pri = EPriorityAbsoluteHighNormal;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteHigh:
        pri = EPriorityAbsoluteHigh;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime1:
        pri = EPriorityAbsoluteRealTime1;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime2:
        pri = EPriorityAbsoluteRealTime2;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime3:
        pri = EPriorityAbsoluteRealTime3;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime4:
        pri = EPriorityAbsoluteRealTime4;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime5:
        pri = EPriorityAbsoluteRealTime5;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime6:
        pri = EPriorityAbsoluteRealTime6;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime7: 
        pri = EPriorityAbsoluteRealTime7;
        break;
	case EMemSpyCmdThreadSetPriorityAbsoluteRealTime8:
        pri = EPriorityAbsoluteRealTime8;
        break;
        }

#ifdef _DEBUG
    RDebug::Printf( "CMemSpyViewThreads::OnCmdSetPriorityL() - pri: %d", pri );
#endif

    CMemSpyThread& thread = CurrentThread();
    thread.SetPriorityL( pri );
    RefreshL();

#ifdef _DEBUG
    RDebug::Printf( "CMemSpyViewThreads::OnCmdSetPriorityL() - END" );
#endif
    }


void CMemSpyViewThreads::OnCmdInfoHandlesL()
    {
    CMemSpyThread& thread = CurrentThread();
    thread.InfoContainerForceSyncronousConstructionL().PrintL();
    }


void CMemSpyViewThreads::SetListBoxModelL()
    {
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( &iParentProcess );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewThreads::HandleListBoxItemActionedL( TInt /*aIndex*/ )
    {
    // Notify observer about an item being 'fired'
    ReportEventL( MMemSpyViewObserver::EEventItemActioned );
    }


void CMemSpyViewThreads::HandleListBoxItemSelectedL( TInt aIndex )
    {
    if  ( iCurrentThread )
        {
        iCurrentThread->Close();
        }

    // Obtain the thread that corresponds to the selected item
    CMemSpyThread& thread = iParentProcess.At( aIndex );
    iCurrentThread = &thread;
    iCurrentThread->Open();

    // Notify observer about item selection
    ReportEventL( MMemSpyViewObserver::EEventItemSelected );
    }



