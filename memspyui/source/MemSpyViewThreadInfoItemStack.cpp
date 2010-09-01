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

#include "MemSpyViewThreadInfoItemStack.h"

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperprocess.h>
#include <memspy/engine/memspyenginehelperstack.h>

// User includes
#include "MemSpyContainerObserver.h"
#include "MemSpyViewThreads.h"
#include "MemSpyViewThreadInfoItemHeap.h"



CMemSpyViewThreadInfoItemStack::CMemSpyViewThreadInfoItemStack( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver, CMemSpyThreadInfoContainer& aContainer )
:   CMemSpyViewThreadInfoItemGeneric( aEngine, aObserver, aContainer, EMemSpyThreadInfoItemTypeStack )
    {
    }


TBool CMemSpyViewThreadInfoItemStack::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
#ifndef __WINS__
	case EMemSpyCmdStackDataUser:
        OnCmdStackDataUserL();
		break;
	case EMemSpyCmdStackDataKernel:
        OnCmdStackDataKernelL();
		break;
#endif
	case EMemSpyCmdStackInfoThread:
        OnCmdStackInfoL();
		break;

    default:
        handled = CMemSpyViewBase::HandleCommandL( aCommand );
        break;
        }
    //
    return handled;
    }


void CMemSpyViewThreadInfoItemStack::OnCmdStackInfoL()
    {
    iEngine.HelperStack().OutputStackInfoL( Thread() );
    }


void CMemSpyViewThreadInfoItemStack::OnCmdStackDataUserL()
    {
    iEngine.HelperStack().OutputStackDataL( Thread(), EMemSpyDriverDomainUser );
    }


void CMemSpyViewThreadInfoItemStack::OnCmdStackDataKernelL()
    {
    iEngine.HelperStack().OutputStackDataL( Thread(), EMemSpyDriverDomainKernel );
    }    


void CMemSpyViewThreadInfoItemStack::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if  ( aResourceId == R_MEMSPY_MENUPANE )
        {
        aMenuPane->SetItemDimmed( EMemSpyCmdStack, Thread().IsDead() );
        }
    }
