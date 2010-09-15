/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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




#include <creator.rsg>

#include "creator.pan"
#include "creator_appui.h"
#include "creator_container.h"
#include "creator.hrh"

#include <bautils.h>
#include <pathinfo.h> 
#include <aknmessagequerydialog.h> 
#include <eikmenub.h>
#include <akntitle.h>
#include <akncontext.h>
#include <aknnotewrappers.h>
#include <avkon.hrh>

// delay before script running and application exiting in seconds
// when script is run from command line
const TInt KCommandLineDelay = 3;

// ConstructL is called by the application framework
void CCreatorAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin);

    iAppView = CCreatorAppView::NewL(ClientRect());    

    AddToStackL(iAppView);

    iEnv=CEikonEnv::Static();

    iEngine = CCreatorEngine::NewL(this);
 
	iMode = ETimerModeNone;
	iTickCount = 0;
    iTimer = CHeartbeat::NewL(0);
    
    // parse script name and random data file name from command line
    iCommandLineScriptName = HBufC::NewL(User::CommandLineLength());
    iCommandLineRandomDataFileName = HBufC::NewL(User::CommandLineLength());
    HBufC* commandLine = HBufC::NewLC(User::CommandLineLength());
    TPtr16 ptr = commandLine->Des();
    User::CommandLine(ptr);
    commandLine->Des().Trim();
    
    TInt pos = commandLine->Des().Find(_L(" "));
    if (pos != KErrNotFound)
    	{
    	iCommandLineScriptName->Des().Copy(commandLine->Des().Left(pos));
    	iCommandLineRandomDataFileName->Des().Copy(commandLine->Des().Mid(pos));
    	iCommandLineRandomDataFileName->Des().Trim();
    	}
    else
    	{
    	iCommandLineScriptName->Des().Copy(commandLine->Des());
    	iCommandLineRandomDataFileName->Des().Copy(KNullDesC);
    	}

    CleanupStack::PopAndDestroy(commandLine);
    
    if (iCommandLineScriptName->Des().Length() > 0)
    	{
    	iMode = ETimerModeStartScript;
    	iTimer->Start(ETwelveOClock, this);
    	}
    }


CCreatorAppUi::CCreatorAppUi()                              
    {
    // add any construction that cannot leave here
    }


CCreatorAppUi::~CCreatorAppUi()
    {

    delete iCommandLineRandomDataFileName;
    delete iCommandLineScriptName;
    
    if (iTimer)
    	{
    	iTimer->Cancel();
        delete iTimer;
        iTimer = NULL;
    	}
    
    delete iEngine;

	if (iAppView)
        {
		RemoveFromStack(iAppView);
        delete iAppView;
        iAppView = NULL;
        }
    }


// handle any menu commands
void CCreatorAppUi::HandleCommandL(TInt aCommand)
    {
    switch(aCommand)
        {
        // main menus, not assigned to any commands
	    case ECmdCreateCalendarEntries:
	    case ECmdCreateBrowserEntries:
	    case ECmdCreateFileEntries:
	    case ECmdCreateLogEntries:
	    case ECmdCreateMessagingEntries:
	    case ECmdCreateMiscEntries:
	    case ECmdCreatePhoneBookEntries:
            {
            User::Panic (_L("Unused command"), 102);
            }
            break;

        // about
	    case ECmdAboutCreator:
            {
	        CAknMessageQueryDialog* dialog = new (ELeave) CAknMessageQueryDialog;
            dialog->ExecuteLD(R_CREATOR_ABOUT_DIALOG);
            }
            break;


        // run script
        case ECmdCreateFromFile:
            {
            iEngine->AsyncRunScript();
			}
			break;

        // select random data file
        case ECmdSelectRandomDataFile:
            {
            TFileName filename;
            if (iEngine->GetRandomDataFilenameL(filename))
            	{
            	iEngine->GetRandomDataFromFileL(filename);
            	}
			}
			break;


        // commands:
	    case ECmdCreateCalendarEntryAppointments:
	    case ECmdCreateCalendarEntryEvents:
	    case ECmdCreateCalendarEntryAnniversaries:
	    case ECmdCreateCalendarEntryToDos:
	    case ECmdCreateCalendarEntryReminders:
        
        case ECmdCreateBrowserBookmarkEntries:
	    case ECmdCreateBrowserBookmarkFolderEntries:
	    case ECmdCreateBrowserSavedPageEntries:
	    case ECmdCreateBrowserSavedPageFolderEntries:

	    case ECmdCreatePhoneBookEntryContacts:
	    case ECmdCreatePhoneBookEntryGroups:

   	    case ECmdCreateMiscEntryNotes:
   	    case ECmdCreateMiscEntryLandmarks:

	    case ECmdCreateLogEntryMissedCalls:
	    case ECmdCreateLogEntryReceivedCalls:
	    case ECmdCreateLogEntryDialledNumbers:

	    case ECmdCreateMiscEntryAccessPoints:
  	    case ECmdCreateMiscEntryIMPSServers:

	    case ECmdCreateMessagingEntryMailboxes:
	    case ECmdCreateMessagingEntryMessages:

	    case ECmdCreateFileEntryEmptyFolder:
	    case ECmdCreateFileEntry3GPP_70kB:
	    case ECmdCreateFileEntryAAC_100kB:
	    case ECmdCreateFileEntryAMR_20kB:
	    case ECmdCreateFileEntryBMP_25kB:
	    case ECmdCreateFileEntryDeck_1kB:
	    case ECmdCreateFileEntryDOC_20kB:
	    case ECmdCreateFileEntryGIF_2kB:
	    case ECmdCreateFileEntryHTML_20kB:
	    case ECmdCreateFileEntryJAD_1kB:
	    case ECmdCreateFileEntryJAR_10kB:
	    case ECmdCreateFileEntryJP2_65kB:
	    case ECmdCreateFileEntryJPEG_200kB:
	    case ECmdCreateFileEntryJPEG_25kB:
	    case ECmdCreateFileEntryJPEG_500kB:
	    case ECmdCreateFileEntryMIDI_10kB:
	    case ECmdCreateFileEntryMP3_250kB:
	    case ECmdCreateFileEntryMP4_200kB:
	    case ECmdCreateFileEntryMXMF_40kB:
	    case ECmdCreateFileEntryPNG_15kB:
	    case ECmdCreateFileEntryPPT_40kB:
	    case ECmdCreateFileEntryRAM_1kB:
	    case ECmdCreateFileEntryRM_95kB:
	    case ECmdCreateFileEntryRNG_1kB:
	    case ECmdCreateFileEntrySVG_15kB:
	    case ECmdCreateFileEntrySWF_15kB:
	    case ECmdCreateFileEntryTIF_25kB:
	    case ECmdCreateFileEntryTXT_10kB:
	    case ECmdCreateFileEntryTXT_70kB:
	    case ECmdCreateFileEntryVCF_1kB:
	    case ECmdCreateFileEntryVCS_1kB:
	    case ECmdCreateFileEntryWAV_20kB:
	    case ECmdCreateFileEntryXLS_15kB:
	    case ECmdCreateFileEntrySISX_10kB:
	    case ECmdCreateFileEntryWMA_50kB:
	    case ECmdCreateFileEntryWMV_200kB:
	    case ECmdDeleteAllEntries:
	    case ECmdDeleteAllCreatorEntries:
	    case ECmdDeleteContacts:
        case ECmdDeleteCreatorContacts:
        case ECmdDeleteContactGroups:
        case ECmdDeleteCreatorContactGroups:
        case ECmdDeleteCalendarEntries:
        case ECmdDeleteCreatorCalendarEntries:
        case ECmdDeleteBrowserBookmarks:
        case ECmdDeleteCreatorBrowserBookmarks:
        case ECmdDeleteBrowserBookmarkFolders:
        case ECmdDeleteCreatorBrowserBookmarkFolders:
        case ECmdDeleteBrowserSavedPages:
        case ECmdDeleteCreatorBrowserSavedPages:
        case ECmdDeleteBrowserSavedPageFolders:
        case ECmdDeleteCreatorBrowserSavedPageFolders:
        case ECmdDeleteCreatorFiles:
        case ECmdDeleteLogs:
        case ECmdDeleteCreatorLogs:
        case ECmdDeleteMessages:
        case ECmdDeleteCreatorMessages:
        case ECmdDeleteMailboxes:
        case ECmdDeleteCreatorMailboxes:
        case ECmdDeleteIAPs:
        case ECmdDeleteCreatorIAPs:
        case ECmdDeleteIMPSs:
        case ECmdDeleteCreatorIMPSs:
        case ECmdDeleteNotes:
        case ECmdDeleteLandmarks:
        case ECmdDeleteCreatorLandmarks:
            {
            iEngine->ExecuteOptionsMenuCommandL(aCommand);
            }
            break;

        case ECmdDeleteEntries:
        case ECmdCreatePhoneBookEntrySubscribedContacts:
	    case ECmdCreateMessagingEntryCBSTopics:
	    case ECmdCreateMessagingEntrySMSMessageCenters:
	    case ECmdCreateMessagingEntryTemplates:
	    case ECmdCreateMiscEntryAppMenuFolders:
	    case ECmdCreateMiscEntrySyncSettings:
        case ECmdCreateMessagingEntryDocumentsFolders:
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("Please contact S60 CO if you need this feature..."));
            }
            break;

		case EEikCmdExit:
		case EAknSoftkeyExit:
			{
			Exit();
			}
            break;

        default:
            User::Panic (_L("Invalid command"), 101);
            break;
        }
    }


void CCreatorAppUi::DynInitMenuPaneL( TInt /*aResourceId*/, CEikMenuPane* /*aMenuPane*/ )
	{
	}

void CCreatorAppUi::RunScriptDone()
	{
	if (iMode == ETimerModeStartScript)
		{
		iMode = ETimerModeExitAppUi;
		iTimer->Start(ETwelveOClock, this);
		}
	}

void CCreatorAppUi::Beat()
	{
	Tick();
	}

void CCreatorAppUi::Synchronize()
	{
	Tick();
	}

void CCreatorAppUi::Tick()
	{
	iTickCount++;
	if (iTickCount > KCommandLineDelay)
		{
		iTimer->Cancel();
		iTickCount = 0;
		if (iMode == ETimerModeStartScript)
			{
        	iEngine->GetRandomDataFromFileL(*iCommandLineRandomDataFileName);
			TInt status = iEngine->RunScriptL(*iCommandLineScriptName);
			if (status != KErrNone)
				{
				TBuf<128> noteMsg;
				if (status == KErrNotFound)
					{
					_LIT(KMessage1, "Error in opening script file.");
					noteMsg.Copy(KMessage1);
					}
				else if (status == KErrCorrupt)
					{
					_LIT(KMessage2, "Error in parsing script file.");
					noteMsg.Copy(KMessage2);
					}
				else if (status == KErrCompletion)
					{
					_LIT(KMessage3, "Script file does not contain any elements.");
					noteMsg.Copy(KMessage3);
					}
				else
					{
					_LIT(KMessage4, "Unknown error in opening script file.");
					noteMsg.Copy(KMessage4);
					}
				CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
				note->ExecuteLD(noteMsg);
				RunScriptDone();
				}
			}
		else if (iMode == ETimerModeExitAppUi)
	    	{
	    	iMode = ETimerModeNone;
	    	Exit();
	    	}
		}
	}
