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





#ifndef __CREATORNOTEPAD_H__
#define __CREATORNOTEPAD_H__

#include "engine.h"
#include "creator_modulebase.h"


#include <e32base.h>
//#include <npdapi.h>
#include <NotesEditor>
#include <AgendaUtil>
#include <AgendaEntry>


static const TInt KNotepadFieldLength = 1024;

class CCreatorEngine;
class CNotepadParameters;


class CCreatorNotepad : public CBase, public MCreatorModuleBase
    {
public: 
    static CCreatorNotepad* NewL(CCreatorEngine* aEngine);
    static CCreatorNotepad* NewLC(CCreatorEngine* aEngine);
    ~CCreatorNotepad();

private:
    CCreatorNotepad();
    void ConstructL(CCreatorEngine* aEngine); // from MCreatorModuleBase

public:
    TBool AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries); // from MCreatorModuleBase
    TInt CreateNoteEntryL(CNotepadParameters *aParameters);
    void DeleteAllL();
    void DeleteAllCreatedByCreatorL();

private:
    AgendaUtil *iAgendaUtil;
    NotesEditor *iNotepadApi;	//QT Notes api 
    //CNotepadApi *iNotepadApi;

    CNotepadParameters* iParameters;
    RFs& iFs;

public:
    };


class CNotepadParameters : public CCreatorModuleBaseParameters
    {
public: 
    HBufC*              iNoteText;

public:
    CNotepadParameters();
    ~CNotepadParameters();
    };



#endif // __CREATORNOTEPAD_H__
