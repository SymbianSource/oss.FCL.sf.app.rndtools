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

#include <memspysession.h>
#include <memspy/api/memspyapiprocess.h>
#include <memspy/engine/memspyengineutils.h>

// User includes
#include "MemSpyUiUtils.h"
#include "MemSpyViewKernel.h"
#include "MemSpyContainerObserver.h"
#include "MemSpyViewKernelObjects.h"

// Literal constants



CMemSpyViewKernelContainers::CMemSpyViewKernelContainers( RMemSpySession& aSession, MMemSpyViewObserver& aObserver )
:   CMemSpyViewBase( aSession, aObserver )
    {
    }


CMemSpyViewKernelContainers::~CMemSpyViewKernelContainers()
    {
    //delete iModel;
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
    CMemSpyViewKernel* parent = new(ELeave) CMemSpyViewKernel( iMemSpySession, iObserver );
    CleanupStack::PushL( parent );
    parent->ConstructL( Rect(), *Parent(), (TAny*) ViewType() );
    CleanupStack::Pop( parent );
    return parent;
    }


CMemSpyViewBase* CMemSpyViewKernelContainers::PrepareChildViewL()
    {	
    CMemSpyViewBase* child = NULL;
    const TInt index = iListBox->CurrentItemIndex();
    //child = new(ELeave) CMemSpyViewKernelObjects( iEngine, iObserver, iModel->At( index ).Type() );
    child = new(ELeave) CMemSpyViewKernelObjects( iMemSpySession, iObserver, iKernelObjects[index]->Type() );
    CleanupStack::PushL( child );
    child->ConstructL( Rect(), *Parent() );
    CleanupStack::Pop( child );
    return child;   
    }


void CMemSpyViewKernelContainers::SetListBoxModelL()
    {	
	iMemSpySession.GetKernelObjects( iKernelObjects );
		
	CDesCArrayFlat* model = new (ELeave) CDesC16ArrayFlat( iKernelObjects.Count() ); //array for formated items
	
	for( TInt i=0 ; i<iKernelObjects.Count() ; i++ )
		{
		TInt count = iKernelObjects[i]->Count();
		TInt size = iKernelObjects[i]->Size();
		const TMemSpySizeText sizeText( MemSpyEngineUtils::FormatSizeText( size, 0 ) ); //TODO: is this OK to call Engine Utils?
								
		HBufC* tempName = HBufC::NewL( iKernelObjects[i]->Name().Length() + 32 ); //TODO: to removed this bulharic constant
		CleanupStack::PushL( tempName );
		TPtr tempNamePtr( tempName->Des() );
		tempNamePtr.Copy( iKernelObjects[i]->Name() );									
		
		_LIT(KNameFormat, "\t%S\t\t%d item");
		TPtr pName( tempName->Des() );
		//
		const TPtrC pType( TypeAsString( iKernelObjects[i]->Type() ) );
		//
		pName.Format( KNameFormat, &pType, count );
		//
		if  ( count == 0 || count > 1 )
			{
			pName.Append( _L("s") );
			}

		pName.AppendFormat( _L(", %S"), &sizeText ); // TODO: to create some ServerUtils class with formating methods for size and type!
		
		model->AppendL( pName );
		
		CleanupStack::PopAndDestroy( tempName); //---
		}
	
	CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );	
	listbox->Model()->SetItemTextArray( model );
	listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );

	/*
    // Take ownership of new model
    CMemSpyEngineHelperKernelContainers& kernelContainerManager = iEngine.HelperKernelContainers();
    CMemSpyEngineGenericKernelObjectContainer* model = kernelContainerManager.ObjectsAllL();
    delete iModel;
    iModel = model;
    
    // Set up list box
    CAknSettingStyleListBox* listbox = static_cast< CAknSettingStyleListBox* >( iListBox );
    listbox->Model()->SetItemTextArray( model );
    listbox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
    */
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
	/* TODO
    CMemSpyEngineOutputSink& sink = iEngine.Sink();
    iModel->OutputL( sink );
    */
    }

//formating methods
TPtrC CMemSpyViewKernelContainers::TypeAsString( TMemSpyDriverContainerType aType )
    {
    _LIT( KTypeUnknown, "Unknown Type" );
    _LIT( KTypeThread, "Thread" );
    _LIT( KTypeProcess, "Process" );
    _LIT( KTypeChunk, "Chunk" );
    _LIT( KTypeLibrary, "Library" );
    _LIT( KTypeSemaphore, "Semaphore" );
    _LIT( KTypeMutex, "Mutex" );
    _LIT( KTypeTimer, "Timer" );
    _LIT( KTypeServer, "Server" );
    _LIT( KTypeSession, "Session" );
    _LIT( KTypeLogicalDevice, "Logical Device" );
    _LIT( KTypePhysicalDevice, "Physical Device" );
    _LIT( KTypeLogicalChannel, "Logical Channel" );
    _LIT( KTypeChangeNotifier, "Change Notifier" );
    _LIT( KTypeUndertaker, "Undertaker" );
    _LIT( KTypeMsgQueue, "Msg. Queue" );
    _LIT( KTypePropertyRef, "Property Ref." );
    _LIT( KTypeCondVar, "Conditional Var." );
     //
    TPtrC pType( KTypeUnknown );
    //
    switch( aType )
        {
    case EMemSpyDriverContainerTypeThread:
        pType.Set( KTypeThread );
        break;
    case EMemSpyDriverContainerTypeProcess:
        pType.Set( KTypeProcess );
        break;
    case EMemSpyDriverContainerTypeChunk:
        pType.Set( KTypeChunk );
        break;
    case EMemSpyDriverContainerTypeLibrary:
        pType.Set( KTypeLibrary );
        break;
    case EMemSpyDriverContainerTypeSemaphore:
        pType.Set( KTypeSemaphore );
        break;
    case EMemSpyDriverContainerTypeMutex:
        pType.Set( KTypeMutex );
        break;
    case EMemSpyDriverContainerTypeTimer:
        pType.Set( KTypeTimer );
        break;
    case EMemSpyDriverContainerTypeServer:
        pType.Set( KTypeServer );
        break;
    case EMemSpyDriverContainerTypeSession:
        pType.Set( KTypeSession );
        break;
    case EMemSpyDriverContainerTypeLogicalDevice:
        pType.Set( KTypeLogicalDevice );
        break;
    case EMemSpyDriverContainerTypePhysicalDevice:
        pType.Set( KTypePhysicalDevice );
        break;
    case EMemSpyDriverContainerTypeLogicalChannel:
        pType.Set( KTypeLogicalChannel );
        break;
    case EMemSpyDriverContainerTypeChangeNotifier:
        pType.Set( KTypeChangeNotifier );
        break;
    case EMemSpyDriverContainerTypeUndertaker:
        pType.Set( KTypeUndertaker );
        break;
    case EMemSpyDriverContainerTypeMsgQueue:
        pType.Set( KTypeMsgQueue );
        break;
    case EMemSpyDriverContainerTypePropertyRef:
        pType.Set( KTypePropertyRef );
        break;
    case EMemSpyDriverContainerTypeCondVar:
        pType.Set( KTypeCondVar );
        break;
        
    default:
        break;
        }
    //
    return pType;
    }

