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



#ifndef __SCREENGRABBER_MODEL_H__
#define __SCREENGRABBER_MODEL_H__

#include <e32std.h>
#include <e32base.h>

#include <CAknMemorySelectionSettingPage.h>
#include <eikenv.h>
#include <mdaaudiotoneplayer.h>
#include <bamdesca.h>
#include "SGStd.h"


#define TRANSPARENCY_INDEX 30
#define TRANSPARENCY_ALTERNATIVE_INDEX 24


// setting keys (do not change uids of existing keys to maintain compatibility to older versions!)
const TUid KSGSettingCaptureMode                            = { 0x00 };

const TUid KSGSettingSingleCaptureHotkey                    = { 0x01 };
const TUid KSGSettingSingleCaptureImageFormat               = { 0x02 };
const TUid KSGSettingSingleCaptureMemoryInUse               = { 0x03 };
const TUid KSGSettingSingleCaptureMemoryInUseMultiDrive     = { 0x04 };
const TUid KSGSettingSingleCaptureFileName                  = { 0x05 };

const TUid KSGSettingSequantialCaptureHotkey                = { 0x06 };
const TUid KSGSettingSequantialCaptureImageFormat           = { 0x07 };
const TUid KSGSettingSequantialCaptureDelay                 = { 0x08 };
const TUid KSGSettingSequantialCaptureMemoryInUse           = { 0x09 };
const TUid KSGSettingSequantialCaptureMemoryInUseMultiDrive = { 0x0A };
const TUid KSGSettingSequantialCaptureFileName              = { 0x0B };

const TUid KSGSettingVideoCaptureHotkey                     = { 0x0C };
const TUid KSGSettingVideoCaptureVideoFormat                = { 0x0D };
const TUid KSGSettingVideoCaptureMemoryInUse                = { 0x0E };
const TUid KSGSettingVideoCaptureMemoryInUseMultiDrive      = { 0x0F };
const TUid KSGSettingVideoCaptureFileName                   = { 0x10 };


class CImageEncoder;
class CFbsBitmap;
class CFrameImageData;
class CAknGlobalConfirmationQuery;
class CSavedQuery;
class CDictionaryFileStore;
class CMGXFileManager;
class CMGXFileNotificationHandler;

class TGrabSettings
    {
public:
    TInt                                        iCaptureMode;
    
    TInt                                        iSingleCaptureHotkey;
    TInt                                        iSingleCaptureImageFormat;
    CAknMemorySelectionSettingPage::TMemory     iSingleCaptureMemoryInUse; 
    TDriveNumber                                iSingleCaptureMemoryInUseMultiDrive;
    TFileName                                   iSingleCaptureFileName;

    TInt                                        iSequantialCaptureHotkey;
    TInt                                        iSequantialCaptureImageFormat;
    TInt                                        iSequantialCaptureDelay;
    CAknMemorySelectionSettingPage::TMemory     iSequantialCaptureMemoryInUse; 
    TDriveNumber                                iSequantialCaptureMemoryInUseMultiDrive;
    TFileName                                   iSequantialCaptureFileName;

    TInt                                        iVideoCaptureHotkey;
    TInt                                        iVideoCaptureVideoFormat;
    CAknMemorySelectionSettingPage::TMemory     iVideoCaptureMemoryInUse; 
    TDriveNumber                                iVideoCaptureMemoryInUseMultiDrive;
    TFileName                                   iVideoCaptureFileName;
    };


class TVideoFrame
    {
public:
    TUint       iWidth;
    TUint       iHeight;
    TUint       iXPos;
    TUint       iYPos;
    TUint       iDelay; // 100 = 1 sec
    TBool       iEnableTransparency;
    TBool       iFillsWholeScreen;
    TFileName   iFileStorePath;
    };


typedef CArrayFixSeg<TVideoFrame> CVideoFrameArray;


class CScreenGrabberModel : public CActive, public MMdaAudioToneObserver
	{
	// active object states
	enum TState 
		{
		EIdle = 0,              // do nothing
		EEncodingImage,         // ICL encoding of an image
		ESequenceDelay,         // waiting till next capture
		ECancelCapturing,       // asking to cancel capturing
		ENextVideoFrame,        // get next frame for video
		ECancelVideoCapturing,  // asking to cancel video capturing  
		EQueryDelay             // waiting till to remove query
		};

public:
	static CScreenGrabberModel* NewL();
	~CScreenGrabberModel();
	void ActivateModelL();
	void DeActivateModelL();

private:
	CScreenGrabberModel();
	void ConstructL();

	void RunL();
	void DoCancel();

    void LoadSettingsL();
    void TakeScreenShotAndSaveL();
    void CaptureFrameForVideoL();
    void SaveVideoL(TInt aErr);
    void CapturingFinishedL(TInt aErr);
    void CleanTemporaryFilesL();
    TBool MemoryCardOK();
    TBool DriveOK(TDriveNumber aNumber);
    void PlayBeepSound();
    void LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TInt& aValue);
    void LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TDes& aValue);
    void SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TInt& aValue);
    void SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TDes& aValue);
    TInt UpdateFileToGallery(const TDesC& aFullPath);

public:
    inline TGrabSettings GrabSettings() { return iGrabSettings; }
    void SaveSettingsL(TGrabSettings aGrabSettings);
    void ActivateCaptureKeysL(TBool aChangeKey=EFalse);
    void CancelCapturing();
    TBool HandleCaptureCommandsL(const TWsEvent& aEvent);
    void MatoPrepareComplete(TInt aError);
    void MatoPlayComplete(TInt aError);

private:
    CMdaAudioToneUtility*   iMdaAudioToneUtility;       // a tone player
    TGrabSettings           iGrabSettings;              // a simple class to store the settings used in the screen capture
    CEikonEnv*              iEnv;                       // pointer to our eikon environment
	RWindowGroup            iRootWin;                   // application's window group
	TInt                    iCapturedKey;               // a handle to the key which is captured
	TInt                    iCapturedKeyUnD;            // a handle to the key which is captured, up and down
	TInt                    iCapturedKeyHash;           // a handle to the hash key which is captured
	TInt                    iCapturedKeyHashUnD;        // a handle to the hash key which is captured, up and down
	CImageEncoder*          iImageEncoder;              // engine from ICL to do the image conversion
    CFbsBitmap*             iPreviouslyCapturedBitmap;  // a previously captured bitmap
	TState                  iState;                     // state of this active object
    TFileName               iSaveFileName;              // target filename of the taken screen shot
    CFrameImageData*        iFrameImageData;            // frame and image block data needed by ICL
    CSavedQuery*            iSavedQuery;                // a class to display global queries
	RTimer                  iTimer;                     // a timer for async delays
    TBool 				    iCapturingInProgress;   	// boolean to check if capturing is in progress
    TBool                   iStopCapturing;             // boolean to specify if to ask capturing
    TUint                   iNumberOfTakenShots;        // a number of the taken screen shots in the sequantial mode
    TUint                   iCurrentFrameNumber;        // a number of taken frames in video mode
    TTime                   iPreviousFrameTaken;        // time when the last frame was taken
    TSize                   iVideoDimensions;           // dimension of the video
    TSize                   iPreviousFrameScreenDimension; // dimension of the screen in previous frame
    CVideoFrameArray*       iVideoFrameArray;           // the frames of the video
    TBool                   iHashKeyCapturingActivated; // captures hash key is activated
    TBool                   iHashKeyDown;               // stores state if hash key is currently down
    TBool                   iGalleryUpdaterSupported;   // specifies if it is possible to update the file to gallery
    TBool                   iGalleryUpdaterInitialized; // specifies if gallery updater has been initialized or not
    RLibrary                iGalleryUpdaterDLL;         // specifies pointer to Media Gallery File API DLL
    CMGXFileManager*        iMGXFileManager;            // Media Gallery FileManager instance
    };


class CSavedQuery : public CActive
    {
public:
    static CSavedQuery* NewL();
    ~CSavedQuery();
    void DisplayL(const TDesC &aText, TBool aDisplayErrorNote=EFalse);

private:
    void DoCancel();
    void RunL();

private: 
    CSavedQuery();
    void ConstructL();
    void DisplayWithGraphicsL(const TDesC &aText, TBool aDisplayErrorNote);
    void DisplayWithoutGraphicsL(const TDesC &aText);

private:
    CAknGlobalConfirmationQuery*    iGlobalConfirmationQuery;
    TBool                           iVisible;
    };


// these classes have been copied Media Gallery File API

class MGXFileManagerFactory
    {
    public:
        static CMGXFileManager* NewFileManagerL( RFs& aFs );
        static 
            CMGXFileNotificationHandler* NewFileNotificationHandlerL();
    };

class CMGXFileManager : public CBase
    {
    public:
        virtual TBool SuccessFileNameL( const TDesC& aFileName,
                                        TDriveNumber aDrive ) = 0;
        virtual void UpdateL() = 0;
        virtual void UpdateL( const TDesC& aFullPath ) = 0;
        virtual void UpdateL( const TDesC& aOldName,
                              const TDesC& aNewName ) = 0;
        virtual void UpdateL( const MDesCArray& aFileNameArray ) = 0;
    };


#endif // __SCREENGRABBER_MODEL_H__