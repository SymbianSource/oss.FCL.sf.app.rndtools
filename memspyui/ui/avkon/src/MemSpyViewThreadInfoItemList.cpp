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

#include "MemSpyViewThreadInfoItemList.h"

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperprocess.h>
#include <memspysession.h>

// UI Utils include
#include "MemSpyUiUtils.h"

// User includes
#include "MemSpyContainerObserver.h"
#include "MemSpyViewThreads.h"
#include "MemSpyViewThreadInfoItemHeap.h"
#include "MemSpyViewThreadInfoItemStack.h"
#include "MemSpyViewThreadInfoItemChunk.h"
#include "MemSpyViewThreadInfoItemCodeSeg.h"
#include "MemSpyViewThreadInfoItemServer.h"
#include "MemSpyViewThreadInfoItemActiveObject.h"
#include "MemSpyViewThreadInfoItemGeneralInfo.h"
#include "MemSpyViewThreadInfoItemMemoryTracking.h"

// Constants
const TInt KMemSpyIdleResetListboxTimerPeriod = 250000;


CMemSpyViewThreadInfoItemList::CMemSpyViewThreadInfoItemList( RMemSpySession& aSession, MMemSpyViewObserver& aObserver, TProcessId aProcess, TThreadId aThread )
:   CMemSpyViewBase( aSession, aObserver ), iParentProcessId( aProcess ), iThreadId( aThread )
    {
    }


CMemSpyViewThreadInfoItemList::~CMemSpyViewThreadInfoItemList()
    {
	/* TODO:
    TRAP_IGNORE( 
        CMemSpyThreadInfoContainer& container = iThread.InfoContainerL();
        container.ObserverRemove( *this );
    );
	*/
    }



void CMemSpyViewThreadInfoItemList::ConstructL( const TRect& aRect, CCoeControl& aContainer, TMemSpyThreadInfoItemType aType )
    {
    _LIT( KTitle, "Thread Objects" );
    SetTitleL( KTitle );
    //
    //iIdleResetListboxTimer = CPeriodic::NewL( CActive::EPriorityIdle );    
    /* TODO: to solve observer
    CMemSpyThreadInfoContainer& container = iThread.InfoContainerL();
    container.ObserverAddL( *this );
    */    
    CMemSpyViewBase::ConstructL( aRect, aContainer );
    //
    if( aType > EMemSpyThreadInfoItemTypeFirst || aType < EMemSpyThreadInfoItemTypeLast )
    	{
		TInt index(aType);
		if  ( index >= 0 && index < iListBox->Model()->NumberOfItems() )
			{
			iListBox->SetCurrentItemIndex( index );
			HandleListBoxItemSelectedL( index );
			}
    	}
    else
    	{
		iListBox->SetCurrentItemIndex( 0 );
        HandleListBoxItemSelectedL( 0 );
    	}
    /*if  ( aSelectionRune )
        {		
        CMemSpyThreadInfoItemBase* selectedItem = reinterpret_cast< CMemSpyThreadInfoItemBase* >( aSelectionRune );
        const TInt index = container.InfoItemIndexByType( selectedItem->Type() );
        if  ( index >= 0 && index < iListBox->Model()->NumberOfItems() )
            {
            iListBox->SetCurrentItemIndex( index );
            HandleListBoxItemSelectedL( index );
            }
        }
    else if ( container.MdcaCount() > 0 )
        {*/
        //iListBox->SetCurrentItemIndex( 0 );
        //HandleListBoxItemSelectedL( 0 );
        //}
    }

const CMemSpyThreadInfoItemBase& CMemSpyViewThreadInfoItemList::CurrentInfoItem() const
    {
    //__ASSERT_ALWAYS( iCurrentInfoItem != NULL, User::Invariant() );
    return *iCurrentInfoItem;
    }

void CMemSpyViewThreadInfoItemList::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewThreadInfoItemList::ViewType() const
    {
    return EMemSpyViewTypeThreadInfoItemList;
    }


CMemSpyViewBase* CMemSpyViewThreadInfoItemList::PrepareParentViewL()
    {
    CMemSpyViewBase* parent = new(ELeave) CMemSpyViewThreads( iMemSpySession, iObserver, iParentProcessId, iThreadId );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewThreadInfoItemList::PrepareChildViewL()
    {
    CMemSpyViewBase* child = NULL;

    TMemSpyThreadInfoItemType type;
    iMemSpySession.GetInfoItemType( iCurrentInfoItemId, iThreadId, type );
    
    switch( type )
        {    	
    case EMemSpyThreadInfoItemTypeHeap:
        child = new(ELeave) CMemSpyViewThreadInfoItemHeap( iMemSpySession, iObserver, iParentProcessId, iThreadId, type );
        break;        
    case EMemSpyThreadInfoItemTypeStack:
        child = new(ELeave) CMemSpyViewThreadInfoItemStack( iMemSpySession, iObserver, iParentProcessId, iThreadId, type );
        break;        
    case EMemSpyThreadInfoItemTypeChunk:
        child = new(ELeave) CMemSpyViewThreadInfoItemChunk( iMemSpySession, iObserver, iParentProcessId, iThreadId, type  );
        break;
    case EMemSpyThreadInfoItemTypeCodeSeg:
        child = new(ELeave) CMemSpyViewThreadInfoItemCodeSeg( iMemSpySession, iObserver, iParentProcessId, iThreadId, type  );
        break;        
    case EMemSpyThreadInfoItemTypeServer:
        child = new(ELeave) CMemSpyViewThreadInfoItemServer( iMemSpySession, iObserver, iParentProcessId, iThreadId, type  );
        break;        
    case EMemSpyThreadInfoItemTypeActiveObject:
        child = new(ELeave) CMemSpyViewThreadInfoItemActiveObject( iMemSpySession, iObserver, iParentProcessId, iThreadId, type  );
        break;             
    case EMemSpyThreadInfoItemTypeGeneralInfo:
        child = new(ELeave) CMemSpyViewThreadInfoItemGeneralInfo( iMemSpySession, iObserver, iParentProcessId, iThreadId, type );
        break;
    /* 
     * This subview is permanently removed from Thread Info Item subview.
     * 
    case EMemSpyThreadInfoItemTypeMemoryTracking:
        child = new(ELeave) CMemSpyViewThreadInfoItemMemoryTracking( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    */
    case EMemSpyThreadInfoItemTypeSession:
    case EMemSpyThreadInfoItemTypeSemaphore:
    case EMemSpyThreadInfoItemTypeMutex:
    case EMemSpyThreadInfoItemTypeTimer:
    case EMemSpyThreadInfoItemTypeLDD:
    case EMemSpyThreadInfoItemTypePDD:
    case EMemSpyThreadInfoItemTypeLogicalChannel:
    case EMemSpyThreadInfoItemTypeChangeNotifier:
    case EMemSpyThreadInfoItemTypeUndertaker:
    case EMemSpyThreadInfoItemTypeMessageQueue:
    case EMemSpyThreadInfoItemTypeConditionalVariable:
    case EMemSpyThreadInfoItemTypeOpenFiles:
    case EMemSpyThreadInfoItemTypeOtherThreads:
    case EMemSpyThreadInfoItemTypeOtherProcesses:
    case EMemSpyThreadInfoItemTypeOwnedThreadHandles:
    case EMemSpyThreadInfoItemTypeOwnedProcessHandles:
        child = new(ELeave) CMemSpyViewThreadInfoItemGeneric( iMemSpySession, iObserver, iParentProcessId, iThreadId, type );
        break;
    
    default:
       // __ASSERT_ALWAYS( EFalse, User::Panic( _L("MemSpy-View"), 0) );
        break;        
        }
       
    CleanupStack::PushL( child );
    child->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( child );
    return child;
    }


TBool CMemSpyViewThreadInfoItemList::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
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

    
void CMemSpyViewThreadInfoItemList::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if  ( aResourceId == R_MEMSPY_MENUPANE )
        {
     //   aMenuPane->SetItemDimmed( EMemSpyCmdThread, iThread.IsDead() ); //TODO
        }
    else if ( aResourceId == MenuCascadeResourceId() )
        {
        // Always remove these items - they are only shown in the master thread view
        aMenuPane->SetItemDimmed( EMemSpyCmdThreadSetPriority, ETrue );
        aMenuPane->SetItemDimmed( EMemSpyCmdThreadEnd, ETrue );
        }
    }


void CMemSpyViewThreadInfoItemList::OnCmdInfoHandlesL()
    {
    iMemSpySession.OutputThreadInfoHandlesL( iThreadId );
    }


void CMemSpyViewThreadInfoItemList::HandleMemSpyEngineInfoContainerEventL( TEvent aEvent, TMemSpyThreadInfoItemType aType )
    {/*
    if  ( aEvent == EInfoItemChanged )
        {
        }
    else if ( aEvent == EInfoItemDestroyed )
        {
        if  ( iCurrentInfoItem && iCurrentInfoItem->Type() == aType )
            {
            iCurrentInfoItem->Close();
            iCurrentInfoItem = NULL;
            }
        }

    iIdleResetListboxTimer->Cancel();
    iIdleResetListboxTimer->Start( KMemSpyIdleResetListboxTimerPeriod, KMemSpyIdleResetListboxTimerPeriod, TCallBack( IdleUpdateListBoxModel, this ) );
    */
    }


void CMemSpyViewThreadInfoItemList::SetListBoxModelL()
    {	
	CDesC16ArrayFlat* iModel = new (ELeave) CDesC16ArrayFlat( 32 );
	
	HBufC* iItem = HBufC::NewL( 32 );
	
	iItem = MemSpyUiUtils::FormatItem( KGeneral );
	TPtr pItem( iItem->Des() );
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KHeap );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KStack );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KChunks );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KCodeSegs );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KOpenFiles );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KActiveObjects );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KThreadHandlers );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KProcessHandlers );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KServers );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KConnections );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KSemaphores );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KThreadReferences );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KProcessReferences );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KMutexes );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KTimers );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KDD );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KChangeNotif );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KUndertakers );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KLogicalDrivers );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();
	
	iItem = MemSpyUiUtils::FormatItem( KPhysicalDrivers );
	pItem = iItem->Des();
	iModel->AppendL( pItem );
	pItem.Zero();		
	
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox ); 
    listbox->Model()->SetItemTextArray( iModel );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewThreadInfoItemList::HandleListBoxItemActionedL( TInt /*aIndex*/ )
    {
    // Notify observer about an item being 'fired'
    ReportEventL( MMemSpyViewObserver::EEventItemActioned );
    }


void CMemSpyViewThreadInfoItemList::HandleListBoxItemSelectedL( TInt aIndex )
    {
	iCurrentInfoItemId = aIndex;
	
    // Notify observer about item selection
    ReportEventL( MMemSpyViewObserver::EEventItemSelected );
    }


TInt CMemSpyViewThreadInfoItemList::IdleUpdateListBoxModel( TAny* aSelf )
    {
    CMemSpyViewThreadInfoItemList* self = reinterpret_cast< CMemSpyViewThreadInfoItemList* >( aSelf );
    TRAP_IGNORE( self->DoIdleUpdateListBoxModelL() );
    return EFalse;
    }


void CMemSpyViewThreadInfoItemList::DoIdleUpdateListBoxModelL()
    {
	/*
    CMemSpyThreadInfoContainer& container = iThread.InfoContainerL();

    // Try to maintain current item selection if at all possible.
    TMemSpyThreadInfoItemType type = EMemSpyThreadInfoItemTypeHeap;
    if  ( iCurrentInfoItem )
        {
        type = iCurrentInfoItem->Type();
        }
   
    // Update list box & model
    SetListBoxModelL();
    iListBox->HandleItemAdditionL();
    RefreshL();
    
    // Try to select previous item if it is still available
    const TInt index = container.InfoItemIndexByType( type );
    if  ( index >= 0 && index < container.MdcaCount() )
        {
        iListBox->SetCurrentItemIndex( index );
        HandleListBoxItemSelectedL( index );
        }

    iIdleResetListboxTimer->Cancel();
    */
    }
