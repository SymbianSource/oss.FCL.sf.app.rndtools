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


#ifndef FILEBROWSER_FILEDLGS_H
#define FILEBROWSER_FILEDLGS_H


//  INCLUDES
#include "FBFileUtils.h"

#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <aknlistquerydialog.h>
#include <apgcli.h>


//  FORWARD DECLARATIONS
class CAknIconArray;
class CAknSingleGraphicStyleListBox;


//  CLASS DEFINITIONS

class CFileBrowserDestinationFolderSelectionDlg : public CAknListQueryDialog
    {
public:
    static CFileBrowserDestinationFolderSelectionDlg* NewL(TDes& aReturnPath, CDriveEntryList* aDriveEntryList, CAknIconArray* aIconArray);
    virtual ~CFileBrowserDestinationFolderSelectionDlg();

private: // Constructors
    CFileBrowserDestinationFolderSelectionDlg(TDes& aReturnPath, CAknIconArray* aIconArray);
    void ConstructL(CDriveEntryList* aDriveEntryList);        

protected:  // From CEikDialog
    TKeyResponse OfferKeyEventL(const TKeyEvent &aKeyEvent, TEventCode aType);
    void PreLayoutDynInitL();
    void PostLayoutDynInitL();
    TBool OkToExitL(TInt aButtonId);    

protected:  // From CCoeControl
    //void HandlePointerEventL(const TPointerEvent &aPointerEvent); 

private: // New methods
    void MoveUpOneLevelL();
    void MoveDownToDirectoryL();
    void RefreshViewL();
    inline TBool IsDriveListViewActive() { return iCurrentPath==KNullDesC; }
    
public:  // New methods
    TBool RunCopyDlgLD();
    TBool RunMoveDlgLD();

private: // Data
    TInt                            iSelectedIndex;
    TBuf<16>                        iLSKActionText;
    TDes&                           iReturnPath;
    RFs                             iFs;
    CDriveEntryList*                iDriveEntryList;
    CFileEntryList*                 iFileEntryList;
    CAknIconArray*                  iIconArray;
    TFileName                       iCurrentPath;
    //TPoint                          iDragStartPoint;
    //TBool                           iIsDragging;
    };


class CFileBrowserAttributeEditorDlg : public CAknForm
    {
public:
    static CFileBrowserAttributeEditorDlg* NewL(TUint& aSetAttMask, TUint& aClearAttMask, TBool& aRecurse);
    virtual ~CFileBrowserAttributeEditorDlg();

private: // Constructors
    CFileBrowserAttributeEditorDlg(TUint& aSetAttMask, TUint& aClearAttMask, TBool& aRecurse);
    void ConstructL();        

protected:  // From CEikDialog
    TBool OkToExitL(TInt aButtonId);    
    void PreLayoutDynInitL();
    
public:  // New methods
    TBool RunEditorLD();

private: // Data
    TUint&                          iSetAttMask;
    TUint&                          iClearAttMask;
    TBool&                          iRecurse;
    };


class CFileBrowserSearchQueryDlg : public CAknForm
    {
public:
    static CFileBrowserSearchQueryDlg* NewL(TSearchAttributes& aSearchAttributes);
    virtual ~CFileBrowserSearchQueryDlg();

private: // Constructors
    CFileBrowserSearchQueryDlg(TSearchAttributes& aSearchAttributes);
    void ConstructL();        

protected:  // From CEikDialog
    TBool OkToExitL(TInt aButtonId);    
    void PreLayoutDynInitL();
    
public:  // New methods
    TBool RunQueryLD();

private: // Data
    TSearchAttributes&              iSearchAttributes;
    };


class CFileBrowserDataTypesDlg : public CAknDialog, public MEikListBoxObserver
    {
public:
    static CFileBrowserDataTypesDlg* NewL();
    virtual ~CFileBrowserDataTypesDlg();

public: // From MEikListBoxObserver
    void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);

public: // From CAknDialog
    void ProcessCommandL(TInt aCommandId);
        
private: // Constructors
    CFileBrowserDataTypesDlg();
    void ConstructL();        

protected: // From CEikDialog
    void PreLayoutDynInitL();
    TBool OkToExitL(TInt aButtonId); 
    
public:  // New methods
    void GetDataTypesL();
    void ChangeMappingL();
    void SetDefaultMappingL();
    TBool RunQueryLD();

private: // Data
    CAknSingleGraphicStyleListBox*          iListBox;
    RApaLsSession                           iLs;
    CDataTypeArray*                         iDTArray;  
    };

#endif

// End of File

