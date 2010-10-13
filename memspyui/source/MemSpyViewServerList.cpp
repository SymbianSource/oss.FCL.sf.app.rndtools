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

#include "MemSpyViewServerList.h"

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperprocess.h>
#include <memspy/engine/memspyenginehelperheap.h>
#include <memspy/engine/memspyenginehelperserver.h>

// User includes
#include "MemSpyUiUtils.h"
#include "MemSpyViewMainMenu.h"
#include "MemSpyViewThreads.h"
#include "MemSpyContainerObserver.h"




CMemSpyViewServerList::CMemSpyViewServerList( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver )
:   CMemSpyViewBase( aEngine, aObserver )
    {
    }


CMemSpyViewServerList::~CMemSpyViewServerList()
    {
    delete iList;
    }


void CMemSpyViewServerList::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Running Servers" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    }


TBool CMemSpyViewServerList::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
    case EMemSpyCmdServerListSortByName:
        OnCmdServerListSortByNameL();
        break;
    case EMemSpyCmdServerListSortBySessionCount:
        OnCmdServerListSortBySessionCountL();
        break;
    case EMemSpyCmdServerListOutputListCSV:
        OnCmdServerListOutputSummaryL();
        break;
    case EMemSpyCmdServerListOutputListDetailed:
        OnCmdServerListOutputDetailedL();
        break;

    default:
        handled = CMemSpyViewBase::HandleCommandL( aCommand );
        break;
        }
    //
    return handled;
    }


void CMemSpyViewServerList::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewServerList::ViewType() const
    {
    return EMemSpyViewTypeServerList;
    }


CMemSpyViewBase* CMemSpyViewServerList::PrepareParentViewL()
    {
    CMemSpyViewMainMenu* parent = new(ELeave) CMemSpyViewMainMenu( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewServerList::PrepareChildViewL()
    {
    CMemSpyViewBase* child = NULL;

    // First, try to find the selected thread
    if  ( iActionedItem )
        {
        // Try to create a view of the thread in question
        CMemSpyProcess* process = NULL;
        CMemSpyThread* thread = NULL;
        //
        const TInt error = iEngine.Container().ProcessAndThreadByThreadId( iActionedItem->Id(), process, thread );
        //
        if  ( error == KErrNone && thread != NULL )
            {
            child = new(ELeave) CMemSpyViewThreads( iEngine, iObserver, thread->Process() );
            CleanupStack::PushL( child );
            child->ConstructL( Rect(), *Parent(), thread );
            CleanupStack::Pop( child );
            }
        }
    //
    return child;
    }


void CMemSpyViewServerList::SetListBoxModelL()
    {
    delete iList;
    iList = NULL;
    iList = iEngine.HelperServer().ServerListL();
    //
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( iList );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


void CMemSpyViewServerList::HandleListBoxItemActionedL( TInt aCurrentIndex )
    {
    if  ( aCurrentIndex >= 0 && aCurrentIndex < iList->Count() )
        {
        const CMemSpyEngineServerEntry& serverInfo = iList->At( aCurrentIndex );
        iActionedItem = &serverInfo;
        }
    else
        {
        iActionedItem = NULL;
        }

    // Notify observer about an item being 'fired'
    ReportEventL( MMemSpyViewObserver::EEventItemActioned );
    }


void CMemSpyViewServerList::OnCmdServerListSortByNameL()
    {
    iList->SortByNameL();
    CMemSpyViewBase::RefreshL();
    }


void CMemSpyViewServerList::OnCmdServerListSortBySessionCountL()
    {
    iList->SortBySessionCountL();
    CMemSpyViewBase::RefreshL();
    }


void CMemSpyViewServerList::OnCmdServerListOutputSummaryL()
    {
    OnCmdServerListOutputGenericL( EFalse );
    }


void CMemSpyViewServerList::OnCmdServerListOutputDetailedL()
    {
    OnCmdServerListOutputGenericL( ETrue );
    }


void CMemSpyViewServerList::OnCmdServerListOutputGenericL( TBool aDetailed )
    {
    // Begin a new data stream
    _LIT( KMemSpyContext, "Server List - " );
    _LIT( KMemSpyFolder, "Servers" );
    iEngine.Sink().DataStreamBeginL( KMemSpyContext, KMemSpyFolder );

    // Set prefix for overall listing
    iEngine.Sink().OutputPrefixSetLC( KMemSpyContext );

    // Create header
    CMemSpyEngineServerList::OutputDataColumnsL( iEngine, aDetailed );
    
    // List items
    const TInt count = iList->Count();
    for(TInt i=0; i<count; i++)
        {
        const CMemSpyEngineServerEntry& server = iList->At( i );
        //
        server.OutputDataL( iEngine.HelperServer(), aDetailed );
        }

    // Tidy up
    CleanupStack::PopAndDestroy(); // prefix
 
    // End data stream
    iEngine.Sink().DataStreamEndL();
    }







