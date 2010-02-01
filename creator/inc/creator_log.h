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




#ifndef __CREATORLOGS_H__
#define __CREATORLOGS_H__

#include "creator_model.h"
#include "creator_modulebase.h"


#include <e32base.h>
#include <logwrap.h>
#include <logcli.h>
#include <logeng.h>



class CCreatorEngine;
class CLogsParameters;

// Creator log ids:
typedef TInt32  TLogIdCreator;
typedef TInt8   TLogDurationTypeCreator;
typedef TUint32 TLogDurationCreator;
typedef TUint32 TLogLinkCreator;

const TLogIdCreator KLogNullIdCreator = -1;
const TLogLinkCreator KLogNullLinkCreator = 0;
const TLogDurationTypeCreator KLogNullDurationTypeCreator = -1;
const TLogDurationCreator KLogNullDurationCreator = 0;
const TLogDurationTypeCreator KLogDurationNoneCreator = 0;
const TLogDurationTypeCreator KLogDurationValidCreator = 1;
const TLogDurationTypeCreator KLogDurationDataCreator = 2;




class CCreatorLogs : public CActive, public MCreatorModuleBase
    {
public: 
    static CCreatorLogs* NewL(CCreatorEngine* aEngine);
    static CCreatorLogs* NewLC(CCreatorEngine* aEngine);
    ~CCreatorLogs();

private:
    CCreatorLogs();
    void ConstructL(CCreatorEngine* aEngine); // from MCreatorModuleBase

    void RunL();  // from CActive
	void DoCancel();  // from CActive

public:
    TBool AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries); // from MCreatorModuleBase

    TInt CreateMissedCallEntryL(CLogsParameters *aParameters);    
    TInt CreateReceivedCallEntryL(CLogsParameters *aParameters);
    TInt CreateDialledNumberEntryL(CLogsParameters *aParameters);
    void DeleteAllL();
    void DeleteAllCreatedByCreatorL();

private:
    CLogClient* iLogClient;
    CLogEvent* iLogEvent;
    CActiveSchedulerWait iLogWriteWait;

    CLogsParameters* iParameters;
    RArray<TInt> iEntryIds; // TLogId ( == TInt32) CLogEvent::Id()

public:
    };


class CLogsParameters : public CCreatorModuleBaseParameters
    {
public: 
    /*HBufC*              iMissedCallPhoneNumber;
    TTime               iMissedCallEventTime;
	
    HBufC*              iReceivedCallPhoneNumber;
    TTime               iReceivedCallEventTime;
    TLogDuration        iReceivedCallDuration;

    HBufC*              iDialledNumberPhoneNumber;
    TTime               iDialledNumberEventTime;
    TLogDuration        iDialledNumberDuration;*/
    HBufC*              iPhoneNumber;
    TTime               iEventTime;
    TLogDuration        iDuration;

public:
    CLogsParameters();
    ~CLogsParameters();
    };



#endif // __CREATORLOGS_H__
