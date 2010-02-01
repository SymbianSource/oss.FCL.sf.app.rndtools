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

#include "MemSpyViewHeapTracking.h"

// System includes
#include <hal.h>

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelpersysmemtracker.h>

// User includes
#include "MemSpyUiUtils.h"
#include "MemSpyViewMainMenu.h"
#include "MemSpyContainerObserver.h"
#include "MemSpyViewHeapTrackingSettings.h"
#include "MemSpyViewHeapTrackingResults.h"


CMemSpyViewHeapTracking::CMemSpyViewHeapTracking( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver )
:   CMemSpyViewBase( aEngine, aObserver ),
    iState( EMemSpyViewHeapTrackingStateIdle )
    {
    if ( iEngine.HelperSysMemTracker().IsActive() )
        {
        iState = EMemSpyViewHeapTrackingStateTimerOn;
        }
    }


CMemSpyViewHeapTracking::~CMemSpyViewHeapTracking()
    {
    delete iStopTimerCallBack;
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    }


void CMemSpyViewHeapTracking::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "System-Wide Memory Tracking" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    
    // Backup current config because it may be overwritten with Basic/Full mode settings.
    CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
    TMemSpyEngineHelperSysMemTrackerConfig config;
    swmt.GetConfig( config );
    iOriginalConfig = config;
    
    if ( config.iMode != TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeCustom )
        {
        // Set config. Needed for updating config for Basic or Full mode.
        SetConfigByModeL( config.iMode, config );        
        }
        
    // Make sure the correct item is selected
    TInt index = 0;
    if  ( aSelectionRune != NULL )
        {
        const TMemSpyViewType viewType = (TMemSpyViewType) ((TInt) aSelectionRune);
        index = IndexByViewType( viewType );
        }
    iListBox->SetCurrentItemIndex( index );
    HandleListBoxItemSelectedL( index );
    }


void CMemSpyViewHeapTracking::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewHeapTracking::ViewType() const
    {
    return EMemSpyViewTypeHeapTracking;
    }


CMemSpyViewBase* CMemSpyViewHeapTracking::PrepareParentViewL()
    {
    
    // Save custom settings even if mode is Basic or Full
    TRAP_IGNORE(
        CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
        TMemSpyEngineHelperSysMemTrackerConfig config;
        swmt.GetConfig( config );
        config.iEnabledCategories = iOriginalConfig.iEnabledCategories;
        config.iThreadNameFilter = iOriginalConfig.iThreadNameFilter;
        config.iDumpData = iOriginalConfig.iDumpData;
        swmt.SetConfigL( config );
        Settings().StoreSettingsL();
        );
    
    CMemSpyViewMainMenu* parent = new(ELeave) CMemSpyViewMainMenu( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewHeapTracking::PrepareChildViewL()
    {
    CMemSpyViewBase* child = NULL;
    const TInt index = iListBox->CurrentItemIndex();
    
    // Get current config
    CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
    TMemSpyEngineHelperSysMemTrackerConfig config;
    swmt.GetConfig( config );
    
    //
    if  ( index == 0 )
        {
        // This is the start/stop toggle so we don't make a child view
        child = NULL; 

        // ... instead we either start or stop the tracker
        if  ( !iEngine.HelperSysMemTracker().IsActive() )
            {
            iState = EMemSpyViewHeapTrackingStateSingleOn;
            // Setting observer to be able to stop SWMT after first cycle is completed
            iEngine.HelperSysMemTracker().SetObserver( this );
            iEngine.HelperSysMemTracker().StartL();
            }

        // Redraw listbox 
        RefreshL();
        }
    else if  ( index == 1 )
        {
        // This is the start/stop toggle so we don't make a child view
        child = NULL; 

        // ... instead we either start or stop the tracker
        if  ( iEngine.HelperSysMemTracker().IsActive() && iState == EMemSpyViewHeapTrackingStateTimerOn )
            {
            iState = EMemSpyViewHeapTrackingStateIdle;
            iEngine.HelperSysMemTracker().StopL();
            }
        else if ( iState == EMemSpyViewHeapTrackingStateIdle )
            {
            iState = EMemSpyViewHeapTrackingStateTimerOn;
            iEngine.HelperSysMemTracker().StartL();
            }

        // Redraw listbox 
        RefreshL();
        }
    else if ( index == 2 )
        {
        switch ( config.iMode )
            {
            case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeBasic:
                {
                // Set Full mode
                SetConfigByModeL( TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeFull, config );
                break;
                }
            case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeFull:
                {
                // Set Custom mode
                SetConfigByModeL( TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeCustom, config );
                break;
                }
            case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeCustom:
                {
                // Set Basic mode
                SetConfigByModeL( TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeBasic, config );
                break;
                }
            default: break;
            }
                
        // Redraw listbox 
        RefreshL();
        }
    else if ( index == 3 )
        {        
        if ( config.iMode == TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeCustom )
            {
            child = new(ELeave) CMemSpyViewHeapTrackingSettings( iEngine, iObserver );
            }
        else
            {
            child = new(ELeave) CMemSpyViewHeapTrackingResults( iEngine, iObserver );
            }
        }
    else if ( index == 4 )
        {
        child = new(ELeave) CMemSpyViewHeapTrackingResults( iEngine, iObserver );
        }
    if  ( child )
        {
        CleanupStack::PushL( child );
        child->ConstructL( Rect(), *Parent() );
        CleanupStack::Pop( child );
        }
    
    //
    return child;
    }


void CMemSpyViewHeapTracking::SetListBoxModelL()
    {
    CDesCArrayFlat* model = new(ELeave) CDesCArrayFlat(5);
    CleanupStack::PushL( model );

    TBuf<KMaxFullName + 1> item;
    
    // Get current config
    CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
    TMemSpyEngineHelperSysMemTrackerConfig config;
    swmt.GetConfig( config );

    // 1st item
    _LIT( KItem1FormatEnable, "\tGet dump now" );
    model->AppendL( KItem1FormatEnable );

    
    // 1st item
    if  ( iEngine.HelperSysMemTracker().IsActive() && iState == EMemSpyViewHeapTrackingStateTimerOn )
        {
        _LIT( KItem1FormatEnable, "\tStop timer\t\t%d (sec)" );
        TName item;
        item.Format( KItem1FormatEnable, config.iTimerPeriod.Int() / 1000000 );
        model->AppendL( item );
        }
    else
        {
        _LIT( KItem1FormatEnable, "\tStart timer\t\t%d (sec)" );
        TName item;
        item.Format( KItem1FormatEnable, config.iTimerPeriod.Int() / 1000000 );
        model->AppendL( item );
        }
    
    // 2nd item
    switch ( config.iMode )
        {
        case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeBasic:
            {
            _LIT( KItem2Format, "\tTracking mode\t\tBasic" );
            model->AppendL( KItem2Format );        
            break;
            }
        case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeFull:
            {
            _LIT( KItem2Format, "\tTracking mode\t\tFull" );
            model->AppendL( KItem2Format );        
            break;
            }
        case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeCustom:
            {
            _LIT( KItem2Format, "\tTracking mode\t\tCustom" );
            model->AppendL( KItem2Format );
            
            // 3rd item
            TFullName item;
            if ( config.DumpData() && 
                config.iEnabledCategories & TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryUserHeap )
                {
                _LIT( KItem3Format, "\tSettings\t\t%d categ., dump" );
                item.Format( KItem3Format, config.EnabledCategoryCount() );
                }
            else
                {
                _LIT( KItem3Format, "\tSettings\t\t%d categories" );
                item.Format( KItem3Format, config.EnabledCategoryCount() );
                }
            model->AppendL( item );            
            break;
            }
        default: break;
        }
    
    // 4th item    
    const TInt cycleCount = iEngine.HelperSysMemTracker().MdcaCount();
    if ( cycleCount )
        {
        _LIT( KItem2Format, "\tResults\t\t%d cycles" );
        TFullName item;
        item.Format( KItem2Format, cycleCount );
        model->AppendL( item );
        }
    else
        {
        _LIT( KItem2Format, "\tResults\t\tNo results" );
        model->AppendL( KItem2Format );
        }

    // Set up list box
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( model );
    listbox->Model()->SetOwnershipType( ELbmOwnsItemArray );
    CleanupStack::Pop( model );
    }


void CMemSpyViewHeapTracking::HandleCyclesResetL()
    {
    }


void CMemSpyViewHeapTracking::HandleCycleStartedL( const CMemSpyEngineHelperSysMemTrackerCycle& /*aCycle*/ )
    {
    }


void CMemSpyViewHeapTracking::HandleCycleFinishedL( const CMemSpyEngineHelperSysMemTrackerCycle& /*aCycle*/ )
    {
    // Stopping SWMT does not work directly from this function.
    // It has to be made asynchronously.
    iStopTimerCallBack = new (ELeave) CAsyncCallBack( TCallBack( CMemSpyViewHeapTracking::AsyncStopTimerCallback, this ), CActive::EPriorityStandard );
    iStopTimerCallBack->CallBack();
    }


TInt CMemSpyViewHeapTracking::IndexByViewType( TMemSpyViewType aType )
    {
    // Get current config
    CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
    TMemSpyEngineHelperSysMemTrackerConfig config;
    swmt.GetConfig( config );
    
    TInt index = 0;
    //
    switch( aType )
        {
    default:
    case EMemSpyViewTypeHeapTrackingResults:
        {
        if ( config.iMode == TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeCustom )
            {
            index = 4;
            }
        else
            {
            index = 3;
            }
        break;
        }
    case EMemSpyViewTypeHeapTrackingSettings:
        index = 3;
        break;
        }
    //
    return index;
    }


void CMemSpyViewHeapTracking::SetConfigByModeL( TMemSpyEngineHelperSysMemTrackerConfig::TMemSpyEngineSysMemTrackerMode aMode, TMemSpyEngineHelperSysMemTrackerConfig& aConfig )
    {
    switch ( aMode )
        {
        case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeBasic:
            {
            aConfig.iMode = aMode;
            aConfig.iEnabledCategories = TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryUserHeap |
                                         TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryUserStacks |
                                         TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryGlobalData |
                                         TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategorySystemMemory;
            aConfig.iDumpData = EFalse;
            break;
            }
        case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeFull:
            {
            aConfig.iMode = aMode;
            aConfig.iEnabledCategories = TMemSpyEngineHelperSysMemTrackerConfig::EMemSpyEngineSysMemTrackerCategoryAll;
            aConfig.iDumpData = ETrue;
            aConfig.iThreadNameFilter = KNullDesC;
            break;
            }
        case TMemSpyEngineHelperSysMemTrackerConfig::MemSpyEngineSysMemTrackerModeCustom:
            {
            aConfig.iMode = aMode;
            aConfig.iEnabledCategories = iOriginalConfig.iEnabledCategories;
            aConfig.iDumpData = iOriginalConfig.iDumpData;
            aConfig.iThreadNameFilter = iOriginalConfig.iThreadNameFilter;
            break;
            }
        default: User::Leave( KErrArgument );
        }
    // Push changes to SWMT
    CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
    swmt.SetConfigL( aConfig );
    Settings().StoreSettingsL();
    }


TInt CMemSpyViewHeapTracking::AsyncStopTimerCallback( TAny* aParam )
    {
    CMemSpyViewHeapTracking* view = static_cast<CMemSpyViewHeapTracking*>( aParam );
    return view->AsyncStopTimerCallback();
    }


TInt CMemSpyViewHeapTracking::AsyncStopTimerCallback()
    {
    iState = EMemSpyViewHeapTrackingStateIdle;
    iEngine.HelperSysMemTracker().RemoveObserver( this );
    TRAP_IGNORE(
        iEngine.HelperSysMemTracker().StopL();
        RefreshL();
        );
    delete iStopTimerCallBack;
    iStopTimerCallBack = NULL;
    return KErrNone;
    }



