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

#include "MemSpyViewKernelHeap.h"

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperheap.h>
#include <memspy/engine/memspyengineoutputlist.h>

// User includes
#include "MemSpyUiUtils.h"
#include "MemSpyViewKernel.h"
#include "MemSpyContainerObserver.h"

// Literal constants



CMemSpyViewKernelHeap::CMemSpyViewKernelHeap( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver )
:   CMemSpyViewBase( aEngine, aObserver )
    {
    }


CMemSpyViewKernelHeap::~CMemSpyViewKernelHeap()
    {
    }


void CMemSpyViewKernelHeap::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Kernel Heap" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    }


void CMemSpyViewKernelHeap::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewKernelHeap::ViewType() const
    {
    return EMemSpyViewTypeKernelHeap;
    }


CMemSpyViewBase* CMemSpyViewKernelHeap::PrepareParentViewL()
    {
    CMemSpyViewKernel* parent = new(ELeave) CMemSpyViewKernel( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewKernelHeap::PrepareChildViewL()
    {
    CMemSpyViewBase* child = NULL;
    return child;
    }


void CMemSpyViewKernelHeap::SetListBoxModelL()
    {
    // Get list contents
    TMemSpyHeapInfo heapInfo;
    iEngine.HelperHeap().GetHeapInfoKernelL( heapInfo );
    CMemSpyEngineOutputList* list = iEngine.HelperHeap().NewHeapSummaryShortLC( heapInfo );

    // Set up list box
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( list );
    listbox->Model()->SetOwnershipType( ELbmOwnsItemArray );
    CleanupStack::Pop( list );
    }


TBool CMemSpyViewKernelHeap::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
    case EMemSpyCmdKernelHeapDump:
        OnCmdDumpKernelHeapL();
        break;

    default:
        handled = CMemSpyViewBase::HandleCommandL( aCommand );
        break;
        }
    //
    return handled;
    }


void CMemSpyViewKernelHeap::OnCmdDumpKernelHeapL()
    {
    iEngine.HelperHeap().OutputHeapDataKernelL();
    }




