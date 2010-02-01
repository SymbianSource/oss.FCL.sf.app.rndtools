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




#ifndef __CREATOR_DOCUMENT_H__
#define __CREATOR_DOCUMENT_H__


#include <AknDoc.h>

// Forward references
class CCreatorAppUi;
class CEikApplication;


/*! 
  @class CCreatorDocument
  
  @discussion An instance of class CCreatorDocument is the Document part of the AVKON
  application framework for the Creator example application
  */
class CCreatorDocument : public CAknDocument
    {
public:

/*!
  @function NewL
  
  @discussion Construct a CCreatorDocument for the AVKON application aApp 
  using two phase construction, and return a pointer to the created object
  @param aApp application creating this document
  @result a pointer to the created instance of CCreatorDocument
  */
    static CCreatorDocument* NewL(CEikApplication& aApp);

/*!
  @function NewLC
  
  @discussion Construct a CCreatorDocument for the AVKON application aApp 
  using two phase construction, and return a pointer to the created object
  @param aApp application creating this document
  @result a pointer to the created instance of CCreatorDocument
  */
    static CCreatorDocument* NewLC(CEikApplication& aApp);

/*!
  @function ~CCreatorDocument
  
  @discussion Destroy the object and release all memory objects
  */
    ~CCreatorDocument();

/*!
  @function CreateAppUiL 
  
  @discussion Create a CCreatorAppUi object and return a pointer to it
  @result a pointer to the created instance of the AppUi created
  */
    CEikAppUi* CreateAppUiL();
private:

/*!
  @function ConstructL
  
  @discussion Perform the second phase construction of a CCreatorDocument object
  */
    void ConstructL();

/*!
  @function CCreatorDocument
  
  @discussion Perform the first phase of two phase construction 
  @param aApp application creating this document
  */
    CCreatorDocument(CEikApplication& aApp);

    };


#endif // __CREATOR_DOCUMENT_H__
