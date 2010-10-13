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

#include "MemSpyViewProcesses.h"

// System includes
#include <aknsfld.h>
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
#include "MemSpyViewThreads.h"
#include "MemSpyViewMainMenu.h"
#include "MemSpyContainerObserver.h"

// Constants
const TInt KMemSpyMaxSearchTextLength = 30;
const TInt KMemSpyMaxSearchTextLengthWithWildcards = KMemSpyMaxSearchTextLength + 4;



CMemSpyViewProcesses::CMemSpyViewProcesses( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver )
:   CMemSpyViewBase( aEngine, aObserver )
    {
    }


CMemSpyViewProcesses::CMemSpyViewProcesses( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, CMemSpyProcess& aProcess )
:   CMemSpyViewBase( aEngine, aObserver ), iCurrentProcess( &aProcess )
    {
    iCurrentProcess->Open();
    }


CMemSpyViewProcesses::~CMemSpyViewProcesses()
    {
    if  ( iCurrentProcess )
        {
        iCurrentProcess->Close();
        }
    //
    delete iSearchField;
    delete iMatcherBuffer;
    }


void CMemSpyViewProcesses::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Processes\n& Threads" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    //
    iMatcherBuffer = HBufC::NewL( KMemSpyMaxSearchTextLengthWithWildcards );
    //
    iSearchField = CAknSearchField::NewL( *this, CAknSearchField::ESearch, NULL, KMemSpyMaxSearchTextLength );
    iSearchField->SetObserver( this );
    iSearchField->SetFocus( ETrue );
    iSearchField->SetComponentsToInheritVisibility( ETrue );
    //
    CMemSpyEngineObjectContainer& container = iEngine.Container();
    if  ( aSelectionRune )
        {
        CMemSpyProcess* selectedItem = reinterpret_cast< CMemSpyProcess* >( aSelectionRune );
        const TInt index = container.ProcessIndexById( selectedItem->Id() );
        if  ( index >= 0 && index < iListBox->Model()->NumberOfItems() )
            {
            iListBox->SetCurrentItemIndex( index );
            HandleListBoxItemSelectedL( index );
            }
        }
    else if ( container.Count() > 0 )
        {
        iListBox->SetCurrentItemIndex( 0 );
        HandleListBoxItemSelectedL( 0 );
        }
    //
    SizeChanged();
    ActivateL();
    }


CMemSpyProcess& CMemSpyViewProcesses::CurrentProcess() const
    {
    __ASSERT_ALWAYS( iCurrentProcess != NULL, User::Invariant() );
    return *iCurrentProcess;
    }


void CMemSpyViewProcesses::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewProcesses::ViewType() const
    {
    return EMemSpyViewTypeProcesses;
    }


CMemSpyViewBase* CMemSpyViewProcesses::PrepareParentViewL()
    {
    CMemSpyViewMainMenu* parent = new(ELeave) CMemSpyViewMainMenu( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewProcesses::PrepareChildViewL()
    {
    CMemSpyViewThreads* child = new(ELeave) CMemSpyViewThreads( iEngine, iObserver, *iCurrentProcess );
    CleanupStack::PushL( child );
    child->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( child );
    return child;
    }


void CMemSpyViewProcesses::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if  ( aResourceId == MenuCascadeResourceId() )
        {
        CMemSpyProcess& process = CurrentProcess();
        const TBool hide = ( iEngine.Container().Count() == 0 ) || process.IsDead();
        //
        aMenuPane->SetItemDimmed( EMemSpyCmdProcessInfo, hide );
        aMenuPane->SetItemDimmed( EMemSpyCmdProcessEnd, hide );
        }
    }


TBool CMemSpyViewProcesses::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
    // Sorting
    case EMemSpyCmdProcessSortById:
        OnCmdSortByIdL();
		break;
    case EMemSpyCmdProcessSortByName:
        OnCmdSortByNameL();
		break;
    case EMemSpyCmdProcessSortByThreadCount:
        OnCmdSortByThreadCountL();
		break;
    case EMemSpyCmdProcessSortByCodeSegs:
        OnCmdSortByCodeSegsL();
		break;
    case EMemSpyCmdProcessSortByHeapUsage:
        OnCmdSortByHeapUsageL();
		break;
    case EMemSpyCmdProcessSortByStackUsage:
        OnCmdSortByStackUsageL();
		break;
    // End
    case EMemSpyCmdProcessEndTerminate:
        OnCmdEndTerminateL();
		break;
    case EMemSpyCmdProcessEndKill:
        OnCmdEndKillL();
		break;
    case EMemSpyCmdProcessEndPanic:
        OnCmdEndPanicL();
		break;
    // Info
    case EMemSpyCmdProcessInfoSummary:
        OnCmdInfoSummaryL();
		break;
    case EMemSpyCmdProcessInfoHandles:
        OnCmdInfoHandlesL();
        break;

    default:
        handled = CMemSpyViewBase::HandleCommandL( aCommand );
        break;
        }
    //
    return handled;
    }


void CMemSpyViewProcesses::OnCmdSortByIdL()
    {
    iEngine.Container().SortById();
    RefreshL();
    }


void CMemSpyViewProcesses::OnCmdSortByNameL()
    {
    iEngine.Container().SortByName();
    RefreshL();
    }


void CMemSpyViewProcesses::OnCmdSortByThreadCountL()
    {
    iEngine.Container().SortByThreadCount();
    RefreshL();
    }


void CMemSpyViewProcesses::OnCmdSortByCodeSegsL()
    {
    iEngine.Container().SortByCodeSegs();
    RefreshL();
    }


void CMemSpyViewProcesses::OnCmdSortByHeapUsageL()
    {
    iEngine.Container().SortByHeapUsage();
    RefreshL();
    }


void CMemSpyViewProcesses::OnCmdSortByStackUsageL()
    {
    iEngine.Container().SortByStackUsage();
    RefreshL();
    }


void CMemSpyViewProcesses::OnCmdInfoSummaryL()
    {
    CMemSpyProcess& process = CurrentProcess();
    iEngine.HelperProcess().OutputProcessInfoL( process );
    }


void CMemSpyViewProcesses::OnCmdInfoHandlesL()
    {
    CMemSpyProcess& process = CurrentProcess();
    //
    const TInt threadCount = process.Count();
    for( TInt i=0; i<threadCount; i++ )
        {
        CMemSpyThread& thread = process.At( i );
        thread.InfoContainerForceSyncronousConstructionL().PrintL();
        }
    }


void CMemSpyViewProcesses::OnCmdEndTerminateL()
    {
    TBool doTerminate = ETrue;
    CMemSpyProcess& process = CurrentProcess();
    //
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
    }


void CMemSpyViewProcesses::OnCmdEndPanicL()
    {
    TBool doTerminate = ETrue;
    CMemSpyProcess& process = CurrentProcess();
    //
    if  ( process.IsSystemPermanent() || process.IsSystemCritical() )
        {
        CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
        doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
        }
    //
    if  ( doTerminate )
        {
        process.PanicL();
        RefreshL();
        }
    }


void CMemSpyViewProcesses::OnCmdEndKillL()
    {
    TBool doTerminate = ETrue;
    CMemSpyProcess& process = CurrentProcess();
    //
    if  ( process.IsSystemPermanent() || process.IsSystemCritical() )
        {
        CAknQueryDialog* importDialog = CAknQueryDialog::NewL();
        doTerminate = ( importDialog->ExecuteLD( R_MEMSPY_PANIC_SYSTEM_CRITICAL_THREAD_OR_PROCESS ) );
        }
    //
    if  ( doTerminate )
        {
        process.KillL();
        RefreshL();
        }
    }




void CMemSpyViewProcesses::SetListBoxModelL()
    {
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( &iEngine.Container() );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewProcesses::HandleListBoxItemActionedL( TInt /*aIndex*/ )
    {
    // Notify observer about an item being 'fired'
    ReportEventL( MMemSpyViewObserver::EEventItemActioned );
    }


void CMemSpyViewProcesses::HandleListBoxItemSelectedL( TInt aIndex )
    {
    if  ( iCurrentProcess )
        {
        iCurrentProcess->Close();
        }

    // Obtain the process that corresponds to the selected item
    CMemSpyEngineObjectContainer& container = iEngine.Container();
    CMemSpyProcess& process = container.At( aIndex );
    iCurrentProcess = &process;
    iCurrentProcess->Open();

    // Notify observer about item selection
    ReportEventL( MMemSpyViewObserver::EEventItemSelected );
    }


void CMemSpyViewProcesses::SizeChanged()
    {
    const TRect rect( Rect() );
    
    if  ( iListBox && iSearchField )
        {
        const TInt lafIndex_H = 1;
        AknLayoutUtils::LayoutControl( iListBox, rect, AknLayout::list_gen_pane( lafIndex_H ) );
        AknLayoutUtils::LayoutControl( iSearchField, rect, AknLayout::find_pane() );
        }
    else
        {
        CMemSpyViewBase::SizeChanged();
        }

/*
    // Search field
    TSize searchFieldSize( iSearchField->MinimumSize() ); // BALLS: search field doesn't implement minimum size correctly?!?!?
    searchFieldSize.SetWidth( rect.Width() );
    const TRect searchFieldRect( TPoint( rect.iTl.iX, rect.iBr.iY ), searchFieldSize );
    iSearchField->SetRect( searchFieldRect );

    // Listbox
    TSize listBoxSize( rect.Size() - TSize( 0, searchFieldSize.iHeight ) );
    iListBox->SetRect( rect.iTl, listBoxSize );
*/
    }


TInt CMemSpyViewProcesses::CountComponentControls() const
    {
    TInt count = 0;
    //
    if  ( iListBox != NULL )
        {
        ++count;
        }
    if  ( iSearchField != NULL )
        {
        ++count;
        }
    //
    return count;
    }


CCoeControl* CMemSpyViewProcesses::ComponentControl( TInt aIndex ) const
    {
    CCoeControl* ret = iListBox;
    //
    if  ( aIndex == 1 )
        {
        ret = iSearchField;
        }
    //
    return ret;
    }


TKeyResponse CMemSpyViewProcesses::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse resp = EKeyWasNotConsumed;
    //
    if  ( aType == EEventKey )
        {
        switch( aKeyEvent.iCode )
            {
        case EKeyUpArrow:
        case EKeyDownArrow:
        case EKeyEnter:
        case EKeyOK:
            if  ( iListBox )
                {
                resp = iListBox->OfferKeyEventL( aKeyEvent, aType );
                }
            break;
        default:
            break;
            }
        //
        if  ( resp == EKeyWasNotConsumed )
            {
            // Do we need to show the find field?
            resp = iSearchField->OfferKeyEventL( aKeyEvent, aType );
            }
        }
    //
    return resp;
    }


void CMemSpyViewProcesses::HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType )
    {
    CMemSpyViewBase::HandleControlEventL( aControl, aEventType );
    //
    if  ( aEventType == MCoeControlObserver::EEventStateChanged )
        {
        if  ( aControl == iSearchField )
            {
            SelectListBoxItemByFindTextL();
            }
        }
    }


void CMemSpyViewProcesses::FocusChanged( TDrawNow /*aDrawNow*/ )
    {
    if  ( iListBox )
        {
        iListBox->SetFocus( IsFocused() );
        }
    if  ( iSearchField )
        {
        iSearchField->SetFocus( IsFocused() );
        }
    }


void CMemSpyViewProcesses::SelectListBoxItemByFindTextL()
    {
    _LIT( KMemSpyWildcardCharacter, "*" );
    
    TPtr pBuffer( iMatcherBuffer->Des() );
    iSearchField->GetSearchText( pBuffer );
    //pBuffer.Insert( 0, KMemSpyWildcardCharacter );
    pBuffer.Append( KMemSpyWildcardCharacter );
    
    CMemSpyEngineObjectContainer& container = iEngine.Container();
    const TInt count = container.Count();
    //
    TInt index = 0;
    while( index < count )
        {
        CMemSpyProcess& process = container.At( index );
        const TPtrC processName( process.Name() );
        //
        if  ( processName.MatchF( pBuffer ) >= 0 )
            {
            HandleListBoxItemSelectedL( index );
            iListBox->ScrollToMakeItemVisible( index );
            iListBox->SetCurrentItemIndexAndDraw( index );
            return;
            }
            
        ++index;
        }
    }


