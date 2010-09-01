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



#ifndef __CREATOR_APPUI_H__
#define __CREATOR_APPUI_H__

#include "creator_model.h"
#include <creator.rsg>
#include <aknappui.h>
#include <aknnotewrappers.h> 


class CCoeEnv;
class CCreatorEngine;

class CCreatorAppView;



/*! 
  @class CCreatorAppUi
  
  @discussion An instance of class CCreatorAppUi is the UserInterface part of the AVKON
  application framework for the Creator example application
  */
class CCreatorAppUi : public CAknAppUi, public MBeating
    {
public:
/*!
  @function ConstructL
  
  @discussion Perform the second phase construction of a CCreatorAppUi object
  this needs to be public due to the way the framework constructs the AppUi 
  */
    void ConstructL();

/*!
  @function CCreatorAppUi
  
  @discussion Perform the first phase of two phase construction.
  This needs to be public due to the way the framework constructs the AppUi 
  */
    CCreatorAppUi();


/*!
  @function ~CCreatorAppUi
  
  @discussion Destroy the object and release all memory objects
  */
    ~CCreatorAppUi();


public: // from CEikAppUi
/*!
  @function HandleCommandL
  
  @discussion Handle user menu selections
  @param aCommand the enumerated code for the option selected
  */
    void HandleCommandL(TInt aCommand);



public: // from MEikMenuPaneObserver
    
    /*!
     @function DynInitMenuPaneL

     @discussion Initialise a menu pane before it is displayed
     @param aMenuId id of menu
     @param aMenuPane handle for menu pane
     */
    void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

    // callback from engine when the commands from the script file have been executed
    void RunScriptDone();

    // from MBeating
    void Beat();
    void Synchronize();

private:
	void Tick();
	
protected:
    CEikonEnv* iEnv;


//private:
public:
    CCreatorAppView* iAppView;
    CCreatorEngine* iEngine;



public:

private:
	HBufC* iCommandLineScriptName;
	HBufC* iCommandLineRandomDataFileName;
	CHeartbeat* iTimer;
	enum TTimerMode
		{
		ETimerModeNone,
		ETimerModeStartScript,
		ETimerModeExitAppUi
		};
	TTimerMode iMode;
	TInt iTickCount;
    };





#endif // __CREATOR_APPUI_H__

