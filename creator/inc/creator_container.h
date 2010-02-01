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




#ifndef __CREATORAPPVIEW_H__
#define __CREATORAPPVIEW_H__

#include <coecntrl.h>

/*! 
  @class CCreatorAppView
  
  @discussion An instance of the Application View object for the Creator 
  example application
  */
class CCreatorAppView : public CCoeControl
    {
public:



/*!
  @function NewL
   
  @discussion Create a CCreatorAppView object, which will draw itself to aRect
  @param aRect the rectangle this view will be drawn to
  @result a pointer to the created instance of CCreatorAppView
  */
    static CCreatorAppView* NewL(const TRect& aRect);

/*!
  @function NewLC
   
  @discussion Create a CCreatorAppView object, which will draw itself to aRect
  @param aRect the rectangle this view will be drawn to
  @result a pointer to the created instance of CCreatorAppView
  */
    static CCreatorAppView* NewLC(const TRect& aRect);


/*!
  @function ~CCreatorAppView
  
  @discussion Destroy the object and release all memory
  */
     ~CCreatorAppView();

/*!
  @function UserDraw
  
  @discussion Draw this CHelloWorldAppView to the screen
  @param aRect the rectangle of this view that needs updating
  */
    void UserDraw() const;


public:  // from CCoeControl
/*!
  @function Draw
  
  @discussion Draw this CCreatorAppView to the screen
  @param aRect the rectangle of this view that needs updating
  */
    void Draw(const TRect& aRect) const;
  

/*!
  @function OfferKeyEventL
  
  @discussion Handle any user keypresses
  @param aKeyEvent holds the data for the event that occurred
  @param aType holds the type of key event that occured
  @result a TKeyResponse indicating if the key was consumed or not
  */
    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

/*!
  @function InputCapabilities
  
  @discussion Return the capabilities of the OfferKeyEventL 
              method for this class
  @result a TCoeInputCapabilities indicating the capabilities 
          for this class
  */
	TCoeInputCapabilities InputCapabilities() const;



private:

/*!
  @function ConstructL
  
  @discussion  Perform the second phase construction of a CCreatorAppView object
  @param aRect the rectangle this view will be drawn to
  */
    void ConstructL(const TRect& aRect);

/*!
  @function CCreatorAppView
  
  @discussion Perform the first phase of two phase construction 
  */
    CCreatorAppView();


    TBool MMC_OK() const;

public:
    void HandleResourceChange(TInt aType);
    };


#endif // __CREATORAPPVIEW_H__
