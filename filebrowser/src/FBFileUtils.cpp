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
#include "FBFileUtils.h"
#include "FBFileOps.h"
#include "FBModel.h"
#include "FBFileDlgs.h"
#include "FBFileEditor.h"
#include "FBFileListContainer.h"
#include "FB.hrh"
#include <filebrowser.rsg>

#include <textresolver.h> 
#include <AknWaitDialog.h>
#include <AknProgressDialog.h>
#include <eikprogi.h>
#include <f32file.h>
#include <eikfutil.h>
#include <apparc.h>
#include <eikenv.h>
#include <bautils.h>
#include <AknIconArray.h>
#include <gulicon.h>
#include <AknsUtils.h>
#include <AknsItemID.h>
#include <aknconsts.h>
#include <aknnotewrappers.h>
#include <AknProgressDialog.h> 
#include <aknPopup.h>
#include <aknmessagequerydialog.h>
#include <AknGlobalNote.h>
#include <babackup.h> 
#include <avkon.mbg>
#include <aknmemorycardui.mbg>
#include <sendui.h>
#include <CMessageData.h>
#include <ezgzip.h>
#include <e32hal.h>
#include <pathinfo.h>
#include <apgcli.h>
#include <apmstd.h>
#include <apgtask.h>
#include <apacmdln.h>
#include <DocumentHandler.h>
#include <AknOpenFileService.h>
#include <msvapi.h>
#include <msvids.h>
#include <msvstd.h>
#include <msvstore.h>
#include <mmsvattachmentmanager.h>
#include <cmsvattachment.h>
#include <hash.h>
#include <connect/sbeclient.h>
#include <e32property.h>
#if (!defined __SERIES60_30__ && !defined __SERIES60_31__) 
#include <driveinfo.h>
#endif // !defined __SERIES60_30__ && !defined __SERIES60_31__

#ifndef __SERIES60_30__
  #include <akniconconfig.h>
#endif

_LIT(KGZipExt, ".gz");
_LIT8(KFileNewLine, "\r\n");
const TInt KForcedFormatTimeout = 1000000;

// copied from coreapplicationuisdomainpskeys.h
const TUid KPSUidCoreApplicationUIs = { 0x101F8767 };
const TUint32 KCoreAppUIsMmcRemovedWithoutEject = 0x00000109;
enum TCoreAppUIsMmcRemovedWithoutEject
    {
    ECoreAppUIsEjectCommandUsedUninitialized = 0,
    ECoreAppUIsEjectCommandNotUsed,
    ECoreAppUIsEjectCommandUsed,
    ECoreAppUIsEjectCommandUsedToDrive // Low 16-bits contain this value and high 16-bits TDriveNumber to eject
    };
const TInt KDriveLetterStringLength = 3; // e.g. C:\

// ================= MEMBER FUNCTIONS =======================

CFileBrowserFileUtils* CFileBrowserFileUtils::NewL(CFileBrowserModel* aModel)
	{
	CFileBrowserFileUtils* self = new(ELeave) CFileBrowserFileUtils(aModel);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

// --------------------------------------------------------------------------------------------

CFileBrowserFileUtils::CFileBrowserFileUtils(CFileBrowserModel* aModel) : CActive(EPriorityStandard), iModel(aModel)
	{
	}

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ConstructL()
	{
    User::LeaveIfError(iTimer.CreateLocal());
    User::LeaveIfError(iFs.Connect());
    User::LeaveIfError(iTz.Connect());
    
    iState = EIdle;
    iListingMode = ENormalEntries;

    iSortMode = EFileBrowserCmdViewSortByName;
    iOrderMode = EFileBrowserCmdViewOrderAscending;
    iCurrentPath = KNullDesC;
    
    iFileOps = CFileBrowserFileOps::NewL(iModel);
    
    iDriveEntryList = new(ELeave) CDriveEntryList(8);
    iFileEntryList = new(ELeave) CFileEntryList(64);
    iFindFileEntryList = new(ELeave) CFileEntryList(64);
    iAppIconList = new(ELeave) CAppIconList(16);
    iClipBoardList = new(ELeave) CFileEntryList(64);

    iDocHandler = CDocumentHandler::NewL();
    iDocHandler->SetExitObserver(this);
    
    // set defaults to the search settings
    iSearchAttributes.iSearchDir = KNullDesC;
    iSearchAttributes.iWildCards = _L("*.jpg");
    iSearchAttributes.iTextInFile = KNullDesC;
    iSearchAttributes.iMinSize = 0;
    iSearchAttributes.iMaxSize = 999999999;
    iSearchAttributes.iMinDate = TDateTime(1980, EJanuary,   0,0,0,0,0);
    iSearchAttributes.iMaxDate = TDateTime(2060, EDecember, 30,0,0,0,0);
    iSearchAttributes.iRecurse = ETrue;
    
    // init the command array
    iCommandArray = new(ELeave) CCommandArray(64);
        
    // get current path from settings
    if (iModel->Settings().iRememberLastPath)
        {
        TFileName settingPath = iModel->Settings().iLastPath;
        if (BaflUtils::PathExists(iFs, settingPath))
            iCurrentPath = settingPath;
        
        if (iCurrentPath != KNullDesC)
            GetDriveListL();
        }

	CActiveScheduler::Add(this);
	}

// --------------------------------------------------------------------------------------------

CFileBrowserFileUtils::~CFileBrowserFileUtils()
	{
	Cancel();
	
	// save current path from settings
    if (iModel->Settings().iRememberLastPath)
        {
        iModel->Settings().iLastPath = iCurrentPath;
        TRAP_IGNORE(iModel->SaveSettingsL(EFalse));
        }

    if (iWaitDialog)
        TRAP_IGNORE(iWaitDialog->ProcessFinishedL()); 
     
    if (iProgressDialog)
        TRAP_IGNORE(iProgressDialog->ProcessFinishedL());
    
    delete iFileOps;    
   
	delete iDocHandler;
	
	if (iOpenFileService)
	    delete iOpenFileService;
	
	delete iClipBoardList;
	delete iAppIconList;
	delete iFindFileEntryList;
    delete iFileEntryList;
    delete iDriveEntryList;
    
    delete iFileMan;
    
    if (iCommandArray)
        {
        ResetCommandArray();
        delete iCommandArray;
        }
        
    iTz.Close();
	iFs.Close();
	iTimer.Close();
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::DoCancel()
	{
    iTimer.Cancel();
	}

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::StartExecutingCommandsL(const TDesC& aLabel)
    {
    if (iCommandArray->Count() >= 2)
        {
        // init progress bar
        iProgressDialog = new(ELeave) CAknProgressDialog((reinterpret_cast<CEikDialog**>(&iProgressDialog)), ETrue);
        iProgressDialog->SetCallback(this);
        iProgressDialog->PrepareLC(R_GENERAL_PROGRESS_NOTE);
        iProgressDialog->SetCurrentLabelL( EAknCtNote, aLabel );
        iProgressInfo = iProgressDialog->GetProgressInfoL();
        iProgressInfo->SetFinalValue( CommandArrayCount() );
        iProgressDialog->RunLD();
        iProgressDialog->MakeVisible( ETrue );        
        }
    else if (iCommandArray->Count() >= 1)
        {
        // init wait dialog
        iWaitDialog = new(ELeave) CAknWaitDialog((reinterpret_cast<CEikDialog**>(&iWaitDialog)), ETrue);
        iWaitDialog->SetCallback(this);
        iWaitDialog->PrepareLC(R_GENERAL_WAIT_NOTE);
        iWaitDialog->SetTextL( aLabel );
        iWaitDialog->RunLD();
        }
    else
        {
        // no commands, just do nothing
        return;
        }


    // start with the first operation
    iCurrentEntry = 0;
    iSucceededOperations = 0;
    iFailedOperations = 0;

    // starts executing commands
    ExecuteCommand();
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ExecuteCommand()
    {
    // make sure the engine isn't active, should never happen
    __ASSERT_ALWAYS(!IsActive(), User::Panic(_L("FileUtils:IsActive"), 333));

    // execute a command after a very short delay (25ms)
    iTimer.After(iStatus, 25);
    SetActive();
    }

// --------------------------------------------------------------------------------------------
    
void CFileBrowserFileUtils::RunL()
    {
    TInt err(KErrNone);
    TInt currentCommand = iCommandArray->At(iCurrentEntry).iCommandId;

    // execute a command from the queue
    switch(currentCommand)
        {
        case EFileBrowserFileOpCommandAttribs:
            {
            CCommandParamsAttribs* params = static_cast<CCommandParamsAttribs*>(iCommandArray->At(iCurrentEntry).iParameters);
            err = iFileOps->Attribs(params->iSourceEntry, params->iSetMask, params->iClearMask, params->iTime, params->iSwitch);
            }
            break;
            
        case EFileBrowserFileOpCommandCopy:
            {
            CCommandParamsCopyOrMove* params = static_cast<CCommandParamsCopyOrMove*>(iCommandArray->At(iCurrentEntry).iParameters);
            err = iFileOps->Copy(params->iSourceEntry, params->iTargetPath, params->iSwitch);
            }
            break;
            
        case EFileBrowserFileOpCommandMove:
            {
            CCommandParamsCopyOrMove* params = static_cast<CCommandParamsCopyOrMove*>(iCommandArray->At(iCurrentEntry).iParameters);
            err = iFileOps->Copy(params->iSourceEntry, params->iTargetPath, params->iSwitch, ETrue);
            }
            break;
                
        case EFileBrowserFileOpCommandRename:
            {
            CCommandParamsRename* params = static_cast<CCommandParamsRename*>(iCommandArray->At(iCurrentEntry).iParameters);
            err = iFileOps->Rename(params->iSourceEntry, params->iTargetPath, params->iSwitch);
            }
            break;

        case EFileBrowserFileOpCommandDelete:
            {
            CCommandParamsDelete* params = static_cast<CCommandParamsDelete*>(iCommandArray->At(iCurrentEntry).iParameters);
            err = iFileOps->Delete(params->iSourceEntry, params->iSwitch);
            }
            break;

        case EFileBrowserFileOpCommandDriveSnapShot:
            {
            CCommandParamsDriveSnapShot* params = static_cast<CCommandParamsDriveSnapShot*>(iCommandArray->At(iCurrentEntry).iParameters);
            err = iFileOps->DriveSnapShot(params->iSourceDriveLetter, params->iTargetDriveLetter);
            }
            break;
                                                
        default:
            User::Panic (_L("Unknown command"), 400);
            break;
        }

    
    if (err == KErrNone)
        {
        iSucceededOperations++;
        }
    else if ( err != KErrCancel )
        {
        iFailedOperations++;
        }
        
    iLastError = err;            

    // check for more commands in queue
    TRAP_IGNORE( CheckForMoreCommandsL() );
    }

// ---------------------------------------------------------------------------

TInt CFileBrowserFileUtils::RunError(TInt aError)
    {
    _LIT(KMessage, "Command error %d");
    TBuf<128> noteMsg;
    noteMsg.Format(KMessage, aError);
            
    iModel->EikonEnv()->InfoMsg(noteMsg);
    
    TRAP_IGNORE( CheckForMoreCommandsL() );
    
    return KErrNone;
    }

// ---------------------------------------------------------------------------

void CFileBrowserFileUtils::CheckForMoreCommandsL()
    {
    //LOGSTRING("Creator: CCreatorEngine::CheckForMoreCommandsL");
    
    // update the progress bar
    if (iProgressInfo)
        iProgressInfo->IncrementAndDraw(1);

	// check if we have more commands to be executed
	if (iCurrentEntry >= CommandArrayCount() - 1)
		{
        //LOGSTRING("Creator: CCreatorEngine::CheckForMoreCommandsL all done");

        // all done, free resources
        ResetCommandArray();
        
        // deactive secure backup if it was activated by a file command
        iFileOps->DeActivateSecureBackUpViaFileOp();

        // dismiss any wait/progress dialogs        
        if (iWaitDialog)
            {
            TRAP_IGNORE(iWaitDialog->ProcessFinishedL()); 
            iWaitDialog = NULL;
            }
        if (iProgressDialog)
            {
            TRAP_IGNORE(iProgressDialog->ProcessFinishedL());
            iProgressDialog = NULL;
            }


        // show result note
        if ( iSucceededOperations == 0 && iFailedOperations == 0 )
            {
            // single operation cancelled -> show nothing
            }
        else if (iSucceededOperations == 1 && iFailedOperations == 0)    
            {
            _LIT(KMessage, "Operation succeeded");
    		CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
    		note->ExecuteLD(KMessage);
            }
        else if (iSucceededOperations > 0 && iFailedOperations == 0)    
            {
            _LIT(KMessage, "%d operations succeeded");
            TBuf<128> noteMsg;
            noteMsg.Format(KMessage, iSucceededOperations);
            
    		CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
    		note->ExecuteLD(noteMsg);
            }
        else if (iSucceededOperations == 0 && iFailedOperations > 1)    
            {
            _LIT(KMessage, "%d operations failed");
            TBuf<128> noteMsg;
            noteMsg.Format(KMessage, iFailedOperations);
            
    		CAknErrorNote* note = new(ELeave) CAknErrorNote;
    		note->ExecuteLD(noteMsg);
            }
        else if (iSucceededOperations > 0 && iFailedOperations > 0)    
            {
            _LIT(KMessage, "%d operations succeeded, %d failed");
            TBuf<128> noteMsg;
            noteMsg.Format(KMessage, iSucceededOperations, iFailedOperations);
            
    		CAknInformationNote* note = new(ELeave) CAknInformationNote;
    		note->ExecuteLD(noteMsg);
            }
        else    
            {
            CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD( textResolver->ResolveErrorString(iLastError, CTextResolver::ECtxNoCtxNoSeparator) );    

            CleanupStack::PopAndDestroy();  //textResolver    
            }

        RefreshViewL();

		}
	else
		{
		// maintain requests
		iCurrentEntry++;

        //LOGSTRING2("Creator: CCreatorEngine::CheckForMoreCommandsL iCurrentEntry=%d", iCurrentEntry);

        ExecuteCommand();
		}
    }    
// --------------------------------------------------------------------------------------------

// This callback function is called when cancel button of the progress bar was pressed
void CFileBrowserFileUtils::DialogDismissedL(TInt aButtonId)
    {
    iProgressDialog = NULL;
    iProgressInfo = NULL;
    iWaitDialog = NULL;
    
    // check if cancel button was pressed
    if (aButtonId == EAknSoftkeyCancel)
        {
        // cancel the active object, command executer 
        Cancel();
        
        iFileOps->CancelOp();
        
        ResetCommandArray();

	    CAknInformationNote* note = new (ELeave) CAknInformationNote;
	    note->ExecuteLD(_L("Cancelled"));
        }
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::AppendToCommandArrayL(TInt aCommand, CCommandParamsBase* aParameters)
    {
    if (iCommandArray)
        iCommandArray->AppendL( TCommand(aCommand, aParameters) );
    else
        User::Leave(KErrNotReady);
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::CommandArrayCount() const
    {
    TInt count(0);
    
    if (iCommandArray)
        count = iCommandArray->Count();
    
    return count;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ResetCommandArray()
    {
    // delete params first
    for (TInt i=0; i<CommandArrayCount(); i++)
        {
        if ( iCommandArray->At(i).iParameters )
            {
            delete iCommandArray->At(i).iParameters;
            iCommandArray->At(i).iParameters = NULL;
            }
        }

    // reset the entries
    iCommandArray->Reset();
    }
                    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::HandleSettingsChangeL()
    {
    if (iModel->FileListContainer())
        {
        iModel->FileListContainer()->SetScreenLayoutL(iModel->Settings().iDisplayMode); 
        iModel->FileListContainer()->CreateListBoxL(iModel->Settings().iFileViewMode);
        //iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);
        RefreshViewL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetSortModeL(TInt aSortMode)
    {
    iSortMode = aSortMode;
    RefreshViewL();
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetOrderModeL(TInt aOrderMode)
    {
    iOrderMode = aOrderMode;
    RefreshViewL();
    }

// --------------------------------------------------------------------------------------------

TKeyResponse CFileBrowserFileUtils::HandleOfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    if(aType != EEventKey)
        return EKeyWasNotConsumed;
    
    TBool shiftKeyPressed = (aKeyEvent.iModifiers & EModifierShift) ||
                            (aKeyEvent.iModifiers & EModifierLeftShift) ||
                            (aKeyEvent.iModifiers & EModifierRightShift);
    TBool controlKeyPressed = (aKeyEvent.iModifiers & EModifierCtrl) || 
                              (aKeyEvent.iModifiers & EModifierRightCtrl); 
    
    // handle left arrow key press
    if (aKeyEvent.iCode == EKeyLeftArrow)
        {
        MoveUpOneLevelL();
        return EKeyWasConsumed;
        }
    
    // handle right arrow key press
    else if (aKeyEvent.iCode == EKeyRightArrow)
        {
        if (IsDriveListViewActive() && iDriveEntryList->Count() > 0)
            {
            MoveDownToDirectoryL();
            return EKeyWasConsumed;
            }
        else if (iFileEntryList->Count() > 0)
            {
            MoveDownToDirectoryL();
            return EKeyWasConsumed;
            }        
        }    

    // skip OK/Enter keys when shift or ctrl is pressed because those are needed by the listbox
    else if ((aKeyEvent.iCode == EKeyOK || aKeyEvent.iCode == EKeyEnter) && (shiftKeyPressed || controlKeyPressed))
        {
        return EKeyWasNotConsumed;
        }
    
    // handle OK/Enter keys
    else if (aKeyEvent.iCode == EKeyOK || aKeyEvent.iCode == EKeyEnter)
        {
        if (IsDriveListViewActive() && iDriveEntryList->Count() > 0)
            {
            MoveDownToDirectoryL();
            return EKeyWasConsumed;
            }
        else if (iFileEntryList->Count() > 0)
            {
            TFileEntry fileEntry = iFileEntryList->At(iModel->FileListContainer()->CurrentListBoxItemIndex());
            
            if (fileEntry.iEntry.IsDir())
                MoveDownToDirectoryL();
            else
                OpenCommonFileActionQueryL();
            
            return EKeyWasConsumed;
            } 
        }
    
    // handle Delete/Backspace keys
    else if (!iModel->FileListContainer()->SearchField() && (aKeyEvent.iCode == EKeyBackspace || aKeyEvent.iCode == EKeyDelete) &&
             !iModel->FileUtils()->IsCurrentDriveReadOnly() && !iModel->FileUtils()->IsDriveListViewActive())
        {
        DeleteL();
        return EKeyWasConsumed;
        }
                          
    return EKeyWasNotConsumed;
    }        
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::RefreshViewL()
    {
    if (iModel->FileListContainer())
        {
        // update navi pane text        
        if (iListingMode == ENormalEntries)
            iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);
        else if (iListingMode == ESearchResults)
            iModel->FileListContainer()->SetNaviPaneTextL(_L("Search results"));
        else if (iListingMode == EOpenFiles)
            iModel->FileListContainer()->SetNaviPaneTextL(_L("Open files"));
        else if (iListingMode == EMsgAttachmentsInbox)
            iModel->FileListContainer()->SetNaviPaneTextL(_L("Attachments in Inbox"));
        else if (iListingMode == EMsgAttachmentsDrafts)
            iModel->FileListContainer()->SetNaviPaneTextL(_L("Attachments in Drafts"));
        else if (iListingMode == EMsgAttachmentsSentItems)
            iModel->FileListContainer()->SetNaviPaneTextL(_L("Attachments in Sent Items"));
        else if (iListingMode == EMsgAttachmentsOutbox)
            iModel->FileListContainer()->SetNaviPaneTextL(_L("Attachments in Outbox"));

        // create a list box if it doesn't already exist
        if (!iModel->FileListContainer()->ListBox())
            iModel->FileListContainer()->CreateListBoxL(iModel->Settings().iFileViewMode);
        
        // clear selections if any
        iModel->FileListContainer()->ListBox()->ClearSelection();

        // make sure that the search field is disabled
        iModel->FileListContainer()->DisableSearchFieldL();
        
        // read directory listing
        GenerateDirectoryDataL();
        
        // set an icon array
        iModel->FileListContainer()->SetListBoxIconArrayL(GenerateIconArrayL());

        // set text items
        iModel->FileListContainer()->SetListBoxTextArrayL(GenerateItemTextArrayL());

        // make sure that the current item index is not out of array
        if (iModel->FileListContainer()->CurrentListBoxItemIndex() == -1 && // -1 is a hardcoded value meaning that no current item index
                iModel->FileListContainer()->ListBox()->Model()->NumberOfItems() > 0)  
            iModel->FileListContainer()->ListBox()->SetCurrentItemIndex( iModel->FileListContainer()->ListBox()->Model()->NumberOfItems() - 1 );    

        }
    
        iModel->FileListContainer()->UpdateToolbar();
    }   
    
// --------------------------------------------------------------------------------------------

CAknIconArray* CFileBrowserFileUtils::GenerateIconArrayL(TBool aGenerateNewBasicIconArray)
    {
    CAknIconArray* iconArray = NULL;
    
    if (aGenerateNewBasicIconArray)
        {
        iconArray = new(ELeave) CAknIconArray(16);    
        }
    else
        {
        if (iModel->FileListContainer())
            iconArray = iModel->FileListContainer()->ListBoxIconArrayL();
        
        if (!iconArray)    
            iconArray = new(ELeave) CAknIconArray(16);
        }

        
    CleanupStack::PushL(iconArray);
    
    // generate basic items if not already existing
    if (iconArray->Count() < EFixedIconListLength)
        {
        // reset arrays
        iconArray->Reset();
        iAppIconList->Reset();
        
        // get default control color
        TRgb defaultColor;
        defaultColor = iModel->EikonEnv()->Color(EColorControlText);
        
        // create a color icon of the marking indicator
        CFbsBitmap* markBitmap = NULL;
        CFbsBitmap* markBitmapMask = NULL;

        AknsUtils::CreateColorIconL(AknsUtils::SkinInstance(),
                KAknsIIDQgnIndiMarkedAdd,
                KAknsIIDQsnIconColors,
                EAknsCIQsnIconColorsCG13,
                markBitmap,
                markBitmapMask,
                AknIconUtils::AvkonIconFileName(),
                EMbmAvkonQgn_indi_marked_add,
                EMbmAvkonQgn_indi_marked_add_mask,
                defaultColor
                );
         
        // 0 marking indicator
        CGulIcon* markIcon = CGulIcon::NewL(markBitmap, markBitmapMask);
        iconArray->AppendL(markIcon);                       

        // 1 empty
        AppendGulIconToIconArrayL(iconArray, AknIconUtils::AvkonIconFileName(),
                                  EMbmAvkonQgn_prop_empty, EMbmAvkonQgn_prop_empty_mask, KAknsIIDQgnPropEmpty);
        // 2 phone memory
        AppendGulIconToIconArrayL(iconArray, AknIconUtils::AvkonIconFileName(),
                                  EMbmAvkonQgn_prop_phone_memc, EMbmAvkonQgn_prop_phone_memc_mask, KAknsIIDQgnPropPhoneMemc);
        // 3 memory card
        AppendGulIconToIconArrayL(iconArray, KMemoryCardUiBitmapFile,
                                  EMbmAknmemorycarduiQgn_prop_mmc_memc, EMbmAknmemorycarduiQgn_prop_mmc_memc_mask, KAknsIIDQgnPropMmcMemc);
        // 4 memory card disabled
        AppendGulIconToIconArrayL(iconArray, KMemoryCardUiBitmapFile,
                                  EMbmAknmemorycarduiQgn_prop_mmc_non, EMbmAknmemorycarduiQgn_prop_mmc_non_mask, KAknsIIDQgnPropMmcNon);
        // 5 folder
        AppendGulIconToIconArrayL(iconArray, AknIconUtils::AvkonIconFileName(),
                                  EMbmAvkonQgn_prop_folder_small, EMbmAvkonQgn_prop_folder_small_mask, KAknsIIDQgnPropFolderSmall);
        // 6 folder with subfolders
        AppendGulIconToIconArrayL(iconArray, AknIconUtils::AvkonIconFileName(),
                                  EMbmAvkonQgn_prop_folder_sub_small, EMbmAvkonQgn_prop_folder_sub_small_mask, KAknsIIDQgnPropFolderSubSmall);
        // 7 current folder
        AppendGulIconToIconArrayL(iconArray, AknIconUtils::AvkonIconFileName(),
                                  EMbmAvkonQgn_prop_folder_current, EMbmAvkonQgn_prop_folder_current_mask, KAknsIIDQgnPropFolderCurrent);
        }

    // append custom icons if not in drive list view and setting is enabled
    if (!aGenerateNewBasicIconArray && !IsDriveListViewActive() && iModel->Settings().iShowAssociatedIcons)
        {
        // init id counter
        TInt idCounter(EFixedIconListLength + iAppIconList->Count());
        
        // loop all items in the file list
        for (TInt i=0; i<iFileEntryList->Count(); i++)
            {
            TFileEntry& fileEntry = iFileEntryList->At(i);

            // just check for normal files            
            if (!fileEntry.iEntry.IsDir())     
                {
                TUid appUid = GetAppUid(fileEntry);
                TInt iconId = AppIconIdForUid(appUid);
                
                if (appUid != KNullUid && iconId == KErrNotFound)
                    {
                    // icon not found from the icon array, generate it
                    CFbsBitmap* bitmap = NULL;
                    CFbsBitmap* mask = NULL;
                    CGulIcon* appIcon = NULL;
        
                    TRAPD(err, 
                        AknsUtils::CreateAppIconLC(AknsUtils::SkinInstance(), appUid, EAknsAppIconTypeList, bitmap, mask);
                        appIcon = CGulIcon::NewL(bitmap, mask);
                        CleanupStack::Pop(2); //bitmap, mask
                        );
                         
                    if (err == KErrNone)
                        {
                        TAppIcon appIconEntry;
                        appIconEntry.iId = idCounter;
                        appIconEntry.iUid = appUid;
                        
                        appIcon->SetBitmapsOwnedExternally(EFalse);
                        iconArray->AppendL(appIcon);
                        iAppIconList->AppendL(appIconEntry);

                        fileEntry.iIconId = idCounter;
                                                
                        idCounter++;      
                        }
                    else
                        {
                        delete bitmap;
                        delete mask;
		
                        TAppIcon appIconEntry;
                        appIconEntry.iId = EFixedIconEmpty;
                        appIconEntry.iUid = appUid;
                        
                        iAppIconList->AppendL(appIconEntry);
                        }
                    }

                else if (appUid != KNullUid && iconId >= 0)
                    {
                    // we already have already generated an icon for this uid, use it
                    fileEntry.iIconId = iconId;
                    }
                }
            }
        }

    CleanupStack::Pop(); //iconArray
    return iconArray;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::AppendGulIconToIconArrayL(CAknIconArray* aIconArray, const TDesC& aIconFile, TInt aIconId, TInt aMaskId, const TAknsItemID aAknsItemId)
    {
    CGulIcon* icon = AknsUtils::CreateGulIconL(
            AknsUtils::SkinInstance(),
            aAknsItemId,
            aIconFile,
            aIconId,
            aMaskId);
    CleanupStack::PushL(icon);
	
	icon->SetBitmapsOwnedExternally(EFalse);
	aIconArray->AppendL(icon);
	
	CleanupStack::Pop(); //icon          
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::AppIconIdForUid(TUid aUid)
    {
    TInt result(KErrNotFound);
    
    for (TInt i=0; i<iAppIconList->Count(); i++)
        {
        if (iAppIconList->At(i).iUid == aUid)
            {
            result = iAppIconList->At(i).iId;
            break;
            }
        }
    
    return result;
    }

// --------------------------------------------------------------------------------------------

TUid CFileBrowserFileUtils::GetAppUid(TFileEntry aFileEntry)
    {
    TFileName fullPath;
    fullPath.Copy(aFileEntry.iPath);
    fullPath.Append(aFileEntry.iEntry.iName);
    
    TParse parsedName;
    parsedName.Set(fullPath, NULL, NULL);

    // this will boost performance in \sys\bin folder    
    if (parsedName.Ext().CompareF(_L(".dll")) == 0 || parsedName.Ext().CompareF(_L(".rsc")) == 0)
        return KNullUid;
    
    TInt err1(KErrNone), err2(KErrNone);
    TUid appUid;
    TDataType dataType;
    TBool isProgram;
    
    err1 = iModel->LsSession().AppForDocument(fullPath, appUid, dataType);
    err2 = iModel->LsSession().IsProgram(fullPath, isProgram);
        
    if (err1 == KErrNone && err2 == KErrNone)
        {
        if (isProgram)
            {
            // get the real app Uid from the app list
            TApaAppInfo appInfo;
            iModel->LsSession().GetAllApps();

            while (iModel->LsSession().GetNextApp(appInfo) == KErrNone)
                {
                if (fullPath.CompareF(appInfo.iFullName) == 0)
                    return appInfo.iUid;
                }
            }
        else
            {
            // return the app Uid associated for it
            return appUid;
            }            
        }

    return KNullUid;
    }
// --------------------------------------------------------------------------------------------

CDesCArray* CFileBrowserFileUtils::GenerateItemTextArrayL()
    {
    CDesCArray* textArray = new(ELeave) CDesCArrayFlat(64);
    CleanupStack::PushL(textArray);

    if (IsDriveListViewActive())
        {
        _LIT(KSimpleDriveEntry, "%d\t%c: <%S>\t\t");
        _LIT(KExtendedDriveEntry, "%d\t%c: <%S>\t%LD / %LD kB\t");

        for (TInt i=0; i<iDriveEntryList->Count(); i++)
            {
            TFileName textEntry;
            TDriveEntry driveEntry = iDriveEntryList->At(i);
            
            if (iModel->Settings().iFileViewMode == EFileViewModeSimple)
                {
                textEntry.Format(KSimpleDriveEntry, driveEntry.iIconId, TUint(driveEntry.iLetter), &driveEntry.iMediaTypeDesc);
                }
            else if (iModel->Settings().iFileViewMode == EFileViewModeExtended)
                {
                textEntry.Format(KExtendedDriveEntry, driveEntry.iIconId, TUint(driveEntry.iLetter), &driveEntry.iMediaTypeDesc, driveEntry.iVolumeInfo.iFree/1024, driveEntry.iVolumeInfo.iSize/1024);
                }
            
            textArray->AppendL(textEntry);
            }
        }
    
    else
        {
        _LIT(KSimpleFileEntry, "%d\t%S\t\t");
        _LIT(KExtendedFileEntry, "%d\t%S\t%S\t");

        for (TInt i=0; i<iFileEntryList->Count(); i++)
            {
            TBuf<KMaxPath+KMaxFileName> textEntry;
            TFileEntry fileEntry = iFileEntryList->At(i);
            
            // format text line entries            
            if (iModel->Settings().iFileViewMode == EFileViewModeSimple)
                {
                textEntry.Format(KSimpleFileEntry, fileEntry.iIconId, &fileEntry.iEntry.iName);
                }
            else if (iModel->Settings().iFileViewMode == EFileViewModeExtended)
                {
                TBuf<128> extraData;
                
                // append entry date
                TTime entryModified = fileEntry.iEntry.iModified;
                if ( iTz.ConvertToLocalTime( entryModified ) == KErrNone )
                    {
                    entryModified = fileEntry.iEntry.iModified;
                    }
                
                _LIT(KCurrentDate,"%D%M%*Y%/0%1%/1%2%/2%3%/3 %-B%:0%J%:1%T%:2%S%:3%+B");
                entryModified.FormatL(extraData, KCurrentDate);

                // for a directory append number of entries and for a file the file size
                if (fileEntry.iEntry.IsDir() && fileEntry.iDirEntries >= 0)
                    {
                    extraData.Append(_L(" - "));
                    extraData.AppendNum(fileEntry.iDirEntries);
                    
                    if (fileEntry.iDirEntries == 1)
                        extraData.Append(_L(" entry"));
                    else
                        extraData.Append(_L(" entries"));
                    }
                
                // normal file entry
                else if (!fileEntry.iEntry.IsDir())
                    {
                    extraData.Append(_L(" - "));

                    if (fileEntry.iEntry.iSize < 1024)
                        {
                        extraData.AppendNum(fileEntry.iEntry.iSize);
                        extraData.Append(_L(" B"));
                        }
                    else if (fileEntry.iEntry.iSize < 1024*1024)
                        {
                        TReal sizeKB = (TReal)fileEntry.iEntry.iSize / 1024;
                        extraData.AppendNum(sizeKB, TRealFormat(KDefaultRealWidth, 1));
                        extraData.Append(_L(" kB"));
                        }
                    else
                        {
                        TReal sizeMB = (TReal)fileEntry.iEntry.iSize / (1024*1024);
                        extraData.AppendNum(sizeMB, TRealFormat(KDefaultRealWidth, 1));
                        extraData.Append(_L(" MB"));
                        }
                    }

                // append attribute flags if any
                TBuf<4> atts;
                if (fileEntry.iEntry.IsArchive())
                    atts.Append(_L("A"));
                if (fileEntry.iEntry.IsHidden())
                    atts.Append(_L("H"));
                if (fileEntry.iEntry.IsReadOnly())
                    atts.Append(_L("R"));
                if (fileEntry.iEntry.IsSystem())
                    atts.Append(_L("S"));
                
                if (atts.Length() > 0)
                    {
                    extraData.Append(_L(" - ["));    
                    extraData.Append(atts);    
                    extraData.Append(_L("]"));    
                    }
                
                // format                
                textEntry.Format(KExtendedFileEntry, fileEntry.iIconId, &fileEntry.iEntry.iName, &extraData);
                }
            
            if( fileEntry.iEntry.iName.Length() + fileEntry.iPath.Length() <= KMaxFileName )
                {
                textArray->AppendL(textEntry);
                }
            else
                {
                // Too long filenames are deleted from the list
                iFileEntryList->Delete(i--);                
                }
            }
        }
    
    CleanupStack::Pop(); //textArray
    return textArray;
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::GenerateDirectoryDataL()
    {
    if (iListingMode == ENormalEntries)
        {
        if (IsDriveListViewActive())
            GetDriveListL();    
        else
            GetDirectoryListingL();        
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::GetDriveListL()
    {
    TDriveList driveList;
    
    // get drive listing depending of the support for network drives
    if (iModel->Settings().iSupportNetworkDrives)
        {
        #ifndef __SERIES60_30__
            #ifndef __SERIES60_31__
                User::LeaveIfError(iFs.DriveList(driveList, KDriveAttAll));
            #endif
        #else    
            User::LeaveIfError(iFs.DriveList(driveList));
        #endif        
        }
    else
        {
        User::LeaveIfError(iFs.DriveList(driveList));
        }
    
    iDriveEntryList->Reset();        
    
    for (TInt i=0; i<KMaxDrives; i++)
        {
        if (driveList[i])
            {
            TDriveEntry driveEntry;
            
            // set default icon
            driveEntry.iIconId = EFixedIconPhoneMemory;

            // get drive letter and number 
            driveEntry.iLetter = 'A' + i;
            iFs.CharToDrive(driveEntry.iLetter, driveEntry.iNumber);
            
            // get volume info and check errors
            if (iFs.Volume(driveEntry.iVolumeInfo, driveEntry.iNumber) == KErrNone)
                {
                // set media type descriptor
                TInt mediaType = driveEntry.iVolumeInfo.iDrive.iType;
                TBool extMountable( EFalse );
                
                if (mediaType == EMediaNotPresent)
                    driveEntry.iMediaTypeDesc = _L("Not present");
                else if (mediaType ==EMediaUnknown )
                    driveEntry.iMediaTypeDesc = _L("Unknown");
                else if (mediaType ==EMediaFloppy )
                    driveEntry.iMediaTypeDesc = _L("Floppy");
                else if (mediaType == EMediaHardDisk)
                    driveEntry.iMediaTypeDesc = _L("Mass storage");
                else if (mediaType == EMediaCdRom)
                    driveEntry.iMediaTypeDesc = _L("CD-ROM");
                else if (mediaType == EMediaRam)
                    driveEntry.iMediaTypeDesc = _L("RAM");
                else if (mediaType == EMediaFlash)
                    driveEntry.iMediaTypeDesc = _L("Flash");
                else if (mediaType == EMediaRom)
                    driveEntry.iMediaTypeDesc = _L("ROM");
                else if (mediaType == EMediaRemote)
                    driveEntry.iMediaTypeDesc = _L("Remote");
                else if (mediaType == EMediaNANDFlash)
                    driveEntry.iMediaTypeDesc = _L("NAND flash");
                
                // get real size of the ROM drive
                if (mediaType == EMediaRom)
                    {
                    TMemoryInfoV1Buf ramMemory;
                    UserHal::MemoryInfo(ramMemory);
                    driveEntry.iVolumeInfo.iSize = ramMemory().iTotalRomInBytes;
                    }
                
                // set attribute descripitions
                if (driveEntry.iVolumeInfo.iDrive.iBattery == EBatNotSupported)
                    driveEntry.iAttributesDesc.Append(_L("Battery not supported"));
                else if (driveEntry.iVolumeInfo.iDrive.iBattery == EBatGood)
                    driveEntry.iAttributesDesc.Append(_L("Battery good"));
                else if (driveEntry.iVolumeInfo.iDrive.iBattery == EBatLow)
                    driveEntry.iAttributesDesc.Append(_L("Battery low"));
                else
                    driveEntry.iAttributesDesc.Append(_L("Battery state unknown"));
                
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttLocal)
                    driveEntry.iAttributesDesc.Append(_L(" + Local"));
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttRom)
                    driveEntry.iAttributesDesc.Append(_L(" + ROM"));
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttRedirected)
                    driveEntry.iAttributesDesc.Append(_L("+ Redirected"));
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttSubsted)
                    driveEntry.iAttributesDesc.Append(_L(" + Substed"));
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttInternal)
                    driveEntry.iAttributesDesc.Append(_L(" + Internal"));
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttRemovable)
                    {
                    driveEntry.iAttributesDesc.Append(_L(" + Removable"));
#if (!defined __SERIES60_30__ && !defined __SERIES60_31__)
                    TUint drvStatus( 0 );
                    if ( !DriveInfo::GetDriveStatus( iFs, driveEntry.iNumber, drvStatus ) &&
                         drvStatus & DriveInfo::EDriveExternallyMountable &&
                         drvStatus & DriveInfo::EDriveInternal )
                        {
                        extMountable = ETrue;
                        // iMediaTypeDesc already set as "Mass storage" 
                        }
                    else
                        {
                        driveEntry.iMediaTypeDesc = _L("Memory card");
                        }
#else
                    driveEntry.iMediaTypeDesc = _L("Memory card");
#endif
                    }
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttRemote)
                    driveEntry.iAttributesDesc.Append(_L(" + Remote"));
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttTransaction)
                    driveEntry.iAttributesDesc.Append(_L(" + Transaction"));
                                
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttVariableSize)
                    driveEntry.iAttributesDesc.Append(_L(" + Variable size"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttDualDensity)
                    driveEntry.iAttributesDesc.Append(_L(" + Dual density"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttFormattable)
                    driveEntry.iAttributesDesc.Append(_L(" + Formattable"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttWriteProtected)
                    driveEntry.iAttributesDesc.Append(_L(" + Write protected"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttLockable)
                    driveEntry.iAttributesDesc.Append(_L(" + Lockable"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttLocked)
                    driveEntry.iAttributesDesc.Append(_L(" + Locked"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttHasPassword)
                    driveEntry.iAttributesDesc.Append(_L(" + Has password"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttReadWhileWrite)
                    driveEntry.iAttributesDesc.Append(_L(" + Read while write"));
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttDeleteNotify)
                    driveEntry.iAttributesDesc.Append(_L(" + Supports DeleteNotify"));
                
                
                // mark a removable media with memory card icon
                if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttRemovable && !extMountable)
                    {
                    if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttLocked || driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAbsent)
                        {                
                        driveEntry.iIconId = EFixedIconMemoryCardDisabled;
                        }                 
                    else
                        {                
                        driveEntry.iIconId = EFixedIconMemoryCard;
                        } 
                    }
                }   
            
            // if this fails, likely it's a memory card which is not present
            else
                {
                TVolumeInfo volumeInfo;
                volumeInfo.iSize = 0;
                volumeInfo.iFree = 0;
                volumeInfo.iDrive.iDriveAtt = KDriveAttRemovable;
                volumeInfo.iDrive.iMediaAtt = KMediaAttWriteProtected;
                driveEntry.iVolumeInfo = volumeInfo;

                driveEntry.iMediaTypeDesc = _L("Not ready");
                driveEntry.iIconId = EFixedIconMemoryCardDisabled;
                }
            
            if ( iPrevFolderName != KNullDesC && iPrevFolderName[0] == driveEntry.iLetter )
                {
                iPrevFolderIndex = iDriveEntryList->Count();
                }
            
            iDriveEntryList->AppendL(driveEntry);
            }
        }
    
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::GetDirectoryListingL()
    {
    iFileEntryList->Reset();

    TInt sortMode(ESortByName);
    if (iSortMode == EFileBrowserCmdViewSortByExtension)    
        sortMode = ESortByExt; 
    else if (iSortMode == EFileBrowserCmdViewSortBySize)    
        sortMode = ESortBySize; 
    else if (iSortMode == EFileBrowserCmdViewSortByDate)    
        sortMode = ESortByDate; 
    
    TInt orderMode(EAscending);
    if (iOrderMode == EFileBrowserCmdViewOrderDescending)    
        orderMode = EDescending; 
    
    CDir* dir = NULL;
    if (iFs.GetDir(iCurrentPath, KEntryAttMatchMask, sortMode | orderMode | EDirsFirst, dir) == KErrNone)
        {
        CleanupStack::PushL(dir);
        
        for (TInt i=0; i<dir->Count(); i++)
            {
            TFileEntry fileEntry;
            fileEntry.iPath = iCurrentPath;
            fileEntry.iEntry = (*dir)[i];
            fileEntry.iDirEntries = KErrNotFound;
            fileEntry.iIconId = EFixedIconEmpty;
            
            if ( iPrevFolderName != KNullDesC && iPrevFolderName == fileEntry.iEntry.iName )
                {
                iPrevFolderIndex = i;
                }
            
            // check for directory entries
            if (fileEntry.iEntry.IsDir())
                {
                fileEntry.iIconId = EFixedIconFolder;

                TFileName subPath = fileEntry.iPath;
                subPath.Append(fileEntry.iEntry.iName);
                subPath.Append(_L("\\"));
                                
                // get number of entries in this directory if extended info about sub directories enabled
                if (iModel->Settings().iShowSubDirectoryInfo && iModel->Settings().iFileViewMode == EFileViewModeExtended)
                    {
                    CDir* subDir = NULL;
                    if (iFs.GetDir(subPath, KEntryAttMatchMask, ESortNone | EDirsFirst, subDir) == KErrNone)
                        {
                        fileEntry.iDirEntries = subDir->Count();
                        delete subDir;    
                        }
                    }
                
                // apply subfolder icon for known directories (speed improvement)
                if (subPath[0]=='Z' && (subPath.CompareF(_L("Z:\\data\\"))==0 || subPath.CompareF(_L("Z:\\Nokia\\"))==0
                    || subPath.Compare(_L("Z:\\private\\"))==0 || subPath.Compare(_L("Z:\\resource\\"))==0
                    || subPath.Compare(_L("Z:\\sys\\"))==0 || subPath.Compare(_L("Z:\\system\\"))==0))
                    {
                    fileEntry.iIconId = EFixedIconFolderSub;
                    }
                else if (subPath[0]=='Z' && (subPath.CompareF(_L("Z:\\sys\\bin\\"))==0))
                    {
                    // normal folder icon for these ones
                    fileEntry.iIconId = EFixedIconFolder;
                    }
                else
                    {
                    // check if this folder has subdirectories
                    CDir* subDir = NULL;
                    if (iFs.GetDir(subPath, KEntryAttDir|KEntryAttMatchMask, ESortNone | EDirsFirst, subDir) == KErrNone)
                        {
                        for (TInt j=0; j<subDir->Count(); j++)
                            {
                            TEntry entry = (*subDir)[j];

                            if (entry.IsDir())
                                {
                                fileEntry.iIconId = EFixedIconFolderSub;
                                break;    
                                }
                            }

                        delete subDir;    
                        }                    
                    }
                }

            iFileEntryList->AppendL(fileEntry);
            }
        
        CleanupStack::PopAndDestroy(); //dir    
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::MoveUpOneLevelL()
    {
    iPrevFolderIndex = KErrNotFound;
    iPrevFolderName.Zero();
    
    // change back to normal mode or move up
    if (iListingMode != ENormalEntries)
        {
        iListingMode = ENormalEntries;
        }
    else
        {
        // do nothing if displaying drive list view
        if (IsDriveListViewActive())
            return;
        
        if (iCurrentPath.Length() <= KDriveLetterStringLength)
            {
            if ( iCurrentPath.Length() > 0 )
                {
                iPrevFolderName.Append( iCurrentPath[0] );
                }
            // move to drive list view if the current path is already short enough
            iCurrentPath = KNullDesC;
            }
        else
            {
            // move one directory up
            TInt marker(iCurrentPath.Length());
            
            // find second last dir marker
            for (TInt i=iCurrentPath.Length()-2; i>=0; i--)
                {
                if (iCurrentPath[i] == '\\')
                    {
                    marker = i;
                    break;
                    }
                    
                }
            
            // update iPrevDir with the directory name that we just left 
            iPrevFolderName.Copy( iCurrentPath.RightTPtr( iCurrentPath.Length() -  marker - 1 ) );
            iPrevFolderName.Delete( iPrevFolderName.Length() -1, 2 ); // remove extra '\\'
            
            iCurrentPath = iCurrentPath.LeftTPtr(marker+1);
            }    
        }

    if (iModel->FileListContainer())
        {
        //iModel->FileListContainer()->DisableSearchFieldL();
        //iModel->FileListContainer()->ListBox()->ClearSelection();
        iModel->FileListContainer()->ListBox()->SetCurrentItemIndex(0);
        //iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);
        }
    
    // update view
    RefreshViewL();
    
    if ( iModel->Settings().iRememberFolderSelection &&
         iModel->FileListContainer() && iPrevFolderIndex > KErrNotFound )
        {
        
        TInt visibleItems = iModel->FileListContainer()->ListBox()->Rect().Height() /
                            iModel->FileListContainer()->ListBox()->ItemHeight() 
                            - 1; // for the title row

        // By default, the selected item would be the last visible item on the listbox.
        // We want the selected item be displayed at the middle of the listbox, so we
        // select one of the successor items first to scroll down a bit
        iModel->FileListContainer()->ListBox()->SetCurrentItemIndex( 
            Min( iModel->FileListContainer()->ListBox()->Model()->ItemTextArray()->MdcaCount() -1,
                 iPrevFolderIndex + visibleItems / 2 ) );
        
        // ...and after that we select the the correct item.
        iModel->FileListContainer()->ListBox()->SetCurrentItemIndex( iPrevFolderIndex );
        
        // update view
        RefreshViewL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::MoveDownToDirectoryL()
    {
    // change back to normal mode
    if (iListingMode != ENormalEntries)
        {
        iListingMode = ENormalEntries;
        }
    
    TInt index = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (index >= 0)
        {
        TBool needsRefresh(EFalse);

        if (IsDriveListViewActive())
            {
            // currently in a drive list view, move to root of selected drive
            if (iDriveEntryList->Count() > index)
                {
                TDriveEntry driveEntry = iDriveEntryList->At(index);
                
                iCurrentPath.Append(driveEntry.iLetter);    
                iCurrentPath.Append(_L(":\\"));
                
                needsRefresh = ETrue;    
                }
            }
        else
            {
            // otherwise just append the new directory
            if (iFileEntryList->Count() > index)
                {
                TFileEntry fileEntry = iFileEntryList->At(index);
                
                if (fileEntry.iEntry.IsDir())
                    {
                    iCurrentPath.Copy(fileEntry.iPath);
                    iCurrentPath.Append(fileEntry.iEntry.iName);
                    iCurrentPath.Append(_L("\\"));

                    needsRefresh = ETrue;    
                    }
                }
            }

        if (needsRefresh && iModel->FileListContainer())
            {
            //iModel->FileListContainer()->DisableSearchFieldL();
            //iModel->FileListContainer()->ListBox()->ClearSelection();
            iModel->FileListContainer()->ListBox()->SetCurrentItemIndex(0);
            //iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);

            // update view
            RefreshViewL();
            }
        }
    }
    
// --------------------------------------------------------------------------------------------

TBool CFileBrowserFileUtils::IsCurrentDriveReadOnly()
    {
    if (iListingMode !=ENormalEntries || iCurrentPath.Length() < 2)
        return EFalse;
    else
        {
        for (TInt i=0; i<iDriveEntryList->Count(); i++)
            {
            TDriveEntry driveEntry = iDriveEntryList->At(i);
            
            if (driveEntry.iLetter == iCurrentPath[0])
                {
                if (driveEntry.iVolumeInfo.iDrive.iMediaAtt == KMediaAttWriteProtected || driveEntry.iVolumeInfo.iDrive.iMediaAtt == KMediaAttLocked || driveEntry.iVolumeInfo.iDrive.iDriveAtt == KDriveAbsent)
                    return ETrue;    
                else
                    return EFalse;
                }
            }
        }

    return EFalse;
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserFileUtils::IsCurrentItemDirectory()
    {
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (currentItemIndex < 0)
        return EFalse;
    else
        {
        if (iFileEntryList->Count() > currentItemIndex)
            {
            TFileEntry fileEntry = iFileEntryList->At(currentItemIndex) ;   
            return fileEntry.iEntry.IsDir();
            }
        else
            return EFalse;
        }
    
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::GetSelectedItemsOrCurrentItemL(CFileEntryList* aFileEntryList)
    {
    aFileEntryList->Reset();
    
    const CArrayFix<TInt>* selectionIndexes = iModel->FileListContainer()->ListBoxSelectionIndexes();

    // by default use selected items
    if (selectionIndexes && selectionIndexes->Count() > 0)
        {
        TInt ref(0);
        TKeyArrayFix key(0, ECmpTUint16);
        TInt index(0);

        for (TInt i=0; i<iFileEntryList->Count(); i++)
            {
            ref = i;

            if (selectionIndexes->Find(ref, key, index) == 0)  
                {
                aFileEntryList->AppendL(iFileEntryList->At(i));
                }
            }
        }
    
    // or if none selected, use the current item index
    else
        {
        TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
        
        if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
            {
            aFileEntryList->AppendL(iFileEntryList->At(currentItemIndex));
            }
        }  

    return aFileEntryList->Count();
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ClipboardCutL()
    {
    iClipBoardMode = EClipBoardModeCut;
    TInt operations = GetSelectedItemsOrCurrentItemL(iClipBoardList);

    _LIT(KMessage, "%d entries cut to clipboard");
    TFileName noteMsg;
    noteMsg.Format(KMessage, operations);
    
    CAknInformationNote* note = new(ELeave) CAknInformationNote;
    note->ExecuteLD(noteMsg);

    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ClipboardCopyL()
    {
    iClipBoardMode = EClipBoardModeCopy;
    TInt operations = GetSelectedItemsOrCurrentItemL(iClipBoardList);

    _LIT(KMessage, "%d entries copied to clipboard");
    TFileName noteMsg;
    noteMsg.Format(KMessage, operations);
    
    CAknInformationNote* note = new(ELeave) CAknInformationNote;
    note->ExecuteLD(noteMsg);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ClipboardPasteL()
    {
    if (iClipBoardMode == EClipBoardModeCut)
        {
        DoCopyToFolderL(iClipBoardList, iCurrentPath, ETrue);
        iClipBoardList->Reset();
        }
    else if (iClipBoardMode == EClipBoardModeCopy)
        {
        DoCopyToFolderL(iClipBoardList, iCurrentPath, EFalse);
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::CopyToFolderL(TBool aMove)
    {
    TFileName destinationFolder;
    
    // generate an icon array
    CAknIconArray* iconArray = GenerateIconArrayL(ETrue);
    CleanupStack::PushL(iconArray);

    // run folder selection dialog
    CFileBrowserDestinationFolderSelectionDlg* dlg = CFileBrowserDestinationFolderSelectionDlg::NewL(destinationFolder, iDriveEntryList, iconArray);

    TBool dialogOK(EFalse);
    
    if (aMove)
        dialogOK = dlg->RunMoveDlgLD();
    else
        dialogOK = dlg->RunCopyDlgLD();
    
    if (dialogOK)
        {
        // get entry list
        CFileEntryList* entryList = new(ELeave) CFileEntryList(32);
        GetSelectedItemsOrCurrentItemL(entryList);
        CleanupStack::PushL(entryList);

        // do the file operations
        if (aMove)
            DoCopyToFolderL(entryList, destinationFolder, ETrue);
        else
            DoCopyToFolderL(entryList, destinationFolder, EFalse);

        CleanupStack::PopAndDestroy(); // entryList   
        }

    CleanupStack::Pop(); //iconArray
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::DoCopyToFolderL(CFileEntryList* aEntryList, const TDesC& aTargetDir, TBool aDeleteSource)
    {
    if (aEntryList && aEntryList->Count() > 0)
        {
        TBool someEntryExists(EFalse);
        TBool doFileOperations(ETrue);
        TInt queryIndex(0);
        TFileName postFix;
        TInt overWriteFlags = CFileMan::EOverWrite;

        // check if some destination entries also exists
        for (TInt i=0; i<aEntryList->Count(); i++)
            {
            TFileEntry fileEntry = aEntryList->At(i);
            
            TFileName targetEntry = aTargetDir;
            targetEntry.Append(fileEntry.iEntry.iName);

            if (fileEntry.iEntry.IsDir())
                {
                targetEntry.Append(_L("\\"));
                
                if (BaflUtils::PathExists(iFs, targetEntry))
                    {
                    someEntryExists = ETrue;
                    break;
                    }
                }
            else
                {
                if (BaflUtils::FileExists(iFs, targetEntry))
                    {
                    someEntryExists = ETrue;
                    break;
                    }
                }
            }        
        
        // show a query if existing entries
        if (someEntryExists)
            {
            CAknListQueryDialog* listQueryDlg = new(ELeave) CAknListQueryDialog(&queryIndex);
            
            if (listQueryDlg->ExecuteLD(R_ENTRY_OVERWRITE_TYPE_QUERY))
                {
                if (queryIndex == EFileActionQueryPostFix)
                    {
                    CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(postFix);
                    textQuery->SetPromptL(_L("Postfix:"));
                    
                    if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
                        {
                            
                        }
                    else
                        {
                        doFileOperations = EFalse;    
                        }
                    }
                else if (queryIndex == EFileActionSkipAllExisting)
                    {
                    overWriteFlags = 0;
                    }
                }
            else
                {
                doFileOperations = EFalse;    
                }
            }
        

        if (doFileOperations)
            {
            // set correct command id depending if we are copying or moving
            TInt commandId(EFileBrowserFileOpCommandCopy);
            if (aDeleteSource)
                commandId = EFileBrowserFileOpCommandMove;
            
            // do the file operations for each entry
            for (TInt i=0; i<aEntryList->Count(); i++)
                {
                TFileEntry fileEntry = aEntryList->At(i);
                
                TFileName targetEntry = aTargetDir;
                targetEntry.Append(fileEntry.iEntry.iName);
                
                if (queryIndex == EFileActionGenerateUniqueFilenames)
                    {
                    CApaApplication::GenerateFileName(iFs, targetEntry);
                    }
                else if (queryIndex == EFileActionQueryPostFix)
                    {
                    targetEntry.Append(postFix);
                    }

                
                // append the new command to the command array
                if (fileEntry.iEntry.IsDir())
                    {
                    AppendToCommandArrayL(commandId,
                                          new(ELeave)CCommandParamsCopyOrMove(fileEntry, targetEntry, overWriteFlags|CFileMan::ERecurse)
                                         );
                    }
                else
                    {
                    AppendToCommandArrayL(commandId,
                                          new(ELeave)CCommandParamsCopyOrMove(fileEntry, targetEntry, overWriteFlags)
                                         );
                    }

                }

            // execute all operations
            if (aDeleteSource)
                StartExecutingCommandsL(_L("Moving"));
            else
                StartExecutingCommandsL(_L("Copying"));
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::DeleteL()
    {
    CFileEntryList* entries = new(ELeave) CFileEntryList(32);
    TInt operations = GetSelectedItemsOrCurrentItemL(entries);
    CleanupStack::PushL(entries);

    CAknQueryDialog* query = CAknQueryDialog::NewL();
    _LIT(KQueryMessage, "Delete %d entries?");
    TFileName queryMsg;
    queryMsg.Format(KQueryMessage, operations);
        
    if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, queryMsg))
        {
        // do the file operations for each entry
        for (TInt i=0; i<entries->Count(); i++)
            {
            TFileEntry fileEntry = entries->At(i);
            
            // append the new command to the command array
            if (fileEntry.iEntry.IsDir())
                {
                AppendToCommandArrayL(EFileBrowserFileOpCommandDelete,
                                      new(ELeave)CCommandParamsDelete(fileEntry, CFileMan::ERecurse)
                                     );
                }
            else
                {
                AppendToCommandArrayL(EFileBrowserFileOpCommandDelete,
                                      new(ELeave)CCommandParamsDelete(fileEntry, 0)
                                     );
                }

            }

        // execute all operations
        StartExecutingCommandsL(_L("Deleting"));
        }

    CleanupStack::PopAndDestroy(); //entries
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::TouchL()
    {
    CFileEntryList* entries = new(ELeave) CFileEntryList(32);
    GetSelectedItemsOrCurrentItemL(entries);
    CleanupStack::PushL(entries);
    
    TBool recurse(EFalse);

    // check if any directories and ask recursion
    for (TInt i=0; i<entries->Count(); i++)
        {
        TFileEntry fileEntry = entries->At(i);
        
        if (fileEntry.iEntry.IsDir())
            {
            CAknQueryDialog* query = CAknQueryDialog::NewL();
            if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Recurse touch for all selected dirs?")))
                {
                recurse = ETrue;
                }
            break;
            }
        }
    
    // now go through all entries    
    for (TInt i=0; i<entries->Count(); i++)
        {
        TFileEntry fileEntry = entries->At(i);

        // touch to current time
        TTime now;
        now.UniversalTime();

        
        // append the new command to the command array
        if (fileEntry.iEntry.IsDir() && recurse)
            {
            AppendToCommandArrayL(EFileBrowserFileOpCommandAttribs,
                                  new(ELeave)CCommandParamsAttribs(fileEntry, NULL, NULL, now, CFileMan::ERecurse)
                                 );
            }
        else
            {
            AppendToCommandArrayL(EFileBrowserFileOpCommandAttribs,
                                  new(ELeave)CCommandParamsAttribs(fileEntry, NULL, NULL, now, 0)
                                 );
            }
        }
       
    // execute all operations
    StartExecutingCommandsL(_L("Touching"));

    CleanupStack::PopAndDestroy(); //entries
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::RenameL()
    {
    CFileEntryList* entries = new(ELeave) CFileEntryList(16);
    GetSelectedItemsOrCurrentItemL(entries);
    CleanupStack::PushL(entries);
    
    for (TInt i=0; i<entries->Count(); i++)
        {
        TFileEntry fileEntry = entries->At(i);
        
        TFileName newName = fileEntry.iEntry.iName;
        
        CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(newName);
        textQuery->SetPromptL(_L("Enter new name:"));
        
        if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
            {
            TFileName targetEntry = fileEntry.iPath;
            targetEntry.Append(newName);
            
            TBool alreadyExists(EFalse);
            TBool doRenameOperation(ETrue);
            
            if (fileEntry.iEntry.IsDir())
                {
                //targetEntry.Append(_L("\\"));
                alreadyExists = BaflUtils::PathExists(iFs, targetEntry);
                }
            else    
                {
                alreadyExists = BaflUtils::FileExists(iFs, targetEntry);
                }                
 
            if (alreadyExists)
                {
                CAknQueryDialog* query = CAknQueryDialog::NewL();
                _LIT(KQueryMessage, "%S already exists, overwrite?");
                TFileName queryMsg;
                queryMsg.Format(KQueryMessage, &newName);
                
                if (!query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, queryMsg))
                    {
                    doRenameOperation = EFalse;                                    
                    }
                }

            if (doRenameOperation)
                {
                // append the new command to the command array
                AppendToCommandArrayL(EFileBrowserFileOpCommandRename,
                                      new(ELeave)CCommandParamsRename(fileEntry, targetEntry, CFileMan::EOverWrite)
                                     );
                }
            }
        }

    // execute all operations
    StartExecutingCommandsL(_L("Renaming"));

    CleanupStack::PopAndDestroy(); //entries
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetAttributesL()
    {
    CFileEntryList* entries = new(ELeave) CFileEntryList(16);
    GetSelectedItemsOrCurrentItemL(entries);
    CleanupStack::PushL(entries);
    
    if (entries->Count() > 0)
        {
        TFileName naviText = _L("Multiple entries");
        
        TUint setAttMask(0);
        TUint clearAttMask(0);
        TBool recurse(EFalse);
        
        // set default masks if only one file selected
        if (entries->Count() == 1)
            {
            TFileEntry fileEntry = entries->At(0);
            
            naviText.Copy(fileEntry.iEntry.iName);
            
            if (fileEntry.iEntry.IsArchive())
                setAttMask |= KEntryAttArchive;
            else
                clearAttMask |= KEntryAttArchive;
            
            if (fileEntry.iEntry.IsHidden())
                setAttMask |= KEntryAttHidden;
            else
                clearAttMask |= KEntryAttHidden;
 
            if (fileEntry.iEntry.IsReadOnly())
                setAttMask |= KEntryAttReadOnly;
            else
                clearAttMask |= KEntryAttReadOnly;

            if (fileEntry.iEntry.IsSystem())
                setAttMask |= KEntryAttSystem;
            else
                clearAttMask |= KEntryAttSystem;
            }
        
        iModel->FileListContainer()->SetScreenLayoutL(EDisplayModeFullScreen);
        iModel->FileListContainer()->SetNaviPaneTextL(naviText);

        CFileBrowserAttributeEditorDlg* dlg = CFileBrowserAttributeEditorDlg::NewL(setAttMask, clearAttMask, recurse);
        TBool dlgResult = dlg->RunEditorLD();

        iModel->FileListContainer()->SetScreenLayoutL(iModel->Settings().iDisplayMode);
        iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);        
        
        if (dlgResult && (setAttMask > 0 || clearAttMask > 0))
            {
            for (TInt i=0; i<entries->Count(); i++)
                {
                TFileEntry fileEntry = entries->At(i);
 
                // append the new command to the command array
                if (fileEntry.iEntry.IsDir() && recurse)
                    {
                    AppendToCommandArrayL(EFileBrowserFileOpCommandAttribs,
                                          new(ELeave)CCommandParamsAttribs(fileEntry, setAttMask, clearAttMask, fileEntry.iEntry.iModified, CFileMan::ERecurse)
                                         );
                    }
                else
                    {
                    AppendToCommandArrayL(EFileBrowserFileOpCommandAttribs,
                                          new(ELeave)CCommandParamsAttribs(fileEntry, setAttMask, clearAttMask, fileEntry.iEntry.iModified, 0)
                                         );
                    }
                }

            // execute all operations
            StartExecutingCommandsL(_L("Changing attributes"));          
            }
        }
    
    CleanupStack::PopAndDestroy(); //entries
    }
       
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SearchL()
    {
	iModel->FileListContainer()->SetScreenLayoutL(EDisplayModeFullScreen);
    iModel->FileListContainer()->SetNaviPaneTextL(KNullDesC);
    
    iSearchAttributes.iSearchDir = iCurrentPath;
    
    CFileBrowserSearchQueryDlg* dlg = CFileBrowserSearchQueryDlg::NewL(iSearchAttributes);
    TBool dlgResult = dlg->RunQueryLD();

    iModel->FileListContainer()->SetScreenLayoutL(iModel->Settings().iDisplayMode);
    iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);        
    
    if (dlgResult)
        {
        iModel->EikonEnv()->BusyMsgL(_L("** Searching **"), TGulAlignment(EHCenterVTop));

        iFileEntryList->Reset();
        
        // if search dir is empty, find from all drives
        if (iSearchAttributes.iSearchDir == KNullDesC)
            {
            for (TInt i=0; i<iDriveEntryList->Count(); i++)
                {
                TDriveEntry driveEntry = iDriveEntryList->At(i);

                TBuf<10> driveRoot;
                driveRoot.Append(driveEntry.iLetter);
                driveRoot.Append(_L(":\\"));
                
                DoSearchFiles(iSearchAttributes.iWildCards, driveRoot);
                
                if (iSearchAttributes.iRecurse)
                    DoSearchFilesRecursiveL(iSearchAttributes.iWildCards, driveRoot);
                
                }
            
            }
        
        // otherwise just search from the selected directory
        else
            {
            DoSearchFiles(iSearchAttributes.iWildCards, iSearchAttributes.iSearchDir);
            
            if (iSearchAttributes.iRecurse)
                DoSearchFilesRecursiveL(iSearchAttributes.iWildCards, iSearchAttributes.iSearchDir);
            }
        
        iModel->EikonEnv()->BusyMsgCancel();
        
        TInt operations = iFileEntryList->Count();
        
        iListingMode = ESearchResults;
        iModel->FileListContainer()->ListBox()->SetCurrentItemIndex(0);
        RefreshViewL();            

        _LIT(KMessage, "%d entries found");
        TFileName noteMsg;
        noteMsg.Format(KMessage, operations);
        
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(noteMsg);
        }
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::DoSearchFiles(const TDesC& aFileName, const TDesC& aPath)
    {
    TFindFile fileFinder(iFs);
    CDir* dir;
    TInt err = fileFinder.FindWildByPath(aFileName, &aPath, dir);

    while (err == KErrNone)
        {
        for (TInt i=0; i<dir->Count(); i++)
            {
            TEntry entry = (*dir)[i];
            
            TTime entryModified = entry.iModified;
            // convert from universal time
            if ( iTz.ConvertToLocalTime( entryModified ) == KErrNone )
                {
                entryModified = entry.iModified;
                }
            if ((entry.IsDir() && entryModified >= iSearchAttributes.iMinDate && entryModified <= iSearchAttributes.iMaxDate) ||
                (!entry.IsDir() &&entry.iSize >= iSearchAttributes.iMinSize && entry.iSize <= iSearchAttributes.iMaxSize &&
                 entryModified >= iSearchAttributes.iMinDate && entryModified <= iSearchAttributes.iMaxDate))
                {
                TParse parsedName;
                parsedName.Set(entry.iName, &fileFinder.File(), NULL);
                
                if (parsedName.Drive().Length() && aPath.Length() && parsedName.Drive()[0] == aPath[0])
                    {
                    TFileEntry fileEntry;
                    fileEntry.iPath = parsedName.DriveAndPath();
                    fileEntry.iEntry = entry;
                    fileEntry.iDirEntries = KErrNotFound;
                    
                    TBool appendToArray(EFalse);
                    
                    // append directories always
                    if (entry.IsDir() && !iSearchAttributes.iTextInFile.Length())
                        {
                        fileEntry.iIconId = EFixedIconFolder;
                        appendToArray = ETrue;    
                        }
                    
                    // normal file
                    else
                        {
                        fileEntry.iIconId = EFixedIconEmpty;
                        
                        // check if a string needs to be found inside the file
                        if (iSearchAttributes.iTextInFile.Length())
                            {
                            // currently only 8-bit searching, so convert from 16-bit
                            TBuf8<256> searchText;
                            searchText.Copy(iSearchAttributes.iTextInFile);
                            
                            // specify buffer size and create a buffer
                            const TInt KReadBufSize = 1024*1024;
                            HBufC8* buffer = HBufC8::NewLC(KReadBufSize);
                            TPtr8 bufferPtr = buffer->Des();

                            // open the file for reading
                            RFile file;
                            if (file.Open(iFs, parsedName.FullName(), EFileRead) == KErrNone)
                                {
                                TInt currentPos(0);
                                
                                for (;;)
                                    {
                                    // read from the file
                                    file.Read(currentPos, bufferPtr, KReadBufSize);

                                    // stop looping if the read buffer isn't long enough
                                    if (bufferPtr.Length() < searchText.Length())
                                        {
                                        break;
                                        }
                                        
                                    // try to find the text
                                    if (bufferPtr.FindF(searchText) >= 0)
                                        {
                                        // match found!
                                        appendToArray = ETrue;
                                        break;
                                        }
                                        
                                    // calculate the next position
                                    currentPos += KReadBufSize - searchText.Length();    
                                    }
                                
                                file.Close();
                                }
                            
                            CleanupStack::PopAndDestroy(); //buffer
                            }
                        else
                            {
                            appendToArray = ETrue;
                            }
                        }

                    if (appendToArray)
                        TRAP(err, iFileEntryList->AppendL(fileEntry));    
                    }
                }
            }

        delete dir;
        err = fileFinder.FindWild(dir);
        }

    return err;
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::DoSearchFilesRecursiveL(const TDesC& aFileName, const TDesC& aPath)
	{
    TInt err(KErrNone);
    CDirScan* scan = CDirScan::NewLC(iFs);
    scan->SetScanDataL(aPath, KEntryAttDir|KEntryAttMatchMask, ESortByName | EAscending | EDirsFirst);
    CDir* dir = NULL;

    for(;;)
        {
        TRAP(err, scan->NextL(dir));
        if (!dir  || (err != KErrNone))
            break;

        for (TInt i=0; i<dir->Count(); i++)
            {
            TEntry entry = (*dir)[i];
            
            if (entry.IsDir())
                {
                TFileName path(scan->FullPath());
                
                if (path.Length())
                    {
                    path.Append(entry.iName);
                    path.Append(_L("\\"));
                    DoSearchFiles(aFileName, path);
                    }
                }
            }
        delete(dir);
        }

    CleanupStack::PopAndDestroy(scan);
    return err;
    }
         
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::NewFileL()
    {
    TFileName newName;
    
    CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(newName);
    textQuery->SetPromptL(_L("Enter filename:"));
    
    if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
        {
        TFileName fullPath = iCurrentPath;
        fullPath.Append(newName);
        
        TParse nameParser;
        TInt err = nameParser.SetNoWild(fullPath, NULL, NULL);
        
        if (err == KErrNone)
            {
            // do not try to recreate the file if it already exists
            if (BaflUtils::PathExists(iFs, nameParser.DriveAndPath()))
                {
                err = iFileOps->CreateEmptyFile(fullPath);
                }
            else
                err = KErrAlreadyExists;
            }

        if (err == KErrNone)    
            {
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("New file created"));    
            }
        
        else if (err == KErrAlreadyExists)    
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("File already exists"));    
            }            

        else    
            {
            CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

            CleanupStack::PopAndDestroy();  //textResolver     
            }

        RefreshViewL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::NewDirectoryL()
    {    
    TFileName newDirectory;
    
    CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(newDirectory);
    textQuery->SetPromptL(_L("Enter directory name:"));
    
    if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
        {
        TFileName fullPath = iCurrentPath;
        fullPath.Append(newDirectory);
        fullPath.Append(_L("\\"));
        
        TInt err = iFileOps->MkDirAll(fullPath, 0, ETrue);

        if (err == KErrNone)    
            {
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("New directory created"));    
            }
        
        else if (err == KErrAlreadyExists)    
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("Directory already exists"));    
            }            

        else    
            {
            CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

            CleanupStack::PopAndDestroy();  //textResolver     
            }

        RefreshViewL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SendToL()
    {
    CFileEntryList* entries = new(ELeave) CFileEntryList(32);
    GetSelectedItemsOrCurrentItemL(entries);
    CleanupStack::PushL(entries);

    TInt operations(0);

    CSendUi* sendUi = CSendUi::NewL();
    CleanupStack::PushL(sendUi);
    CMessageData* messageData = CMessageData::NewL();
    CleanupStack::PushL(messageData);            
        
    for (TInt i=0; i<entries->Count(); i++)
        {
        TFileEntry fileEntry = entries->At(i);

        // only supported for normal file entries
        if (!fileEntry.iEntry.IsDir())
            {
            TFileName fullPath = fileEntry.iPath;
            fullPath.Append(fileEntry.iEntry.iName);
            
            messageData->AppendAttachmentL(fullPath);
            operations++;
            
            }
        }
    
    if (operations > 0)
        {
        sendUi->ShowQueryAndSendL(messageData, TSendingCapabilities(0, 0, TSendingCapabilities::ESupportsAttachments));

        // I guess this note is not needed..
        /* 
        _LIT(KMessage, "%d entries sent");
        TFileName noteMsg;
        noteMsg.Format(KMessage, operations);
        
        CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
        note->ExecuteLD(noteMsg);    
        */
        }
    else
        {
        _LIT(KMessage, "Nothing to send");
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(KMessage);    
        }        
        
    CleanupStack::PopAndDestroy(3); // entries, sendUi, messageData    
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::CompressL()
    {
    CFileEntryList* entries = new(ELeave) CFileEntryList(16);
    GetSelectedItemsOrCurrentItemL(entries);
    CleanupStack::PushL(entries);
    
    for (TInt i=0; i<entries->Count(); i++)
        {
        TFileEntry fileEntry = entries->At(i);
        
        TFileName sourceEntry = fileEntry.iPath;
        sourceEntry.Append(fileEntry.iEntry.iName);
        
        TFileName targetEntry = sourceEntry;
        targetEntry.Append(KGZipExt);
        
        TFileName targetEntryShort;
        targetEntryShort.Copy(fileEntry.iEntry.iName);
        targetEntryShort.Append(KGZipExt);
        
        if (BaflUtils::FileExists(iFs, targetEntry))
            {
            _LIT(KMessage, "%S already exists");
            TFileName noteMsg;
            noteMsg.Format(KMessage, &targetEntryShort);
            
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(noteMsg);           
            }
        else
            {
            TRAPD(err,
                RFile inputFile;
                User::LeaveIfError(inputFile.Open(iFs, sourceEntry, EFileStream | EFileRead | EFileShareAny));
                CleanupClosePushL(inputFile);
                
                CEZFileToGZip* gZip = CEZFileToGZip::NewLC(iFs, targetEntry, inputFile);

                while (gZip->DeflateL())
                    ;

                CleanupStack::PopAndDestroy(2); //inputFile, gZip
                );
             
             if (err == KErrNone)
                {
                _LIT(KMessage, "%S created succesfully");
                TFileName noteMsg;
                noteMsg.Format(KMessage, &targetEntryShort);
                
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(noteMsg);
                }
             else
                {
                _LIT(KMessage, "Unable to create %S");
                TFileName noteMsg;
                noteMsg.Format(KMessage, &targetEntryShort);
                
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(noteMsg);
                }
                
            RefreshViewL();
            }
        }

    CleanupStack::PopAndDestroy(); //entries
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::DecompressL()
    {
    CFileEntryList* entries = new(ELeave) CFileEntryList(16);
    GetSelectedItemsOrCurrentItemL(entries);
    CleanupStack::PushL(entries);
    
    for (TInt i=0; i<entries->Count(); i++)
        {
        TFileEntry fileEntry = entries->At(i);
        
        TFileName sourceEntry = fileEntry.iPath;
        sourceEntry.Append(fileEntry.iEntry.iName);
        
        TFileName targetEntry;
        TFileName targetEntryShort;

        TInt sourceNameLen = fileEntry.iEntry.iName.Length();
        TInt gZipExtLen = KGZipExt().Length();

        if (sourceNameLen > gZipExtLen && sourceEntry.Right(gZipExtLen).CompareF(KGZipExt) == 0)
            {
            targetEntry = sourceEntry.Left(sourceEntry.Length() - gZipExtLen);
            targetEntryShort = fileEntry.iEntry.iName.Left(sourceNameLen - gZipExtLen);
            
            if (BaflUtils::FileExists(iFs, targetEntry))
                {
                _LIT(KMessage, "%S already exists");
                TFileName noteMsg;
                noteMsg.Format(KMessage, &targetEntryShort);
                
                CAknInformationNote* note = new(ELeave) CAknInformationNote;
                note->ExecuteLD(noteMsg);           
                }
            else
                {
                TRAPD(err,
                    RFile outputFile;
                    User::LeaveIfError(outputFile.Create(iFs, targetEntry, EFileStream | EFileWrite | EFileShareExclusive));
                    CleanupClosePushL(outputFile);
                    
                    CEZGZipToFile* gZip = CEZGZipToFile::NewLC(iFs, sourceEntry, outputFile);

                    while (gZip->InflateL())
                        ;            

                    CleanupStack::PopAndDestroy(2); //outputFile, gZip
                    );
                 
                 if (err == KErrNone)
                    {
                    _LIT(KMessage, "%S decompressed succesfully");
                    TFileName noteMsg;
                    noteMsg.Format(KMessage, &fileEntry.iEntry.iName);
                    
                    CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                    note->ExecuteLD(noteMsg);
                    }
                 else
                    {
                    _LIT(KMessage, "Unable to decompress %S");
                    TFileName noteMsg;
                    noteMsg.Format(KMessage, &fileEntry.iEntry.iName);
                    
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD(noteMsg);
                    }
                    
                RefreshViewL();
                }
            }
        else
            {
            _LIT(KMessage, "%S does not have %S extension");
            TFileName noteMsg;
            noteMsg.Format(KMessage, &fileEntry.iEntry.iName, &KGZipExt);
            
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(noteMsg);              
            }
        }

    CleanupStack::PopAndDestroy(); //entries
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::PropertiesL()
    {
    TBool showDialog(EFalse);
    
    _LIT(KPropertiesEntryStr,       "%S\t%S");
    _LIT(KPropertiesEntryInt,       "%S\t%d");
    _LIT(KPropertiesEntryUintHex,   "%S\t0x%x");
    
    _LIT(KDateFormat,               "%D%M%Y%/0%1%/1%2%/2%3%/3");
    _LIT(KTimeFormat,               "%-B%:0%J%:1%T%:2%S%:3%+B");
    
    _LIT(KMediaType,                "Media");
    _LIT(KAttributes,               "Atts");
    _LIT(KVolumeName,               "Name");
    _LIT(KUniqueID,                 "ID");
    _LIT(KNumberOfEntries,          "Entries");
    _LIT(KNumberOfFiles,            "Files");
    _LIT(KUsed,                     "Used (B)");
    _LIT(KFree,                     "Free (B)");
    _LIT(KSize,                     "Size (B)");
    _LIT(KPath,                     "Path");
    _LIT(KDate,                     "Date");
    _LIT(KTime,                     "Time");
    _LIT(KMimeType,                 "Mime");
    _LIT(KOpensWith,                "Opens");
 

    // create an array for the items
	CDesCArray* entryLines = new(ELeave) CDesCArrayFlat(16);
	CleanupStack::PushL(entryLines);
    TFileName titleText;
    
    // just get current item
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (IsDriveListViewActive() && iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        // it is a drive entry
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
        TFileName textEntry;
        
        // set title
        titleText.Append(driveEntry.iLetter);
        titleText.Append(_L(":"));
        
        // media type
        textEntry.Format(KPropertiesEntryStr, &KMediaType, &driveEntry.iMediaTypeDesc);
        entryLines->AppendL(textEntry);

        // attributes
        textEntry.Format(KPropertiesEntryStr, &KAttributes, &driveEntry.iAttributesDesc);
        entryLines->AppendL(textEntry);

        // volume name
        if (driveEntry.iVolumeInfo.iName.Length())
            {
            textEntry.Format(KPropertiesEntryStr, &KVolumeName, &driveEntry.iVolumeInfo.iName);
            entryLines->AppendL(textEntry);
            }

        // unique id
        textEntry.Format(KPropertiesEntryUintHex, &KUniqueID, driveEntry.iVolumeInfo.iUniqueID);
        entryLines->AppendL(textEntry);
        
        // number of files
        if (iModel->Settings().iShowSubDirectoryInfo)
            {
            iFindFileEntryList->Reset();
            
            TBuf<10> driveRoot;
            driveRoot.Append(driveEntry.iLetter);
            driveRoot.Append(_L(":\\"));

            DoFindFiles(_L("*"), driveRoot);
            DoFindFilesRecursiveL(_L("*"), driveRoot);
            
            textEntry.Format(KPropertiesEntryInt, &KNumberOfFiles, iFindFileEntryList->Count());
            entryLines->AppendL(textEntry);
            
            iFindFileEntryList->Reset();
            }
      
        // used
        TBuf<32> usedBuf;
        usedBuf.AppendNum(driveEntry.iVolumeInfo.iSize-driveEntry.iVolumeInfo.iFree, TRealFormat(KDefaultRealWidth, 0));
        textEntry.Format(KPropertiesEntryStr, &KUsed, &usedBuf);
        entryLines->AppendL(textEntry);        

        // free
        TBuf<32> freeBuf;
        freeBuf.AppendNum(driveEntry.iVolumeInfo.iFree, TRealFormat(KDefaultRealWidth, 0));
        textEntry.Format(KPropertiesEntryStr, &KFree, &freeBuf);
        entryLines->AppendL(textEntry);
                
        // size
        TBuf<32> sizeBuf;
        sizeBuf.AppendNum(driveEntry.iVolumeInfo.iSize, TRealFormat(KDefaultRealWidth, 0));
        textEntry.Format(KPropertiesEntryStr, &KSize, &sizeBuf);
        entryLines->AppendL(textEntry);   

        showDialog = ETrue;
        }
    
    else if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        // it is a file or a directory entry
        TFileEntry fileEntry = iFileEntryList->At(currentItemIndex);
        TFileName textEntry;
        
        // set title
        titleText.Append(fileEntry.iEntry.iName);
        
        // path
        textEntry.Format(KPropertiesEntryStr, &KPath, &fileEntry.iPath);
        entryLines->AppendL(textEntry);

        // date
        TTime entryModified = fileEntry.iEntry.iModified;
        
        // convert from universal time
        if ( iTz.ConvertToLocalTime( entryModified ) == KErrNone )
            {
            entryModified = fileEntry.iEntry.iModified; // use universal time
            }
        
        TBuf<32> dateBuf;
        entryModified.FormatL(dateBuf, KDateFormat);
        textEntry.Format(KPropertiesEntryStr, &KDate, &dateBuf);
        entryLines->AppendL(textEntry);

        // time
        TBuf<32> timeBuf;
        entryModified.FormatL(timeBuf, KTimeFormat);
        textEntry.Format(KPropertiesEntryStr, &KTime, &timeBuf);
        entryLines->AppendL(textEntry);

        if (!fileEntry.iEntry.IsDir())
            {
            // size
            TBuf<32> sizeBuf;
            sizeBuf.AppendNum(fileEntry.iEntry.iSize, TRealFormat(KDefaultRealWidth, 0));
            textEntry.Format(KPropertiesEntryStr, &KSize, &sizeBuf);
            entryLines->AppendL(textEntry);            
            }
        else if (fileEntry.iEntry.IsDir() && iModel->Settings().iShowSubDirectoryInfo)
            {
            iFindFileEntryList->Reset();
            
            TFileName fullPath;
            fullPath.Append(fileEntry.iPath);
            fullPath.Append(fileEntry.iEntry.iName);
            fullPath.Append(_L("\\"));
            
            // number of entries
            CDir* subDir = NULL;
            if (iFs.GetDir(fullPath, KEntryAttMatchMask, ESortNone | EDirsFirst, subDir) == KErrNone)
                {
                CleanupStack::PushL(subDir);
                textEntry.Format(KPropertiesEntryInt, &KNumberOfEntries, subDir->Count());
                entryLines->AppendL(textEntry);
                CleanupStack::PopAndDestroy(); //subDir 
                }
            
            // number of files
            DoFindFiles(_L("*"), fullPath);
            DoFindFilesRecursiveL(_L("*"), fullPath);
            textEntry.Format(KPropertiesEntryInt, &KNumberOfFiles, iFindFileEntryList->Count());
            entryLines->AppendL(textEntry);
            
            // size
            TInt64 size(0);
            for (TInt i=0; i<iFindFileEntryList->Count(); i++)
                {
                TFileEntry fileEntry = iFindFileEntryList->At(i);
                size += fileEntry.iEntry.iSize;
                }
            
            TBuf<32> sizeBuf;
            sizeBuf.AppendNum(size, TRealFormat(KDefaultRealWidth, 0));
            textEntry.Format(KPropertiesEntryStr, &KSize, &sizeBuf);
            entryLines->AppendL(textEntry); 
            
            iFindFileEntryList->Reset();            
            }
        
        // attributes
        TBuf<32> attBuf;
        if (fileEntry.iEntry.IsArchive())
            attBuf.Append(_L("Arc "));
        if (fileEntry.iEntry.IsHidden())
            attBuf.Append(_L("Hid "));
        if (fileEntry.iEntry.IsReadOnly())
            attBuf.Append(_L("R/O "));
        if (fileEntry.iEntry.IsSystem())
            attBuf.Append(_L("Sys"));
        textEntry.Format(KPropertiesEntryStr, &KAttributes, &attBuf);
        entryLines->AppendL(textEntry);            

        if (!fileEntry.iEntry.IsDir())
            {
            // mime type
            TFileName fullPath = fileEntry.iPath;
            fullPath.Append(fileEntry.iEntry.iName);
            TDataType dataType;
            TUid appUid;
            if (iModel->LsSession().AppForDocument(fullPath, appUid, dataType) == KErrNone)
                {
                TBuf<128> mimeTypeBuf;
                mimeTypeBuf.Copy(dataType.Des8());
                if (mimeTypeBuf == KNullDesC)
                    mimeTypeBuf.Copy(_L("N/A"));
                textEntry.Format(KPropertiesEntryStr, &KMimeType, &mimeTypeBuf);
                entryLines->AppendL(textEntry); 
                }

            // opens with
            TApaAppInfo appInfo;
            if (iModel->LsSession().GetAppInfo(appInfo, appUid) == KErrNone)
                {
                TFileName opensWithBuf;
                _LIT(KOpensWithFormat, "%S (0x%08X)");
                opensWithBuf.Format(KOpensWithFormat, &appInfo.iCaption, appInfo.iUid);
                textEntry.Format(KPropertiesEntryStr, &KOpensWith, &opensWithBuf);
                entryLines->AppendL(textEntry); 
                }
            }
        
        showDialog = ETrue;
        }   


    if (showDialog)
        {
        CAknSingleHeadingPopupMenuStyleListBox* listBox = new(ELeave) CAknSingleHeadingPopupMenuStyleListBox;
        CleanupStack::PushL(listBox);
        
        CAknPopupList* popupList = CAknPopupList::NewL(listBox, R_AVKON_SOFTKEYS_OK_EMPTY, AknPopupLayouts::EMenuDoubleWindow);
        CleanupStack::PushL(popupList);

        listBox->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
        listBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
        listBox->CreateScrollBarFrameL(ETrue);
        listBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
        listBox->Model()->SetItemTextArray(entryLines);
        listBox->Model()->SetOwnershipType(ELbmDoesNotOwnItemArray);
        listBox->HandleItemAdditionL();
        
        popupList->SetTitleL(titleText);
        popupList->ExecuteLD();
        
        CleanupStack::Pop(); // popupList
        CleanupStack::PopAndDestroy(); // listBox
        }

    CleanupStack::PopAndDestroy(); //entryLines
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::OpenWithApparcL()
    {
    // just get current item
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TFileEntry fileEntry = iFileEntryList->At(currentItemIndex);
        
        TFileName fullPath = fileEntry.iPath;
        fullPath.Append(fileEntry.iEntry.iName);
        
        TInt err1(KErrNone), err2(KErrNone);
        TUid appUid;
        TDataType dataType;
        TBool isProgram;
        
        err1 = iModel->LsSession().AppForDocument(fullPath, appUid, dataType);
        err2 = iModel->LsSession().IsProgram(fullPath, isProgram);
        
        if (err1 == KErrNone && err2 == KErrNone)
            {
            if (appUid != KNullUid)
                {
                // found an app, run using it
                TApaTaskList taskList(iModel->EikonEnv()->WsSession());
                TApaTask task = (taskList.FindApp(appUid));
                if (task.Exists())
                    {
                    User::LeaveIfError(task.SwitchOpenFile(fullPath));
                    task.BringToForeground();
                    }
                else
                    {
                    TThreadId id;
                    iModel->LsSession().StartDocument(fullPath, appUid, id, RApaLsSession::ESwitchFiles);
                    }
                }
            else if (isProgram)
                {
                LaunchProgramL(fullPath);
                }            
            }
        }   
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::HandleServerAppExit(TInt aReason)
	{
	if (iOpenFileService)
		{
		delete iOpenFileService;
		iOpenFileService = NULL;
		}
	MAknServerAppExitObserver::HandleServerAppExit(aReason);
	}

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::OpenWithDocHandlerL(TBool aEmbed)
    {
    // just get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TFileEntry fileEntry = iFileEntryList->At(currentItemIndex);
        
        TFileName fullPath = fileEntry.iPath;
        fullPath.Append(fileEntry.iEntry.iName);

        RFile sharableFile;
        iDocHandler->OpenTempFileL(fullPath, sharableFile);
        CleanupClosePushL(sharableFile);
        TDataType dataType;
        if (aEmbed)
    	    iDocHandler->OpenFileEmbeddedL(sharableFile, dataType);
        else
    	    iDocHandler->OpenFileL(sharableFile, dataType);
        CleanupStack::PopAndDestroy();  //sharableFile
        }   
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::OpenWithFileServiceL()
    {
    // just get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TFileEntry fileEntry = iFileEntryList->At(currentItemIndex);
        
        TFileName fullPath = fileEntry.iPath;
        fullPath.Append(fileEntry.iEntry.iName);

    	TBool itemSelected(EFalse);
        RFile sharableFile;
        TUid appUidExtracted;
        
        TRAPD(err,
           
        	// use doc handler to get a sharable file handle
            iDocHandler->OpenTempFileL(fullPath, sharableFile);
            CleanupClosePushL(sharableFile);
            
            // find the data type
            TDataRecognitionResult dataType;
        	User::LeaveIfError(iModel->LsSession().RecognizeData(sharableFile, dataType));

        	// get all apps that support OpenFileService for this datatype
        	CApaAppServiceInfoArray* apps = iModel->LsSession().GetServiceImplementationsLC(KOpenServiceUid, dataType.iDataType);
        	TArray<TApaAppServiceInfo> appServiceInfos = apps->Array();
    	
        	if (appServiceInfos.Count() > 0)
        		{
        		// build an array of the app names
        		CDesCArray* names = new (ELeave) CDesCArrayFlat(16);
        		CleanupStack::PushL(names);
        		for (TInt ii=0; ii<appServiceInfos.Count(); ii++)
        			{
        			TUid appUid = appServiceInfos[ii].Uid();
        			TApaAppInfo appInfo;
        			if (iModel->LsSession().GetAppInfo(appInfo, appUid) == KErrNone)
        				{
        				names->AppendL(appInfo.iCaption);
        				}
        			else
        				{
        				TBuf<10> buf;
        				buf.Format(_L("0x%08x"), appUid);
        				names->AppendL(buf);
        				}
        			}
    		
        		// use a list query to select the app
        		TInt index(0);
        		CAknListQueryDialog* dlg = new(ELeave) CAknListQueryDialog(&index);
        		dlg->PrepareLC(R_APP_SELECTION_QUERY);
        		dlg->SetItemTextArray(names);
        		dlg->SetOwnershipType(ELbmDoesNotOwnItemArray);

        		if(dlg->RunLD())
        			{
        			// extract the chosen UID
        			appUidExtracted = appServiceInfos[index].Uid();
        			itemSelected = ETrue;
        			}
    		
        		CleanupStack::PopAndDestroy(); // names
        		}

            if (itemSelected)
                {
                // start OpenFileService for the selected app
                CAiwGenericParamList& paramList = iDocHandler->InParamListL();
                
                if (iOpenFileService)
                    {
                    delete iOpenFileService;
                    iOpenFileService = NULL;
                    }

                iOpenFileService = CAknOpenFileService::NewL(appUidExtracted, sharableFile, this, &paramList);
                }

            CleanupStack::PopAndDestroy(2); // sharableFile, apps

            ); // TRAPD

        if (err != KErrNone)
            {
            _LIT(KMessage, "No file service available for %S");
            TFileName noteMsg;
            noteMsg.Format(KMessage, &fileEntry.iEntry.iName);
            
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(noteMsg);            
            }
        }   
    }
    
// --------------------------------------------------------------------------------------------

TBool CFileBrowserFileUtils::FileExists(const TDesC& aPath)
    {
    return BaflUtils::FileExists(iFs, aPath);
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::LaunchProgramL(const TDesC& aPath)
    {
    CApaCommandLine* commandLine = CApaCommandLine::NewLC();
    commandLine->SetExecutableNameL(aPath);
    commandLine->SetCommandL(EApaCommandRun);
    TInt err = iModel->LsSession().StartApp(*commandLine);
    CleanupStack::PopAndDestroy(); //commandLine
    return err;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::MemoryInfoPopupL()
    {
    _LIT(KUsedBytesEntry, "Used: %S bytes\n");
    _LIT(KFreeBytesEntry, "Free: %S bytes\n");
    _LIT(KSizeBytesEntry, "Size: %S bytes\n");
        
    HBufC* messageBuf = HBufC::NewLC(2048);
    TPtr messagePtr = messageBuf->Des();
    TBuf<128> entryLine;

    // RAM
    TMemoryInfoV1Buf ramMemory;
    UserHal::MemoryInfo(ramMemory);
    messagePtr.Append(_L("RAM\n"));

    // used
    TBuf<32> usedBuf;
    usedBuf.AppendNum(ramMemory().iMaxFreeRamInBytes-ramMemory().iFreeRamInBytes, TRealFormat(KDefaultRealWidth, 0));
    entryLine.Format(KUsedBytesEntry, &usedBuf);
    messagePtr.Append(entryLine);        

    // free
    TBuf<32> freeBuf;
    freeBuf.AppendNum(ramMemory().iFreeRamInBytes, TRealFormat(KDefaultRealWidth, 0));
    entryLine.Format(KFreeBytesEntry, &freeBuf);
    messagePtr.Append(entryLine);
            
    // size
    TBuf<32> sizeBuf;
    sizeBuf.AppendNum(ramMemory().iMaxFreeRamInBytes, TRealFormat(KDefaultRealWidth, 0));
    entryLine.Format(KSizeBytesEntry, &sizeBuf);
    messagePtr.Append(entryLine); 
    
    // All drives
    for (TInt i=0; i<iDriveEntryList->Count(); i++)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(i);

        messagePtr.Append(_L("\n"));
        messagePtr.Append(driveEntry.iLetter);
        messagePtr.Append(_L(":\n"));

        // used
        TBuf<32> usedBuf;
        usedBuf.AppendNum(driveEntry.iVolumeInfo.iSize-driveEntry.iVolumeInfo.iFree, TRealFormat(KDefaultRealWidth, 0));
        entryLine.Format(KUsedBytesEntry, &usedBuf);
        messagePtr.Append(entryLine);        

        // free
        TBuf<32> freeBuf;
        freeBuf.AppendNum(driveEntry.iVolumeInfo.iFree, TRealFormat(KDefaultRealWidth, 0));
        entryLine.Format(KFreeBytesEntry, &freeBuf);
        messagePtr.Append(entryLine);
                
        // size
        TBuf<32> sizeBuf;
        sizeBuf.AppendNum(driveEntry.iVolumeInfo.iSize, TRealFormat(KDefaultRealWidth, 0));
        entryLine.Format(KSizeBytesEntry, &sizeBuf);
        messagePtr.Append(entryLine); 
        }

    CAknMessageQueryDialog* dialog = new(ELeave) CAknMessageQueryDialog;
    dialog->PrepareLC(R_MEMORYINFO_DIALOG);
    dialog->SetMessageTextL(messagePtr);
    dialog->RunLD();
    
    CleanupStack::PopAndDestroy(); //messageBuf
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ShowFileCheckSumsL(TInt aType)
    {
     // get current item
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TFileEntry fileEntry = iFileEntryList->At(currentItemIndex);
        TFileName fullPath = fileEntry.iPath;
        fullPath.Append(fileEntry.iEntry.iName);
        
        // try to open the file for reading
        RFile fileP;
        TInt err = fileP.Open(iModel->EikonEnv()->FsSession(), fullPath, EFileRead|EFileShareReadersOnly);
        
        if (err == KErrNone)
            {
            CleanupClosePushL(fileP);

            TBuf<128> checkSum;
            HBufC* messageBuf = HBufC::NewLC(2048);
            TPtr messagePtr = messageBuf->Des();
            
            // get MD5 checksum
            if (aType == EFileBrowserCmdFileChecksumsMD5)
                {
                CMD5* md5 = CMD5::NewL();
                CleanupStack::PushL(md5);
                HBufC8* md5Buf = MessageDigestInHexLC(md5, fileP);
                checkSum.Copy(*md5Buf);
                messagePtr.Append(_L("MD5\n"));
                messagePtr.Append(checkSum);
                CleanupStack::PopAndDestroy(2); // md5, md5Buf
                }

            // get MD2 checksum
            else if (aType == EFileBrowserCmdFileChecksumsMD2)
                {                
                CMD2* md2 = CMD2::NewL();
                CleanupStack::PushL(md2);
                HBufC8* md2Buf = MessageDigestInHexLC(md2, fileP);
                checkSum.Copy(*md2Buf);
                messagePtr.Append(_L("MD2\n"));
                messagePtr.Append(checkSum);
                CleanupStack::PopAndDestroy(2); // md2, md2Buf
                }

            // get SHA-1 checksum
            else if (aType == EFileBrowserCmdFileChecksumsSHA1)
                {
                CSHA1* sha1 = CSHA1::NewL();
                CleanupStack::PushL(sha1);
                HBufC8* sha1Buf = MessageDigestInHexLC(sha1, fileP);
                checkSum.Copy(*sha1Buf);
                messagePtr.Append(_L("SHA-1\n"));
                messagePtr.Append(checkSum);
                CleanupStack::PopAndDestroy(2); // sha1, sha1Buf
                }
            
            else
                {
                User::Panic(_L("Inv.CS.Type"), 723);
                }
                            
            // show dialog
            CAknMessageQueryDialog* dialog = new(ELeave) CAknMessageQueryDialog;
            dialog->PrepareLC(R_CHECKSUMS_DIALOG);
            dialog->SetMessageTextL(messagePtr);
            dialog->SetHeaderTextL(fileEntry.iEntry.iName);
            dialog->RunLD();
    
            CleanupStack::PopAndDestroy(2); // messageBuf, fileP
            }
        else
            {
            // open failed, show an error note
            CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

            CleanupStack::PopAndDestroy();  //textResolver               
            }
            
        }
    }

// --------------------------------------------------------------------------------------------

HBufC8* CFileBrowserFileUtils::MessageDigestInHexLC(CMessageDigest* aMD, RFile& aFile)
    {
    // seek to file start
    TInt startPos(0);
    aFile.Seek(ESeekStart, startPos);

    // create result buffer
    HBufC8* result = HBufC8::NewLC(128);

    const TInt KBufSize(1024);
    TInt fileSize(0);
    TInt fileOffset(0);

    // get file size
    aFile.Size(fileSize);

    HBufC8* buf = HBufC8::NewMaxLC(KBufSize);
    TPtr8 bufPtr(buf->Des());

    // read to buffer
    while (fileOffset < fileSize - KBufSize)
        {
        aFile.Read(bufPtr, KBufSize);
        aMD->Hash(bufPtr);
        fileOffset += bufPtr.Length();
        }

    aFile.Read(bufPtr, fileSize - fileOffset);
    bufPtr.SetLength(fileSize - fileOffset);
    
    // get final message digest
    TPtrC8 hashedSig(aMD->Final(bufPtr));
    
    // change size of the result buffer
    result->ReAllocL(hashedSig.Length() * 2);
    TPtr8 resultPtr = result->Des();

    // convert to hex format
    for (TInt i=0; i<hashedSig.Length(); i++)
        {
        resultPtr.AppendFormat(_L8("%+02x"), hashedSig[i]);
        }
                    
    CleanupStack::PopAndDestroy(); // buf
    
    return result;
    }
        
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetErrRdL(TBool aEnable)
    {
    #ifndef FILEBROWSER_LITE
    if (iModel->Settings().iBypassPlatformSecurity)
        {
        TInt err(KErrNone);
        
        if (aEnable)
            {
            // make sure that the directory exists
            iFileOps->MkDirAll(KErrRdPath);        

            // create the file
            err = iFileOps->CreateEmptyFile(KErrRdPath);
            }
        else    
            {
            // get TEntry of ErrRd
            TEntry entry;
            err = iFs.Entry(KErrRdPath, entry);
            
            if (err == KErrNone)
                {
                TFileEntry fileEntry;
                fileEntry.iPath = KErrRdDir;
                fileEntry.iEntry = entry;
                
                err = iFileOps->Delete(fileEntry);                
                }
            }        

        // update view
        RefreshViewL(); 
    
        if (err == KErrNone)    
            {
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("State changed"));    
            }
        else    
            {
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD(_L("Cannot change the state"));    
            }        
        }
    else
        {
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(_L("Enable \"bypass platform security\" from the settings first"));          
        }
    #else
        aEnable = aEnable;
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(_L("Not supported in lite version"));          
    #endif
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::EnableAvkonIconCacheL(TBool aEnable)
    {
#ifndef __SERIES60_30__
    if (AknIconConfig::EnableAknIconSrvCache(aEnable) == KErrNone)
        {
        CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
        note->ExecuteLD(_L("State changed"));        
        }
    else
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("Unknown error occured"));        
        }
 #else
    aEnable = aEnable;
    CAknInformationNote* note = new(ELeave) CAknInformationNote;
    note->ExecuteLD(_L("Not supported in S60 3.0"));  
 #endif
 
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SimulateLeaveL()
    {
    TInt leaveCode(-6);

    CAknNumberQueryDialog* numberQuery = CAknNumberQueryDialog::NewL(leaveCode);
    numberQuery->SetPromptL(_L("Leave code"));

    if (numberQuery->ExecuteLD(R_GENERAL_NUMERIC_QUERY))    
        {
        User::Leave(leaveCode);
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SimulatePanicL()
    {
    TInt panicCode(555);
    TBuf<128> panicCategory;
    panicCategory.Copy(_L("Test Category"));
    
    CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(panicCategory);
    textQuery->SetPromptL(_L("Panic category"));
    
    if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
        {
        CAknNumberQueryDialog* numberQuery = CAknNumberQueryDialog::NewL(panicCode);
        numberQuery->SetPromptL(_L("Panic code"));

        if (numberQuery->ExecuteLD(R_GENERAL_NUMERIC_QUERY))    
            {
            User::Panic(panicCategory, panicCode);
            }        
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SimulateExceptionL()
    {
    TInt exceptionCode(0);

    CAknNumberQueryDialog* numberQuery = CAknNumberQueryDialog::NewL(exceptionCode);
    numberQuery->SetPromptL(_L("Exception code"));

    if (numberQuery->ExecuteLD(R_GENERAL_NUMERIC_QUERY))    
        {
        User::RaiseException((TExcType)exceptionCode);
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetDebugMaskL()
    {
    // get current debug mask
    TUint32 dbgMask = UserSvr::DebugMask();
    
    // convert the value in hex format
    TBuf<64> hexBuf;
    hexBuf.Copy(_L("0x"));
    hexBuf.AppendNum(dbgMask, EHex);
    
    // do the query
    CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(hexBuf);
    textQuery->SetPromptL(_L("Kernel debug mask in hex format"));

    if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
        {
        // check value
        if (hexBuf.Length() > 2 && hexBuf[0]=='0' && hexBuf[1]=='x')
            {
            // remove 0x from the beginning
            hexBuf.Copy(hexBuf.Right(hexBuf.Length()-2));
            
            // convert the string back to decimal
            TLex converter;
            converter.Assign(hexBuf);
            
            if (converter.Val(dbgMask, EHex) == KErrNone)
                {
                User::SetDebugMask(dbgMask);
                
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(_L("Changed"));            
                }
            else
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Cannot convert value"));            
                }        
            }            
        else        
            {
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD(_L("Not in hex format"));            
            }
        }
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::WriteAllAppsL()
    {
    _LIT(KAllAppsFileName, "AllApps.txt");
    TFileName allAppsPath = PathInfo::PhoneMemoryRootPath();
    allAppsPath.Append(KAllAppsFileName);

    RFile file;
    if (file.Replace(iFs, allAppsPath, EFileWrite) == KErrNone)
        {
        CleanupClosePushL(file);
        
        CDesC8Array* appsArray = new(ELeave) CDesC8ArrayFlat(192);
        CleanupStack::PushL(appsArray);
        
        TBuf8<KMaxFileName> fileEntry;
        TBuf8<KMaxFileName> appFullPath;
        _LIT8(KAppEntryFormat, "%S (0x%08X)%S");
        TApaAppInfo appInfo;

        iModel->LsSession().GetAllApps();        

        while (iModel->LsSession().GetNextApp(appInfo) == KErrNone)
            {
            appFullPath.Copy(appInfo.iFullName);
            fileEntry.Format(KAppEntryFormat, &appFullPath, appInfo.iUid, &KFileNewLine);
            appsArray->AppendL(fileEntry);    
            }
        
        appsArray->Sort();
        
        for (TInt i=0; i<appsArray->Count(); i++)
            {
            file.Write(appsArray->MdcaPoint(i));
            }
        
        CleanupStack::PopAndDestroy(2); //appsArray, file
        
        _LIT(KMessage, "App list written to %S");
        TFileName noteMsg;
        noteMsg.Format(KMessage, &allAppsPath);
        
        CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote();
        note->SetTimeout(CAknNoteDialog::ENoTimeout);
        note->ExecuteLD(noteMsg);
        }
    else
        {
        _LIT(KMessage, "Failed writing to %S");
        TFileName noteMsg;
        noteMsg.Format(KMessage, &allAppsPath);
        
        CAknErrorNote* note = new(ELeave) CAknErrorNote();
        note->SetTimeout(CAknNoteDialog::ENoTimeout);
        note->ExecuteLD(noteMsg);
        }    
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::WriteAllFilesL()
    {
    _LIT(KAllFilesFileName, "AllFiles.txt");
    TFileName allFilesPath = PathInfo::PhoneMemoryRootPath();
    allFilesPath.Append(KAllFilesFileName);

    RFile file;
    if (file.Replace(iFs, allFilesPath, EFileWrite) == KErrNone)
        {
        CleanupClosePushL(file);
        iFindFileEntryList->Reset();
        
        iModel->EikonEnv()->BusyMsgL(_L("** Generating **"), TGulAlignment(EHCenterVTop));
        
        for (TInt i=0; i<iDriveEntryList->Count(); i++)
            {
            TDriveEntry driveEntry = iDriveEntryList->At(i);

            TBuf<10> driveRoot;
            driveRoot.Append(driveEntry.iLetter);
            driveRoot.Append(_L(":\\"));

            // current dir
            DoFindFiles(_L("*"), driveRoot);
            
            // recurse into sub directories
            DoFindFilesRecursiveL(_L("*"), driveRoot);
            }
        
        // write entries
        TBuf8<KMaxFileName> writeBuf;
        for (TInt i=0; i<iFindFileEntryList->Count(); i++)
            {
            TFileEntry fileEntry = iFindFileEntryList->At(i);
            
            writeBuf.Copy(fileEntry.iPath);
            writeBuf.Append(fileEntry.iEntry.iName);
            writeBuf.Append(KFileNewLine);
            
            file.Write(writeBuf);
            }
        
        iModel->EikonEnv()->BusyMsgCancel();
        
        CleanupStack::PopAndDestroy(); //file
        iFindFileEntryList->Reset();
        
        _LIT(KMessage, "File list written to %S");
        TFileName noteMsg;
        noteMsg.Format(KMessage, &allFilesPath);
        
        CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote();
        note->SetTimeout(CAknNoteDialog::ENoTimeout);
        note->ExecuteLD(noteMsg);
        }
    else
        {
        _LIT(KMessage, "Failed writing to %S");
        TFileName noteMsg;
        noteMsg.Format(KMessage, &allFilesPath);
        
        CAknErrorNote* note = new(ELeave) CAknErrorNote();
        note->SetTimeout(CAknNoteDialog::ENoTimeout);
        note->ExecuteLD(noteMsg);
        }    
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::DoFindFiles(const TDesC& aFileName, const TDesC& aPath)
    {
    TFindFile fileFinder(iFs);
    CDir* dir;
    TInt err = fileFinder.FindWildByPath(aFileName, &aPath, dir);

    while (err == KErrNone)
        {
        for (TInt i=0; i<dir->Count(); i++)
            {
            TEntry entry = (*dir)[i];

            // ignore any directory entries
            if (!entry.IsDir() && entry.iName.Length() && aPath.Length())
                {
                TParse parsedName;
                parsedName.Set(entry.iName, &fileFinder.File(), NULL);
                
                if (parsedName.Drive().Length() && aPath.Length() && parsedName.Drive()[0] == aPath[0])
                    {
                    TFileEntry fileEntry;
                    fileEntry.iPath = parsedName.DriveAndPath();
                    fileEntry.iEntry = entry;
                    fileEntry.iDirEntries = KErrNotFound;
                    fileEntry.iIconId = EFixedIconEmpty;
                    
                    TRAP(err, iFindFileEntryList->AppendL(fileEntry));             
                    }
                }
            }

        delete dir;
        dir = NULL;
        err = fileFinder.FindWild(dir);
        }

    return err;
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileUtils::DoFindFilesRecursiveL(const TDesC& aFileName, const TDesC& aPath)
	{
    TInt err(KErrNone);
    CDirScan* scan = CDirScan::NewLC(iFs);
    scan->SetScanDataL(aPath, KEntryAttDir|KEntryAttMatchMask, ESortByName | EAscending | EDirsFirst);
    CDir* dir = NULL;

    for(;;)
        {
        TRAP(err, scan->NextL(dir));
        if (!dir  || (err != KErrNone))
            break;

        for (TInt i=0; i<dir->Count(); i++)
            {
            TEntry entry = (*dir)[i];
            
            if (entry.IsDir())
                {
                TFileName path(scan->FullPath());
                
                if (path.Length())
                    {
                    path.Append(entry.iName);
                    path.Append(_L("\\"));
                    DoFindFiles(aFileName, path);
                    }
                }
            }
        delete(dir);
        }

    CleanupStack::PopAndDestroy(scan);
    return err;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ListOpenFilesL()
    {
    iFileEntryList->Reset();

    CFileList* dir = NULL;
    TOpenFileScan fileScan(iFs);
    fileScan.NextL(dir);
    
    while (dir)
        {
        for (TInt i=0; i<dir->Count(); i++)
            {
            TEntry entry = (*dir)[i];
            TFileName fullPath;

            // TOpenFileScan does not return drive letters for the entries, so try to guess it
            if (entry.iName.Length() > 2 && entry.iName[1] != ':')
                {
                for (TInt i=0; i<iDriveEntryList->Count(); i++)
                    {
                    TDriveEntry driveEntry = iDriveEntryList->At(i);
                    
                    TFileName guessPath;
                    guessPath.Append(driveEntry.iLetter);
                    guessPath.Append(_L(":"));
                    guessPath.Append(entry.iName);
                    
                    // check if the file exists
                    if (BaflUtils::FileExists(iFs, guessPath))
                        {
                        // if it's on read only drive, then take it or check if it's really in use
                        TBool readOnlyDrive(ETrue);
                        BaflUtils::DiskIsReadOnly(iFs, guessPath, readOnlyDrive);
                        
                        if (readOnlyDrive || iFs.SetEntry(guessPath, entry.iModified, NULL, NULL) == KErrInUse)
                            {
                            fullPath = guessPath;
                            break;
                            }
                        }
                    }
                }
            else
                {
                fullPath = entry.iName;   
                }

            TParse nameParser;
            if (fullPath.Length() && nameParser.SetNoWild(fullPath, NULL, NULL) == KErrNone)
                {
                entry.iName = nameParser.NameAndExt();
                
                TFileEntry fileEntry;
                fileEntry.iPath = nameParser.DriveAndPath();
                fileEntry.iEntry = entry;
                fileEntry.iDirEntries = KErrNotFound;
                fileEntry.iIconId = EFixedIconEmpty;
                
                iFileEntryList->AppendL(fileEntry);                
                }
            }
            
        delete dir;
        dir = NULL;
        fileScan.NextL(dir);
        }

    TInt operations = iFileEntryList->Count();
    
    iListingMode = EOpenFiles;
    iModel->FileListContainer()->ListBox()->SetCurrentItemIndex(0);
    RefreshViewL();            

    _LIT(KMessage, "%d open files found");
    TFileName noteMsg;
    noteMsg.Format(KMessage, operations);
    
    CAknInformationNote* note = new(ELeave) CAknInformationNote;
    note->ExecuteLD(noteMsg);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ListMessageAttachmentsL(TInt aType)
    {
    iFileEntryList->Reset();

    CMsvSession* session = CMsvSession::OpenSyncL(*this);
    CleanupStack::PushL(session);
    
    CMsvEntry* context = NULL;
    
    if (aType == EFileBrowserCmdToolsMsgAttachmentsInbox)
        context = session->GetEntryL(KMsvGlobalInBoxIndexEntryId);
    else if (aType == EFileBrowserCmdToolsMsgAttachmentsDrafts)
        context = session->GetEntryL(KMsvDraftEntryId);
    else if (aType == EFileBrowserCmdToolsMsgAttachmentsSentItems)
        context = session->GetEntryL(KMsvSentEntryId);
    else if (aType == EFileBrowserCmdToolsMsgAttachmentsOutbox)
        context = session->GetEntryL(KMsvGlobalOutBoxIndexEntryId);
    else
        User::Panic(_L("MsgAtt.Mode"), 100);
    
    CleanupStack::PushL(context);
    
    CDesCArray* attPaths = new(ELeave) CDesCArrayFlat(128);
    CleanupStack::PushL(attPaths);
    
    // read attachment paths
    ReadAttachmentPathsRecursiveL(session, context, attPaths);
    
    // create file entries of the paths
    for (TInt i=0; i<attPaths->MdcaCount(); i++)
        {
        TFileEntry fileEntry;
        TEntry entry;
        TParse nameParser;
        
        if (attPaths->MdcaPoint(i).Length() &&
            nameParser.SetNoWild(attPaths->MdcaPoint(i), NULL, NULL) == KErrNone &&
            iFs.Entry(attPaths->MdcaPoint(i), entry) == KErrNone)
            {
            TFileEntry fileEntry;
            fileEntry.iPath = nameParser.DriveAndPath();
            fileEntry.iEntry = entry;
            fileEntry.iDirEntries = KErrNotFound;
            fileEntry.iIconId = EFixedIconEmpty;

            iFileEntryList->AppendL(fileEntry);                
            }
        }
    
    CleanupStack::PopAndDestroy(3); //session, context, attPaths
            
    TInt operations = iFileEntryList->Count();
    
    if (aType == EFileBrowserCmdToolsMsgAttachmentsInbox)
        iListingMode = EMsgAttachmentsInbox;
    else if (aType == EFileBrowserCmdToolsMsgAttachmentsDrafts)
        iListingMode = EMsgAttachmentsDrafts;
    else if (aType == EFileBrowserCmdToolsMsgAttachmentsSentItems)
        iListingMode = EMsgAttachmentsSentItems;
    else if (aType == EFileBrowserCmdToolsMsgAttachmentsOutbox)
        iListingMode = EMsgAttachmentsOutbox;

    iModel->FileListContainer()->ListBox()->SetCurrentItemIndex(0);
    RefreshViewL();            

    _LIT(KMessage, "%d files found");
    TFileName noteMsg;
    noteMsg.Format(KMessage, operations);
    
    CAknInformationNote* note = new(ELeave) CAknInformationNote;
    note->ExecuteLD(noteMsg);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ReadAttachmentPathsRecursiveL(CMsvSession* aSession, CMsvEntry* aContext, CDesCArray* aAttPaths)
    {
    CMsvEntrySelection* entrySelection = aContext->ChildrenL(); 
    CleanupStack::PushL(entrySelection);
    
    for (TInt i=0; i<entrySelection->Count(); i++)
        {
        CMsvEntry* entry = aSession->GetEntryL((*entrySelection)[i]);
        CleanupStack::PushL(entry);
        
        CMsvStore* store = NULL;
        TRAPD(err, store = entry->ReadStoreL());
        
        if (err == KErrNone)
            {
            CleanupStack::PushL(store);

            for (TInt j=0; j<store->AttachmentManagerL().AttachmentCount(); j++)
                {
                CMsvAttachment* attachment = store->AttachmentManagerL().GetAttachmentInfoL(j);
                CleanupStack::PushL(attachment);
                
                aAttPaths->AppendL(attachment->FilePath());
                
                CleanupStack::PopAndDestroy(); //attachment
                }
            CleanupStack::PopAndDestroy(); // store
            }

        // recurse into children
        if (entry->Count() > 0)
            {
            ReadAttachmentPathsRecursiveL(aSession, entry, aAttPaths);
            }

        CleanupStack::PopAndDestroy(); // entry
        }      
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::WriteMsgStoreWalkL()
    {
    _LIT(KAllAppsFileName, "MsgStoreWalk.txt");
    TFileName allAppsPath = PathInfo::PhoneMemoryRootPath();
    allAppsPath.Append(KAllAppsFileName);

    if (iMsgStoreWalkFile.Replace(iFs, allAppsPath, EFileWrite) == KErrNone)
        {
        CMsvSession* session = CMsvSession::OpenSyncL(*this);
        CleanupStack::PushL(session);
        
        CMsvEntry* context = session->GetEntryL(KMsvRootIndexEntryId);
        CleanupStack::PushL(context);
        
        iMsgStoreWalkFile.Write(_L8("id  service_id  related_id  type  mtm  date  size  error  biotype  mtmdata1  mtmdata2  mtmdata3  description  details\r\n\r\n"));
        
        // write details about the root entry
        DoWriteMessageEntryInfoL(context, iMsgStoreWalkFile, 0);
        
        // get and write info recursively
        TInt level(1);        
        WriteMessageEntryInfoRecursiveL(session, context, iMsgStoreWalkFile, level);

        CleanupStack::PopAndDestroy(2); //session, context
        
        iMsgStoreWalkFile.Flush();
        iMsgStoreWalkFile.Close();
        
        _LIT(KMessage, "Msg. store walk written to %S");
        TFileName noteMsg;
        noteMsg.Format(KMessage, &allAppsPath);
        
        CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote();
        note->SetTimeout(CAknNoteDialog::ENoTimeout);
        note->ExecuteLD(noteMsg);
        }
    else
        {
        _LIT(KMessage, "Failed writing to %S");
        TFileName noteMsg;
        noteMsg.Format(KMessage, &allAppsPath);
        
        CAknErrorNote* note = new(ELeave) CAknErrorNote();
        note->SetTimeout(CAknNoteDialog::ENoTimeout);
        note->ExecuteLD(noteMsg);
        }    
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::WriteMessageEntryInfoRecursiveL(CMsvSession* aSession, CMsvEntry* aContext, RFile& aFile, TInt& aLevel)
    {
    CMsvEntrySelection* entrySelection = aContext->ChildrenL(); 
    CleanupStack::PushL(entrySelection);
    
    for (TInt i=0; i<entrySelection->Count(); i++)
        {
        CMsvEntry* entry = aSession->GetEntryL((*entrySelection)[i]);
        CleanupStack::PushL(entry);
        
        DoWriteMessageEntryInfoL(entry, aFile, aLevel);

        // recurse into children
        if (entry->Count() > 0)
            {
            aLevel++;
            WriteMessageEntryInfoRecursiveL(aSession, entry, aFile, aLevel);
            aLevel--;
            }

        CleanupStack::PopAndDestroy(); // entry
        }      
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::DoWriteMessageEntryInfoL(CMsvEntry* aContext, RFile& aFile, TInt aLevel)
    {
    TMsvEntry messageEntry = aContext->Entry();
    
    for (TInt j=0; j<aLevel; j++)
        {
        aFile.Write(_L8("     "));
        }

    TBuf8<256> desc;
    desc.Copy(messageEntry.iDescription);

    TBuf8<256> details;
    details.Copy(messageEntry.iDetails);

    _LIT8(KLine, "-> %d  %d  %d  %d  %d  %Ld  %d  %d  %d  %d  %d  %d  %S  %S\r\n");
    TBuf8<1024> buf;
    buf.Format(KLine, messageEntry.Id(), messageEntry.iServiceId, messageEntry.iRelatedId, messageEntry.iType.iUid,
        messageEntry.iMtm.iUid, messageEntry.iDate.Int64(), messageEntry.iSize, messageEntry.iError, messageEntry.iBioType,
        messageEntry.iMtmData1, messageEntry.iMtmData2, messageEntry.iMtmData3, &desc, &details);

    aFile.Write(buf);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::HandleSessionEventL(TMsvSessionEvent /*aEvent*/, TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/)
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::FileEditorL(TInt aType)
    {
    // just get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TFileEntry fileEntry = iFileEntryList->At(currentItemIndex);
        
        // only valid for files
        if (!fileEntry.iEntry.IsDir())
            {
            TFileName fullPath = fileEntry.iPath;
            fullPath.Append(fileEntry.iEntry.iName);

            iModel->FileListContainer()->SetNaviPaneTextL(fileEntry.iEntry.iName);
    
            TInt editorMode(0);
            
            if (aType == EFileBrowserCmdFileViewText)
                editorMode = EFileEditorViewAsText;
            else if (aType == EFileBrowserCmdFileViewHex)
                editorMode = EFileEditorViewAsHex;
            else if (aType == EFileBrowserCmdFileEditText)
                editorMode = EFileEditorEditAsText;
            else if (aType == EFileBrowserCmdFileEditHex)
                editorMode = EFileEditorEditAsHex;
            else
                User::Panic(_L("Inv.Ed.Mode"), 843);
            
            // launch dialog
            CFileBrowserFileEditorDlg* dlg = CFileBrowserFileEditorDlg::NewL(fullPath, editorMode);
            dlg->RunDlgLD();

            iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);
            
            if (aType == EFileBrowserCmdFileEditText || aType == EFileBrowserCmdFileEditHex)
                RefreshViewL();        
            }
        }
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserFileUtils::DriveSnapShotPossible()
    {
    // check that E-drive is available
    TBool EDriveIsOK(EFalse);
    
    for (TInt i=0; i<iDriveEntryList->Count(); i++)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(i);
        
        if (driveEntry.iNumber == EDriveE)
            {
            if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttLocked || driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAbsent)
                EDriveIsOK = EFalse;
            else
                EDriveIsOK = ETrue;
            }
        }
    
    if (!EDriveIsOK)
        return EFalse;
    
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
 
        // drive snapshot not possible when E-drive is highlighted, since that's where we are copying
        if (driveEntry.iNumber == EDriveE)
            return EFalse;
        else
            return ETrue;
        }
    
    return EFalse;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::DriveSnapShotL()
    {
    if (iModel->Settings().iBypassPlatformSecurity)
        {
        // get current item 
        TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();

        if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
            {
            TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
            
            TChar sourceDriveLetter = driveEntry.iLetter;
            TChar targetDriveLetter = 'E';  // hardcoded drive letter, could be better if would be queried from the user
            
            // append to the command array
            AppendToCommandArrayL(EFileBrowserFileOpCommandDriveSnapShot,
                                  new(ELeave)CCommandParamsDriveSnapShot(sourceDriveLetter, targetDriveLetter)
                                 );
                                 
            // execute the operation
            StartExecutingCommandsL(_L("Copying"));
            }
        }
    else
        {
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(_L("Enable \"bypass platform security\" from the settings first"));          
        }
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetDrivePasswordL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TMediaPassword oldPassword;
        TMediaPassword newPassword;
        TFileName oldPassword16;
        TFileName newPassword16;
        
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
        
        // query existing password if it already exists
        if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttHasPassword)
            {
            CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(oldPassword16);
            textQuery->SetPromptL(_L("Existing password:"));
            
            if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
                {
                ConvertCharsToPwd(oldPassword16, oldPassword);    
                }
            else
                {
                return;    
                }            
            }
        
        // query new password    
        newPassword16.Copy(oldPassword16);
        
        CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(newPassword16);
        textQuery->SetPromptL(_L("New password:"));
        
        if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
            {
            ConvertCharsToPwd(newPassword16, newPassword);
            
            // set the password, does not actually lock the drive
            TInt err = iFs.LockDrive(driveEntry.iNumber, oldPassword, newPassword, ETrue);
            
            if (err == KErrNone)
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(_L("Password set"));                     
                }
            else if (err == KErrNotSupported)
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Not supported for this drive"));                     
                }
            else
                {
                CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                    
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                CleanupStack::PopAndDestroy();  //textResolver                       
                }
            
            RefreshViewL();
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::UnlockDriveL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);

        // check if the drive is locked
        if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttLocked)
            {
            TMediaPassword oldPassword;
            TFileName oldPassword16;

            CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(oldPassword16);
            textQuery->SetPromptL(_L("Existing password:"));

            if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
                {
                ConvertCharsToPwd(oldPassword16, oldPassword);
                
                // unlock the drive
                TInt err = iFs.UnlockDrive(driveEntry.iNumber, oldPassword, ETrue) == KErrNone;
                
                if (err == KErrNone)
                    {
                    CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                    note->ExecuteLD(_L("Drive unlocked"));                     
                    }
                else if (err == KErrNotSupported)
                    {
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD(_L("Not supported for this drive"));                     
                    }
                else
                    {
                    CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                        
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                    CleanupStack::PopAndDestroy();  //textResolver                       
                    }
                
                RefreshViewL();
                }
            }
        else
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("This drive is not locked"));             
            }

        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ClearDrivePasswordL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
        
        // check if the drive has a password
        if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttHasPassword)
            {
            TMediaPassword oldPassword;
            TFileName oldPassword16;

            CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(oldPassword16);
            textQuery->SetPromptL(_L("Existing password:"));

            if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
                {
                ConvertCharsToPwd(oldPassword16, oldPassword);
                
                // clear the password from the drive
                TInt err = iFs.ClearPassword(driveEntry.iNumber, oldPassword);
                
                if (err == KErrNone)
                    {
                    CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                    note->ExecuteLD(_L("Password cleared"));                     
                    }
                else if (err == KErrNotSupported)
                    {
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD(_L("Not supported for this drive"));                     
                    }
                else
                    {
                    CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                        
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                    CleanupStack::PopAndDestroy();  //textResolver                       
                    }
                
                RefreshViewL();
                }
            }
        else
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("This drive has no password"));                     
            }    
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::EraseDrivePasswordL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
        
        // check if the drive has a password
        if (driveEntry.iVolumeInfo.iDrive.iMediaAtt & KMediaAttHasPassword)
            {
            CAknQueryDialog* query = CAknQueryDialog::NewL();
            
            if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Are you sure? All data can be lost!")))
                {
                // erase the password from the drive
                TInt err(KErrNone);
                err = iFs.ErasePassword(driveEntry.iNumber);
                
                if (err == KErrNone)
                    {
                    CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                    note->ExecuteLD(_L("Password erased"));                     
                    }
                else if (err == KErrNotSupported)
                    {
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD(_L("Not supported for this drive"));                     
                    }
                else
                    {
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD(_L("Cannot erase, you may have to format the drive first"));                     
                    }

                RefreshViewL();
                }
            }
        else
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("This drive has no password"));                     
            }         
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::FormatDriveL(TBool aQuickFormat)
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
        
        CAknQueryDialog* query = CAknQueryDialog::NewL();
        
        if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Are you sure? All data will be lost!")))
            {
            TInt err(KErrNone);
            TInt formatCount(0);
            
            TBuf<10> driveRoot;
            driveRoot.Append(driveEntry.iLetter);
            driveRoot.Append(_L(":"));
            
            TUint formatMode(0);
            if (aQuickFormat)
                formatMode = ESpecialFormat|EQuickFormat;
            else
                formatMode = ESpecialFormat|EFullFormat;
            
            // set as system application to prevent getting shut down events
            iModel->EikonEnv()->SetSystem(ETrue);
            
            // first close any open applications            
            CBaBackupSessionWrapper* BSWrapper = CBaBackupSessionWrapper::NewL();
            CleanupStack::PushL(BSWrapper);

            TBackupOperationAttributes atts(MBackupObserver::EReleaseLockNoAccess, MBackupOperationObserver::EStart);
            BSWrapper->NotifyBackupOperationL(atts);
            
            CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
            BSWrapper->CloseAll( MBackupObserver::EReleaseLockNoAccess, waiter->iStatus );
            waiter->StartAndWait();
            CleanupStack::PopAndDestroy(); //waiter

            // some delay to make sure all applications have been closed
            User::After(1000000);

            // format the drive
            RFormat format;
            err = format.Open(iFs, driveRoot, formatMode, formatCount);
            
            // Forced format for locked card
            if ( err == KErrLocked )
                {
                // Erase password and try again
                err = iFs.ErasePassword( driveEntry.iNumber );
                if ( !err )
                    {
                    err = format.Open(iFs, driveRoot, formatMode, formatCount);
                    }
                }

            if( err != KErrNone &&  err != KErrLocked )
                {
                CAknQueryDialog* lockQuery = CAknQueryDialog::NewL();
                if ( lockQuery->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Memory card in use by other application. Proceed anyway?")))
                    {
                    // If format could not be started, someone is still keeping
                    // files open on media. Let's try to dismount file system, then 
                    // remount with own extension embedded and try again.

                    TFullName fsName;
                    err = iFs.FileSystemName( fsName, driveEntry.iNumber );

                    if ( err == KErrNone && fsName.Length() > 0 )
                        {
                        // Prevent SysAp shutting down applications
                        RProperty::Set(
                            KPSUidCoreApplicationUIs,
                            KCoreAppUIsMmcRemovedWithoutEject,
                            ECoreAppUIsEjectCommandUsed );

                        TRequestStatus stat;
                        iFs.NotifyDismount( driveEntry.iNumber, stat, EFsDismountForceDismount );
                        User::WaitForRequest( stat );

                        // Unfortunately, at the moment we have to wait until clients have received 
                        // notification about card dismount. Otherwise at least causes problems with 
                        // theme selected from card. In future clients should use new notify-API.
                        User::After( KForcedFormatTimeout );

                        // Let's set priority higher than normally. This is done to decrease the chance 
                        // that someone reopens files on memory card after mandatory file system 
                        // remounting hence preventing formatting again.
                        TThreadPriority priority( RThread().Priority() );
                        RThread().SetPriority( EPriorityAbsoluteHigh );

                        // Mount file system back...
                        err = iFs.MountFileSystem( fsName, driveEntry.iNumber );
                        err = format.Open(iFs, driveRoot, formatMode, formatCount);
                        RThread().SetPriority( priority );
                        }
                    }
                }
            
            
            if (err == KErrNone)
                {
                // needs to be implemented with active objects
                // CAknProgressDialog* dlg = new(ELeave) CAknProgressDialog(formatCount, 1, 1, NULL);
                // dlg->SetTone(CAknNoteDialog::ENoTone);
                // dlg->ExecuteLD(R_FORMAT_PROGRESS_NOTE);
                
                while (formatCount && err == KErrNone)
                    {
                    err = format.Next(formatCount);
                    }
                }

            format.Close();

            // restart closed applications
            TBackupOperationAttributes atts2(MBackupObserver::ETakeLock, MBackupOperationObserver::EEnd);
            BSWrapper->NotifyBackupOperationL(atts2);
            BSWrapper->RestartAll();
            CleanupStack::PopAndDestroy(); //BSWrapper

            // system status not needed anymore
            iModel->EikonEnv()->SetSystem(EFalse);
                                    
                                    
            if (err == KErrNone)
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(_L("Format succeeded"));                     
                }
            else if (err == KErrNotSupported)
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Not supported for this drive"));                     
                }
            else
                {
                CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                    
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                CleanupStack::PopAndDestroy();  //textResolver                       
                }

            RefreshViewL();
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::EraseMBRL()
    {
    #ifndef FILEBROWSER_LITE
    if (iModel->Settings().iBypassPlatformSecurity)
        {
        // get current item 
        TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
        
        if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
            {
            TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
            
            CAknQueryDialog* query = CAknQueryDialog::NewL();
            if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Are you sure? Your media driver must support this!")))
                {

                CAknQueryDialog* query2 = CAknQueryDialog::NewL();
                if (query2->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Are you really sure you know what are you doing ?!?")))
                    {
                    // warn if the selected drive is not detected as removable
                    TBool proceed(EFalse);                    

                    if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttRemovable)
                        {
                        proceed = ETrue;
                        }
                    else
                        {
                        CAknQueryDialog* query3 = CAknQueryDialog::NewL();
                        if (query3->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Selected drive is not removable, really continue?")))
                            proceed = ETrue;
                        else
                            proceed = EFalse;                        
                        }
                                    
                    if (proceed)
                        {
                        TInt err(KErrNone);
                        
                        CAknInformationNote* note = new(ELeave) CAknInformationNote;
                        note->ExecuteLD(_L("Please wait, this may take 30 seconds"));
                        
                        // set as system application to prevent getting shut down events
                        iModel->EikonEnv()->SetSystem(ETrue);
                        
                        // first close any open applications            
                        CBaBackupSessionWrapper* BSWrapper = CBaBackupSessionWrapper::NewL();
                        CleanupStack::PushL(BSWrapper);
            
                        TBackupOperationAttributes atts(MBackupObserver::EReleaseLockNoAccess, MBackupOperationObserver::EStart);
                        BSWrapper->NotifyBackupOperationL(atts);
                        
                        CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
                        BSWrapper->CloseAll( MBackupObserver::EReleaseLockNoAccess, waiter->iStatus );
                        waiter->StartAndWait();
                        CleanupStack::PopAndDestroy(); //waiter
            
                        // some delay to make sure all applications have been closed
                        User::After(1000000);
                        
                        
                        // do the erase MBR operation
                        err = iFileOps->EraseMBR(driveEntry.iNumber);
                        
                        
                        // restart closed applications
                        TBackupOperationAttributes atts2(MBackupObserver::ETakeLock, MBackupOperationObserver::EEnd);
                        BSWrapper->NotifyBackupOperationL(atts2);
                        BSWrapper->RestartAll();
                        CleanupStack::PopAndDestroy(); //BSWrapper
            
                        // system status not needed anymore
                        iModel->EikonEnv()->SetSystem(EFalse);
                                                
                                                
                        if (err == KErrNone)
                            {
                            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                            note->ExecuteLD(_L("Erase MBR succeeded"));                     
                            }
                        else if (err == KErrNotSupported)
                            {
                            CAknErrorNote* note = new(ELeave) CAknErrorNote;
                            note->ExecuteLD(_L("Not supported for this drive"));                     
                            }
                        else
                            {
                            CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                                
                            CAknErrorNote* note = new(ELeave) CAknErrorNote;
                            note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                            CleanupStack::PopAndDestroy();  //textResolver                       
                            }
            
                        RefreshViewL();
                        }
                    }
                }
            }
        }
    else
        {
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(_L("Enable \"bypass platform security\" from the settings first"));          
        }
    #else
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(_L("Not supported in lite version"));          
    #endif
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::PartitionDriveL()
    {
    #ifndef FILEBROWSER_LITE
    if (iModel->Settings().iBypassPlatformSecurity)
        {
        // get current item 
        TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
        
        if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
            {
            TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
            
            CAknQueryDialog* query = CAknQueryDialog::NewL();
            if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Are you sure? Your media driver must support this!")))
                {

                CAknQueryDialog* query2 = CAknQueryDialog::NewL();
                if (query2->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Are you really sure you know what are you doing ?!?")))
                    {
                    // warn if the selected drive is not detected as removable
                    TBool proceed(EFalse);                    

                    if (driveEntry.iVolumeInfo.iDrive.iDriveAtt & KDriveAttRemovable)
                        {
                        proceed = ETrue;
                        }
                    else
                        {
                        CAknQueryDialog* query3 = CAknQueryDialog::NewL();
                        if (query3->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Selected drive is not removable, really continue?")))
                            proceed = ETrue;
                        else
                            proceed = EFalse;                        
                        }
                                    
                    if (proceed)
                        {
                        // query if erase mbr
                        TBool eraseMBR(EFalse);
                        
                        CAknQueryDialog* query4 = CAknQueryDialog::NewL();
                        if (query4->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Erase MBR first (normally needed)?")))
                            {
                            eraseMBR = ETrue;
                            }
                        
                        
                        // query amount of partitions from user
                        TInt queryIndex(0);
                        CAknListQueryDialog* listQueryDlg = new(ELeave) CAknListQueryDialog(&queryIndex);

                        if (listQueryDlg->ExecuteLD(R_PARTITION_AMOUNT_QUERY))
                            {
                            TInt amountOfPartitions = queryIndex + 1;  // selections are 1..4
               
                            TInt err(KErrNone);
                            
                            CAknInformationNote* note = new(ELeave) CAknInformationNote;
                            note->ExecuteLD(_L("Please wait, this may take 30 seconds"));

                            // set as system application to prevent getting shut down events
                            iModel->EikonEnv()->SetSystem(ETrue);
                            
                            // first close any open applications            
                            CBaBackupSessionWrapper* BSWrapper = CBaBackupSessionWrapper::NewL();
                            CleanupStack::PushL(BSWrapper);
                
                            TBackupOperationAttributes atts(MBackupObserver::EReleaseLockNoAccess, MBackupOperationObserver::EStart);
                            BSWrapper->NotifyBackupOperationL(atts);
                            
                            CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
                            BSWrapper->CloseAll( MBackupObserver::EReleaseLockNoAccess, waiter->iStatus );
                            waiter->StartAndWait();
                            CleanupStack::PopAndDestroy(); //waiter
                
                            // some delay to make sure all applications have been closed
                            User::After(1000000);
                            
                            // do the erase operation
                            if (eraseMBR)
                                {
                                err = iFileOps->EraseMBR(driveEntry.iNumber);
                                
                                if (err != KErrNone)
                                    {
                                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                                    note->ExecuteLD(_L("Erase MBR failed"));                     
                                    }

                                User::After(500000);
                                }
                            
                            
                            // do the partition operation
                            err = iFileOps->PartitionDrive(driveEntry.iNumber, amountOfPartitions);
                            
                            
                            // restart closed applications
                            TBackupOperationAttributes atts2(MBackupObserver::ETakeLock, MBackupOperationObserver::EEnd);
                            BSWrapper->NotifyBackupOperationL(atts2);
                            BSWrapper->RestartAll();
                            CleanupStack::PopAndDestroy(); //BSWrapper
                
                            // system status not needed anymore
                            iModel->EikonEnv()->SetSystem(EFalse);
                                                    
                                                    
                            if (err == KErrNone)
                                {
                                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                                note->ExecuteLD(_L("Partition succeeded"));                     
                                }
                            else if (err == KErrNotSupported)
                                {
                                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                                note->ExecuteLD(_L("Not supported for this drive"));                     
                                }
                            else
                                {
                                CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                                    
                                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                                note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                                CleanupStack::PopAndDestroy();  //textResolver                       
                                }
                
                            RefreshViewL();
                            }
                        }
                    }
                }
            }
        }
    else
        {
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(_L("Enable \"bypass platform security\" from the settings first"));          
        }
    #else
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(_L("Not supported in lite version"));          
    #endif
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::CheckDiskL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
        
        TBuf<10> driveRoot;
        driveRoot.Append(driveEntry.iLetter);
        driveRoot.Append(_L(":"));
        
        // check disk
        TInt err = iFs.CheckDisk(driveRoot);

        if (err == KErrNone)
            {
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("Integrity of the disk is ok"));                     
            }
        else if (err == KErrNotReady)
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("Disk is empty"));                     
            }
        else if (err == KErrNotSupported)
            {
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD(_L("Not supported for this drive"));                     
            }
        else
            {
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD(_L("Disk is corrupted"));                     
            }        

        RefreshViewL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ScanDriveL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        CAknQueryDialog* query = CAknQueryDialog::NewL();
        
        if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("This finds errors on disk and corrects them. Proceed?")))
            {
            TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
            
            TBuf<10> driveRoot;
            driveRoot.Append(driveEntry.iLetter);
            driveRoot.Append(_L(":"));
            
            // scan disk
            TInt err = iFs.ScanDrive(driveRoot);

            if (err == KErrNone)
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(_L("Run succesfully"));                     
                }
            else if (err == KErrNotSupported)
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Not supported for this drive"));                     
                }
            else
                {
                CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                    
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                CleanupStack::PopAndDestroy();  //textResolver                       
                }        

            RefreshViewL();
            }
        }
    }


// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetDriveNameL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);

        TFileName driveName;
        
        // get existing drive name
        iFs.GetDriveName(driveEntry.iNumber, driveName);
        
        CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(driveName);
        textQuery->SetPromptL(_L("New name:"));

        if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
            {
            // set drive name
            TInt err = iFs.SetDriveName(driveEntry.iNumber, driveName);

            if (err == KErrNone)
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(_L("Name changed"));                     
                }
            else if (err == KErrNotSupported)
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Not supported for this drive"));                     
                }
            else
                {
                CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                    
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                CleanupStack::PopAndDestroy();  //textResolver                       
                }              

            RefreshViewL();
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SetDriveVolumeLabelL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);

        // get existing volume label
        TFileName volumeLabel;
        volumeLabel.Copy(driveEntry.iVolumeInfo.iName);
        
        CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(volumeLabel);
        textQuery->SetPromptL(_L("New volume label:"));

        if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
            {
            // set volume label
            TInt err = iFs.SetVolumeLabel(volumeLabel, driveEntry.iNumber);

            if (err == KErrNone)
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(_L("Volume label changed"));                     
                }
            else if (err == KErrNotSupported)
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Not supported for this drive"));                     
                }
            else
                {
                CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                    
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                CleanupStack::PopAndDestroy();  //textResolver                       
                }              

            RefreshViewL();
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::EjectDriveL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
        
        TInt err(KErrNone);
        
        // get current filesystem name
        TFileName fileSystemName;
        err = iFs.FileSystemName(fileSystemName, driveEntry.iNumber);
        
        if (err == KErrNone)
            {
            // Prevent SysAp shutting down applications
            RProperty::Set(
                KPSUidCoreApplicationUIs,
                KCoreAppUIsMmcRemovedWithoutEject,
                ECoreAppUIsEjectCommandUsed );
            
            // dismount the file system
            err = iFs.DismountFileSystem(fileSystemName, driveEntry.iNumber);
            
            if (err == KErrNone)
                {
                // remount the file system
                err = iFs.MountFileSystem(fileSystemName, driveEntry.iNumber);
                
                if (err == KErrInUse)
                    {
                    // try to remount after a while if locked
                    User::After(1000000);
                    err = iFs.MountFileSystem(fileSystemName, driveEntry.iNumber);
                    }                
                }
            }

        if (err == KErrNone)
            {
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("Ejected succesfully"));                     
            }
        else if (err == KErrNotSupported)
            {
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD(_L("Not supported for this drive"));                     
            }
        else
            {
            CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

            CleanupStack::PopAndDestroy();  //textResolver                       
            }              

        RefreshViewL();
        }
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::DismountFileSystemL()
    {
    // get current item 
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iDriveEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        CAknQueryDialog* query = CAknQueryDialog::NewL();
        
        if (query->ExecuteLD(R_GENERAL_CONFIRMATION_QUERY, _L("Are you sure you know what are you doing?")))
            {
            TDriveEntry driveEntry = iDriveEntryList->At(currentItemIndex);
            
            TInt err(KErrNone);
            
            // get current filesystem name
            TFileName fileSystemName;
            err = iFs.FileSystemName(fileSystemName, driveEntry.iNumber);
            
            if (err == KErrNone)
                {
                // Prevent SysAp shutting down applications
                RProperty::Set(
                    KPSUidCoreApplicationUIs,
                    KCoreAppUIsMmcRemovedWithoutEject,
                    ECoreAppUIsEjectCommandUsed );
                
                // dismount the file system
                err = iFs.DismountFileSystem(fileSystemName, driveEntry.iNumber);
                }

            if (err == KErrNone)
                {
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(_L("Dismounted succesfully"));                     
                }
            else if (err == KErrNotSupported)
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Not supported for this drive"));                     
                }
            else
                {
                CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                    
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

                CleanupStack::PopAndDestroy();  //textResolver                       
                }              

            RefreshViewL();
            }
        }
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::ConvertCharsToPwd(TDesC& aWord, TDes8& aConverted) const
    {
    aConverted.FillZ(aConverted.MaxLength());
    aConverted.Zero();
    
    if (aWord.Length())
        {
        aConverted.Copy( (TUint8*)(&aWord[0]), aWord.Size() );
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::EditDataTypesL()
    {
    iModel->FileListContainer()->SetScreenLayoutL(EDisplayModeNormal);
    iModel->FileListContainer()->SetNaviPaneTextL(KNullDesC);
    
    CFileBrowserDataTypesDlg* dlg = CFileBrowserDataTypesDlg::NewL();
    dlg->RunQueryLD();

    iModel->FileListContainer()->SetScreenLayoutL(iModel->Settings().iDisplayMode);
    iModel->FileListContainer()->SetNaviPaneTextL(iCurrentPath);     
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::SecureBackupL(TInt aType)
    {
    TInt err(KErrNone);
    TInt showStatus(EFalse);
    
    if (aType == EFileBrowserCmdToolsSecureBackupStartBackup || aType == EFileBrowserCmdToolsSecureBackupStartRestore)
        {
        TInt queryIndexState(0);
        TInt queryIndexType(0);
        
        CAknListQueryDialog* listQueryDlgState = new(ELeave) CAknListQueryDialog(&queryIndexState);
        if (listQueryDlgState->ExecuteLD(R_SECUREBACKUP_STATE_QUERY))
            {

            CAknListQueryDialog* listQueryDlgType = new(ELeave) CAknListQueryDialog(&queryIndexType);
            if (listQueryDlgType->ExecuteLD(R_SECUREBACKUP_TYPE_QUERY))
                {
                conn::TBURPartType partType = conn::EBURNormal;
                conn::TBackupIncType backupIncType = conn::ENoBackup;
                
                if (aType == EFileBrowserCmdToolsSecureBackupStartBackup)
                    {
                    if (queryIndexState == ESecureBackupStateFull)
                        partType = conn::EBURBackupFull;
                    else if (queryIndexState == ESecureBackupStatePartial)
                        partType = conn::EBURBackupPartial;
                    }
                else if (aType == EFileBrowserCmdToolsSecureBackupStartRestore)
                    {
                    if (queryIndexState == ESecureBackupStateFull)
                        partType = conn::EBURRestoreFull;
                    else if (queryIndexState == ESecureBackupStatePartial)
                        partType = conn::EBURRestorePartial;
                    }
                    
                if (queryIndexType == ESecureBackupTypeBase)
                    backupIncType = conn::EBackupBase;
                else if (queryIndexType == ESecureBackupTypeIncremental)    
                    backupIncType = conn::EBackupIncrement;    

                // start secure backup
                err = iFileOps->ActivateSecureBackUp(partType, backupIncType);
                showStatus = ETrue;
                }
            }
        }

    else if (aType == EFileBrowserCmdToolsSecureBackupStop)
        {
        err = iFileOps->DeActivateSecureBackUp();
        showStatus = ETrue;
        }

    else
        User::Panic(_L("Sec.br.mode"), 101);
    
    
    if (showStatus)
        {
        if (err == KErrNone)
            {
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("Succeeded"));                     
            }
        else
            {
            CTextResolver* textResolver = CTextResolver::NewLC(*iModel->EikonEnv());     
                
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD( textResolver->ResolveErrorString(err, CTextResolver::ECtxNoCtxNoSeparator) );    

            CleanupStack::PopAndDestroy();  //textResolver                       
            }  
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileUtils::OpenCommonFileActionQueryL()
    {
    // just get current item
    TInt currentItemIndex = iModel->FileListContainer()->CurrentListBoxItemIndex();
    
    if (iFileEntryList->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        TInt queryIndex(0);
        
        CAknListQueryDialog* listQueryDlg = new(ELeave) CAknListQueryDialog(&queryIndex);
        
        if (listQueryDlg->ExecuteLD(R_COMMON_FILE_ACTION_QUERY))
            {
            if (queryIndex == ECommonFileActionViewAsTextHex)
                {
                TFileEntry fileEntry = iFileEntryList->At(currentItemIndex);
                
                TFileName fullPath = fileEntry.iPath;
                fullPath.Append(fileEntry.iEntry.iName);
                
                TInt viewerType(EFileBrowserCmdFileViewHex);
                
                // check from mime type if it's text
                TDataType dataType;
                TUid appUid;
                if (iModel->LsSession().AppForDocument(fullPath, appUid, dataType) == KErrNone)
                    {
                    TBuf<128> mimeTypeBuf;
                    mimeTypeBuf.Copy(dataType.Des8());
                    if (mimeTypeBuf == KNullDesC)
                        mimeTypeBuf.Copy(_L("N/A"));
                    
                    if (mimeTypeBuf.CompareF(_L("text/plain")) == 0)
                        viewerType = EFileBrowserCmdFileViewText;
                    }
                    
                FileEditorL(viewerType);   
                }

            else if (queryIndex == ECommonFileActionOpenWithApparc)
                {
                OpenWithApparcL();
                }

            else if (queryIndex == ECommonFileActionOpenWithDocHandlerEmbed)
                {
                OpenWithDocHandlerL(ETrue);
                }

            else
                User::Panic(_L("Unk.Com.Act"), 221);    
                            
            }
        }
    }
    
// --------------------------------------------------------------------------------------------
	            
// End of File
