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

#include "MemSpyViewThreadInfoItemHeap.h"

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperprocess.h>
#include <memspy/engine/memspyenginehelperheap.h>

// User includes
#include "MemSpyContainerObserver.h"
#include "MemSpyViewThreads.h"
#include "MemSpyViewThreadInfoItemHeap.h"



CMemSpyViewThreadInfoItemHeap::CMemSpyViewThreadInfoItemHeap( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, CMemSpyThreadInfoContainer& aContainer )
:   CMemSpyViewThreadInfoItemGeneric( aEngine, aObserver, aContainer, EMemSpyThreadInfoItemTypeHeap )
    {
    }


TBool CMemSpyViewThreadInfoItemHeap::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
#ifndef __WINS__
	case EMemSpyCmdHeapDataDump:
        OnCmdHeapDataL();
        break;
#endif
	case EMemSpyCmdHeapCellListing:
        OnCmdHeapCellListingL();
        break;
    case EMemSpyCmdHeapInfoThread:
        OnCmdHeapInfoL();
        break;

    default:
        handled = CMemSpyViewBase::HandleCommandL( aCommand );
        break;
        }
    //
    return handled;
    }


void CMemSpyViewThreadInfoItemHeap::OnCmdHeapDataL()
    {
    iEngine.HelperHeap().OutputHeapDataUserL( Thread() );
    }


void CMemSpyViewThreadInfoItemHeap::OnCmdHeapCellListingL()
    {
    iEngine.HelperHeap().OutputCellListingUserL( Thread() );
    }


void CMemSpyViewThreadInfoItemHeap::OnCmdHeapInfoL()
    {
    iEngine.HelperHeap().OutputHeapInfoUserL( Thread() );
    }


void CMemSpyViewThreadInfoItemHeap::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if  ( aResourceId == R_MEMSPY_MENUPANE )
        {
        aMenuPane->SetItemDimmed( EMemSpyCmdHeap, Thread().IsDead() );
        }
    }
