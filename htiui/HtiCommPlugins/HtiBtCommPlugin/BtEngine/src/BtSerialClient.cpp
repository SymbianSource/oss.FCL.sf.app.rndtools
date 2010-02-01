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
* Description:  Bluetooth serial client.
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <btengsettings.h>  // CBTEngSettings

#include "BtSerialClient.h"
#include "MessageServiceSearcher.h"
#include "BtSerialEngine.pan"
#include "HtiBtEngineLogging.h"
#include "socketswriter.h"
#include "socketsreader.h"

const TInt KMaxBtStartWaitLoop   = 5;
const TInt KBtStateQueryInterval = 1000000; // microseconds


// ----------------------------------------------------------------------------
EXPORT_C CBtSerialClient* CBtSerialClient::NewL(MBtSerialClientObserver& aObserver)
    {
    CBtSerialClient* self = new (ELeave) CBtSerialClient(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ----------------------------------------------------------------------------
CBtSerialClient::CBtSerialClient(MBtSerialClientObserver& aObserver)
: CActive( CActive::EPriorityStandard ), iObserver(aObserver),
  iState( EWaitingToGetDevice ), iCurrentServiceIndex( 0 )
    {
    CActiveScheduler::Add( this );
    }

// ----------------------------------------------------------------------------
EXPORT_C CBtSerialClient::~CBtSerialClient()
    {
    delete iSocketReader;
    delete iSocketWriter;

    Cancel();

    iSocket.Close();
    iSocketServer.Close();

    delete iServiceSearcher;
    }

// ----------------------------------------------------------------------------
void CBtSerialClient::ConstructL()
    {
    LOG_D( "CBtSerialClient::ConstructL()" );

   // Check Bluetooth power state
    TBTPowerStateValue powerState = EBTPowerOff;
    CBTEngSettings* btSettings = CBTEngSettings::NewLC();
    TInt err = btSettings->GetPowerState( powerState );
    if ( err )
        {
        LOGFMT_W( "CBtSerialClient::ConstructL(): GetPowerState error %d", err );
        powerState = EBTPowerOff;
        }

    // If BT not on - try to set it on
    if ( powerState == EBTPowerOff )
        {
        LOG_I( "CBtSerialClient::ConstructL(): BT not on - setting power on" );
        err = btSettings->SetPowerState( EBTPowerOn );
        if ( err )
            {
            LOGFMT_E( "CBtSerialClient::ConstructL(): SetPowerState error %d", err );
            User::Leave( err );
            }

        // Wait until BT is reported to be on (or waiting time exceeds)
        TInt loopCounter = 0;
        while ( powerState == EBTPowerOff && loopCounter < KMaxBtStartWaitLoop )
            {
            btSettings->GetPowerState( powerState );
            LOGFMT_D( "CBtSerialClient::ConstructL(): BT power state %d", powerState );
            User::After( KBtStateQueryInterval );
            loopCounter++;
            }

        if ( powerState == EBTPowerOff )
            {
            LOG_E( "CBtSerialClient::ConstructL(): Could not turn BT on" );
            User::Leave( KErrCouldNotConnect );
            }
        LOG_I( "CBtSerialClient::ConstructL(): Continuing BT connect" );
        }
    CleanupStack::PopAndDestroy(); // btSettings

    iServiceSearcher = CMessageServiceSearcher::NewL();
    User::LeaveIfError( iSocketServer.Connect() );
    iSocketReader = CSocketsReader::NewL( *this, iSocket );
    iSocketWriter = CSocketsWriter::NewL( *this, iSocket );
    }

// ----------------------------------------------------------------------------
void CBtSerialClient::DoCancel()
    {
    LOG_W("CBtSerialClient::DoCancel(): Doing nothing");
    }

// ----------------------------------------------------------------------------
void CBtSerialClient::RunL()
    {
    if ( iStatus != KErrNone )
        {
        switch ( iState )
            {
            case EGettingDevice:
                if ( iStatus == KErrCancel )
                    {
                    LOG_W( "CBtSerialClient: No device selected" );
                    }
                iState = EWaitingToGetDevice;
                iObserver.ConnectedToServer( iStatus.Int() );
                break;
            case EGettingService:
                LOGFMT_W( "CBtSerialClient: Failed to fetch remote service: %d", iStatus.Int() );
                iObserver.ConnectedToServer( iStatus.Int() );
                iState = EWaitingToGetDevice;
                break;
            case EGettingConnection:
                LOGFMT_W( "CBtSerialClient: Failed to connect to remote service: %d", iStatus.Int() );
                if ( iCurrentServiceIndex < iServiceSearcher->ServiceCount() )
                    {
                    // Try another service
                    iCurrentServiceIndex++;
                    ConnectToServerL(); // establish RFComm connection
                    }
                else
                    {
                    iState = EWaitingToGetDevice;
                    iObserver.ConnectedToServer( iStatus.Int() );
                    }
                break;
            case EConnected:
                LOGFMT_I( "CBtSerialClient: Lost connection: %d", iStatus.Int() )
                DisconnectFromServer();
                iState = EDisconnecting;
                break;
            case EDisconnecting:
                LOGFMT_I("CBtSerialClient: Disconnected from server: %d", iStatus.Int() );
                iSocket.Close();
                iState = EWaitingToGetDevice;
                iObserver.DisconnectedFromServer();
                break;
            default:
                Panic( EBTPointToPointInvalidLogicState );
                break;
            }
        }
    else // iStatus = KErrNone
        {
        switch ( iState )
            {
            case EGettingDevice:
                // found a device now search for a suitable service
                LOGFMT_I("CBtSerialClient: Found device: %S. Finding correct service.", &(iServiceSearcher->ResponseParams().DeviceName()) );
                iState = EGettingService;
                iStatus = KRequestPending;  // this means that the RunL can not
                                            // be called until this program
                                            // does something to iStatus
                iBTServerDevice = iServiceSearcher->BTDevAddr();
                iServiceSearcher->FindServiceL( iBTServerDevice, iStatus );
                SetActive();
                break;
            case EGettingService:
                LOGFMT_I("CBtSerialClient: Found %d remote services", iServiceSearcher->ServiceCount());
                iState = EGettingConnection;
                ConnectToServerL(); // establish RFComm connection
                break;
            case EGettingConnection:
                LOG_I( "CBtSerialClient: Connected to remote service" );
                iState = EConnected;
                iObserver.ConnectedToServer( KErrNone );
                break;
            case EDisconnecting:
                LOG_I( "CBtSerialClient: Disconnecting" );
                iSocket.Close();
                iState = EWaitingToGetDevice;
                iObserver.DisconnectedFromServer();
                break;
            default:
                LOGFMT_E( "CBtSerialClient: Invalid logic state in RunL: %d. Will panic", iState );
                Panic( EBTPointToPointInvalidLogicState );
                break;
            };
        }
    }

// ----------------------------------------------------------------------------
EXPORT_C void CBtSerialClient::ConnectL()
    {
    if ( iState == EWaitingToGetDevice && !IsActive() )
        {
        LOG_D( "CBtSerialClient: Connecting by user selection" );
        iState = EGettingDevice;
        iServiceSearcher->SelectDeviceByDiscoveryL( iStatus );
        SetActive();
        }
    else
        {
        LOG_W( "CBtSerialClient: Already connecting or connected" );
        User::Leave( KErrInUse );
        }
    }

// ----------------------------------------------------------------------------
EXPORT_C void CBtSerialClient::ConnectL( const TBTDevAddr aBTServerDevice,
                                         const TInt aPort )
    {
    if ( iState == EWaitingToGetDevice && !IsActive() )
        {
        if ( aPort >= 0 )
            {
            // If there was a valid port given, add it as the first port to try
            iServiceSearcher->AppendPort( aPort );
            }
        LOG_D( "CBtSerialClient: Connecting by address" );
        iBTServerDevice = aBTServerDevice;
        iServiceSearcher->FindServiceL( iBTServerDevice, iStatus );
        iState = EGettingService;
        iStatus = KRequestPending;  // this means that the RunL can not
                                    // be called until this program
                                    // does something to iStatus
        SetActive();
        }
    else
        {
        LOG_W("CBtSerialClient: Already connecting or connected");
        User::Leave( KErrInUse );
        }
    }

// ----------------------------------------------------------------------------
EXPORT_C void CBtSerialClient::ConnectL( const TDesC& aBTServerDeviceName,
                                         const TInt aPort )
    {
    if ( iState == EWaitingToGetDevice && !IsActive() )
        {
        if ( aPort >= 0 )
            {
            // If there was a valid port given, add it as the first port to try
            iServiceSearcher->AppendPort( aPort );
            }
        LOG_D( "CBtSerialClient: Connecting by name" );
        iState = EGettingDevice;
        iServiceSearcher->SelectDeviceByNameL( aBTServerDeviceName, iStatus );
        SetActive();
        }
    else
        {
        LOG_W( "CBtSerialClient: Already connecting or connected" );
        User::Leave( KErrInUse );
        }
    }

// ----------------------------------------------------------------------------
EXPORT_C TBTDevAddr CBtSerialClient::ServerAddressL()
    {
    if ( !Connected() )
        {
        User::Leave( KErrNotReady );
        }
    return iBTServerDevice;
    }

// ----------------------------------------------------------------------------
EXPORT_C void CBtSerialClient::Disconnect()
    {
    if ( iState == EConnected )
        {
        DisconnectFromServer();
        iState = EDisconnecting;
        }
    else
        {
        LOG_W( "CBtSerialClient: Disconnect: Not connected" );
        User::Leave( KErrDisconnected );
        }
    }

// ----------------------------------------------------------------------------
void CBtSerialClient::DisconnectFromServer()
    {
    // Terminate all operations
    iSocket.CancelAll();
    Cancel();
    iSocketReader->Cancel();
    iSocketWriter->CancelSending();

    LOG_I( "CBtSerialClient: Disconnecting from server" );
    iSocket.Shutdown( RSocket::ENormal, iStatus );
    SetActive();
    }

// ----------------------------------------------------------------------------
void CBtSerialClient::ConnectToServerL()
    {
    LOG_I("CBtSerialClient: ConnectToServerL")
    User::LeaveIfError( iSocket.Open( iSocketServer, KStrRFCOMM ) );

    TBTSockAddr address;
    address.SetBTAddr( iServiceSearcher->BTDevAddr() );
    address.SetPort( iServiceSearcher->Port( iCurrentServiceIndex ) );

    LOGFMT_I("CBtSerialClient: ConnectToServerL: Port = %d", address.Port() );
    iSocket.Connect( address, iStatus );

#ifdef __WINS__
    User::After( 1 ); // Needed to allow emulator client to connect to server
#endif

    SetActive();
    }

// ----------------------------------------------------------------------------
EXPORT_C void CBtSerialClient::SendL(const TDesC8& aData)
    {
    if ( !Connected() )
        {
        User::Leave( KErrNotReady );
        }

    LOGFMT_D("CBtSerialClient::SendL: Sending data (max first 32 bytes): \"%S\"", &(aData.Left(32)));
    iSocketWriter->SendL( aData ); // Add to data queue and start sending
    LOG_D("CBtSerialClient::SendL: Sent to socket");
    }

EXPORT_C void CBtSerialClient::ReadAsyncL()
    {
    if ( !Connected() )
        {
        User::Leave( KErrNotReady );
        }
    iSocketReader->ReadAsync();
    }

// ----------------------------------------------------------------------------
EXPORT_C TInt CBtSerialClient::FreeSpaceInSendBuffer()
    {
    return iSocketWriter->FreeSpaceInSendBuffer();
    }

// ----------------------------------------------------------------------------
EXPORT_C TInt CBtSerialClient::SendBufferMaxSize()
    {
    return iSocketWriter->SendBufferMaxSize();
    }

// ----------------------------------------------------------------------------
EXPORT_C TBool CBtSerialClient::Connected()
    {
    return (iState == EConnected);
    }

// ----------------------------------------------------------------------------
EXPORT_C TBool CBtSerialClient::Connecting()
    {
    return ( ( iState == EGettingDevice ) ||
             ( iState == EGettingService ) ||
             ( iState == EGettingConnection ) );
    }

// ----------------------------------------------------------------------------
EXPORT_C TInt CBtSerialClient::ServicePort()
    {
    if ( !Connected() )
        {
        return KErrDisconnected;
        }
    return iServiceSearcher->Port( iCurrentServiceIndex );
    }

// ----------------------------------------------------------------------------
void CBtSerialClient::ReportError( TErrorType aErrorType, TInt aErrorCode )
    {
    LOGFMT_W( "CBtSerialClient::ReportError: %d", aErrorCode );
    // From socket reader or writer
    switch ( aErrorType )
        {
        case EDisconnected:
            {
            LOG_I( "CBtSerialClient: disconnected" );
            }
            break;
        case ETimeOutOnWrite:
            {
            LOG_I( "CBtSerialClient: timout writing data. Disconnecting from server" );
            }
            break;
        case EGeneralReadError:
            {
            LOG_I( "CBtSerialClient: general read error. Disconnecting from server" );
            }
            break;
        case EGeneralWriteError:
            {
            LOG_I( "CBtSerialClient: general write error. Disconnecting from server" );
            }
            break;
        }
    Disconnect();
    aErrorCode = aErrorCode;
    }

// ----------------------------------------------------------------------------
 void CBtSerialClient::NewData( const TDesC8& aData )
    {
    iObserver.DataFromServer( aData );
    }

// ----------------------------------------------------------------------------
void CBtSerialClient::AllBufferedDataSent()
    {
    iObserver.AllBufferedDataSent();
    }

// End of File
