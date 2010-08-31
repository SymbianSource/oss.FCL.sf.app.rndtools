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

#include "MemSpyViewHeapTrackingResults.h"

// System includes
#include <hal.h>
#ifdef __EPOC32__
#include <e32rom.h>
#endif

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyenginehelpersysmemtrackercycle.h>
#include <memspy/engine/memspyenginehelpersysmemtrackercyclechange.h>
#include <memspy/engine/memspyenginehelpersysmemtrackerenums.h>
#include <memspy/engine/memspyenginehelpersysmemtracker.h>
#include <memspy/engine/memspyenginehelpersysmemtrackerconfig.h>
#include <memspyui.rsg>

// User includes
#include "MemSpyUiUtils.h"
#include "MemSpyContainer.h"
#include "MemSpyViewHeapTracking.h"
#include "MemSpyContainerObserver.h"


CMemSpyViewHeapTrackingResults::CMemSpyViewHeapTrackingResults( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver )
:   CMemSpyViewBase( aEngine, aObserver )
    {
    iEngine.HelperSysMemTracker().SetObserver( this );
    }


CMemSpyViewHeapTrackingResults::~CMemSpyViewHeapTrackingResults()
    {
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    }


void CMemSpyViewHeapTrackingResults::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Results Summary" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );

    // Make sure the correct item is selected
    const TInt cycleNumber = ((TInt) aSelectionRune);
    const TInt maxCycle = iEngine.HelperSysMemTracker().MdcaCount();
    if  ( cycleNumber > 0 && cycleNumber <= maxCycle )
        {
        iListBox->SetCurrentItemIndex( cycleNumber - 1 );
        HandleListBoxItemSelectedL( cycleNumber );
        }
    }


void CMemSpyViewHeapTrackingResults::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewHeapTrackingResults::ViewType() const
    {
    return EMemSpyViewTypeHeapTrackingResults;
    }


CMemSpyViewBase* CMemSpyViewHeapTrackingResults::PrepareParentViewL()
    {
    CMemSpyViewHeapTracking* parent = new(ELeave) CMemSpyViewHeapTracking( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    //
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    //
    return parent;
    }


CMemSpyViewBase* CMemSpyViewHeapTrackingResults::PrepareChildViewL()
    {
    const TInt index = iListBox->CurrentItemIndex();
    //
    CMemSpyViewBase* child = NULL;
    if  ( index >= 0 )
        {
        const CMemSpyEngineHelperSysMemTrackerCycle* cycle = iEngine.HelperSysMemTracker().CompletedCycles()[ index ];
        //
        child = new(ELeave) CMemSpyViewHeapTrackingResultsCycleInfo( iEngine, iObserver, *cycle );
        CleanupStack::PushL( child );
        child->ConstructL( Rect(), *Parent() );
        CleanupStack::Pop( child );
        //
        iEngine.HelperSysMemTracker().RemoveObserver( this );
        }
    //
    return child;
    }


void CMemSpyViewHeapTrackingResults::SetListBoxModelL()
    {
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    MDesCArray* contents = &iEngine.HelperSysMemTracker();
    listbox->Model()->SetItemTextArray( contents );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewHeapTrackingResults::HandleCyclesResetL()
    {
    CMemSpyViewBase::RefreshL();
    }


void CMemSpyViewHeapTrackingResults::HandleCycleStartedL( const CMemSpyEngineHelperSysMemTrackerCycle& /*aCycle*/ )
    {
    }


void CMemSpyViewHeapTrackingResults::HandleCycleFinishedL( const CMemSpyEngineHelperSysMemTrackerCycle& /*aCycle*/ )
    {
    SetListBoxCurrentItemIndexL( iEngine.HelperSysMemTracker().MdcaCount() - 1 );
    CMemSpyViewBase::RefreshL();
    }
































CMemSpyViewHeapTrackingResultsCycleInfo::CMemSpyViewHeapTrackingResultsCycleInfo( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, const CMemSpyEngineHelperSysMemTrackerCycle& aCycle )
:   CMemSpyViewBase( aEngine, aObserver ), iCycle( aCycle )
    {
    iEngine.HelperSysMemTracker().SetObserver( this );
    }


CMemSpyViewHeapTrackingResultsCycleInfo::~CMemSpyViewHeapTrackingResultsCycleInfo()
    {
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    }


void CMemSpyViewHeapTrackingResultsCycleInfo::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitleFormat, "Cycle Summary [%04d]" );
    TBuf<30> buf;
    buf.Format( KTitleFormat, iCycle.CycleNumber() );
    SetTitleL( buf );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );

    // Make sure the correct item is selected
    const TInt index = ((TInt) aSelectionRune);
    iListBox->SetCurrentItemIndex( index );
    HandleListBoxItemSelectedL( index );
    }


void CMemSpyViewHeapTrackingResultsCycleInfo::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewHeapTrackingResultsCycleInfo::ViewType() const
    {
    return EMemSpyViewTypeHeapTrackingResultsCycleInfo;
    }


CMemSpyViewBase* CMemSpyViewHeapTrackingResultsCycleInfo::PrepareParentViewL()
    {
    CMemSpyViewHeapTrackingResults* parent = new(ELeave) CMemSpyViewHeapTrackingResults( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) iCycle.CycleNumber() );
    CleanupStack::Pop( parent );
    //
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    //
    return parent;
    }


void CMemSpyViewHeapTrackingResultsCycleInfo::SetListBoxModelL()
    {
    MDesCArray& model = const_cast< MDesCArray& >( static_cast< const MDesCArray&>( iCycle ) );
    //
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( &model );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewHeapTrackingResultsCycleInfo::HandleCyclesResetL()
    {
    Container().NavigateToParentViewL();
    }


void CMemSpyViewHeapTrackingResultsCycleInfo::HandleCycleStartedL( const CMemSpyEngineHelperSysMemTrackerCycle& /*aCycle*/ )
    {
    }


void CMemSpyViewHeapTrackingResultsCycleInfo::HandleCycleFinishedL( const CMemSpyEngineHelperSysMemTrackerCycle& /*aCycle*/ )
    {
    }








































/*



CMemSpyViewHeapTrackingResultsChangeDescriptor::CMemSpyViewHeapTrackingResultsChangeDescriptor( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, const CMemSpyEngineHelperSysMemTrackerCycle& aCycle, const CMemSpyEngineHelperSysMemTrackerCycleChange& aChangeDescriptor, TInt aIndex )
:   CMemSpyViewBase( aEngine, aObserver ), iCycle( aCycle ), iChangeDescriptor( aChangeDescriptor ), iIndex( aIndex )
    {
    iEngine.HelperSysMemTracker().SetObserver( this );
    }


CMemSpyViewHeapTrackingResultsChangeDescriptor::~CMemSpyViewHeapTrackingResultsChangeDescriptor()
    {
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    }


void CMemSpyViewHeapTrackingResultsChangeDescriptor::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    TBuf<30> buf;
    if  ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeHeapUser )
        {
        _LIT( KTitleFormat, "User Heap Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeHeapKernel )
        {
        _LIT( KTitleFormat, "Kernel Heap Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeChunk )
        {
        _LIT( KTitleFormat, "Chunk Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeRamDrive )
        {
        _LIT( KTitleFormat, "RAM Drive Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeStack )
        {
        _LIT( KTitleFormat, "Stack Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeGlobalData )
        {
        _LIT( KTitleFormat, "Global Data Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeCode )
        {
        _LIT( KTitleFormat, "RAM-Loaded Code Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeHandle )
        {
        _LIT( KTitleFormat, "Handle Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeOpenFile )
        {
        _LIT( KTitleFormat, "Open File Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeDiskSpace )
        {
        _LIT( KTitleFormat, "Disk Space Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    else if ( iChangeDescriptor.Type() == EMemSpyEngineSysMemTrackerTypeBitmap )
        {
        _LIT( KTitleFormat, "Bitmap Change [%04d]" );
        buf.Format( KTitleFormat, iCycle.CycleNumber() );
        }
    //
    SetTitleL( buf );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    }


void CMemSpyViewHeapTrackingResultsChangeDescriptor::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewHeapTrackingResultsChangeDescriptor::ViewType() const
    {
    return EMemSpyViewTypeHeapTrackingResultsCycleChangeDescriptor;
    }


CMemSpyViewBase* CMemSpyViewHeapTrackingResultsChangeDescriptor::PrepareParentViewL()
    {
    CMemSpyViewHeapTrackingResultsCycleInfo* parent = new(ELeave) CMemSpyViewHeapTrackingResultsCycleInfo( iEngine, iObserver, iCycle );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) iIndex );
    CleanupStack::Pop( parent );
    //
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    //
    return parent;
    }


void CMemSpyViewHeapTrackingResultsChangeDescriptor::SetListBoxModelL()
    {
    MDesCArray& model = const_cast< MDesCArray& >( static_cast< const MDesCArray&>( iChangeDescriptor ) );
    //
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( &model );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewHeapTrackingResultsChangeDescriptor::HandleCyclesResetL()
    {
    CMemSpyViewHeapTrackingResults* mainResultsMenu = new(ELeave) CMemSpyViewHeapTrackingResults( iEngine, iObserver );
    CleanupStack::PushL( mainResultsMenu );
    mainResultsMenu->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( mainResultsMenu );
    //
    Container().SetNewActiveViewL( mainResultsMenu );
    }


void CMemSpyViewHeapTrackingResultsChangeDescriptor::HandleCycleStartedL( const CMemSpyEngineHelperSysMemTrackerCycle& aCycle )
    {
    }


void CMemSpyViewHeapTrackingResultsChangeDescriptor::HandleCycleFinishedL( const CMemSpyEngineHelperSysMemTrackerCycle& aCycle )
    {
    }
*/

