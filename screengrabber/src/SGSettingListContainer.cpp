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
#include "SGSettingListContainer.h"
#include <screengrabber.rsg>
#include "SG.hrh"
#include "SGDocument.h"


#ifdef SCREENGRABBER_MULTIDRIVE_SUPPORT
  #include <CAknMemorySelectionSettingItemMultiDrive.h>
#else
  #include <CAknMemorySelectionSettingItem.h>
#endif

#include <eikappui.h>
#include <akntitle.h>
#include <eikspane.h> 

#include <AknDef.h>


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CScreenGrabberSettingListContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CScreenGrabberSettingListContainer::ConstructL(const TRect& aRect)
    {
    CreateWindowL();
    
    // set title of the app
    CEikStatusPane* statusPane = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* title = static_cast<CAknTitlePane*>( statusPane->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    title->SetTextL( _L("Settings") );

    // get an instance of the engine
    iModel = static_cast<CScreenGrabberDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();

    // get the settings from the engine
    iGrabSettings = iModel->GrabSettings();

    // construct the settings list from resources
    ConstructFromResourceL(R_SCREENGRABBER_SETTINGLIST);
    
    // set visibilities
    SetVisibilitiesOfSettingItems();

    // set correct drawing area for the listbox
    SetRect(aRect);
    HandleResourceChange(KEikDynamicLayoutVariantSwitch);

    ActivateL();
    }


// Destructor
CScreenGrabberSettingListContainer::~CScreenGrabberSettingListContainer()
    {
    }


// ---------------------------------------------------------
// CScreenGrabberSettingListView::CreateSettingItemL( TInt aIdentifier )
// ---------------------------------------------------------
//
CAknSettingItem* CScreenGrabberSettingListContainer::CreateSettingItemL( TInt aIdentifier )
    {
    CAknSettingItem* settingItem = NULL;

    switch (aIdentifier)
        {
        case ESettingListCaptureModeSelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iGrabSettings.iCaptureMode);
            break;
      
            
        case ESettingListSingleCaptureHotkeySelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iGrabSettings.iSingleCaptureHotkey);
            break;
        
        case ESettingListSingleCaptureImageFormatSelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iGrabSettings.iSingleCaptureImageFormat);
            break;
        
        case ESettingListSingleCaptureMemorySelection:
#ifdef SCREENGRABBER_MULTIDRIVE_SUPPORT
            settingItem = new(ELeave) CAknMemorySelectionSettingItemMultiDrive(aIdentifier, iGrabSettings.iSingleCaptureMemoryInUseMultiDrive);
#else
            settingItem = new(ELeave) CAknMemorySelectionSettingItem(aIdentifier, iGrabSettings.iSingleCaptureMemoryInUse);
#endif
            break;

        case ESettingListSingleCaptureFileNameSelection:
            settingItem = new(ELeave) CAknTextSettingItem(aIdentifier, iGrabSettings.iSingleCaptureFileName);
            break;


        case ESettingListSequantialCaptureHotkeySelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iGrabSettings.iSequantialCaptureHotkey);
            break;
        
        case ESettingListSequantialCaptureImageFormatSelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iGrabSettings.iSequantialCaptureImageFormat);
            break;

        case ESettingListSequantialCaptureDelaySelection:
            settingItem = new(ELeave) CAknIntegerEdwinSettingItem(aIdentifier, iGrabSettings.iSequantialCaptureDelay);
            break;
                    
        case ESettingListSequantialCaptureMemorySelection:
#ifdef SCREENGRABBER_MULTIDRIVE_SUPPORT
            settingItem = new(ELeave) CAknMemorySelectionSettingItemMultiDrive(aIdentifier, iGrabSettings.iSequantialCaptureMemoryInUseMultiDrive);
#else
            settingItem = new(ELeave) CAknMemorySelectionSettingItem(aIdentifier, iGrabSettings.iSequantialCaptureMemoryInUse);
#endif
            break;

        case ESettingListSequantialCaptureFileNameSelection:
            settingItem = new(ELeave) CAknTextSettingItem(aIdentifier, iGrabSettings.iSequantialCaptureFileName);
            break;


        case ESettingListVideoCaptureHotkeySelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iGrabSettings.iVideoCaptureHotkey);
            break;
        
        case ESettingListVideoCaptureVideoFormatSelection:
            settingItem = new(ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iGrabSettings.iVideoCaptureVideoFormat);
            break;
        
        case ESettingListVideoCaptureMemorySelection:
#ifdef SCREENGRABBER_MULTIDRIVE_SUPPORT
            settingItem = new(ELeave) CAknMemorySelectionSettingItemMultiDrive(aIdentifier, iGrabSettings.iVideoCaptureMemoryInUseMultiDrive);
#else
            settingItem = new(ELeave) CAknMemorySelectionSettingItem(aIdentifier, iGrabSettings.iVideoCaptureMemoryInUse);
#endif
            break;

        case ESettingListVideoCaptureFileNameSelection:
            settingItem = new(ELeave) CAknTextSettingItem(aIdentifier, iGrabSettings.iVideoCaptureFileName);
            break;

        }

    return settingItem;
    }


// ---------------------------------------------------------
// CScreenGrabberSettingListView::UpdateSettings()
// ---------------------------------------------------------
//
void CScreenGrabberSettingListContainer::UpdateSettingsL()
    {
    // get the modified settings to our own variables
    StoreSettingsL();  // from CAknSettingItemList

    // store new settings in engine and save the settings in disk
    TRAP_IGNORE(iModel->SaveSettingsL(iGrabSettings));
    
    // Change the keys used for capturing
    iModel->ActivateCaptureKeysL(ETrue);
    }


// ---------------------------------------------------------
// CScreenGrabberSettingListView::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
// ---------------------------------------------------------
//
void CScreenGrabberSettingListContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
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

// ---------------------------------------------------------
// CScreenGrabberSettingListView::ShowSettingPageL(TInt aCalledFromMenu)
// ---------------------------------------------------------
//
void CScreenGrabberSettingListContainer::ShowSettingPageL(TInt aCalledFromMenu) 
    {
    // open the setting page
    TInt listIndex = ListBox()->CurrentItemIndex();
    TInt realIndex = SettingItemArray()->ItemIndexFromVisibleIndex(listIndex);
    EditItemL(realIndex, aCalledFromMenu);

    // update and save the settings
    UpdateSettingsL();
    
    // set visibilities
    SetVisibilitiesOfSettingItems();

    // refresh the screen 
    DrawNow();
    }

// ---------------------------------------------------------
// CScreenGrabberSettingListContainer::SetVisibilitiesOfSettingItems()
// ---------------------------------------------------------
//
void CScreenGrabberSettingListContainer::SetVisibilitiesOfSettingItems() 
    {

    switch (iGrabSettings.iCaptureMode)
        {
        case ECaptureModeSingleCapture:
            {
            ((*SettingItemArray())[ESettingListSingleCaptureHotkeySelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSingleCaptureImageFormatSelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSingleCaptureMemorySelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSingleCaptureFileNameSelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSequantialCaptureHotkeySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureImageFormatSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureDelaySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureMemorySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureFileNameSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureHotkeySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureVideoFormatSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureMemorySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureFileNameSelection])->SetHidden(ETrue);
            }
            break;
        
        case ECaptureModeSequantialCapture:
            {
            ((*SettingItemArray())[ESettingListSingleCaptureHotkeySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSingleCaptureImageFormatSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSingleCaptureMemorySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSingleCaptureFileNameSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureHotkeySelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSequantialCaptureImageFormatSelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSequantialCaptureDelaySelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSequantialCaptureMemorySelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListSequantialCaptureFileNameSelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListVideoCaptureHotkeySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureVideoFormatSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureMemorySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureFileNameSelection])->SetHidden(ETrue);
            }
            break;            

        case ECaptureModeVideoCapture:
            {
            ((*SettingItemArray())[ESettingListSingleCaptureHotkeySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSingleCaptureImageFormatSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSingleCaptureMemorySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSingleCaptureFileNameSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureHotkeySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureImageFormatSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureDelaySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureMemorySelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListSequantialCaptureFileNameSelection])->SetHidden(ETrue);
            ((*SettingItemArray())[ESettingListVideoCaptureHotkeySelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListVideoCaptureVideoFormatSelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListVideoCaptureMemorySelection])->SetHidden(EFalse);
            ((*SettingItemArray())[ESettingListVideoCaptureFileNameSelection])->SetHidden(EFalse);
            }
            break;             

        default:
            User::Panic(_L("Inv.capt.mode"), 50);
            break;
        }
        
    HandleChangeInItemArrayOrVisibilityL(); 
    }

// ---------------------------------------------------------
// CScreenGrabberSettingListView::HandleResourceChange(TInt aType)
// ---------------------------------------------------------
//
void CScreenGrabberSettingListContainer::HandleResourceChange(TInt aType)
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);

        TSize outputRectSize;
        AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, outputRectSize);
        TRect outputRect(outputRectSize);
        ListBox()->SetRect(outputRect);
        }
    else
        {
        CCoeControl::HandleResourceChange(aType);
        }
    }

// End of File  
