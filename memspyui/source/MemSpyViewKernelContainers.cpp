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

#include "MemSpyViewKernelContainers.h"

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyengineobjectprocess.h>
#include <memspy/engine/memspyengineobjectthread.h>
#include <memspy/engine/memspyengineobjectcontainer.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>
#include <memspy/engine/memspyengineobjectthreadinfocontainer.h>
#include <memspy/engine/memspyenginehelperkernelcontainers.h>

// User includes
#include "MemSpyUiUtils.h"
#include "MemSpyViewKernel.h"
#include "MemSpyContainerObserver.h"
#include "MemSpyViewKernelObjects.h"

// Literal constants



CMemSpyViewKernelContainers::CMemSpyViewKernelContainers( CMemSpyEngine& aEngine, MMemSpyViewObserver& aObserver )
:   CMemSpyViewBase( aEngine, aObserver )
    {
    }


CMemSpyViewKernelContainers::~CMemSpyViewKernelContainers()
    {
    delete iModel;
    }


void CMemSpyViewKernelContainers::ConstructL( const TRect& aRect, CCoeControl& aContainer, TAny* aSelectionRune )
    {
    _LIT( KTitle, "Kernel Objects" );
    SetTitleL( KTitle );
    //
    CMemSpyViewBase::ConstructL( aRect, aContainer, aSelectionRune );
    }


void CMemSpyViewKernelContainers::RefreshL()
    {
    SetListBoxModelL();
    CMemSpyViewBase::RefreshL();
    }


TMemSpyViewType CMemSpyViewKernelContainers::ViewType() const
    {
    return EMemSpyViewTypeKernelContainers;
    }


CMemSpyViewBase* CMemSpyViewKernelContainers::PrepareParentViewL()
    {
    CMemSpyViewKernel* parent = new(ELeave) CMemSpyViewKernel( iEngine, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewKernelContainers::PrepareChildViewL()
    {
    CMemSpyViewBase* child = NULL;
    const TInt index = iListBox->CurrentItemIndex();
    child = new(ELeave) CMemSpyViewKernelObjects( iEngine, iObserver, iModel->At( index ).Type() );
    CleanupStack::PushL( child );
    child->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( child );
    return child;
    }


void CMemSpyViewKernelContainers::SetListBoxModelL()
    {
    // Take ownership of new model
    CMemSpyEngineHelperKernelContainers& kernelContainerManager = iEngine.HelperKernelContainers();
    CMemSpyEngineGenericKernelObjectContainer* model = kernelContainerManager.ObjectsAllL();
    delete iModel;
    iModel = model;
    
    // Set up list box
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( model );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    }


TBool CMemSpyViewKernelContainers::HandleCommandL( TInt aCommand )
    {
    TBool handled = ETrue;
    //
    switch ( aCommand )
        {
    case EMemSpyCmdKernelContainersOutput:
        OnCmdOutputAllContainerContentsL();
        break;

    default:
        handled = CMemSpyViewBase::HandleCommandL( aCommand );
        break;
        }
    //
    return handled;
    }


void CMemSpyViewKernelContainers::OnCmdOutputAllContainerContentsL()
    {
    CMemSpyEngineOutputSink& sink = iEngine.Sink();
    iModel->OutputL( sink );
    }




