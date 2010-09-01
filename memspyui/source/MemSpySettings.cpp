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

#include "MemSpySettings.h"

// System includes
#include <e32svr.h>
#include <s32file.h>

// Engine includes
#include <memspy/engine/memspyengine.h>
#include <memspy/engine/memspyenginelogger.h>
#include <memspy/engine/memspyenginehelperprocess.h>
#include <memspy/engine/memspyenginehelpersysmemtracker.h>
#include <memspy/engine/memspyenginehelpersysmemtrackerconfig.h>

// Constants
_LIT( KMemSpySettingsFileName, "settings.dat" );

// Version 03 dumped some of the system wide memory tracker settings
const TInt KMemSpySettingsFileFormatVersion = 6;


CMemSpySettings::CMemSpySettings( RFs& aFsSession, CMemSpyEngine& aEngine )
:   iFsSession( aFsSession ), iEngine( aEngine )
    {
    }


CMemSpySettings::~CMemSpySettings()
    {
    TRACE( RDebug::Printf( "CMemSpySettings::~CMemSpySettings() - START" ) );
    TRAP_IGNORE( StoreSettingsL() );
    TRACE( RDebug::Printf( "CMemSpySettings::~CMemSpySettings() - END" ) );
    }


void CMemSpySettings::ConstructL()
    {
    TRACE( RDebug::Print( _L("CMemSpySettings::ConstructL() - START") ) );

    TRAP_IGNORE( RestoreSettingsL() );

    TRACE( RDebug::Print( _L("CMemSpySettings::ConstructL() - END") ) );
    }


CMemSpySettings* CMemSpySettings::NewL( RFs& aFsSession, CMemSpyEngine& aEngine )
    {
    CMemSpySettings* self = new(ELeave) CMemSpySettings( aFsSession, aEngine );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


void CMemSpySettings::GetSettingsFileNameL( TDes& aFileName )
    {
    GetSettingsPathL( aFileName );
    aFileName.Append( KMemSpySettingsFileName );
    TRACE( RDebug::Print( _L("CMemSpySettings::GetSettingsFileNameL() - aFileName: %S"), &aFileName ) );
    }


void CMemSpySettings::GetSettingsPathL( TDes& aPath )
    {
    TRACE( RDebug::Print( _L("CMemSpySettings::GetSettingsPathL() - START") ) );
    aPath.Zero();

    // Get private data cage path
    TInt err = iFsSession.PrivatePath( aPath );
    TRACE( RDebug::Print( _L("CMemSpySettings::GetSettingsPathL() - priv path err: %d"), err ) );
    User::LeaveIfError( err );

    // Combine with C: drive
    const TDriveUnit cDrive( EDriveC );
    const TDriveName cDriveName( cDrive.Name() );
    aPath.Insert( 0, cDriveName );

    iFsSession.MkDirAll( aPath );
    TRACE( RDebug::Print( _L("CMemSpySettings::GetSettingsPathL() - END - %S"), &aPath ) );
    }


RFile CMemSpySettings::SettingsFileLC( TBool aReplace )
    {
    TRACE( RDebug::Print( _L("CMemSpySettings::SettingsFileLC() - START - aReplace: %d"), aReplace ) );

    TFileName* fileName = new(ELeave) TFileName();
    CleanupStack::PushL( fileName );
    GetSettingsFileNameL( *fileName );
    TRACE( RDebug::Print( _L("CMemSpySettings::SettingsFileLC() - fileName: %S"), fileName ) );

    RFile file;
    TInt error = KErrNone;
    //
    if  ( aReplace )
        {
        error = file.Replace( iFsSession, *fileName, EFileWrite );
        TRACE( RDebug::Print( _L("CMemSpySettings::SettingsFileLC() - replace err: %d"), error ) );
        }
    else
        {
        error = file.Open( iFsSession, *fileName, EFileWrite );
        TRACE( RDebug::Print( _L("CMemSpySettings::SettingsFileLC() - open err: %d"), error ) );
        //
        if  ( error == KErrNotFound )
            {
            error = file.Create( iFsSession, *fileName, EFileWrite );
            }
        }
    //
    User::LeaveIfError( error );
    CleanupStack::PopAndDestroy( fileName );
    CleanupClosePushL( file );
    //
    TRACE( RDebug::Print( _L("CMemSpySettings::SettingsFileLC() - END") ) );
    return file;
    }


void CMemSpySettings::StoreSettingsL()
    {
    TRACE( RDebug::Printf( "CMemSpySettings::StoreSettingsL() - START" ) );

    RFile file = SettingsFileLC( ETrue );
    RFileWriteStream stream( file );
    CleanupStack::Pop(); // file
    CleanupClosePushL( stream );
    
    // Verion info
    stream.WriteInt32L( KMemSpySettingsFileFormatVersion );
    
    // Engine settings
    TRACE( RDebug::Printf( "CMemSpySettings::StoreSettingsL() - sinkType: %d", iEngine.SinkType() ) );
    stream.WriteUint8L( iEngine.SinkType() );

    // Get SWMT config
    CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
    TMemSpyEngineHelperSysMemTrackerConfig swmtConfig;
    swmt.GetConfig( swmtConfig );

    // Write SWMT settings
    stream.WriteInt32L( swmtConfig.TimerPeriod().Int() );
    stream.WriteUint8L( swmtConfig.DumpData() );
    
    // Write memory tracking auto-start process list
    const RArray<TUid>& processUidList = iEngine.HelperProcess().MemoryTrackingAutoStartProcessList();
    stream.WriteInt32L( processUidList.Count() );
    for( TInt i=0; i<processUidList.Count(); i++ )
        {
        const TUid uid = processUidList[ i ];
        TRACE( RDebug::Printf( "CMemSpySettings::StoreSettingsL() - process tracker uid[%02d]: 0x%08x", i, uid.iUid ) );
        stream << uid;
        }

    // Write memory tracking categories
    stream.WriteInt32L( swmtConfig.iEnabledCategories );
    
    // Write heap tracking thread name filter
    stream.WriteInt32L( swmtConfig.iThreadNameFilter.Length() );
    if ( swmtConfig.iThreadNameFilter.Length() > 0 )
        {
        stream.WriteL( swmtConfig.iThreadNameFilter, swmtConfig.iThreadNameFilter.Length() );
        }
    
    // Write mode
    stream.WriteInt32L( swmtConfig.iMode );
    
    stream.CommitL();
    CleanupStack::PopAndDestroy( &stream ); // Closes file
    TRACE( RDebug::Printf( "CMemSpySettings::StoreSettingsL() - END - sinkType: %d", iEngine.SinkType() ) );
    }


void CMemSpySettings::RestoreSettingsL()
    {
    TRACE( RDebug::Printf( "CMemSpySettings::RestoreSettingsL() - START - current engine sinkType: %d", iEngine.SinkType() ) );

    RFile file = SettingsFileLC();
    RFileReadStream stream( file );
    CleanupStack::Pop(); // file
    CleanupClosePushL( stream );
    
    // Version info
    const TInt version = stream.ReadInt32L(); // discarded for now
    TRACE( RDebug::Printf( "CMemSpySettings::RestoreSettingsL() - version: %d", version ) );

    // Engine settings
    TMemSpySinkType type = static_cast< TMemSpySinkType >( stream.ReadUint8L() );
    TRACE( RDebug::Printf( "CMemSpySettings::RestoreSettingsL() - read sinkType: %d", type ) );
    iEngine.InstallSinkL( type );
    
    // Set SWMT config
    TMemSpyEngineHelperSysMemTrackerConfig swmtConfig;
    swmtConfig.iTimerPeriod = TTimeIntervalMicroSeconds32( stream.ReadInt32L() );
    swmtConfig.iDumpData = static_cast< TBool >( stream.ReadUint8L() );

    if  ( version < 3 )
        {
        // Restore but ignore old delta tracker settings which aren't used anymore
        //
        /* iHeapDeltaTrackerIncludeKernel =*/ static_cast< TBool >( stream.ReadUint8L() );
        /* iHeapDeltaTrackerCheckAllocCellCounts =*/ static_cast< TBool >( stream.ReadUint8L() );
        /* iHeapDeltaTrackerCheckFreeCellCounts =*/ static_cast< TBool >( stream.ReadUint8L() );
        }
    
    // Restore memory tracking auto-start process uids if file format supports it...
    if ( version >= 2 )
        {
        RArray<TUid> list;
        CleanupClosePushL( list );
        //
        const TInt count = stream.ReadInt32L();
        for( TInt i=0; i<count; i++ )
            {
            TUid processUid;
            stream >> processUid;
            //
            TRACE( RDebug::Printf( "CMemSpySettings::RestoreSettingsL() - process tracker uid[%02d]: 0x%08x", i, processUid.iUid ) );
            User::LeaveIfError( list.Append( processUid ) );
            }
        //
        CMemSpyEngineHelperProcess& processHelper = iEngine.HelperProcess();
        processHelper.SetMemoryTrackingAutoStartProcessListL( list );
        CleanupStack::PopAndDestroy( &list );
        }
    
    // Restore memory tracking categories 
    if ( version > 3 )
        {
        swmtConfig.iEnabledCategories = stream.ReadInt32L();
        }
    
    // Write heap tracking thread name filter 
    if ( version > 4 )
        {
        TInt len = stream.ReadInt32L();
        if ( len > 0 )
            {
            stream.ReadL( swmtConfig.iThreadNameFilter, len );
            }
        }

    // Write mode
    if ( version > 5 )
        {
        swmtConfig.iMode = (TMemSpyEngineHelperSysMemTrackerConfig::TMemSpyEngineSysMemTrackerMode)stream.ReadInt32L();
        }
    
    CMemSpyEngineHelperSysMemTracker& swmt = iEngine.HelperSysMemTracker();
    swmt.SetConfigL( swmtConfig );

    CleanupStack::PopAndDestroy( &stream ); // Closes file
    TRACE( RDebug::Printf( "CMemSpySettings::RestoreSettingsL() - END - engine sink type: %d", iEngine.SinkType() ) );
    }


