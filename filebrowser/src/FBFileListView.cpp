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
#include <aknViewAppUi.h>
#include <avkon.hrh>
#include <apgtask.h>
#include <aknmessagequerydialog.h> 
#include <remconinterfaceselector.h>
#include <remconcoreapitarget.h>

#include <filebrowser.rsg>
#include "FB.hrh"
#include "FBFileListView.h"
#include "FBFileListContainer.h"
#include "FBDocument.h" 
#include "FBModel.h"
#include "FBFileUtils.h"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CFileBrowserFileListView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CFileBrowserFileListView::ConstructL()
    {
    BaseConstructL( R_FILEBROWSER_VIEW_FILELIST );
    iModel = static_cast<CFileBrowserDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();
    
    iRemConSelector = CRemConInterfaceSelector::NewL();
    iRemConTarget = CRemConCoreApiTarget::NewL(*iRemConSelector, *this);
    iRemConSelector->OpenTargetL();
    }

// ---------------------------------------------------------
// CFileBrowserFileListView::~CFileBrowserFileListView()
// ?implementation_description
// ---------------------------------------------------------
//
CFileBrowserFileListView::~CFileBrowserFileListView()
    {
    delete iRemConSelector;

    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CFileBrowserFileListView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CFileBrowserFileListView::Id() const
    {
    return KFileListViewUID;
    }

// ---------------------------------------------------------
// TUid CFileBrowserFileListView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
// ?implementation_description
// ---------------------------------------------------------
//

void CFileBrowserFileListView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    TBool emptyListBox = iContainer->ListBoxNumberOfVisibleItems() == 0;
    TBool driveListActive = iModel->FileUtils()->IsDriveListViewActive();
    TBool normalModeActive = iModel->FileUtils()->IsNormalModeActive();
    TBool currentDriveReadOnly = iModel->FileUtils()->IsCurrentDriveReadOnly();
    TBool currentItemDirectory = iModel->FileUtils()->IsCurrentItemDirectory();
    TBool listBoxSelections = iContainer->ListBoxSelectionIndexesCount() == 0;
    TBool emptyClipBoard = iModel->FileUtils()->ClipBoardList()->Count() == 0;
    TBool showSnapShot = iModel->FileUtils()->DriveSnapShotPossible();
    
    TBool hideEditMenu(EFalse);
    if (driveListActive)
        {
        if (!showSnapShot || emptyListBox && emptyClipBoard)
            hideEditMenu = ETrue;    
        else
            hideEditMenu = EFalse;
        }
    else
        {
        if (emptyListBox && emptyClipBoard)
            hideEditMenu = ETrue;    
        else
            hideEditMenu = EFalse;
        }
    

    if (aResourceId == R_FILEBROWSER_VIEW_FILELIST_MENU)
	    {
        aMenuPane->SetItemDimmed(EFileBrowserCmdEdit, hideEditMenu);
        aMenuPane->SetItemDimmed(EFileBrowserCmdDiskAdmin, !driveListActive);
	    }    

    else if (aResourceId == R_FILEBROWSER_FILE_SUBMENU)
	    {
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileBack, driveListActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileOpen, emptyListBox || driveListActive || currentItemDirectory);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileOpenDrive, emptyListBox || !driveListActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileOpenDirectory, emptyListBox || driveListActive || !currentItemDirectory);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileView, emptyListBox || !listBoxSelections || currentItemDirectory || driveListActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileEdit, emptyListBox || !listBoxSelections || currentItemDirectory || driveListActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileSendTo, emptyListBox || driveListActive || currentItemDirectory);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileNew, driveListActive || currentDriveReadOnly);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileDelete, emptyListBox || driveListActive || currentDriveReadOnly);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileRename, emptyListBox || driveListActive || currentDriveReadOnly || !listBoxSelections);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileTouch, emptyListBox || driveListActive || currentDriveReadOnly);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileProperties, emptyListBox || !listBoxSelections);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileChecksums, emptyListBox || !listBoxSelections || currentItemDirectory || driveListActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileSetAttributes, emptyListBox || driveListActive || currentDriveReadOnly);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileCompress, currentDriveReadOnly || emptyListBox || !listBoxSelections || currentItemDirectory || driveListActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileDecompress, currentDriveReadOnly || emptyListBox || !listBoxSelections || currentItemDirectory || driveListActive);
	    }
    
    else if (aResourceId == R_FILEBROWSER_EDIT_SUBMENU)
	    {
        TBool currentSelected = iContainer->ListBox()->View()->ItemIsSelected(iContainer->ListBox()->View()->CurrentItemIndex());
        TBool allSelected = iContainer->ListBox()->SelectionIndexes()->Count() == iContainer->ListBox()->Model()->NumberOfItems();
        TBool noneSelected = iContainer->ListBox()->SelectionIndexes()->Count() == 0;
        
        aMenuPane->SetItemDimmed(EFileBrowserCmdSnapShot, !driveListActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditCut, driveListActive || currentDriveReadOnly || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditCopy, driveListActive || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditPaste, driveListActive || emptyClipBoard || currentDriveReadOnly);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditCopyToFolder, driveListActive || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditMoveToFolder, driveListActive || currentDriveReadOnly || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditSelect, driveListActive || currentSelected || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditUnselect, driveListActive || !currentSelected || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditSelectAll, driveListActive || allSelected || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdEditUnselectAll, driveListActive || noneSelected || emptyListBox);
	    }

	else if (aResourceId == R_FILEBROWSER_VIEW_SUBMENU)
	    {
        aMenuPane->SetItemDimmed(EFileBrowserCmdViewSort, !normalModeActive || driveListActive || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdViewOrder, !normalModeActive || driveListActive || emptyListBox);
        aMenuPane->SetItemDimmed(EFileBrowserCmdViewRefresh, !normalModeActive);
        aMenuPane->SetItemDimmed(EFileBrowserCmdViewFilterEntries, emptyListBox);
	    }

	else if (aResourceId == R_FILEBROWSER_VIEW_SORT_SUBMENU)
	    {
        aMenuPane->SetItemButtonState(iModel->FileUtils()->SortMode(), EEikMenuItemSymbolOn);
	    }

	else if (aResourceId == R_FILEBROWSER_VIEW_ORDER_SUBMENU)
	    {
        aMenuPane->SetItemButtonState(iModel->FileUtils()->OrderMode(), EEikMenuItemSymbolOn);
	    }

	else if (aResourceId == R_FILEBROWSER_TOOLS_SUBMENU)
	    {
	    TBool noLocalCon = !iModel->FileUtils()->FileExists(KIRAppPath) && !iModel->FileUtils()->FileExists(KBTAppPath) && !iModel->FileUtils()->FileExists(KUSBAppPath);
	    
        aMenuPane->SetItemDimmed(EFileBrowserCmdToolsDisableExtErrors, !iModel->FileUtils()->FileExists(KErrRdPath));
        aMenuPane->SetItemDimmed(EFileBrowserCmdToolsEnableExtErrors, iModel->FileUtils()->FileExists(KErrRdPath));
        aMenuPane->SetItemDimmed(EFileBrowserCmdToolsLocalConnectivity, noLocalCon);
	    }

	else if (aResourceId == R_FILEBROWSER_TOOLS_LOCALCONNECTIVITY_SUBMENU)
	    {
        aMenuPane->SetItemDimmed(EFileBrowserCmdToolsLocalConnectivityActivateInfrared, !iModel->FileUtils()->FileExists(KIRAppPath));
        aMenuPane->SetItemDimmed(EFileBrowserCmdToolsLocalConnectivityLaunchBTUI, !iModel->FileUtils()->FileExists(KBTAppPath));
        aMenuPane->SetItemDimmed(EFileBrowserCmdToolsLocalConnectivityLaunchUSBUI, !iModel->FileUtils()->FileExists(KUSBAppPath));
	    }	    
    else
        {
        AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
        }
    }

// ---------------------------------------------------------
// CFileBrowserFileListView::MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct)
// Used to handle multimedia keys
// ---------------------------------------------------------
//

void CFileBrowserFileListView::MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction /*aButtonAct*/)
    {
    switch(aOperationId)
        {
        case ERemConCoreApiVolumeUp:
            {
            // simulate a keyevent, so that it can be handled as all other key events
            TKeyEvent keyEvent;
            keyEvent.iCode = EKeyIncVolume;
            keyEvent.iScanCode = 0;
            keyEvent.iModifiers = 0;
            keyEvent.iRepeats = 0;
            
            //TRAP_IGNORE( iContainer->OfferKeyEventL(keyEvent, EEventKey) );
            
            //RWsSession& wsSession=iCoeEnv->WsSession();
            //wsSession.SimulateKeyEvent(keyEvent);
            //wsSession.Flush();

            iCoeEnv->SimulateKeyEventL(keyEvent,EEventKeyDown);
            iCoeEnv->SimulateKeyEventL(keyEvent,EEventKey);
            iCoeEnv->SimulateKeyEventL(keyEvent,EEventKeyUp);
            
            break;
            }
        case ERemConCoreApiVolumeDown:
            {
            // simulate a keyevent, so that it can be handled as all other key events
            TKeyEvent keyEvent;
            keyEvent.iCode = EKeyDecVolume;
            keyEvent.iScanCode = 0;
            keyEvent.iModifiers = 0;
            keyEvent.iRepeats = 0;
                        
            //TRAP_IGNORE( iContainer->OfferKeyEventL(keyEvent, EEventKey) );
            
            //RWsSession& wsSession=iCoeEnv->WsSession();
            //wsSession.SimulateKeyEvent(keyEvent);
            //wsSession.Flush();
            
            iCoeEnv->SimulateKeyEventL(keyEvent,EEventKeyDown);
            iCoeEnv->SimulateKeyEventL(keyEvent,EEventKey);
            iCoeEnv->SimulateKeyEventL(keyEvent,EEventKeyUp);
            
            break;
            }
        }
    }

  
// ---------------------------------------------------------
// CFileBrowserFileListView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CFileBrowserFileListView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {

        case EFileBrowserCmdFileBack:
            {
            iModel->FileUtils()->MoveUpOneLevelL();
            break;
            }
                    
        case EFileBrowserCmdFileOpenApparc:
            {
            iModel->FileUtils()->OpenWithApparcL();
            break;
            }

        case EFileBrowserCmdFileOpenDocHandlerEmbed:
            {
            iModel->FileUtils()->OpenWithDocHandlerL(ETrue);
            break;
            }

        case EFileBrowserCmdFileOpenDocHandlerStandAlone:
            {
            iModel->FileUtils()->OpenWithDocHandlerL(EFalse);
            break;
            }

        case EFileBrowserCmdFileOpenFileService:
            {
            iModel->FileUtils()->OpenWithFileServiceL();
            break;
            }
            
        case EFileBrowserCmdFileOpenDrive:
        case EFileBrowserCmdFileOpenDirectory:
            {
            iModel->FileUtils()->MoveDownToDirectoryL();
            break;
            }

        case EFileBrowserCmdFileViewText:
        case EFileBrowserCmdFileViewHex:
        case EFileBrowserCmdFileEditText:
        case EFileBrowserCmdFileEditHex:
            {
            iModel->FileUtils()->FileEditorL(aCommand);
            break;
            }

        case EFileBrowserCmdFileSearch:
            {
            iModel->FileListContainer()->HideToolbar();
            iModel->FileUtils()->SearchL();
            break;
            }

        case EFileBrowserCmdFileSendTo:
            {
            iModel->FileUtils()->SendToL();
            break;
            }

        case EFileBrowserCmdFileNewFile:
            {
            iModel->FileUtils()->NewFileL();
            break;
            }

        case EFileBrowserCmdFileNewDirectory:
            {
            iModel->FileUtils()->NewDirectoryL();
            break;
            }

        case EFileBrowserCmdFileDelete:
            {
            iModel->FileUtils()->DeleteL();
            break;
            }

        case EFileBrowserCmdFileRename:
            {
            iModel->FileUtils()->RenameL();
            break;
            }

        case EFileBrowserCmdFileTouch:
            {
            iModel->FileUtils()->TouchL();
            break;
            }

        case EFileBrowserCmdFileProperties:
            {
            iModel->FileUtils()->PropertiesL();
            break;
            }

        case EFileBrowserCmdFileChecksumsMD5:
        case EFileBrowserCmdFileChecksumsMD2:
        case EFileBrowserCmdFileChecksumsSHA1:
            {
            iModel->FileUtils()->ShowFileCheckSumsL(aCommand);
            break;
            }
            
        case EFileBrowserCmdFileSetAttributes:
            {
            iModel->FileListContainer()->HideToolbar();
            iModel->FileUtils()->SetAttributesL();
            break;
            }

        case EFileBrowserCmdFileCompress:
            {
            iModel->FileUtils()->CompressL();
            break;
            }

        case EFileBrowserCmdFileDecompress:
            {
            iModel->FileUtils()->DecompressL();
            break;
            }
///        
        case EFileBrowserCmdSnapShot:
            {
            iModel->FileUtils()->DriveSnapShotL();
            break;
            }
            
        case EFileBrowserCmdEditCut:
            {
            iModel->FileUtils()->ClipboardCutL();
            break;
            }

        case EFileBrowserCmdEditCopy:
            {
            iModel->FileUtils()->ClipboardCopyL();
            break;
            }

        case EFileBrowserCmdEditPaste:
            {
            iModel->FileUtils()->ClipboardPasteL();
            break;
            }

        case EFileBrowserCmdEditCopyToFolder:
            {
            iModel->FileUtils()->CopyToFolderL();
            break;
            }

        case EFileBrowserCmdEditMoveToFolder:
            {
            iModel->FileUtils()->CopyToFolderL(ETrue);
            break;
            }            

        case EFileBrowserCmdEditSelect:
            {
            iModel->FileListContainer()->UpdateToolbar();
            AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdMark, iContainer->ListBox() );
            break;
            }

        case EFileBrowserCmdEditUnselect:
            {
            iModel->FileListContainer()->UpdateToolbar();
            AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdUnmark, iContainer->ListBox() );
            break;
            }

        case EFileBrowserCmdEditSelectAll:
            {
            iModel->FileListContainer()->UpdateToolbar();
            AknSelectionService::HandleMarkableListProcessCommandL( EAknMarkAll, iContainer->ListBox() );
            break;
            }
            
        case EFileBrowserCmdEditUnselectAll:
            {
            iModel->FileListContainer()->UpdateToolbar();
            AknSelectionService::HandleMarkableListProcessCommandL( EAknUnmarkAll, iContainer->ListBox() );
            break;
            }
///
        case EFileBrowserCmdViewSortByName:
        case EFileBrowserCmdViewSortByExtension:
        case EFileBrowserCmdViewSortByDate:
        case EFileBrowserCmdViewSortBySize:
            {
            iModel->FileUtils()->SetSortModeL(aCommand);
            break;
            }

        case EFileBrowserCmdViewOrderAscending:
        case EFileBrowserCmdViewOrderDescending:
            {
            iModel->FileUtils()->SetOrderModeL(aCommand);
            break;
            }
	
        case EFileBrowserCmdViewFilterEntries:
            {
            iContainer->EnableSearchFieldL();
            break;
            }

        case EFileBrowserCmdViewRefresh:
            {
            iModel->FileUtils()->RefreshViewL();
            break;
            }
///
        case EFileBrowserCmdDiskAdminSetDrivePassword:
            {
            iModel->FileUtils()->SetDrivePasswordL();
            break;
            }

        case EFileBrowserCmdDiskAdminUnlockDrive:
            {
            iModel->FileUtils()->UnlockDriveL();
            break;
            }

        case EFileBrowserCmdDiskAdminClearDrivePassword:
            {
            iModel->FileUtils()->ClearDrivePasswordL();
            break;
            }

        case EFileBrowserCmdDiskAdminEraseDrivePassword:
            {
            iModel->FileUtils()->EraseDrivePasswordL();
            break;
            }

        case EFileBrowserCmdDiskAdminFormatDrive:
            {
            iModel->FileUtils()->FormatDriveL(EFalse);
            break;
            }

        case EFileBrowserCmdDiskAdminQuickFormatDrive:
            {
            iModel->FileUtils()->FormatDriveL(ETrue);
            break;
            }

        case EFileBrowserCmdDiskAdminCheckDisk:
            {
            iModel->FileUtils()->CheckDiskL();
            break;
            }

        case EFileBrowserCmdDiskAdminScanDrive:
            {
            iModel->FileUtils()->ScanDriveL();
            break;
            }

        case EFileBrowserCmdDiskAdminSetDriveName:
            {
            iModel->FileUtils()->SetDriveNameL();
            break;
            }

        case EFileBrowserCmdDiskAdminSetDriveVolumeLabel:
            {
            iModel->FileUtils()->SetDriveVolumeLabelL();
            break;
            }

        case EFileBrowserCmdDiskAdminEjectDrive:
            {
            iModel->FileUtils()->EjectDriveL();
            break;
            }
            
        case EFileBrowserCmdDiskAdminDismountFileSystem:
            {
            iModel->FileUtils()->DismountFileSystemL();
            break;
            }
            
        case EFileBrowserCmdDiskAdminEraseMBR:
            {
            iModel->FileUtils()->EraseMBRL();
            break;
            }
            
        case EFileBrowserCmdDiskAdminDrivePartition:
            {
            iModel->FileUtils()->PartitionDriveL();
            break;
            }

///
        case EFileBrowserCmdToolsAllFiles:
            {
            iModel->FileUtils()->WriteAllFilesL();
            break;
            }

        case EFileBrowserCmdToolsAvkonIconCacheEnable:
            {
            iModel->FileUtils()->EnableAvkonIconCacheL(ETrue);
            break;
            }

        case EFileBrowserCmdToolsAvkonIconCacheDisable:
            {
            iModel->FileUtils()->EnableAvkonIconCacheL(EFalse);
            break;
            }

        case EFileBrowserCmdToolsEditDataTypes:
            {
            iModel->FileUtils()->EditDataTypesL();
            break;
            }
                        
        case EFileBrowserCmdToolsDisableExtErrors:
            {
            iModel->FileUtils()->SetErrRdL(EFalse);
            break;
            }

        case EFileBrowserCmdToolsEnableExtErrors:
            {
            iModel->FileUtils()->SetErrRdL(ETrue);
            break;
            }

        case EFileBrowserCmdToolsErrorSimulateLeave:
            {
            iModel->FileUtils()->SimulateLeaveL();
            break;
            }

        case EFileBrowserCmdToolsErrorSimulatePanic:
            {
            iModel->FileUtils()->SimulatePanicL();
            break;
            }
        
       case EFileBrowserCmdToolsErrorSimulateException:
            {
            iModel->FileUtils()->SimulateExceptionL();
            break;
            }
                            
        case EFileBrowserCmdToolsLocalConnectivityActivateInfrared:
            {
            iModel->FileUtils()->LaunchProgramL(KIRAppPath);
            break;
            }

        case EFileBrowserCmdToolsLocalConnectivityLaunchBTUI:
            {
            iModel->FileUtils()->LaunchProgramL(KBTAppPath);
            break;
            }

        case EFileBrowserCmdToolsLocalConnectivityLaunchUSBUI:
            {
            iModel->FileUtils()->LaunchProgramL(KUSBAppPath);
            break;
            }

        case EFileBrowserCmdToolsInstalledApps:
            {
            iModel->FileUtils()->WriteAllAppsL();
            break;
            }

        case EFileBrowserCmdToolsOpenFiles:
            {
            iModel->FileUtils()->ListOpenFilesL();
            break;
            }

        case EFileBrowserCmdToolsSetDebugMask:
            {
            iModel->FileUtils()->SetDebugMaskL();
            break;
            }
            
        case EFileBrowserCmdToolsMsgAttachmentsInbox:
        case EFileBrowserCmdToolsMsgAttachmentsDrafts:
        case EFileBrowserCmdToolsMsgAttachmentsSentItems:
        case EFileBrowserCmdToolsMsgAttachmentsOutbox:
            {
            iModel->FileUtils()->ListMessageAttachmentsL(aCommand);
            break;
            }

        case EFileBrowserCmdToolsMsgStoreWalk:
            {
            iModel->FileUtils()->WriteMsgStoreWalkL();
            break;
            }

        case EFileBrowserCmdToolsMemoryInfo:
            {
            iModel->FileUtils()->MemoryInfoPopupL();
            break;
            }

        case EFileBrowserCmdToolsSecureBackupStartBackup:
        case EFileBrowserCmdToolsSecureBackupStartRestore:
        case EFileBrowserCmdToolsSecureBackupStop:
            {
            iModel->FileUtils()->SecureBackupL(aCommand);
            break;
            }

///
        case EFileBrowserCmdSettings:
            {
            if (iModel->LaunchSettingsDialogL() == EAknCmdExit)
                AppUi()->HandleCommandL( EEikCmdExit );
            break;
            }

        case EFileBrowserCmdAbout:
            {
	        CAknMessageQueryDialog* dialog = new(ELeave) CAknMessageQueryDialog;
            dialog->ExecuteLD(R_FILEBROWSER_ABOUT_DIALOG);
            }
            break;
 
         case EAknSoftkeyCancel:
            {
            iContainer->DisableSearchFieldL();
            break;
            }
                   
        case EAknSoftkeyExit:
            {
            AppUi()->HandleCommandL( EEikCmdExit );
            break;
            }

        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CFileBrowserFileListView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CFileBrowserFileListView::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CFileBrowserFileListView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CFileBrowserFileListView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    if (!iContainer)
        {
        iContainer = new (ELeave) CFileBrowserFileListContainer;
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() );
        AppUi()->AddToStackL( *this, iContainer );
        } 
   }

// ---------------------------------------------------------
// CFileBrowserFileListView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CFileBrowserFileListView::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    
    delete iContainer;
    iContainer = NULL;
    }

// End of File

