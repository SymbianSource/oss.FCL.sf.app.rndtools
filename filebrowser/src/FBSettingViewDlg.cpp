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
#include "FBSettingViewDlg.h"
#include "FBModel.h"
#include "FB.hrh"
#include "FBStd.h"
#include <filebrowser.rsg>

#include <aknsettingitemlist.h>
#include <CAknMemorySelectionSettingItem.h> 
#include <aknlists.h>
#include <akntitle.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <StringLoader.h>
#include <aknnotewrappers.h> 

// ===================================== MEMBER FUNCTIONS =====================================

CFileBrowserSettingViewDlg* CFileBrowserSettingViewDlg::NewL(TFileBrowserSettings& aSettings)
    {
    CFileBrowserSettingViewDlg* self = new(ELeave) CFileBrowserSettingViewDlg(aSettings);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
    }

// --------------------------------------------------------------------------------------------

CFileBrowserSettingViewDlg::~CFileBrowserSettingViewDlg()
    {
    // restore default navi pane
    if (iNaviContainer)
        iNaviContainer->Pop();
    
    if (iSettingItemArray)
        {
        iSettingItemArray->ResetAndDestroy();
        delete iSettingItemArray; 
        }
    
    if (iDecoratedTabGroup)
        delete iDecoratedTabGroup;   
    }

// --------------------------------------------------------------------------------------------

CFileBrowserSettingViewDlg::CFileBrowserSettingViewDlg(TFileBrowserSettings& aSettings) : iSettings(aSettings)
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::ConstructL()
    {
    // construct a menu bar
    CAknDialog::ConstructL(R_FILEBROWSER_SETTINGS_MENUBAR);
   
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    
    // set empty navi pane label
    iNaviContainer = static_cast<CAknNavigationControlContainer*>(sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi)));
    iNaviContainer->PushDefaultL();
    
    // set title text
    CAknTitlePane* tp = static_cast<CAknTitlePane*>( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    tp->SetTextL( _L("Settings") );
    
    // create a tab group
    iDecoratedTabGroup = iNaviContainer->CreateTabGroupL(this);
    iTabGroup = static_cast<CAknTabGroup*>(iDecoratedTabGroup->DecoratedControl());
    iTabGroup->SetTabFixedWidthL(KTabWidthWithTwoLongTabs); 
    
    // add tabs
    iTabGroup->AddTabL(ETabGeneral, _L("General"));
    iTabGroup->AddTabL(ETabFileOps, _L("Operations"));
    iTabGroup->SetActiveTabByIndex(ETabGeneral);
    
    iNaviContainer->PushL( *iDecoratedTabGroup );
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
    {
    switch (aEventType)
        {
        case EEventEnterKeyPressed:
        case EEventItemDoubleClicked:
            ShowSettingPageL(EFalse);
            break;
        default:
            break;
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::TabChangedL(TInt /*aIndex*/)
    {
    iListBox->SetCurrentItemIndex(0);

    SetVisibilitiesOfSettingItemsL();   
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::ProcessCommandL(TInt aCommandId)
    {
    CAknDialog::ProcessCommandL(aCommandId);

    switch (aCommandId)
        {
        case EFileBrowserCmdSettingsChange:
            ShowSettingPageL(ETrue);
            break;
        case EFileBrowserCmdSettingsExit:
            TryExitL(EAknCmdExit);
            break;
        default:
            break;
        }
    }

// --------------------------------------------------------------------------------------------

TKeyResponse CFileBrowserSettingViewDlg::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    if (iTabGroup == NULL)
        {
        return EKeyWasNotConsumed;
        }

    TInt active = iTabGroup->ActiveTabIndex();
    TInt count = iTabGroup->TabCount();

    switch ( aKeyEvent.iCode )
        {
        case EKeyLeftArrow:
            if (active > 0)
                {
                active--;
                iTabGroup->SetActiveTabByIndex(active);
                TabChangedL(active); 
                }
            break;
        
        case EKeyRightArrow:
            if((active + 1) < count)
                {
                active++;
                iTabGroup->SetActiveTabByIndex(active);
                TabChangedL(active); 
                }
            break;
        }

    return CAknDialog::OfferKeyEventL(aKeyEvent, aType);
    }    

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::PreLayoutDynInitL()
    {
    iListBox = static_cast<CAknSettingStyleListBox*>( Control(EFileBrowserSettingItemList) );
    iListBox->SetMopParent(this);
    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
    iListBox->SetListBoxObserver(this);

    iSettingItemArray = new(ELeave) CAknSettingItemArray(16, EFalse, 0);

    CTextListBoxModel* model = iListBox->Model();
    model->SetItemTextArray(iSettingItemArray);
    model->SetOwnershipType(ELbmDoesNotOwnItemArray);

    UpdateListBoxL();    
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserSettingViewDlg::OkToExitL(TInt aButtonId)
    {
    return CAknDialog::OkToExitL(aButtonId);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::ShowSettingPageL(TInt aCalledFromMenu)
    {
    TInt listIndex = iListBox->CurrentItemIndex();
    TInt realIndex = iSettingItemArray->ItemIndexFromVisibleIndex(listIndex);
    CAknSettingItem* item = iSettingItemArray->At(realIndex);
    item->EditItemL(aCalledFromMenu);
    item->StoreL();
    SetVisibilitiesOfSettingItemsL();
    DrawNow();
    
    if (realIndex == ESettingListBypassPlatformSecuritySelection && iSettings.iBypassPlatformSecurity)
        {
        _LIT(KMessage, "Gives TCB rights to file operations, use at your own risk");
        CAknInformationNote* note = new(ELeave) CAknInformationNote;
        note->ExecuteLD(KMessage);    
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::SetVisibilitiesOfSettingItemsL()
    {
    if (iSettingItemArray->Count() > 0)
        {
        switch (iTabGroup->ActiveTabIndex())
            {
            case ETabGeneral:
                {
                ((*iSettingItemArray)[ESettingListDisplayModeSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListFileViewModeSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListShowSubdirectoryInfoSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListShowAssociatedIconsSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListRememberLastPathSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListRememberFolderSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListEnableToolbar])->SetHidden(EFalse);
                
                ((*iSettingItemArray)[ESettingListSupportNetworkDrivesSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListBypassPlatformSecuritySelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListRemoveFileLocksSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListIgnoreProtectionsAttsSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListRemoveROMWriteProtectionSelection])->SetHidden(ETrue);
                }
                break;
            
            case ETabFileOps:
                {
                ((*iSettingItemArray)[ESettingListDisplayModeSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListFileViewModeSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListShowSubdirectoryInfoSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListShowAssociatedIconsSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListRememberLastPathSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListRememberFolderSelection])->SetHidden(ETrue);
                ((*iSettingItemArray)[ESettingListEnableToolbar])->SetHidden(ETrue);

                ((*iSettingItemArray)[ESettingListSupportNetworkDrivesSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListBypassPlatformSecuritySelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListRemoveFileLocksSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListIgnoreProtectionsAttsSelection])->SetHidden(EFalse);
                ((*iSettingItemArray)[ESettingListRemoveROMWriteProtectionSelection])->SetHidden(EFalse);
                }
                break;            

            default:
                User::Panic(_L("TabIOOB"), 50);
                break;
            }
            
        #ifdef FILEBROWSER_LITE
          ((*iSettingItemArray)[ESettingListBypassPlatformSecuritySelection])->SetHidden(ETrue);
        #endif
          
        #if(defined __SERIES60_30__ || defined __SERIES60_31__ || defined __S60_32__)
          ((*iSettingItemArray)[ESettingListEnableToolbar])->SetHidden(ETrue);
        #else
        if ( !AknLayoutUtils::PenEnabled() )
            {
            ((*iSettingItemArray)[ESettingListEnableToolbar])->SetHidden(ETrue);
            }
        #endif
        
        iSettingItemArray->RecalculateVisibleIndicesL();
        iListBox->HandleItemAdditionL();
        iListBox->UpdateScrollBarsL();
        }
    }   

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::UpdateListBoxL()
    {
    iSettingItemArray->ResetAndDestroy();
    
    // create items
    TInt ordinal(0);
 
    AddSettingItemL(ESettingListDisplayModeSelection,
                    R_DISPLAYMODESELECTION_SETTING_TITLE,
                    R_DISPLAYMODESELECTION_SETTING_PAGE,
                    R_DISPLAYMODESELECTION_SETTING_TEXTS,
                    ordinal++);
                    
    AddSettingItemL(ESettingListFileViewModeSelection,
                    R_FILEVIEWMODESELECTION_SETTING_TITLE,
                    R_FILEVIEWMODESELECTION_SETTING_PAGE,
                    R_FILEVIEWMODESELECTION_SETTING_TEXTS,
                    ordinal++); 

    AddSettingItemL(ESettingListShowSubdirectoryInfoSelection,
                    R_SHOWSUBDIRECTORYINFOSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++); 

    AddSettingItemL(ESettingListShowAssociatedIconsSelection,
                    R_SHOWASSOCIATEDICONSSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++); 
 
    AddSettingItemL(ESettingListRememberLastPathSelection,
                    R_REMEMBERLASTPATHSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++); 

    AddSettingItemL(ESettingListRememberFolderSelection,
                    R_REMEMBERFOLDERSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++);

    AddSettingItemL(ESettingListEnableToolbar,
                    R_ENABLETOOLBAR_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++);
    
    AddSettingItemL(ESettingListSupportNetworkDrivesSelection,
                    R_SUPPORTNETWORKDRIVESSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++); 
                     
    AddSettingItemL(ESettingListBypassPlatformSecuritySelection,
                    R_BYPASSPLATFORMSECURITYSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++); 
 
    AddSettingItemL(ESettingListRemoveFileLocksSelection,
                    R_REMOVEFILELOCKSSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++);

    AddSettingItemL(ESettingListIgnoreProtectionsAttsSelection,
                    R_IGNOREPROTECTIONATTSSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++);   

    AddSettingItemL(ESettingListRemoveROMWriteProtectionSelection,
                    R_REMOVEROMWRITEPROTECTIONSELECTION_SETTING_TITLE,
                    R_BINARY_SETTING_PAGE,
                    R_YESNO_BINARYSELECTION_TEXTS,
                    ordinal++);
                                                              
    SetVisibilitiesOfSettingItemsL(); 
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSettingViewDlg::AddSettingItemL(TInt aId,
                                                 TInt aTitleResource,
                                                 TInt aSettingPageResource,
                                                 TInt aAssociatedResource,
                                                 TInt aOrdinal)
    {
    // create a setting item
    CAknSettingItem* settingItem = NULL;
    
    switch (aId)
        {
        case ESettingListDisplayModeSelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aId, iSettings.iDisplayMode);
            break;
            
        case ESettingListFileViewModeSelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aId, iSettings.iFileViewMode);
            break;

        case ESettingListShowSubdirectoryInfoSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iShowSubDirectoryInfo);
            break;  

        case ESettingListShowAssociatedIconsSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iShowAssociatedIcons);
            break;  
            
        case ESettingListRememberLastPathSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iRememberLastPath);
            break;
            
        case ESettingListRememberFolderSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iRememberFolderSelection);
            break;

        case ESettingListEnableToolbar:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iEnableToolbar);
            break;

        case ESettingListSupportNetworkDrivesSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iSupportNetworkDrives);
            break; 
            
        case ESettingListBypassPlatformSecuritySelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iBypassPlatformSecurity);
            break;  

        case ESettingListRemoveFileLocksSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iRemoveFileLocks);
            break;  

        case ESettingListIgnoreProtectionsAttsSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iIgnoreProtectionsAtts);
            break;
            
        case ESettingListRemoveROMWriteProtectionSelection:
            settingItem = new(ELeave) CAknBinaryPopupSettingItem(aId, iSettings.iRemoveROMWriteProrection);
            break;            


        default:
            User::Panic(_L("NotSetItem"), 50);
            break;
        }
        
    CleanupStack::PushL(settingItem);

    // get title text
    HBufC* itemTitle = StringLoader::LoadLC(aTitleResource);

    // construct the setting item
    settingItem->ConstructL(EFalse, aOrdinal, *itemTitle, NULL, aSettingPageResource,
                            EAknCtPopupSettingList, NULL, aAssociatedResource);

    // append the setting item to settingitem array
    iSettingItemArray->InsertL(aOrdinal, settingItem);

    CleanupStack::PopAndDestroy(); //itemTitle
    CleanupStack::Pop(); //settingItem
    }

// --------------------------------------------------------------------------------------------
    
// End of File

