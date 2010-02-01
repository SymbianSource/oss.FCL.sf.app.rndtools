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
* Description:  PIMServicePlugin implementation
*
*/


// INCLUDE FILES
#include "HtiPIMServicePlugin.h"
#include "PIMHandler.h"
#include "HtiBookmarkHandler.h"
#include "HtiSimDirHandlerVPbk.h"

#include <HtiDispatcherInterface.h>
#include <HTILogging.h>

// CONSTANTS
_LIT8( KErrorMissingCommand, "Command was not given - message was empty" );

// ----------------------------------------------------------------------------
// Create instance of concrete ECOM interface implementation
CHtiPIMServicePlugin* CHtiPIMServicePlugin::NewL()
    {
    CHtiPIMServicePlugin* self = new (ELeave) CHtiPIMServicePlugin;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------------------------
CHtiPIMServicePlugin::CHtiPIMServicePlugin()
    {
    }

// ----------------------------------------------------------------------------
CHtiPIMServicePlugin::~CHtiPIMServicePlugin()
    {
    HTI_LOG_TEXT("CHtiPIMServicePlugin destroy");
    delete iPimHandler;
    delete iBookmarkHandler;
    delete iSimDirHandler;
    }

// ----------------------------------------------------------------------------
void CHtiPIMServicePlugin::ConstructL()
    {
    HTI_LOG_TEXT("CHtiPIMServicePlugin::ConstructL");
    }

// ----------------------------------------------------------------------------
void CHtiPIMServicePlugin::ProcessMessageL( const TDesC8& aMessage,
    THtiMessagePriority aPriority )
    {
    HTI_LOG_FUNC_IN( "CHtiPIMServicePlugin::ProcessMessageL" );
    HTI_LOG_FORMAT( "Msg len: %d.", aMessage.Length() );

    if ( aMessage.Length() == 0 )
        {
        User::LeaveIfError( iDispatcher->DispatchOutgoingErrorMessage(
            KErrArgument, KErrorMissingCommand, KPIMServiceUid ) );
        return;
        }

    TUint8 aCommand = aMessage.Ptr()[0];
    if ( aCommand < ESimCardInfo )
        {
        if ( iPimHandler == NULL )
            {
            iPimHandler = CPIMHandler::NewL();
            iPimHandler->SetDispatcher( iDispatcher );
            }
        iPimHandler->ProcessMessageL( aMessage, aPriority );
        }
    else if ( aCommand < ECreateBookmark )
        {
        if ( iSimDirHandler == NULL )
            {
            iSimDirHandler = CHtiSimDirHandlerVPbk::NewL();
            iSimDirHandler->SetDispatcher( iDispatcher );
            }
        iSimDirHandler->ProcessMessageL( aMessage, aPriority );
        }
    else
        {
        if ( iBookmarkHandler == NULL )
            {
            iBookmarkHandler = CHtiBookmarkHandler::NewL();
            iBookmarkHandler->SetDispatcher( iDispatcher );
            }
        iBookmarkHandler->ProcessMessageL( aMessage, aPriority );
        }

    HTI_LOG_FUNC_OUT( "CHtiPIMServicePlugin::ProcessMessageL" );
    }


// ----------------------------------------------------------------------------
TBool CHtiPIMServicePlugin::IsBusy()
    {
    if ( iPimHandler )
        {
        if ( iPimHandler->IsBusy() )
            return ETrue;
        }
    if ( iSimDirHandler )
        {
        if ( iSimDirHandler->IsBusy() )
            return ETrue;
        }
    if ( iBookmarkHandler )
        {
        if ( iBookmarkHandler->IsBusy() )
            return ETrue;
        }
    return EFalse;
    }
