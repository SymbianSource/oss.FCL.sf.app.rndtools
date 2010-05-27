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

#ifndef MEMSPYUIUTILS_H
#define MEMSPYUIUTILS_H

// System includes
#include <e32std.h>

// Engine includes
#include <memspy/engine/memspyengineutils.h>
#include <memspy/engine/memspyengineobjectthreadinfoobjects.h>

_LIT( KTab, "\t" );
	
_LIT( KTypeUnknown, "Unknown Type" );
_LIT( KGeneral, "General" );
_LIT( KHeap, "Heap" );
_LIT( KStack, "Stack" );
_LIT( KChunks, "Chunks" );
_LIT( KCodeSegs, "Code Segments" );
_LIT( KOpenFiles, "Open Files" );
_LIT( KActiveObjects, "Active Objects" );
_LIT( KThreadHandlers, "Handlers to other Threads" );
_LIT( KProcessHandlers, "Handlers to other Processes" );
_LIT( KServers, "Servers running in Thread" );
_LIT( KConnections, "Client <> Server Connections" );
_LIT( KSemaphores, "Semaphores" );
_LIT( KThreadReferences, "References in Thread" );
_LIT( KProcessReferences, "References in Process" );
_LIT( KMutexes, "Mutexes" );
_LIT( KTimers, "Timers" );
_LIT( KDD, "Logical DD Channels" );
_LIT( KChangeNotif, "Change Notifiers" );
_LIT( KUndertakers, "Undertakers" );
_LIT( KLogicalDrivers, "Logical Device Drivers" );
_LIT( KPhysicalDrivers, "Physical Device Drivers" );	

class MemSpyUiUtils : public MemSpyEngineUtils
    {
public:
    static void Format( TDes& aBuf, TInt aResourceId, ...);
    static void GetErrorText( TDes& aBuf, TInt aError );
    static HBufC* FormatItem( const TDesC& aCaption );
    static TDesC& ThreadInfoItemNameByType( TMemSpyThreadInfoItemType aType );
    };




#endif
