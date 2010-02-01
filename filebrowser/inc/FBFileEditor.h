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


#ifndef FILEBROWSER_FILEVIEWERDLG_H
#define FILEBROWSER_FILEVIEWERDLG_H

//  INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <AknDialog.h>
#include <eiksbobs.h>

//  FORWARD DECLARATIONS
class CFileBrowserFileEditorViewControl;
class CEikEdwin;
class CParaFormat;
class CEikScrollBarFrame;


//  CLASS DEFINITIONS

class CFileBrowserFileEditorDlg : public CAknDialog
    {
public:
    static CFileBrowserFileEditorDlg* NewL(const TDesC& aFileName, TInt aMode);
    virtual ~CFileBrowserFileEditorDlg();

private: // Constructors
    CFileBrowserFileEditorDlg(const TDesC& aFileName, TInt aMode);
    void ConstructL();        

protected: // From CAknDialog
    void ProcessCommandL(TInt aCommandId);
    void PreLayoutDynInitL();
    void PostLayoutDynInitL();
        
protected: // From CEikDialog
    void SetSizeAndPosition(const TSize& aSize);
    SEikControlInfo CreateCustomControlL(TInt aControlType);

protected: // From MEikMenuObserver
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

public:  // New methods
    TBool RunDlgLD();

private:  // New methods
    void OpenFileLC(RFile& aFile, const TDesC& aFileName);
    void LoadFileL();    
    void GetTextFileMode(RFile& aFile, TInt& aFileSize);
    HBufC16* ConvertBuf8ToBuf16L(HBufC8* aBuf8);
    HBufC8* ConvertBuf16ToBuf8L(HBufC16* aBuf16);
    void SaveEditorDataL(TBool aQueryNewName=EFalse);
    TBool IsHexChar(TInt aCh);

private: // Data
    CFileBrowserFileEditorViewControl*  iViewer;
    CEikEdwin*                          iTextEditor;
    CParaFormat*                        iEditorPF;
    TFileName			                iFileName;
    TInt                                iActiveMode;
    TInt                                iTextFormat;
    };



class CFileBrowserFileEditorViewControl : public CCoeControl, MEikScrollBarObserver
    {
public:
    virtual ~CFileBrowserFileEditorViewControl();

public: // Constructors
    CFileBrowserFileEditorViewControl();
    void ConstructL();        

protected: // From CCoeControl
    void Draw(const TRect& aRect) const; 
    TKeyResponse OfferKeyEventL(const TKeyEvent &aKeyEvent, TEventCode aType);
    TSize MinimumSize();
    CCoeControl* ComponentControl(TInt aIndex) const;
    TInt CountComponentControls() const;    
    void HandleResourceChange(TInt aType);
    TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

protected: // From MEikScrollBarObserver
    void HandleScrollEventL(CEikScrollBar* aScrollBar, TEikScrollEvent aEventType);

public: // New methods
    void FormatAsTextL(HBufC16* aBuf=NULL);
    void FormatAsHexL(HBufC8* aBuf=NULL);

private: // New methods
    void InitVariables();
    void ResetVariables(TBool aResetData=ETrue);
    void UpdateScrollBarL();
    
private: // Data
    TInt			        iActiveMode;
    TRect                   iViewerRect;
    TInt                    iDrawingWidth;
    TInt                    iLeftDrawingPosition;
    TReal                   iX_factor;
    TReal                   iY_factor;
    TInt                    iNumberOfLinesFitsScreen;
    CEikScrollBarFrame*     iScrollBarFrame;
    CFont*                  iFont;
    TInt                    iCurrentLine;
    TInt                    iTotalNumberOfLines;
    
    HBufC8*                 iHexesBuffer;
    TInt                    iHexGrougWidth;
    TInt                    iAmountOfHexesFitsHorizontally;
    
    HBufC16*                iTextBuf;
    CArrayFix<TPtrC>*       iWrappedTextArray;
    };

#endif

// End of File

