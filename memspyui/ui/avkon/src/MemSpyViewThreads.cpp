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
#include <memspysession.h>

// User includes
#include "MemSpyContainerObserver.h"
#include "MemSpyViewProcesses.h"
#include "MemSpyViewThreadInfoItemList.h"



CMemSpyViewThreads::CMemSpyViewThreads( RMemSpySession& aSession, MMemSpyViewObserver& aObserver, TProcessId aId, TThreadId aThreadId )
:   CMemSpyViewBase( aSession, aObserver ), iParentProcessId( aId ), iCurrentThreadId( aThreadId )
    {    
    }


CMemSpyViewThreads::~CMemSpyViewThreads()
    {
    }

 
void CMemSpyViewThreads::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
	iMemSpySession.GetThreadsL( iParentProcessId, iThreads );
	
    _LIT( KTitle, "Threads" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    //    
    if( iCurrentThreadId > 0 )
        {		
		for( TInt i = 0; i < iThreads.Count() ; i++ )
			{
			if( iThreads[i]->Id() == iCurrentThreadId )
				{
				const TInt index = i;
				if  ( index >= 0 && index < iListBox->Model()->NumberOfItems() )
					{
					iListBox->SetCurrentItemIndex( index );
					HandleListBoxItemSelectedL( index );
					}
				}			
			}
        }
    else if( iThreads.Count() > 0 )
    	{
		iListBox->SetCurrentItemIndex( 0 );
		HandleListBoxItemSelectedL( 0 );
    	}		
    }


TProcessId CMemSpyViewThreads::Process() const
    {
	return iParentProcessId;
    }


TThreadId CMemSpyViewThreads::CurrentThread()
    { 
	return iCurrentThreadId;
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
    CMemSpyViewProcesses* parent = new(ELeave) CMemSpyViewProcesses( iMemSpySession, iObserver, iParentProcessId );
    CleanupStack::PushL( parent );
    //parent->ConstructL( Rect(), *Parent(), &iParentProcessId );
    parent->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewThreads::PrepareChildViewL()
    {	
    CMemSpyViewThreadInfoItemList* child = new(ELeave) CMemSpyViewThreadInfoItemList( iMemSpySession, iObserver, iParentProcessId, iCurrentThreadId );
    CleanupStack::PushL( child );
    child->ConstructL( Rect(), *Parent(), EMemSpyThreadInfoItemTypeFirst );
    CleanupStack::Pop( child );
    return child;    
    }


void CMemSpyViewThreads::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if  ( aResourceId == R_MEMSPY_MENUPANE )
        {
		/*
        CMemSpyThread& thread = CurrentThread();
        aMenuPane->SetItemDimmed( EMemSpyCmdThread, thread.IsDead() );
        */
		aMenuPane->SetItemDimmed( EMemSpyCmdThread, (iThreads[iListBox->CurrentItemIndex()]->ExitType() !=  EExitPending ));
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
	/*
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
    */
    }


void CMemSpyViewThreads::OnCmdEndTerminateL()
    {
	/*
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
    */
    }


void CMemSpyViewThreads::OnCmdEndPanicL()
    {
	/*
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
        */
    }


void CMemSpyViewThreads::OnCmdSetPriorityL( TInt aCommand )
    {
	/*
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
	*/
    }


void CMemSpyViewThreads::OnCmdInfoHandlesL()
    {
	iMemSpySession.OutputThreadInfoHandlesL( iCurrentThreadId );	
    }


void CMemSpyViewThreads::SetListBoxModelL()
    {
	//iMemSpySession.GetThreadsL( iParentProcessId, iThreads );
	
	iModel = new (ELeave) CDesC16ArrayFlat( iThreads.Count() ); //array for formated items
		
	_LIT( KTab, "\t" );	
		
		
		for( TInt i=0; i < iThreads.Count(); i++ )
			{
			HBufC* tempName = HBufC::NewL( iThreads[i]->Name().Length() + 16 );
	    	CleanupStack::PushL( tempName );
	    	TPtr tempNamePtr( tempName->Des() );
	    	tempNamePtr.Copy( KTab );
	    	tempNamePtr.Append( iThreads[i]->Name() );
	    	iModel->AppendL( tempNamePtr );
	    	
	    	CleanupStack::PopAndDestroy( tempName ); 
			}			
	
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );    
    listbox->Model()->SetItemTextArray( iModel );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewThreads::HandleListBoxItemActionedL( TInt /*aIndex*/ )
    {
    // Notify observer about an item being 'fired'
    ReportEventL( MMemSpyViewObserver::EEventItemActioned );
    }


void CMemSpyViewThreads::HandleListBoxItemSelectedL( TInt aIndex )
	{	
	iCurrentThreadId = iThreads[ aIndex ]->Id();
	
    // Notify observer about item selection
    ReportEventL( MMemSpyViewObserver::EEventItemSelected );
    }



