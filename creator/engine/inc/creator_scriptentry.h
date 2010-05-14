/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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





#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__


#include "engine.h"

#include <e32base.h>
#include <f32file.h>
#include <badesca.h>
#include <aknglobalnote.h>
#include <akniconarray.h> 
#include <aknmemorycardui.mbg>
#include <msvapi.h>


class CCreatorEngine;

class CreatorFileUtils
{
public:
    static TInt FindFiles(CDesCArrayFlat* aFileArray, const TDesC& aFileName, const TDesC& aPath);
    static TInt FindFilesRecursiveL(CDesCArrayFlat* aFileArray, const TDesC& aFileName, const TDesC& aPath);
};

class CCommandParser : public CBase, public MMsvSessionObserver
    {
public:
    static CCommandParser* NewL(CCreatorEngine* aEngine);
    static CCommandParser* NewLC(CCreatorEngine* aEngine);
    ~CCommandParser();
    
    void StrParserL(HBufC*& aDestinationBuf, TUint aToken);
    void StrParserL(HBufC8*& aDestinationBuf, TUint aToken);

private:
    CCommandParser();
    void ConstructL(CCreatorEngine* aEngine);

    TInt FindFilesRecursiveL(const TDesC& aFileName, const TDesC& aPath);
    TInt FindFiles(const TDesC& aFileName, const TDesC& aPath);
    TInt ReadLineFromFileL(RFile& aInputFile);    
    //void ParseCommandFromDescriptorL();

    void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3); // from MMsvSessionObserver

public:
    void OpenScriptL();
    TBool OpenScriptL(RFile& aScriptFile);
    TBool GetRandomDataFilenameL(TDes& aFilename);

private:
    CCreatorEngine* iEngine;
    CDesCArrayFlat* iSearchArray;

    TInt iParserPosition;
    TInt iParserOldPosition;

    HBufC8* iReadBuf;

    };



#endif // __FILEUTILS_H__

