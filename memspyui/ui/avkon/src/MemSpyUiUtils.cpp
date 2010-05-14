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

#include "MemSpyUiUtils.h"

// System includes
#include <coemain.h>
#include <memspyui.rsg>




void MemSpyUiUtils::Format( TDes& aBuf, TInt aResourceId, ...)
    {
	VA_LIST list;
    VA_START(list,aResourceId);
	TBuf<128> format;
	CCoeEnv::Static()->ReadResource( format, aResourceId );
	aBuf.FormatList( format, list );
    }


void MemSpyUiUtils::GetErrorText( TDes& aBuf, TInt aError )
    {
    if  ( aError == KErrNotSupported )
        {
        _LIT( KMemSpyErrorText, "Not Supported" );
        aBuf = KMemSpyErrorText;
        }
    else if ( aError == KErrNotReady )
        {
        _LIT( KMemSpyErrorText, "Not Ready" );
        aBuf = KMemSpyErrorText;
        }
    else if ( aError == KErrNotFound )
        {
        _LIT( KMemSpyErrorText, "Missing" );
        aBuf = KMemSpyErrorText;
        }
    else if ( aError == KErrGeneral )
        {
        _LIT( KMemSpyErrorText, "General Error" );
        aBuf = KMemSpyErrorText;
        }
    else
        {
        _LIT( KMemSpyItemValueError, "Error: %d" );
        aBuf.Format( KMemSpyItemValueError, aError );
        }
    }

HBufC* MemSpyUiUtils::FormatItem( const TDesC& aCaption )
	{
	HBufC* retBuf = HBufC::NewL( 32 );
	TPtr pRetBuf( retBuf->Des() );
	pRetBuf.Zero();
	pRetBuf.Append( _L("\t") );
	pRetBuf.Append( aCaption );
	return retBuf;
	}

TDesC& MemSpyUiUtils::ThreadInfoItemNameByType( TMemSpyThreadInfoItemType aType )
	{	
	TPtrC pType( KTypeUnknown );
	
	switch( aType )
		{
		case EMemSpyThreadInfoItemTypeFirst:			
			pType.Set(KGeneral);
			break;			
		case EMemSpyThreadInfoItemTypeHeap:			
			pType.Set(KHeap);
			break;				
		case EMemSpyThreadInfoItemTypeStack:			
			pType.Set(KStack);
			break;			
		case EMemSpyThreadInfoItemTypeChunk:			
			pType.Set(KChunks);
			break;
		case EMemSpyThreadInfoItemTypeCodeSeg:
			pType.Set(KCodeSegs);
			break;
		case EMemSpyThreadInfoItemTypeOpenFiles:
			pType.Set(KOpenFiles);
			break;	
		case EMemSpyThreadInfoItemTypeActiveObject:
			pType.Set(KActiveObjects);
			break;
		case EMemSpyThreadInfoItemTypeOwnedThreadHandles:
			pType.Set(KThreadHandlers);
			break;
		case EMemSpyThreadInfoItemTypeOwnedProcessHandles:
			pType.Set(KProcessHandlers);
			break;
		case EMemSpyThreadInfoItemTypeServer:
			pType.Set(KServers);
			break;
		case EMemSpyThreadInfoItemTypeSession:
			pType.Set(KConnections);
			break;
		case EMemSpyThreadInfoItemTypeSemaphore:
			pType.Set(KSemaphores);
			break;
		case EMemSpyThreadInfoItemTypeOtherThreads:
			pType.Set(KThreadReferences);
			break;
		case EMemSpyThreadInfoItemTypeOtherProcesses:
			pType.Set(KProcessReferences);
			break;
		case EMemSpyThreadInfoItemTypeMutex:
			pType.Set(KMutexes);
			break;
		case EMemSpyThreadInfoItemTypeTimer:
			pType.Set(KTimers);					
			break;
		case EMemSpyThreadInfoItemTypeLogicalChannel:
			pType.Set(KDD);
			break;
		case EMemSpyThreadInfoItemTypeChangeNotifier:
			pType.Set(KChangeNotif);
			break;
		case EMemSpyThreadInfoItemTypeUndertaker:
			pType.Set(KUndertakers);
			break;				
		case EMemSpyThreadInfoItemTypeLDD:
			pType.Set(KLogicalDrivers);
			break;
		case EMemSpyThreadInfoItemTypePDD:
			pType.Set(KPhysicalDrivers);
			break;
			
	    default:	       
	    	break;  
		}
	return pType;
	}

