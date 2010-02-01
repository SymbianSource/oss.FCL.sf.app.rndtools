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
#include "FBFileDlgs.h"
#include "FBFileUtils.h"
#include "FB.hrh"
#include "FBStd.h"
#include <filebrowser.rsg>

#include <aknlistquerydialog.h>
#include <AknIconArray.h>
#include <aknlists.h>
#include <eikbtgpc.h> 
#include <akntitle.h>
#include <AknPopupFieldText.h>
#include <aknnotewrappers.h> 
#include <eikfutil.h>
#include <eikclbd.h> 

const TInt KAttOn = 1;
const TInt KAttOff = 2;
const TInt KAttEnableRecursion = 0;
const TInt KAttDisableRecursion = 1;
const TInt64 KAlmostaDayInMicroSeconds = 86399999999;
 

// ===================================== MEMBER FUNCTIONS =====================================

CFileBrowserDestinationFolderSelectionDlg* CFileBrowserDestinationFolderSelectionDlg::NewL(
    TDes& aReturnPath, CDriveEntryList* aDriveEntryList, CAknIconArray* aIconArray)
    {
    CFileBrowserDestinationFolderSelectionDlg* self = new(ELeave) CFileBrowserDestinationFolderSelectionDlg(aReturnPath, aIconArray);
    CleanupStack::PushL(self);
    self->ConstructL(aDriveEntryList);
    CleanupStack::Pop(self);
    return self;    
    }

// --------------------------------------------------------------------------------------------

CFileBrowserDestinationFolderSelectionDlg::~CFileBrowserDestinationFolderSelectionDlg()
    {
    delete iFileEntryList;
    delete iDriveEntryList;
    
    iFs.Close();

    iEikonEnv->InfoMsgCancel();
    }

// --------------------------------------------------------------------------------------------

CFileBrowserDestinationFolderSelectionDlg::CFileBrowserDestinationFolderSelectionDlg(
    TDes& aReturnPath, CAknIconArray* aIconArray) : CAknListQueryDialog(&iSelectedIndex),
    iReturnPath(aReturnPath), iIconArray(aIconArray)
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDestinationFolderSelectionDlg::ConstructL(CDriveEntryList* aDriveEntryList)
    {
    User::LeaveIfError( iFs.Connect() );
    iDriveEntryList = new(ELeave) CDriveEntryList(8);
    iFileEntryList = new(ELeave) CFileEntryList(32);
    iCurrentPath = KNullDesC;
    
    //iIsDragging = EFalse;
    //EnableDragEvents();
    
    // get only writeable drives
    for (TInt i=0; i<aDriveEntryList->Count(); i++)
        {
        TDriveEntry driveEntry = aDriveEntryList->At(i);
        
        if (driveEntry.iVolumeInfo.iDrive.iMediaAtt != KMediaAttWriteProtected && driveEntry.iVolumeInfo.iDrive.iMediaAtt != KMediaAttLocked && driveEntry.iVolumeInfo.iDrive.iDriveAtt != KDriveAbsent)
            {
            iDriveEntryList->AppendL(driveEntry);
            }
        }
    }

// --------------------------------------------------------------------------------------------

/*
void CFileBrowserDestinationFolderSelectionDlg::HandlePointerEventL(const TPointerEvent &aPointerEvent)
    {
    // detect direction of dragging by comparing the start and finish points
    
    if (aPointerEvent.iType == TPointerEvent::EButton1Down)
        {
        iDragStartPoint = aPointerEvent.iPosition;
        iIsDragging = EFalse;
        }
    else if (aPointerEvent.iType == TPointerEvent::EDrag)
        {
        iIsDragging = ETrue;
        
        return;
        }
    else if (aPointerEvent.iType == TPointerEvent::EButton1Up)
        {
        if (iIsDragging)
            {
            const TInt KDelta = iDragStartPoint.iX - aPointerEvent.iPosition.iX;
            const TInt KThreshold = 30;
            
            if (KDelta < -KThreshold) // dragging to right
                {
                // "emulate" right key press
                
                TKeyEvent keyEvent;
                keyEvent.iCode = EKeyRightArrow;
                keyEvent.iModifiers = 0;
                
                TEventCode type = EEventKey;
        
                OfferKeyEventL(keyEvent, type);
                
                return;
                }
            else if (KDelta > KThreshold) // dragging to left
                {
                // "emulate" left key press
                
                TKeyEvent keyEvent;
                keyEvent.iCode = EKeyLeftArrow;
                keyEvent.iModifiers = 0;
                
                TEventCode type = EEventKey;
        
                OfferKeyEventL(keyEvent, type);
                
                return;
                }            
            }
        iIsDragging = EFalse;
        }
    else
        {
        iIsDragging = EFalse;        
        }

    CAknListQueryDialog::HandlePointerEventL(aPointerEvent);    
    }
*/
       
// --------------------------------------------------------------------------------------------

TKeyResponse CFileBrowserDestinationFolderSelectionDlg::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    if(aType != EEventKey)
        return EKeyWasNotConsumed;
    
    if (aKeyEvent.iCode == EKeyLeftArrow && !IsDriveListViewActive())
        {
        MoveUpOneLevelL(); 
        return EKeyWasConsumed;    
        }
    else if (aKeyEvent.iCode == EKeyRightArrow && ((ListBox()->CurrentItemIndex() > 0 && !IsDriveListViewActive()) || IsDriveListViewActive()))
        {
        MoveDownToDirectoryL();    
        return EKeyWasConsumed;    
        }
    else if (aKeyEvent.iCode == EKeyOK || aKeyEvent.iCode == EKeyEnter)    
        {
        if (IsDriveListViewActive())
            {
            MoveDownToDirectoryL();    
            return EKeyWasConsumed;    
            }            
        else if (ListBox()->CurrentItemIndex() == 0)
            {
            TryExitL(EAknSoftkeyOk);    
            return EKeyWasConsumed;    
            }
        else if (ListBox()->CurrentItemIndex() > 0)
            {
            MoveDownToDirectoryL();    
            return EKeyWasConsumed;    
            }            
        }

    TKeyResponse result = CAknDialog::OfferKeyEventL(aKeyEvent, aType);
    
    // update LSK label
    if (!IsDriveListViewActive())
        {
        if (ListBox()->CurrentItemIndex() == 0)
            {
            ButtonGroupContainer().SetCommandL(0, EAknSoftkeyOk, iLSKActionText);
            ButtonGroupContainer().DrawNow();                
            }
        else
            {
            ButtonGroupContainer().SetCommandL(0, EAknSoftkeyOk, _L("Open dir"));
            ButtonGroupContainer().DrawNow();                
            }
        }

    return result;    
    }    

// --------------------------------------------------------------------------------------------

void CFileBrowserDestinationFolderSelectionDlg::PreLayoutDynInitL()
    {
    CAknListQueryDialog::PreLayoutDynInitL();

    static_cast<CEikFormattedCellListBox*>(ListBox())->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDestinationFolderSelectionDlg::PostLayoutDynInitL()
    {
    CAknListQueryDialog::PostLayoutDynInitL();
    
    SetIconArrayL(static_cast<CArrayPtr<CGulIcon>*>(iIconArray)); 
    
    RefreshViewL();
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserDestinationFolderSelectionDlg::OkToExitL(TInt aButtonId)
    {
    if (aButtonId == GetLeftCBAShortKeyPress())
        {
        if (IsDriveListViewActive())
            {
            MoveDownToDirectoryL();    
            return EFalse;    
            }            
        else if (ListBox()->CurrentItemIndex() == 0)
            {
            // close the dialog
            iReturnPath = iCurrentPath;    
            return ETrue;    
            }
        else if (ListBox()->CurrentItemIndex() > 0)
            {
            MoveDownToDirectoryL();    
            return EFalse;    
            } 
        else
            return EFalse;
        }    
    else if (aButtonId == GetRightCBAShortKeyPress())
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDestinationFolderSelectionDlg::MoveUpOneLevelL()
    {
    if (iCurrentPath.Length() <= 3)
        {
        // move to drive list view is the current path is already short enough
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
        iCurrentPath = iCurrentPath.LeftTPtr(marker+1);
        }    

    // update view
    RefreshViewL();        
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDestinationFolderSelectionDlg::MoveDownToDirectoryL()
    {
    TInt index = ListBox()->CurrentItemIndex();
    
    if (index >= 0)
        {
        if (IsDriveListViewActive())
            {
            // currently in a drive list view, move to root of selected drive
            if (iDriveEntryList->Count() > index)
                {
                TDriveEntry driveEntry = iDriveEntryList->At(index);
                
                iCurrentPath.Append(driveEntry.iLetter);    
                iCurrentPath.Append(_L(":\\"));
                }
            }
        else
            {
            // this needed because we have an extra item in the listbox
            index--;

            // append the new directory
            if (iFileEntryList->Count() > index)
                {
                TFileEntry fileEntry = iFileEntryList->At(index);
                
                if (fileEntry.iEntry.IsDir())
                    {
                    iCurrentPath.Append(fileEntry.iEntry.iName);
                    iCurrentPath.Append(_L("\\"));
                    }
                }
            }
                   
        // update view
        RefreshViewL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDestinationFolderSelectionDlg::RefreshViewL()
    {
    CDesCArray* textArray = new(ELeave) CDesCArrayFlat(16);
    
    if (IsDriveListViewActive())
        {
        _LIT(KSimpleDriveEntry, "%d\t%c: <%S>\t\t");

        for (TInt i=0; i<iDriveEntryList->Count(); i++)
            {
            TFileName textEntry;
            TDriveEntry driveEntry = iDriveEntryList->At(i);
            
            textEntry.Format(KSimpleDriveEntry, driveEntry.iIconId, TUint(driveEntry.iLetter), &driveEntry.iMediaTypeDesc);
            
            textArray->AppendL(textEntry);
            }

        // set default LSK label
        ButtonGroupContainer().SetCommandL(0, EAknSoftkeyOk, _L("Open drive"));
        ButtonGroupContainer().DrawNow();
        }
    
    else
        {
        // first get the directory list
        iFileEntryList->Reset();
            
        CDir* dir = NULL;
        if (iFs.GetDir(iCurrentPath, KEntryAttDir|KEntryAttMatchMask, ESortByName | EAscending | EDirsFirst, dir) == KErrNone)
            {
            CleanupStack::PushL(dir);
            
            for (TInt i=0; i<dir->Count(); i++)
                {
                TFileEntry fileEntry;
                fileEntry.iPath = iCurrentPath;
                fileEntry.iEntry = (*dir)[i];
                fileEntry.iDirEntries = KErrNotFound;
                fileEntry.iIconId = EFixedIconEmpty;
                
                // check for directory entries
                if (fileEntry.iEntry.IsDir())
                    {
                    fileEntry.iIconId = EFixedIconFolder;
                    
                    TFileName subPath = fileEntry.iPath;
                    subPath.Append(fileEntry.iEntry.iName);
                    subPath.Append(_L("\\"));

                    // check if any sub directories                
                    CDir* subDir = NULL;
                    if (iFs.GetDir(subPath, KEntryAttDir|KEntryAttMatchMask, ESortNone | EDirsFirst, subDir) == KErrNone)
                        {
                        fileEntry.iDirEntries = subDir->Count();

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

                iFileEntryList->AppendL(fileEntry);
                }
            
            CleanupStack::PopAndDestroy(); //dir    
            }        
        
         
        _LIT(KSimpleFileEntry, "%d\t%S\t\t");

        
        // append current folder item
        TParse nameParser;
        TInt err = nameParser.SetNoWild(iCurrentPath.LeftTPtr(iCurrentPath.Length()-1), NULL, NULL);
        TFileName currentFolderName = nameParser.Name();
        if (currentFolderName == KNullDesC)
            currentFolderName.Copy(_L("[root level]"));

        TFileName currentDirTextEntry;
        currentDirTextEntry.Format(KSimpleFileEntry, EFixedIconFolderCurrent, &currentFolderName);
        textArray->AppendL(currentDirTextEntry);


        for (TInt i=0; i<iFileEntryList->Count(); i++)
            {
            TFileName textEntry;
            TFileEntry fileEntry = iFileEntryList->At(i);
            
            textEntry.Format(KSimpleFileEntry, fileEntry.iIconId, &fileEntry.iEntry.iName);
            
            textArray->AppendL(textEntry);
            }

         // set default LSK label
        ButtonGroupContainer().SetCommandL(0, EAknSoftkeyOk, iLSKActionText);
        ButtonGroupContainer().DrawNow();
        }

    if (iCurrentPath == KNullDesC)
        iEikonEnv->InfoMsgWithDuration(_L("<- up dir  down dir ->"), TTimeIntervalMicroSeconds32(KMaxTInt));
    else
        iEikonEnv->InfoMsgWithDuration(iCurrentPath, TTimeIntervalMicroSeconds32(KMaxTInt));
   
    SetItemTextArray(textArray);
    SetOwnershipType(ELbmOwnsItemArray);
    ListBox()->HandleItemAdditionL();
    Layout();
    ListBox()->SetCurrentItemIndex(0);
    DrawNow();
    }

// --------------------------------------------------------------------------------------------
    
TBool CFileBrowserDestinationFolderSelectionDlg::RunCopyDlgLD()
    {
    iLSKActionText.Copy(_L("Copy"));
    return ExecuteLD(R_COPY_TO_FOLDER_SELECTION_QUERY); 
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserDestinationFolderSelectionDlg::RunMoveDlgLD()
    {
    iLSKActionText.Copy(_L("Move"));
    return ExecuteLD(R_MOVE_TO_FOLDER_SELECTION_QUERY); 
    }

// --------------------------------------------------------------------------------------------

TInt CAknQueryDialog::GetLeftCBAShortKeyPress()
    {
    return TInt16(0xffff & TInt16(ButtonGroupContainer().ButtonGroup()->CommandId(0)));
    }

// --------------------------------------------------------------------------------------------
  
TInt CAknQueryDialog::GetRightCBAShortKeyPress()
    {
    return TInt16(0xffff & TInt16(ButtonGroupContainer().ButtonGroup()->CommandId(2)));
    }
       
        
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------


CFileBrowserAttributeEditorDlg* CFileBrowserAttributeEditorDlg::NewL(TUint& aSetAttMask, TUint& aClearAttMask, TBool& aRecurse)
    {
    CFileBrowserAttributeEditorDlg* self = new(ELeave) CFileBrowserAttributeEditorDlg(aSetAttMask, aClearAttMask, aRecurse);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
    }

// --------------------------------------------------------------------------------------------

CFileBrowserAttributeEditorDlg::~CFileBrowserAttributeEditorDlg()
    {
    }

// --------------------------------------------------------------------------------------------

CFileBrowserAttributeEditorDlg::CFileBrowserAttributeEditorDlg(TUint& aSetAttMask, TUint& aClearAttMask, TBool& aRecurse) :
    iSetAttMask(aSetAttMask), iClearAttMask(aClearAttMask), iRecurse(aRecurse)
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserAttributeEditorDlg::ConstructL()
    {
    CAknForm::ConstructL();

    // set title text
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* tp = static_cast<CAknTitlePane*>( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    tp->SetTextL( _L("Set attributes") );
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserAttributeEditorDlg::PreLayoutDynInitL()
    {
    CAknForm::PreLayoutDynInitL();
    
    // set default LSK label
    ButtonGroupContainer().SetCommandL(0, EAknSoftkeyOk, _L("Save"));

    // load values
    CAknPopupFieldText* archivePopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetArchive));
    CAknPopupFieldText* hiddenPopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetHidden));
    CAknPopupFieldText* readOnlyPopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetReadOnly));
    CAknPopupFieldText* systemPopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetSystem));
    CAknPopupFieldText* recursePopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetRecurse));
    
    if (iSetAttMask & KEntryAttArchive)
        archivePopup->SetCurrentValueIndex(KAttOn);
    else if (iClearAttMask & KEntryAttArchive)
        archivePopup->SetCurrentValueIndex(KAttOff);
    
    if (iSetAttMask & KEntryAttHidden)
        hiddenPopup->SetCurrentValueIndex(KAttOn);
    else if (iClearAttMask & KEntryAttHidden)
        hiddenPopup->SetCurrentValueIndex(KAttOff);
    
    if (iSetAttMask & KEntryAttReadOnly)
        readOnlyPopup->SetCurrentValueIndex(KAttOn);
    else if (iClearAttMask & KEntryAttReadOnly)
        readOnlyPopup->SetCurrentValueIndex(KAttOff);
    
    if (iSetAttMask & KEntryAttSystem)
        systemPopup->SetCurrentValueIndex(KAttOn);
    else if (iClearAttMask & KEntryAttSystem)
        systemPopup->SetCurrentValueIndex(KAttOff);
    
    if (iRecurse)
        recursePopup->SetCurrentValueIndex(KAttEnableRecursion);
    else
        recursePopup->SetCurrentValueIndex(KAttDisableRecursion);
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserAttributeEditorDlg::OkToExitL(TInt aButtonId)
    {
    if (aButtonId == EAknSoftkeyOk)
        {
        // save changes
        CAknPopupFieldText* archivePopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetArchive));
        CAknPopupFieldText* hiddenPopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetHidden));
        CAknPopupFieldText* readOnlyPopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetReadOnly));
        CAknPopupFieldText* systemPopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetSystem));
        CAknPopupFieldText* recursePopup = static_cast<CAknPopupFieldText*>(Control(EFileBrowserAttributeEditorSetRecurse));

        iSetAttMask = 0;
        iClearAttMask = 0;
        
        if (archivePopup->CurrentValueIndex() == KAttOn)
            iSetAttMask |= KEntryAttArchive;
        else if (archivePopup->CurrentValueIndex() == KAttOff)
            iClearAttMask |= KEntryAttArchive;
        
        if (hiddenPopup->CurrentValueIndex() == KAttOn)
            iSetAttMask |= KEntryAttHidden;
        else if (hiddenPopup->CurrentValueIndex() == KAttOff)
            iClearAttMask |= KEntryAttHidden;
        
        if (readOnlyPopup->CurrentValueIndex() == KAttOn)
            iSetAttMask |= KEntryAttReadOnly;
        else if (readOnlyPopup->CurrentValueIndex() == KAttOff)
            iClearAttMask |= KEntryAttReadOnly;
                
        if (systemPopup->CurrentValueIndex() == KAttOn)
            iSetAttMask |= KEntryAttSystem;
        else if (systemPopup->CurrentValueIndex() == KAttOff)
            iClearAttMask |= KEntryAttSystem;

        if (recursePopup->CurrentValueIndex() == KAttEnableRecursion)
            iRecurse = ETrue;
        else if (recursePopup->CurrentValueIndex() == KAttDisableRecursion)
            iRecurse = EFalse;
        }
    
    return ETrue;
    }       
// --------------------------------------------------------------------------------------------

TBool CFileBrowserAttributeEditorDlg::RunEditorLD()
    {
    return ExecuteLD(R_ATTRIBUTE_EDITOR_FORM_DIALOG);
    }
      
       
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------


CFileBrowserSearchQueryDlg* CFileBrowserSearchQueryDlg::NewL(TSearchAttributes& aSearchAttributes)
    {
    CFileBrowserSearchQueryDlg* self = new(ELeave) CFileBrowserSearchQueryDlg(aSearchAttributes);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
    }

// --------------------------------------------------------------------------------------------

CFileBrowserSearchQueryDlg::~CFileBrowserSearchQueryDlg()
    {
    }

// --------------------------------------------------------------------------------------------

CFileBrowserSearchQueryDlg::CFileBrowserSearchQueryDlg(TSearchAttributes& aSearchAttributes) :
    iSearchAttributes(aSearchAttributes)
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSearchQueryDlg::ConstructL()
    {
    CAknForm::ConstructL();

    // set title text
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* tp = static_cast<CAknTitlePane*>( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    tp->SetTextL( _L("Search") );
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserSearchQueryDlg::PreLayoutDynInitL()
    {
    CAknForm::PreLayoutDynInitL();
    
    // set default LSK label
    ButtonGroupContainer().SetCommandL(0, EAknSoftkeyOk, _L("Search"));

    // load values
    CEikEdwin* searchDir = static_cast<CEikEdwin*>(Control(EFileBrowserSearchQuerySearchDir));
    CEikEdwin* wildCards = static_cast<CEikEdwin*>(Control(EFileBrowserSearchQueryWildCards));
    CEikEdwin* textInFile = static_cast<CEikEdwin*>(Control(EFileBrowserSearchQueryTextInFile));
    CEikNumberEditor* minSize = static_cast<CEikNumberEditor*>(Control(EFileBrowserSearchQueryMinSize));
    CEikNumberEditor* maxSize = static_cast<CEikNumberEditor*>(Control(EFileBrowserSearchQueryMaxSize));
    CEikDateEditor* minDate = static_cast<CEikDateEditor*>(Control(EFileBrowserSearchQueryMinDate));
    CEikDateEditor* maxDate = static_cast<CEikDateEditor*>(Control(EFileBrowserSearchQueryMaxDate));
    CAknPopupFieldText* recurse = static_cast<CAknPopupFieldText*>(Control(EFileBrowserSearchQueryRecurse));

    searchDir->SetTextL(&iSearchAttributes.iSearchDir);
    wildCards->SetTextL(&iSearchAttributes.iWildCards);
    textInFile->SetTextL(&iSearchAttributes.iTextInFile);
    minSize->SetNumber(iSearchAttributes.iMinSize);
    maxSize->SetNumber(iSearchAttributes.iMaxSize);
    minDate->SetDate(iSearchAttributes.iMinDate);
    maxDate->SetDate(iSearchAttributes.iMaxDate);
    recurse->SetCurrentValueIndex(iSearchAttributes.iRecurse);
    }
// --------------------------------------------------------------------------------------------

TBool CFileBrowserSearchQueryDlg::OkToExitL(TInt aButtonId)
    {
    if (aButtonId == EAknSoftkeyOk)
        {
        // save changes
        CEikEdwin* searchDir = static_cast<CEikEdwin*>(Control(EFileBrowserSearchQuerySearchDir));
        CEikEdwin* wildCards = static_cast<CEikEdwin*>(Control(EFileBrowserSearchQueryWildCards));
        CEikEdwin* textInFile = static_cast<CEikEdwin*>(Control(EFileBrowserSearchQueryTextInFile));
        CEikNumberEditor* minSize = static_cast<CEikNumberEditor*>(Control(EFileBrowserSearchQueryMinSize));
        CEikNumberEditor* maxSize = static_cast<CEikNumberEditor*>(Control(EFileBrowserSearchQueryMaxSize));
        CEikDateEditor* minDate = static_cast<CEikDateEditor*>(Control(EFileBrowserSearchQueryMinDate));
        CEikDateEditor* maxDate = static_cast<CEikDateEditor*>(Control(EFileBrowserSearchQueryMaxDate));
        CAknPopupFieldText* recurse = static_cast<CAknPopupFieldText*>(Control(EFileBrowserSearchQueryRecurse));

        // get the current value of search dir and make sure it has a trailing backslash
        TFileName searchDirBuf;
        searchDir->GetText(searchDirBuf);
        if (searchDirBuf.Length() && searchDirBuf[searchDirBuf.Length()-1] != '\\')
            searchDirBuf.Append('\\');

        // check that the path is valid
        if (searchDirBuf.Length() && !EikFileUtils::PathExists(searchDirBuf))
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("Given search directory does not exists"));
            
            return EFalse;
            }

        iSearchAttributes.iSearchDir = searchDirBuf; 
        wildCards->GetText(iSearchAttributes.iWildCards);
        textInFile->GetText(iSearchAttributes.iTextInFile);
        iSearchAttributes.iMinSize = minSize->Number();
        iSearchAttributes.iMaxSize = maxSize->Number();
        iSearchAttributes.iMinDate = minDate->Date();
        iSearchAttributes.iMaxDate = maxDate->Date();
        // Adjust the MaxDate to the end of the day:
        iSearchAttributes.iMaxDate += TTimeIntervalMicroSeconds( KAlmostaDayInMicroSeconds );
        iSearchAttributes.iRecurse = recurse->CurrentValueIndex();
        }
    
    return ETrue;
    }       
// --------------------------------------------------------------------------------------------

TBool CFileBrowserSearchQueryDlg::RunQueryLD()
    {
    return ExecuteLD(R_SEARCH_QUERY_FORM_DIALOG);
    }
  
       
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------


CFileBrowserDataTypesDlg* CFileBrowserDataTypesDlg::NewL()
    {
    CFileBrowserDataTypesDlg* self = new(ELeave) CFileBrowserDataTypesDlg();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
    }

// --------------------------------------------------------------------------------------------

CFileBrowserDataTypesDlg::~CFileBrowserDataTypesDlg()
    {
    delete iDTArray;
    
    iLs.Close();
    }

// --------------------------------------------------------------------------------------------

CFileBrowserDataTypesDlg::CFileBrowserDataTypesDlg()
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDataTypesDlg::ConstructL()
    {
    // construct a menu bar
    CAknDialog::ConstructL(R_FILEBROWSER_DATA_TYPES_MENUBAR);

    // set title text
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* tp = static_cast<CAknTitlePane*>( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    tp->SetTextL( _L("Data types") );
    
    User::LeaveIfError(iLs.Connect());
    
    iDTArray = new(ELeave) CDataTypeArray(32);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDataTypesDlg::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
    {
    switch (aEventType)
        {
        case EEventEnterKeyPressed:
        case EEventItemDoubleClicked:
            // reserved for future extensions;
            break;
        default:
            break;
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDataTypesDlg::ProcessCommandL(TInt aCommandId)
    {
    CAknDialog::ProcessCommandL(aCommandId);

    switch (aCommandId)
        {
        case EFileBrowserCmdDataTypesChangeMapping:
            ChangeMappingL();
            break;
        case EFileBrowserCmdDataTypesSetDefaultMapping:
            SetDefaultMappingL();
            break;
        case EFileBrowserCmdDataTypesRefresh:
            GetDataTypesL();
            break;
        case EFileBrowserCmdDataTypesExit:
            TryExitL(EAknCmdExit);
            break;
        default:
            break;
        }
    } 
    
// --------------------------------------------------------------------------------------------

void CFileBrowserDataTypesDlg::PreLayoutDynInitL()
    {
    CAknDialog::PreLayoutDynInitL();

    iListBox = static_cast<CAknSingleGraphicStyleListBox*>( Control(EFileBrowserDataTypesList) );
    iListBox->SetContainerWindowL(*this);
    iListBox->View()->SetListEmptyTextL(_L("No data types"));

    iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
        
    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
    iListBox->SetListBoxObserver(this);
    
    iListBox->Model()->SetOwnershipType(ELbmOwnsItemArray);

    GetDataTypesL(); 
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDataTypesDlg::GetDataTypesL()
    {
    // get an instance of the listbox's item array
	MDesCArray* tempArray = iListBox->Model()->ItemTextArray();
	CDesCArray* listBoxArray = (CDesCArray*)tempArray;
	listBoxArray->Reset();
	
	// create an icon array
	CAknIconArray* iconArray = new(ELeave) CAknIconArray(32);
    CleanupStack::PushL(iconArray);

    // get data types
    iDTArray->Reset();
    User::LeaveIfError(iLs.GetSupportedDataTypesL(*iDTArray));
    
    // sort the array
    TKeyArrayFix key(0, ECmpFolded8);  // iDataType is located in offset 0 and contain 8-bit descriptor
    iDTArray->Sort(key);

    
    // loop each data type
    for (TInt i=0; i<iDTArray->Count(); i++)
        {
        TDataType& dt = iDTArray->At(i);
        
        // get app uid which handles this data type
        TUid appUid;
        TInt err = iLs.AppForDataType(dt, appUid);
        
        // generate icon which is the app icon which handles this mime type
        CFbsBitmap* bitmap = NULL;
        CFbsBitmap* mask = NULL;
        CGulIcon* appIcon = NULL;
        
        TRAPD(err2, 
            AknsUtils::CreateAppIconLC(AknsUtils::SkinInstance(), appUid, EAknsAppIconTypeList, bitmap, mask);
            appIcon = CGulIcon::NewL(bitmap, mask);
            CleanupStack::Pop(2); //bitmap, mask
            );
                         
        // append to the icon array    
        appIcon->SetBitmapsOwnedExternally(EFalse);
        iconArray->AppendL(appIcon);
        
        TFileName entry;
        entry.AppendNum(i);
        entry.Append(_L("\t"));
        entry.Append(dt.Des());
        entry.Append(_L("  <<"));
        

        // get caption of the handling app
        TFileName appCaption;
        
        if (err || err2 || appUid == KNullUid)
            entry.Append(_L("none"));
        else
            {
            TApaAppInfo appInfo;
            
            if (iLs.GetAppInfo(appInfo, appUid) == KErrNone)
                entry.Append(appInfo.iCaption);
            else
                entry.Append(_L("none"));
            }
        
        entry.Append(_L(">>"));

        listBoxArray->AppendL( entry );
        }


    // clear any previous icon array
    CAknIconArray* oldIconArray = static_cast<CAknIconArray*>(iListBox->ItemDrawer()->ColumnData()->IconArray());
    if (oldIconArray)
        delete oldIconArray;

    
    CleanupStack::Pop(); //iconArray

    // updates the listbox
    iListBox->ItemDrawer()->ColumnData()->SetIconArray(iconArray);
    iListBox->HandleItemAdditionL();    
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDataTypesDlg::ChangeMappingL()
    {
    TInt currentItemIndex = iListBox->CurrentItemIndex();
    
    if (iDTArray->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        // create list of TApaAppInfo
        CArrayFixFlat<TApaAppInfo>* appInfoArray = new(ELeave) CArrayFixFlat<TApaAppInfo>(64);
        CleanupStack::PushL(appInfoArray);

        // get list of applications
        TApaAppInfo appInfo;
        User::LeaveIfError(iLs.GetAllApps());

        while (iLs.GetNextApp(appInfo) == KErrNone)
            {
            appInfoArray->AppendL(appInfo);
            }

        // sort the array
        TKeyArrayFix key(_FOFF(TApaAppInfo,iCaption), ECmpFolded16);
        appInfoArray->Sort(key);

        // create a popup with listbox
        CAknSingleGraphicPopupMenuStyleListBox* listBox = new(ELeave) CAknSingleGraphicPopupMenuStyleListBox();
        CleanupStack::PushL(listBox);
        CAknPopupList* popupList = CAknPopupList::NewL(listBox, R_AVKON_SOFTKEYS_SELECT_CANCEL, AknPopupLayouts::EMenuGraphicWindow );
        CleanupStack::PushL(popupList);
        listBox->ConstructL( popupList, 0 );
        listBox->CreateScrollBarFrameL(ETrue);
        listBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
        listBox->View()->SetListEmptyTextL(_L("No apps"));
        listBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);        
        listBox->Model()->SetOwnershipType(ELbmOwnsItemArray);
        popupList->SetTitleL(_L("Select app"));
        popupList->EnableFind();


        // get an instance of the listbox's item array
    	MDesCArray* tempArray = listBox->Model()->ItemTextArray();
    	CDesCArray* listBoxArray = (CDesCArray*)tempArray;

    	// create an icon array
    	CAknIconArray* iconArray = new(ELeave) CAknIconArray(32);
        CleanupStack::PushL(iconArray);
        
        // loop each app info
        for (TInt i=0; i<appInfoArray->Count(); i++)
            {
                   
            // get icon of this app
            CFbsBitmap* bitmap = NULL;
            CFbsBitmap* mask = NULL;
            CGulIcon* appIcon = NULL;
            
            TRAP_IGNORE( 
                AknsUtils::CreateAppIconLC(AknsUtils::SkinInstance(), appInfoArray->At(i).iUid, EAknsAppIconTypeList, bitmap, mask);
                appIcon = CGulIcon::NewL(bitmap, mask);
                CleanupStack::Pop(2); //bitmap, mask
                );
                             
            // append to the icon array    
            appIcon->SetBitmapsOwnedExternally(EFalse);
            iconArray->AppendL(appIcon);            
            
            TFileName entry;
            entry.AppendNum(i);
            entry.Append(_L("\t"));
            entry.Append(appInfoArray->At(i).iCaption);
            
            listBoxArray->AppendL(entry);
            }


        CleanupStack::Pop(); //iconArray

        // updates the listbox
        listBox->ItemDrawer()->ColumnData()->SetIconArray(iconArray);
        listBox->HandleItemAdditionL(); 

    
        // execute the popup to query the application
        if (popupList->ExecuteLD())
            {

            // query priority
            TInt queryIndex(0);
            CAknListQueryDialog* listQueryDlg = new(ELeave) CAknListQueryDialog(&queryIndex);

            if (listQueryDlg->ExecuteLD(R_DATA_TYPE_PRIORITY_QUERY))
                {
                TInt32 selectedPriority(0);
                
                if (queryIndex == EDataTypePriorityMaximum)
                    selectedPriority = KDataTypeUnTrustedPriorityThreshold;
                else if (queryIndex == EDataTypePriorityHigh)
                    selectedPriority = KDataTypePriorityHigh;
                else if (queryIndex == EDataTypePriorityNormal)
                    selectedPriority = KDataTypePriorityNormal;
                else if (queryIndex == EDataTypePriorityLow)
                    selectedPriority = KDataTypePriorityLow;
                else
                    selectedPriority = KDataTypePriorityLastResort;
                
                
                TApaAppInfo& selectedAppInfo = appInfoArray->At(listBox->CurrentItemIndex());

                if (iLs.InsertDataMapping(iDTArray->At(currentItemIndex), selectedPriority, selectedAppInfo.iUid) == KErrNone)
                    {
                    GetDataTypesL();

                    CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                    note->ExecuteLD(_L("Mapping changed"));
                    }
                else
                    {
                    CAknErrorNote* note = new(ELeave) CAknErrorNote;
                    note->ExecuteLD(_L("Unable to change mapping"));
                    }  
                }
            }

        CleanupStack::Pop(); // popupList
        CleanupStack::PopAndDestroy(2); // listBox, appInfoArray
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserDataTypesDlg::SetDefaultMappingL()
    {
    TInt currentItemIndex = iListBox->CurrentItemIndex();
    
    if (iDTArray->Count() > currentItemIndex && currentItemIndex >= 0)
        {
        if (iLs.DeleteDataMapping(iDTArray->At(currentItemIndex)) == KErrNone)
            {
            GetDataTypesL();
            
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("Restored default"));
            }
        else
            {
            CAknInformationNote* note = new(ELeave) CAknInformationNote;
            note->ExecuteLD(_L("Nothing to restore"));
            }
        }
    }
        
// --------------------------------------------------------------------------------------------

TBool CFileBrowserDataTypesDlg::OkToExitL(TInt aButtonId)
    {
    return CAknDialog::OkToExitL(aButtonId);
    }
           
// --------------------------------------------------------------------------------------------

TBool CFileBrowserDataTypesDlg::RunQueryLD()
    {
    return ExecuteLD(R_DATA_TYPES_DIALOG);
    }
       
// --------------------------------------------------------------------------------------------
      
// End of File

