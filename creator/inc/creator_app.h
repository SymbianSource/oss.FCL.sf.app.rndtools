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




#ifndef __CREATOR_APPLICATION_H__
#define __CREATOR_APPLICATION_H__

#include <aknapp.h>

static const TUid KUidCreatorApp = {0x20011383}; 

/*! 
  @class CCreatorApplication
  
  @discussion An instance of CCreatorApplication is the application part of the AVKON
  application framework for the Creator example application
  */
class CCreatorApplication : public CAknApplication
    {
public:  // from CApaApplication

/*! 
  @function AppDllUid
  
  @discussion Returns the application DLL UID value
  @result the UID of this Application/Dll
  */
    TUid AppDllUid() const;

protected: // from CEikApplication
/*! 
  @function CreateDocumentL
  
  @discussion Create a CApaDocument object and return a pointer to it
  @result a pointer to the created document
  */
    CApaDocument* CreateDocumentL();
    };

#endif // __CREATOR_APPLICATION_H__
