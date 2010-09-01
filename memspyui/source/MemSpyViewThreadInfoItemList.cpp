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


CMemSpyViewThreadInfoItemList::CMemSpyViewThreadInfoItemList( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, CMemSpyThread& aThread )
:   CMemSpyViewBase( aEngine, aObserver ), iThread( aThread )
    {
    iThread.Process().Open();
    iThread.Open();
    }


CMemSpyViewThreadInfoItemList::~CMemSpyViewThreadInfoItemList()
    {
    delete iIdleResetListboxTimer;

    TRAP_IGNORE( 
        CMemSpyThreadInfoContainer& container = iThread.InfoContainerL();
        container.ObserverRemove( *this );
    );

    if  ( iCurrentInfoItem )
        {
        iCurrentInfoItem->Close();
        }

    iThread.Process().Close();
    iThread.Close();
    }



void CMemSpyViewThreadInfoItemList::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Thread Objects" );
    SetTitleL( KTitle );
    //
    iIdleResetListboxTimer = CPeriodic::NewL( CActive::EPriorityIdle );
    //
    CMemSpyThreadInfoContainer& container = iThread.InfoContainerL();
    container.ObserverAddL( *this );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    //
    if  ( aSelectionRune )
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
        {
        iListBox->SetCurrentItemIndex( 0 );
        HandleListBoxItemSelectedL( 0 );
        }
    }


const CMemSpyProcess& CMemSpyViewThreadInfoItemList::Process() const
    {
    return iThread.Process();
    }


const CMemSpyThread& CMemSpyViewThreadInfoItemList::Thread() const
    {
    return iThread;
    }


const CMemSpyThreadInfoItemBase& CMemSpyViewThreadInfoItemList::CurrentInfoItem() const
    {
    __ASSERT_ALWAYS( iCurrentInfoItem != NULL, User::Invariant() );
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
    CMemSpyViewBase* parent = new(ELeave) CMemSpyViewThreads( iEngine, iObserver, iThread.Process() );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), &iThread );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewThreadInfoItemList::PrepareChildViewL()
    {
    __ASSERT_ALWAYS( iCurrentInfoItem != NULL, User::Invariant() );
    CMemSpyViewBase* child = NULL;

    // Decide what type of child view to create...
    const TMemSpyThreadInfoItemType type = iCurrentInfoItem->Type();
    //
    switch( type )
        {
    case EMemSpyThreadInfoItemTypeHeap:
        child = new(ELeave) CMemSpyViewThreadInfoItemHeap( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    case EMemSpyThreadInfoItemTypeStack:
        child = new(ELeave) CMemSpyViewThreadInfoItemStack( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    case EMemSpyThreadInfoItemTypeChunk:
        child = new(ELeave) CMemSpyViewThreadInfoItemChunk( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    case EMemSpyThreadInfoItemTypeCodeSeg:
        child = new(ELeave) CMemSpyViewThreadInfoItemCodeSeg( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    case EMemSpyThreadInfoItemTypeServer:
        child = new(ELeave) CMemSpyViewThreadInfoItemServer( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    case EMemSpyThreadInfoItemTypeActiveObject:
        child = new(ELeave) CMemSpyViewThreadInfoItemActiveObject( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    case EMemSpyThreadInfoItemTypeGeneralInfo:
        child = new(ELeave) CMemSpyViewThreadInfoItemGeneralInfo( iEngine, iObserver, iThread.InfoContainerL() );
        break;
    case EMemSpyThreadInfoItemTypeMemoryTracking:
        child = new(ELeave) CMemSpyViewThreadInfoItemMemoryTracking( iEngine, iObserver, iThread.InfoContainerL() );
        break;
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
        child = new(ELeave) CMemSpyViewThreadInfoItemGeneric( iEngine, iObserver, iThread.InfoContainerL(), type );
        break;
    
    default:
        __ASSERT_ALWAYS( EFalse, User::Panic( _L("MemSpy-View"), 0) );
        break;
        }
    //
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
        aMenuPane->SetItemDimmed( EMemSpyCmdThread, iThread.IsDead() );
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
    iThread.InfoContainerForceSyncronousConstructionL().PrintL();
    }


void CMemSpyViewThreadInfoItemList::HandleMemSpyEngineInfoContainerEventL( TEvent aEvent, TMemSpyThreadInfoItemType aType )
    {
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
    }


void CMemSpyViewThreadInfoItemList::SetListBoxModelL()
    {
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( &iThread.InfoContainerL() );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewThreadInfoItemList::HandleListBoxItemActionedL( TInt /*aIndex*/ )
    {
    // Notify observer about an item being 'fired'
    ReportEventL( MMemSpyViewObserver::EEventItemActioned );
    }


void CMemSpyViewThreadInfoItemList::HandleListBoxItemSelectedL( TInt aIndex )
    {
    if  ( iCurrentInfoItem )
        {
        CMemSpyThreadInfoItemBase* item = iCurrentInfoItem;
        iCurrentInfoItem = NULL;
        item->Close();
        }

    // Identify the type of item to display...
    iCurrentInfoItem = &iThread.InfoContainerL().Item( aIndex );
    iCurrentInfoItem->Open();

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
    }

