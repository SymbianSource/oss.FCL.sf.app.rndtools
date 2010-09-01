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


#ifndef FILEBROWSER_FILEUTILS_H
#define FILEBROWSER_FILEUTILS_H

// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include <w32std.h>
#include <badesca.h>
#include <coedef.h>
#include <AknServerApp.h>
#include <msvapi.h>
#include <AknProgressDialog.h> 
#include <tz.h>

_LIT(KIRAppPath, "z:\\sys\\bin\\irapp.exe");
_LIT(KBTAppPath, "z:\\sys\\bin\\btui.exe");
_LIT(KUSBAppPath, "z:\\sys\\bin\\usbclasschangeui.exe");
_LIT(KErrRdPath, "c:\\resource\\ErrRd");
_LIT(KErrRdDir, "c:\\resource\\");


// FORWARD DECLARATIONS
class CFileBrowserModel;
class CFileBrowserFileOps;
class CAknIconArray;
class TAknsItemID;
class CDocumentHandler;
class CAknWaitDialog;
class CAknProgressDialog;
class CEikProgressInfo;
class CFBFileOpClient;
class CAknProgressDialog;
class CEikProgressInfo;
class CAknOpenFileService;
class CMessageDigest;

// CLASS DECLARATIONS

class TSearchAttributes
	{
public:
    TFileName       iSearchDir;
    TFileName       iWildCards;
    TFileName       iTextInFile;
    TUint           iMinSize;
    TUint           iMaxSize;
    TTime           iMinDate;
    TTime           iMaxDate;
    TBool           iRecurse;
	};

class TDriveEntry
	{
public:
    TChar           iLetter;
    TInt            iNumber;
    TVolumeInfo     iVolumeInfo;
    TBuf<64>        iMediaTypeDesc;
    TBuf<128>       iAttributesDesc;
    TInt            iIconId;
	};

class TFileEntry
	{
public:
    TFileName       iPath;
    TEntry          iEntry;
    TInt            iDirEntries;
    TInt            iIconId;
	};

class TAppIcon
	{
public:
    TInt            iId;
    TUid            iUid;
	};

typedef CArrayFixSeg<TDriveEntry> CDriveEntryList;
typedef CArrayFixSeg<TFileEntry> CFileEntryList;
typedef CArrayFixSeg<TAppIcon> CAppIconList;


class CCommandParamsBase : public CBase
    {
    };

class CCommandParamsAttribs : public CCommandParamsBase
    {
public:
    TFileEntry iSourceEntry;
    TUint iSetMask;
    TUint iClearMask;
    TTime iTime;
    TUint iSwitch;
public:
    CCommandParamsAttribs(const TFileEntry& aSourceEntry, TUint aSetMask, TUint aClearMask, const TTime& aTime, TUint aSwitch) : iSourceEntry(aSourceEntry), iSetMask(aSetMask), iClearMask(aClearMask), iTime(aTime), iSwitch(aSwitch) {}
    };
    
class CCommandParamsCopyOrMove : public CCommandParamsBase
    {
public:
    TFileEntry iSourceEntry;
    TFileName iTargetPath;
    TUint iSwitch;
public:
    CCommandParamsCopyOrMove(const TFileEntry& aSourceEntry, const TDesC& aTargetPath, TUint aSwitch) : iSourceEntry(aSourceEntry), iTargetPath(aTargetPath), iSwitch(aSwitch) {}
    };
        
class CCommandParamsRename : public CCommandParamsBase
    {
public:
    TFileEntry iSourceEntry;
    TFileName iTargetPath;
    TUint iSwitch;
public:
    CCommandParamsRename(const TFileEntry& aSourceEntry, const TDesC& aTargetPath, TUint aSwitch) : iSourceEntry(aSourceEntry), iTargetPath(aTargetPath), iSwitch(aSwitch) {}
    };

class CCommandParamsDelete : public CCommandParamsBase
    {
public:
    TFileEntry iSourceEntry;
    TUint iSwitch;
public:
    CCommandParamsDelete(const TFileEntry& aSourceEntry, TUint aSwitch) : iSourceEntry(aSourceEntry), iSwitch(aSwitch) {}
    };

class CCommandParamsDriveSnapShot : public CCommandParamsBase
    {
public:
    TInt iSourceDriveLetter;
    TInt iTargetDriveLetter;
public:
    CCommandParamsDriveSnapShot(TChar aSourceDriveLetter, TChar aTargetDriveLetter) : iSourceDriveLetter(aSourceDriveLetter), iTargetDriveLetter(aTargetDriveLetter) {}
    };
    
class TCommand
    {
public:
    TInt iCommandId;
    CCommandParamsBase* iParameters;
public:
    TCommand(TInt aCommandId, CCommandParamsBase* aParameters) : iCommandId(aCommandId), iParameters(aParameters) {}
    };

typedef CArrayFixSeg<TCommand> CCommandArray;



class CFileBrowserFileUtils : public CActive, public MAknServerAppExitObserver, public MMsvSessionObserver, public MProgressDialogCallback 
	{
private:
    enum TState // active object states
    	{
    	EIdle = 0,              // do nothing
    	};

    enum TClipBoardMode
    	{
    	EClipBoardModeCut = 0,
    	EClipBoardModeCopy
    	};

    enum TListingMode
    	{
    	ENormalEntries = 0,
    	ESearchResults,
    	EOpenFiles,
    	EMsgAttachmentsInbox,
    	EMsgAttachmentsDrafts,
    	EMsgAttachmentsSentItems,
    	EMsgAttachmentsOutbox
    	};
    	    	
public:
	static CFileBrowserFileUtils* NewL(CFileBrowserModel* aModel);
	~CFileBrowserFileUtils();

private:
	CFileBrowserFileUtils(CFileBrowserModel* aModel);
	void ConstructL();

private: // from CActive
	void RunL();
    TInt RunError(TInt aError);
	void DoCancel();

private: // from MAknServerAppExitObserver
    void HandleServerAppExit(TInt aReason);

private: // from MMsvSessionObserver
    void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3);

private:  //from MProgressDialogCallback
    void DialogDismissedL(TInt aButtonId);  
    
private: // command handling
    void StartExecutingCommandsL(const TDesC& aLabel);
    void ExecuteCommand();
    void CheckForMoreCommandsL();
    void AppendToCommandArrayL(TInt aCommand, CCommandParamsBase* aParameters);
    TInt CommandArrayCount() const;
    void ResetCommandArray();       

private: // misc functionality
    void GenerateDirectoryDataL();
    void GetDriveListL();
    void GetDirectoryListingL();
    CAknIconArray* GenerateIconArrayL(TBool aGenerateNewBasicIconArray=EFalse);
    void AppendGulIconToIconArrayL(CAknIconArray* aIconArray, const TDesC& aIconFile, TInt aIconId, TInt aMaskId, const TAknsItemID aAknsItemId);
    TInt AppIconIdForUid(TUid aUid);
    TUid GetAppUid(TFileEntry aFileEntry);
    CDesCArray* GenerateItemTextArrayL();
    TInt GetSelectedItemsOrCurrentItemL(CFileEntryList* aFileEntryList);
    void DoCopyToFolderL(CFileEntryList* aEntryList, const TDesC& aTargetDir, TBool aDeleteSource);
    TInt DoSearchFiles(const TDesC& aFileName, const TDesC& aPath);
    TInt DoSearchFilesRecursiveL(const TDesC& aFileName, const TDesC& aPath);
    TInt DoFindFiles(const TDesC& aFileName, const TDesC& aPath);
    TInt DoFindFilesRecursiveL(const TDesC& aFileName, const TDesC& aPath);
    void ReadAttachmentPathsRecursiveL(CMsvSession* aSession, CMsvEntry* aContext, CDesCArray* aAttPaths);
    void WriteMessageEntryInfoRecursiveL(CMsvSession* aSession, CMsvEntry* aContext, RFile& aFile, TInt& aLevel);
    void DoWriteMessageEntryInfoL(CMsvEntry* aContext, RFile& aFile, TInt aLevel);
    void ConvertCharsToPwd(TDesC& aWord, TDes8& aConverted) const;
    HBufC8* MessageDigestInHexLC(CMessageDigest* aMD, RFile& aFile);
    void OpenCommonFileActionQueryL();
    
public: // public interfaces
    TKeyResponse HandleOfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
    void HandleSettingsChangeL();
    void SetSortModeL(TInt aSortMode);
    void SetOrderModeL(TInt aOrderMode);
    void RefreshViewL();
    TBool IsCurrentDriveReadOnly();
    TBool IsCurrentItemDirectory();
    void MoveUpOneLevelL();
    void MoveDownToDirectoryL();
    void ClipboardCutL();
    void ClipboardCopyL();
    void ClipboardPasteL();
    void CopyToFolderL(TBool aMove=EFalse);
    void DeleteL();
    void TouchL();
    void RenameL();
    void SetAttributesL();
    void SearchL();
    void NewFileL();
    void NewDirectoryL();
    void SendToL();
    void CompressL();
    void DecompressL();
    void PropertiesL();
    void OpenWithApparcL();
    void OpenWithDocHandlerL(TBool aEmbed);
    void OpenWithFileServiceL();
    TBool FileExists(const TDesC& aPath);
    TInt LaunchProgramL(const TDesC& aPath);
    void MemoryInfoPopupL();
    void ShowFileCheckSumsL(TInt aType);
    void SetErrRdL(TBool aEnable);
    void EnableAvkonIconCacheL(TBool aEnable);
    void SimulateLeaveL();
    void SimulatePanicL();
    void SimulateExceptionL();
    void SetDebugMaskL();
    void WriteAllAppsL();
    void WriteAllFilesL();
    void ListOpenFilesL();
    void ListMessageAttachmentsL(TInt aType);
    void WriteMsgStoreWalkL();
    void FileEditorL(TInt aType);
    void SetDrivePasswordL();
    void UnlockDriveL();
    void ClearDrivePasswordL();
    void EraseDrivePasswordL();
    void FormatDriveL(TBool aQuickFormat);
    void CheckDiskL();
    void ScanDriveL();
    void SetDriveNameL();
    void SetDriveVolumeLabelL();
    void EjectDriveL();
    void DismountFileSystemL();
    void EraseMBRL();
    void PartitionDriveL();
    TBool DriveSnapShotPossible();
    void DriveSnapShotL();
    void EditDataTypesL();
    void SecureBackupL(TInt aType);
    
public:    
    inline TInt SortMode() { return iSortMode; }
    inline TInt OrderMode() { return iOrderMode; }
    inline CFileEntryList* ClipBoardList() { return iClipBoardList; }
    inline TBool IsDriveListViewActive() { return iCurrentPath==KNullDesC && iListingMode==ENormalEntries; }
    inline TBool IsNormalModeActive() { return iListingMode==ENormalEntries; }

private:
	TState                          iState;
    CFileBrowserModel*              iModel;
    CFileBrowserFileOps*            iFileOps;
    CAknWaitDialog*                 iWaitDialog;
    CAknProgressDialog*             iProgressDialog;
    CEikProgressInfo*               iProgressInfo;
    CCommandArray*                  iCommandArray;
    TInt                            iCurrentEntry;
    TInt                            iSucceededOperations;
    TInt                            iFailedOperations;
    TInt                            iLastError;
    RTimer                          iTimer;
	RFs                             iFs;
	TListingMode                    iListingMode;
	CFileMan*                       iFileMan;
	TInt                            iViewMode;
    TFileName                       iCurrentPath;
    TInt                            iSortMode;
    TInt                            iOrderMode;
    TInt                            iClipboardMode;
    CDesCArray*                     iClipboardPaths;
    CDriveEntryList*                iDriveEntryList;
    CFileEntryList*                 iFileEntryList;
    CFileEntryList*                 iFindFileEntryList;
    CAppIconList*                   iAppIconList;
    TClipBoardMode                  iClipBoardMode;
    CFileEntryList*                 iClipBoardList;
    TSearchAttributes               iSearchAttributes;
    CDocumentHandler*               iDocHandler;
    CAknOpenFileService*		    iOpenFileService;
    RFile                           iMsgStoreWalkFile;
    TInt                            iPrevFolderIndex;
    TFileName                       iPrevFolderName;
    RTz                             iTz;
    };


// utility class for waiting for asychronous requests
class CAsyncWaiter : public CActive
	{
public:
	static CAsyncWaiter* NewL( TInt aPriority = EPriorityStandard );
	static CAsyncWaiter* NewLC( TInt aPriority = EPriorityStandard );
	~CAsyncWaiter();
	
	void StartAndWait();
	TInt Result() const;
	
private:
	CAsyncWaiter( TInt aPriority );
	
	// from CActive
	void RunL();
	void DoCancel();
	
private:
    CActiveSchedulerWait iWait;
	TInt iError;
	};
	
	
#endif

// End of File

