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


#ifndef __FILEBROWSER_MODEL_H__
#define __FILEBROWSER_MODEL_H__

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <apgcli.h>
#include <CAknMemorySelectionSettingPage.h>


// setting keys (do not change uids of existing keys to maintain compatibility to older versions!)
const TUid KFBSettingDisplayMode                   = { 0x00 };
const TUid KFBSettingFileViewMode                  = { 0x01 };
const TUid KFBSettingShowSubDirectoryInfo          = { 0x02 };
const TUid KFBSettingShowAssociatedIcons           = { 0x03 };
const TUid KFBSettingRememberLastPath              = { 0x04 };
const TUid KFBSettingLastPath                      = { 0x05 };

const TUid KFBSettingSupportNetworkDrives          = { 0x06 };
const TUid KFBSettingBypassPlatformSecurity        = { 0x07 };
const TUid KFBSettingRemoveFileLocks               = { 0x08 };
const TUid KFBSettingIgnoreProtectionsAtts         = { 0x09 };
const TUid KFBSettingRemoveROMWriteProtection      = { 0x0A };

const TUid KFBSettingFolderSelection               = { 0x0B };
const TUid KFBSettingEnableToolbar                 = { 0x0C };


// FORWARD DECLARATIONS
class CFileBrowserFileListContainer;
class CFileBrowserScreenCapture;
class CFileBrowserFileUtils;
class CEikonEnv;
class CAknGlobalConfirmationQuery;
class CDictionaryFileStore;

// CLASS DECLARATIONS

class TFileBrowserSettings
    {
public:
    TInt                                        iDisplayMode;
    TInt                                        iFileViewMode;
    TBool                                       iShowSubDirectoryInfo;
    TBool                                       iShowAssociatedIcons;
    TBool                                       iRememberLastPath;
    TFileName                                   iLastPath;
    TBool                                       iRememberFolderSelection;
    TBool                                       iEnableToolbar;

    TBool                                       iSupportNetworkDrives;
    TBool                                       iBypassPlatformSecurity;
    TBool                                       iRemoveFileLocks;
    TBool                                       iIgnoreProtectionsAtts;
    TBool                                       iRemoveROMWriteProrection;
    };


class CFileBrowserModel : public CBase
	{
public:
	static CFileBrowserModel* NewL();
	~CFileBrowserModel();

private:
	CFileBrowserModel();
	void ConstructL();
    void LoadSettingsL();
    void GetHashKeySelectionStatus();
    void LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TInt& aValue);
    void LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TDes& aValue);
    void SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TInt& aValue);
    void SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TDes& aValue);

public:
    void ActivateModelL();
    void DeActivateModelL();
    void SaveSettingsL(TBool aNotifyModules=ETrue);
    void SetFileListContainer(CFileBrowserFileListContainer* aFileListContainer);
    TInt LaunchSettingsDialogL();
    inline TFileBrowserSettings& Settings() { return iSettings; }
    inline CEikonEnv* EikonEnv() { return iEnv; }
    inline RApaLsSession& LsSession() { return iLs; }
    inline CFileBrowserScreenCapture* ScreenCapture() { return iScreenCapture; }
    inline CFileBrowserFileUtils* FileUtils() { return iFileUtils; }
    inline CFileBrowserFileListContainer* FileListContainer() { return iFileListContainer; }
    inline TBool IsHashKeySelectionInUse() { return iIsHashKeySelectionInUse; }

private:
    CFileBrowserFileListContainer*  iFileListContainer;
    CFileBrowserScreenCapture*      iScreenCapture;
    CFileBrowserFileUtils*          iFileUtils;
    CEikonEnv*                      iEnv;
    TFileBrowserSettings            iSettings;
    RApaLsSession                   iLs;
    TBool                           iIsHashKeySelectionInUse; 
    };
   

#endif // __FILEBROWSER_MODEL_H__