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

#ifndef __LAUNCHER_ENGINE_H__
#define __LAUNCHER_ENGINE_H__

#include "launcher.hrh"
#include "launcherparserobserver.h"

#include <aknappui.h>
#include <e32base.h>
#include <apgcli.h>
#include "../symbian_version.hrh"
#if ( SYMBIAN_VERSION_SUPPORT < SYMBIAN_4 )
#include <sendui.h>
#endif 
#include <AknProgressDialog.h>
#include <MAknFileSelectionObserver.h>

class CLauncherAppUi;
class CLauncherContainerApps;
class CLauncherContainerOutput;

class CLauncherXMLParser;
class CLauncherDLLParser;

class CAppThreadChecker;
class CAppRunningChecker;

class CAknGlobalNote;


class CLauncherEngine : public CActive, public MLauncherParserObserver, public MProgressDialogCallback
	{
public:
	static CLauncherEngine* NewL(CLauncherAppUi* aAppUi);
	~CLauncherEngine();

public:
    void SetContainerApps(CLauncherContainerApps* aContainerApps);
    void SetContainerOutput(CLauncherContainerOutput* aContainerOutput);

    CDesCArray* ListOfAllAppsL();
    void StartAppLaunchingL(const CArrayFix<TInt>* aSelectedApps, TBool aAutoClose);
    void CheckWhyThreadDiedL();
    void CheckIfAppIsRunningL();
    TInt DeleteLogFile();
    TInt DeleteBCLogFile();
    TBool LogFileExists();
    TBool BCLogFileExists();
    void AnalyseDLLsL();
    void StopLaunchingL();
	#if ( SYMBIAN_VERSION_SUPPORT < SYMBIAN_4 )
    void SendLogViaSendUiL(CSendUi* aSendUi);
    void SendListOfSystemDllsViaSendUiL(CSendUi* aSendUi);
	#endif 
    TBool SelectRequiredDLLsFileL();
        
    // From MLauncherParserObserver
    void ElementParsedL(const CLauncherDLLElement& aDllElement);
    void DocumentParsedL(TInt aErrorCode);
    void ParsingProgressedL(TInt aBytes);
    
    // From MProgressDialogCallback
    void DialogDismissedL( TInt aButtonId );    
    
private:
	CLauncherEngine();
	void ConstructL(CLauncherAppUi* aAppUi);

	void RunL();
	void DoCancel();

	void IssueLaunch();
	void LaunchApplicationL();
	void DoLaunchApplicationL();
		
    void CheckForMoreAppsL();
    void WriteInitialStuffToTheLogL(const TDesC& aOwnData, RFile& aFile);
    TInt FindFilesRecursiveL(const TDesC& aFileName, const TDesC& aPath);
    TInt FindFiles(const TDesC& aFileName, const TDesC& aPath);
    TInt ReadLineFromFileL(RFile& aFile, TDes& aReadBuf);
    void ChangeFocusToOutputView();
    
    /**
     * Performs binary compatibility analysis based on reference data given
     * in XML-file.
     */
    void DoBCAnalysisL();
    
    /**
     * Compares given DLL list to system DLLs
     */
    void DoCompareDLLListsL();
    
    /**
     * IsCurrentThreadOpen
     */
    TBool IsCurrentThreadOpen() const;
        
    CDesCArray* DependencyCheckForE32ImageL();
    
private:
	RTimer          iTimer;
	CEikonEnv*      iEnv;
	RApaLsSession   iLs;
    RWsSession      iWs;
    HBufC8*         iLogWriteBuf;    
    RFile           iLogFile;
    RFile           iBCLogFile;  
    TFileName       iLogFilePath;
    TFileName       iBCLogFilePath;
    TFileName       iSystemDllsFilePath;
    TFileName       iRequiredDllsFilePath;        
    
    CDesCArray*     iAppsArray;
    CDesCArray*     iAllAppsArray;
    CDesCArray*     iSystemDllArray;
    CDesCArray*     iRequiredDllArray;

    TBool           iLaunchingIsActive;
    TBool           iDLLAnalysisIsActive;
    TBool           iSkipHiddenAndEmbedOnly;
    TBool           iAutoClose;
    TInt            iAppLaunchCounter;
    TInt            iFailedCases;
    TInt            iOkCases;
    TInt            iSkippedCases;
    TInt            iTotalNumberOfCases;

    TUid            iCurrentAppUid;
    TThreadId       iCurrentAppThreadId;
    TBuf<128>       iCurrentAppNameAndExt;
    RThread         iCurrentAppThread;

    CAppThreadChecker*          iAppThreadChecker;
    CAppRunningChecker*         iAppRunningChecker;

    CLauncherAppUi*             iAppUi;
    CLauncherContainerApps*     iContainerApps;
    CLauncherContainerOutput*   iContainerOutput;
    
    CLauncherXMLParser*     iXMLParser;
    CLauncherDLLParser*     iDLLParser;
    CLauncherDLLElement*    iDLLElement;
    
    CEikProgressInfo*       iProgressInfo;
    CAknProgressDialog*     iWaitDialog;
    TInt                    iWaitDialogId;    
    TInt                iFoundBCIssues;
    HBufC*              iBCIssuesBigBuffer;

public:
    inline TBool LaunchingIsActive()                   { return iLaunchingIsActive; };
    inline TBool SkipHiddenAndEmbedOnly()              { return iSkipHiddenAndEmbedOnly; };
    inline void SetSkipHiddenAndEmbedOnly(TBool aSkip) { iSkipHiddenAndEmbedOnly = aSkip; };
    inline CAppThreadChecker*  AppThreadChecker()      { return iAppThreadChecker; };
    inline CAppRunningChecker* AppRunningChecker()     { return iAppRunningChecker; };
	};


class CAppRunningChecker : public CActive
	{
public:
	static CAppRunningChecker* NewL(CLauncherEngine* aLauncherEngine);
	~CAppRunningChecker();

	void StartTesting();

private:
	CAppRunningChecker();
	void ConstructL(CLauncherEngine* aLauncherEngine);

	void RunL();
	void DoCancel();

private:
	RTimer iTimer;
	CEikonEnv* iEnv;
    CLauncherEngine* iLauncherEngine;
	};


class CAppThreadChecker : public CActive
	{
public:
	static CAppThreadChecker* NewL(CLauncherEngine* aLauncherEngine);
	~CAppThreadChecker();

	void ActivateChecking();

private:
	CAppThreadChecker();
	void ConstructL(CLauncherEngine* aLauncherEngine);

	void RunL();
	void DoCancel();

private:
	CEikonEnv* iEnv;
    CLauncherEngine* iLauncherEngine;
	};

#endif // __LAUNCHER_ENGINE_H__
