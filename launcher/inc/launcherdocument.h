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



#ifndef __LAUNCHER_DOCUMENT_H__
#define __LAUNCHER_DOCUMENT_H__


#include <akndoc.h>

// Forward references
class CLauncherAppUi;
class CEikApplication;


/*! 
  @class CLauncherDocument
  
  @discussion An instance of class CLauncherDocument is the Document part of the AVKON
  application framework for the Launcher example application
  */
class CLauncherDocument : public CAknDocument
    {
public:

/*!
  @function NewL
  
  @discussion Construct a CLauncherDocument for the AVKON application aApp 
  using two phase construction, and return a pointer to the created object
  @param aApp application creating this document
  @result a pointer to the created instance of CLauncherDocument
  */
    static CLauncherDocument* NewL(CEikApplication& aApp);

/*!
  @function NewLC
  
  @discussion Construct a CLauncherDocument for the AVKON application aApp 
  using two phase construction, and return a pointer to the created object
  @param aApp application creating this document
  @result a pointer to the created instance of CLauncherDocument
  */
    static CLauncherDocument* NewLC(CEikApplication& aApp);

/*!
  @function ~CLauncherDocument
  
  @discussion Destroy the object and release all memory objects
  */
    ~CLauncherDocument();

/*!
  @function CreateAppUiL 
  
  @discussion Create a CLauncherAppUi object and return a pointer to it
  @result a pointer to the created instance of the AppUi created
  */
    CEikAppUi* CreateAppUiL();
private:

/*!
  @function ConstructL
  
  @discussion Perform the second phase construction of a CLauncherDocument object
  */
    void ConstructL();

/*!
  @function CLauncherDocument
  
  @discussion Perform the first phase of two phase construction 
  @param aApp application creating this document
  */
    CLauncherDocument(CEikApplication& aApp);

    };


#endif // __LAUNCHER_DOCUMENT_H__
