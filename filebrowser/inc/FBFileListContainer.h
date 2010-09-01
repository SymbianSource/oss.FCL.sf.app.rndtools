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


#ifndef FILEBROWSERMAINCONTAINER_H
#define FILEBROWSERMAINCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <aknlists.h>
#include <eiklbo.h>

// FORWARD DECLARATIONS
class CFileBrowserModel;
class CEikTextListBox;
class CAknSearchField;
class CAknNavigationControlContainer;
class CAknNavigationDecorator;
class CAknIconArray;
class CAknInfoPopupNoteController;
class CFont;
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    class CFileBrowserToolbar;
#endif

// CLASS DECLARATIONS

class CFileBrowserFileListContainer : public CCoeControl, MCoeControlObserver, MEikListBoxObserver
    {
public:
    void ConstructL(const TRect& aRect);
    ~CFileBrowserFileListContainer();

private:
    void SizeChanged();
    TInt CountComponentControls() const;
    CCoeControl* ComponentControl(TInt aIndex) const;
    void Draw(const TRect& aRect) const;
    void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
    void HandleResourceChange(TInt aType);
    void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType); // From MEikListBoxObserver
    void HandlePointerEventL(const TPointerEvent &aPointerEvent);

public:
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode /*aType*/);
    void CreateEmptyNaviPaneLabelL();
    void DeleteNaviPane();
    void CreateListBoxL(TInt aFileViewMode);
    TInt CurrentListBoxItemIndex();
    const CArrayFix<TInt>* ListBoxSelectionIndexes();
    TInt ListBoxSelectionIndexesCount();
    TInt ListBoxNumberOfVisibleItems();
    CAknIconArray* ListBoxIconArrayL();
    void SetListBoxIconArrayL(CAknIconArray* aIconArray);
    void SetListBoxTextArrayL(CDesCArray* aTextArray);
    void EnableSearchFieldL();
    TBool IsSearchFieldEnabled();
    void DisableSearchFieldL();
    void SetScreenLayoutL(TInt aLayoutMode);
    void SetNaviPaneTextL(const TDesC& aText);
    inline CEikTextListBox* ListBox() { return iListBox; }
    inline CAknSearchField* SearchField() { return iSearchField; }
    void HandleSettingsChangeL();
    void HideToolbar();
    void UpdateToolbar();

private:
    CFileBrowserModel*                  iModel;
    CEikTextListBox*                    iListBox;
    CAknSearchField*                    iSearchField;
    CAknNavigationControlContainer*     iNaviContainer;
    CAknNavigationDecorator*            iNaviDecorator;
    CAknInfoPopupNoteController*        iInfoPopup;
    TInt                                iFileViewMode;
    TPoint                              iDragStartPoint;
    TBool                               iIsDragging;
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    CFileBrowserToolbar*                iToolbar;
#endif
    };


class CSimpleFileViewListBox : public CAknSingleGraphicStyleListBox
    {
private:
    ~CSimpleFileViewListBox();
    void ReleaseFonts();
    void SizeChanged();
    void SizeChangedL();
private:
    CFont*     iFont;
    };


class CExtendedFileViewListBox : public CAknDoubleGraphicStyleListBox
    {
private:    
    ~CExtendedFileViewListBox();
    void ReleaseFonts();
    void SizeChanged();
    void SizeChangedL();
private:
    CFont*     iFont1;
    CFont*     iFont2;
    };
    
#endif

// End of File