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


// INCLUDE FILES

#include "FBModel.h"
#include "FBApp.h"
#include "FB.hrh"
#include "FBSettingViewDlg.h"
#include "FBFileUtils.h"
#include "FBFileListContainer.h"
#include "FBStd.h"
#include <filebrowser.rsg>

#include <coeutils.h>
#include <bautils.h>
#include <apaid.h>
#include <AknGlobalConfirmationQuery.h>
#include <s32file.h>

// hash key selection related includes
#ifndef __SERIES60_30__
  #include <centralrepository.h>
  #include <AknFepInternalCRKeys.h>
  #include <AvkonInternalCRKeys.h>
  #include <e32property.h> 
#endif

const TInt KSettingsDrive = EDriveC;
_LIT(KSettingsFileName, "filebrowser_settings.ini");

// ===================================== MEMBER FUNCTIONS =====================================

CFileBrowserModel* CFileBrowserModel::NewL()
	{
	CFileBrowserModel* self = new(ELeave) CFileBrowserModel;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

// --------------------------------------------------------------------------------------------

CFileBrowserModel::CFileBrowserModel()
	{
	}

// --------------------------------------------------------------------------------------------

void CFileBrowserModel::ConstructL()
	{
    iEnv = CEikonEnv::Static();
    User::LeaveIfError(iLs.Connect());
	}

// --------------------------------------------------------------------------------------------

CFileBrowserModel::~CFileBrowserModel()
	{
	if (iFileUtils)
	    delete iFileUtils;

	iLs.Close();
    }

// ---------------------------------------------------------------------------

void CFileBrowserModel::ActivateModelL()
	{
    TRAP_IGNORE( LoadSettingsL() );

    iFileUtils = CFileBrowserFileUtils::NewL(this);
    
    // get hash key selection value
    GetHashKeySelectionStatus();
	}

// ---------------------------------------------------------------------------

void CFileBrowserModel::DeActivateModelL()
	{
	}
	
// --------------------------------------------------------------------------------------------

void CFileBrowserModel::SetFileListContainer(CFileBrowserFileListContainer* aFileListContainer)
    {
    iFileListContainer = aFileListContainer;
    }

// ---------------------------------------------------------------------------

void CFileBrowserModel::LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TInt& aValue)
    {
    if (aDicFS->IsPresentL(aUid))
        {
        RDictionaryReadStream in;
        in.OpenLC(*aDicFS, aUid);
        aValue = in.ReadInt16L();
        CleanupStack::PopAndDestroy(); // in        
        }
    }

// ---------------------------------------------------------------------------

void CFileBrowserModel::LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TDes& aValue)
    {
    if (aDicFS->IsPresentL(aUid))
        {
        RDictionaryReadStream in;
        in.OpenLC(*aDicFS, aUid);
        TInt bufLength = in.ReadInt16L();   // get length of descriptor
        in.ReadL(aValue, bufLength);        // get the descriptor itself
        CleanupStack::PopAndDestroy(); // in
        }
    }

// ---------------------------------------------------------------------------

void CFileBrowserModel::SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TInt& aValue)
    {
    RDictionaryWriteStream out;
    out.AssignLC(*aDicFS, aUid);
    out.WriteInt16L(aValue);
    out.CommitL(); 	
    CleanupStack::PopAndDestroy(1);// out
    }

// ---------------------------------------------------------------------------

void CFileBrowserModel::SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TDes& aValue)
    {
    RDictionaryWriteStream out;
    out.AssignLC(*aDicFS, aUid);
    out.WriteInt16L(aValue.Length());       // write length of the descriptor
    out.WriteL(aValue, aValue.Length());    // write the descriptor itself
    out.CommitL(); 	
    CleanupStack::PopAndDestroy(1);// out
    }
        
// --------------------------------------------------------------------------------------------

void CFileBrowserModel::LoadSettingsL()
    {
    const TSize screenSize = iEnv->ScreenDevice()->SizeInPixels();

    // set defaults
    iSettings.iDisplayMode = EDisplayModeFullScreen;
    iSettings.iFileViewMode = IsQHD(screenSize) ? EFileViewModeExtended : EFileViewModeSimple;
    iSettings.iShowSubDirectoryInfo = EFalse;
    iSettings.iShowAssociatedIcons = EFalse;
    iSettings.iRememberLastPath = EFalse;
    iSettings.iLastPath = KNullDesC;
    iSettings.iRememberFolderSelection = ETrue;
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    if ( AknLayoutUtils::PenEnabled() )
        {
        iSettings.iEnableToolbar = ETrue;
        }
    else
        {
        iSettings.iEnableToolbar = EFalse;
        }
#else
    iSettings.iEnableToolbar = EFalse;
#endif

    iSettings.iSupportNetworkDrives = EFalse;
    iSettings.iBypassPlatformSecurity = EFalse;
    iSettings.iRemoveFileLocks = ETrue;
    iSettings.iIgnoreProtectionsAtts = ETrue;
    iSettings.iRemoveROMWriteProrection = ETrue;

    // build specific defaults
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
    iSettings.iSupportNetworkDrives = ETrue;
#endif 


    // make sure that the private path of this app in c-drive exists
    iEnv->FsSession().CreatePrivatePath( KSettingsDrive ); // c:\\private\\102828d6\\
    
    // handle settings always in the private directory 
    if (iEnv->FsSession().SetSessionToPrivate( KSettingsDrive ) == KErrNone)
        {
        // open or create a dictionary file store
        CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(iEnv->FsSession(), KSettingsFileName, KUidFileBrowser);

        LoadDFSValueL(settingsStore, KFBSettingDisplayMode,                 iSettings.iDisplayMode);
        LoadDFSValueL(settingsStore, KFBSettingFileViewMode,                iSettings.iFileViewMode);
        LoadDFSValueL(settingsStore, KFBSettingShowSubDirectoryInfo,        iSettings.iShowSubDirectoryInfo);
        LoadDFSValueL(settingsStore, KFBSettingShowAssociatedIcons,         iSettings.iShowAssociatedIcons);
        LoadDFSValueL(settingsStore, KFBSettingRememberLastPath,            iSettings.iRememberLastPath);
        LoadDFSValueL(settingsStore, KFBSettingLastPath,                    iSettings.iLastPath);
        LoadDFSValueL(settingsStore, KFBSettingFolderSelection,             iSettings.iRememberFolderSelection);
        LoadDFSValueL(settingsStore, KFBSettingEnableToolbar,               iSettings.iEnableToolbar);

        LoadDFSValueL(settingsStore, KFBSettingSupportNetworkDrives,        iSettings.iSupportNetworkDrives);
        LoadDFSValueL(settingsStore, KFBSettingBypassPlatformSecurity,      iSettings.iBypassPlatformSecurity);
        LoadDFSValueL(settingsStore, KFBSettingRemoveFileLocks,             iSettings.iRemoveFileLocks);
        LoadDFSValueL(settingsStore, KFBSettingIgnoreProtectionsAtts,       iSettings.iIgnoreProtectionsAtts);
        LoadDFSValueL(settingsStore, KFBSettingRemoveROMWriteProtection,    iSettings.iRemoveROMWriteProrection);

        CleanupStack::PopAndDestroy(); // settingsStore         
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserModel::SaveSettingsL(TBool aNotifyModules)
    {
    // handle settings always in c:\\private\\102828d6\\
    if (iEnv->FsSession().SetSessionToPrivate( KSettingsDrive ) == KErrNone)
        {
        // delete existing store to make sure that it is clean and not eg corrupted
        if (BaflUtils::FileExists(iEnv->FsSession(), KSettingsFileName))
            {
            iEnv->FsSession().Delete(KSettingsFileName);
            }
        
        // create a dictionary file store
        CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(iEnv->FsSession(), KSettingsFileName, KUidFileBrowser);

        SaveDFSValueL(settingsStore, KFBSettingDisplayMode,                 iSettings.iDisplayMode);
        SaveDFSValueL(settingsStore, KFBSettingFileViewMode,                iSettings.iFileViewMode);
        SaveDFSValueL(settingsStore, KFBSettingShowSubDirectoryInfo,        iSettings.iShowSubDirectoryInfo);
        SaveDFSValueL(settingsStore, KFBSettingShowAssociatedIcons,         iSettings.iShowAssociatedIcons);
        SaveDFSValueL(settingsStore, KFBSettingRememberLastPath,            iSettings.iRememberLastPath);
        SaveDFSValueL(settingsStore, KFBSettingLastPath,                    iSettings.iLastPath);
        SaveDFSValueL(settingsStore, KFBSettingFolderSelection,             iSettings.iRememberFolderSelection);
        SaveDFSValueL(settingsStore, KFBSettingEnableToolbar,               iSettings.iEnableToolbar);

        SaveDFSValueL(settingsStore, KFBSettingSupportNetworkDrives,        iSettings.iSupportNetworkDrives);
        SaveDFSValueL(settingsStore, KFBSettingBypassPlatformSecurity,      iSettings.iBypassPlatformSecurity);
        SaveDFSValueL(settingsStore, KFBSettingRemoveFileLocks,             iSettings.iRemoveFileLocks);
        SaveDFSValueL(settingsStore, KFBSettingIgnoreProtectionsAtts,       iSettings.iIgnoreProtectionsAtts);
        SaveDFSValueL(settingsStore, KFBSettingRemoveROMWriteProtection,    iSettings.iRemoveROMWriteProrection);
        
        settingsStore->CommitL();
        CleanupStack::PopAndDestroy(); // settingsStore             
        }

    // update changes to modules
    if (aNotifyModules)
        {
        //iScreenCapture->HandleSettingsChangeL();
        iFileUtils->HandleSettingsChangeL();
        iFileListContainer->HandleSettingsChangeL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserModel::GetHashKeySelectionStatus()
    {
    TBool hashKeySelectionInUse(EFalse);
    
#ifndef __SERIES60_30__
    
    // get hash key selection value
    TRAP_IGNORE(
        CRepository* repository = CRepository::NewLC(KCRUidAknFep);
        repository->Get(KAknFepHashKeySelection, hashKeySelectionInUse);
        CleanupStack::PopAndDestroy();
    );
    
    // even if hash key selection is in use, ignore the value in qwerty mode
    if (hashKeySelectionInUse)
        {
        TBool qwertyMode(EFalse);
        RProperty qwertyModeStatusProperty;
        qwertyModeStatusProperty.Attach(KCRUidAvkon, KAknQwertyInputModeActive);
        qwertyModeStatusProperty.Get(qwertyMode);
        qwertyModeStatusProperty.Close();
        
        if (qwertyMode)
            hashKeySelectionInUse = EFalse;        
        }

#endif

    iIsHashKeySelectionInUse = hashKeySelectionInUse;
    }
    	
// --------------------------------------------------------------------------------------------

TInt CFileBrowserModel::LaunchSettingsDialogL()
    {
    // set to normal mode
    iFileListContainer->SetScreenLayoutL(EDisplayModeNormal);
    iFileListContainer->DeleteNaviPane();
    iFileListContainer->HideToolbar();
    
    // launch the dialog and save settings
    CFileBrowserSettingViewDlg* dlg = CFileBrowserSettingViewDlg::NewL(iSettings);
    TInt retValue = dlg->ExecuteLD(R_FILEBROWSER_SETTINGS_DIALOG);        
    FileListContainer()->CreateEmptyNaviPaneLabelL();
    TRAP_IGNORE(SaveSettingsL());
    return retValue;
    }
	
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

CAsyncWaiter* CAsyncWaiter::NewL(TInt aPriority)
	{
	CAsyncWaiter* self = new(ELeave) CAsyncWaiter(aPriority);
	return self;
	}

CAsyncWaiter* CAsyncWaiter::NewLC(TInt aPriority)
	{
	CAsyncWaiter* self = new(ELeave) CAsyncWaiter(aPriority);
	CleanupStack::PushL(self);
	return self;
	}
	
CAsyncWaiter::CAsyncWaiter(TInt aPriority) : CActive(aPriority)
	{
	CActiveScheduler::Add(this);
	}	

CAsyncWaiter::~CAsyncWaiter()
	{
	Cancel();
	}
	
void CAsyncWaiter::StartAndWait()
	{
    iStatus = KRequestPending;
    SetActive();
    iWait.Start();
	}
	
TInt CAsyncWaiter::Result() const
	{
	return iError;
	}
	
void CAsyncWaiter::RunL()
	{
	iError = iStatus.Int();
	CAknEnv::StopSchedulerWaitWithBusyMessage( iWait );
	}
	
void CAsyncWaiter::DoCancel()
	{
	iError = KErrCancel;
    if( iStatus == KRequestPending )
        {
        TRequestStatus* s=&iStatus;
        User::RequestComplete( s, KErrCancel );
        }

    CAknEnv::StopSchedulerWaitWithBusyMessage( iWait );
	}

// --------------------------------------------------------------------------------------------

// End of File
