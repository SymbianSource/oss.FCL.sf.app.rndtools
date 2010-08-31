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


#ifndef PERFMON_ENGINE_H
#define PERFMON_ENGINE_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <apgcli.h>
#include <gdi.h>

#include "perfmon.hrh"


// setting keys (do not change uids of existing keys to maintain compatibility to older versions!)
const TUid KPMSettingHeartBeat                              = { 0x00 };
const TUid KPMSettingMaxSamples                             = { 0x01 };
const TUid KPMSettingPriority                               = { 0x02 };
const TUid KPMSettingCPUMode                                = { 0x03 };
const TUid KPMSettingKeepBackLightOn                        = { 0x04 };

const TUid KPMSettingDataPopupVisbility                     = { 0x05 };
const TUid KPMSettingDataPopupLocation                      = { 0x06 };
const TUid KPMSettingDataPopupSources                       = { 0x07 };

const TUid KPMSettingGraphsVerticalBarPeriod                = { 0x08 };
const TUid KPMSettingGraphsSources                          = { 0x09 };

const TUid KPMSettingLoggingMode                            = { 0x0A };
const TUid KPMSettingLoggingFilePath                        = { 0x0B };
const TUid KPMSettingLoggingSources                         = { 0x0C };

// FORWARD DECLARATIONS
class CPerfMonValuesContainer;
class CPerfMonGraphsContainer;
class CPerfMonDataPopupContainer;
class CEikonEnv;
class CCoeControl;
class CDictionaryFileStore;


// CLASS DECLARATIONS


class TPerfMonSources
    {
public:
    TBool iSrcEnabled[ESourcesLength];

public:
    inline void SetDefaults1()
        {
        iSrcEnabled[ESourceCPU] = ETrue;
        iSrcEnabled[ESourceRAM] = ETrue;
        iSrcEnabled[ESourceC]   = ETrue;
        iSrcEnabled[ESourceD]   = EFalse;
        iSrcEnabled[ESourceE]   = EFalse;
        iSrcEnabled[ESourceF]   = EFalse;
        iSrcEnabled[ESourceG]   = EFalse;
        iSrcEnabled[ESourceH]   = EFalse;
        iSrcEnabled[ESourceI]   = EFalse;        
        }
    inline void SetDefaults2()
        {
        iSrcEnabled[ESourceCPU] = ETrue;
        iSrcEnabled[ESourceRAM] = ETrue;
        iSrcEnabled[ESourceC]   = EFalse;
        iSrcEnabled[ESourceD]   = EFalse;
        iSrcEnabled[ESourceE]   = EFalse;
        iSrcEnabled[ESourceF]   = EFalse;
        iSrcEnabled[ESourceG]   = EFalse;
        iSrcEnabled[ESourceH]   = EFalse;
        iSrcEnabled[ESourceI]   = EFalse;        
        }
    TInt EnabledSourcesCount()
        {
        TInt srcCount(0);
        
        for (TInt i=0; i<ESourcesLength; i++)
            {
            if (iSrcEnabled[i])
                srcCount++;
            }
        
        return srcCount;
        }
    };
    

class TPerfMonSettings
    {
public:
    TInt                iHeartBeat;
    TInt                iMaxSamples;
    TInt                iPriority;
    TInt                iCPUMode;
    TBool               iKeepBacklightOn;

    TInt                iDataPopupVisibility;
    TInt                iDataPopupLocation;
    TPerfMonSources     iDataPopupSources;

    TInt                iGraphsVerticalBarPeriod;
    TPerfMonSources     iGraphsSources;

    TInt                iLoggingMode;
    TFileName           iLoggingFilePath;
    TPerfMonSources     iLoggingSources;
    
    TBool               iLoggingEnabled;
    };


class TSampleData
	{
public:
    TInt64                          iFree;
    TInt64                          iSize;
    TTimeIntervalMicroSeconds       iTimeFromStart;
	}; 

typedef CArrayFixSeg<TSampleData> CSampleDataArray;


class TSampleEntry
	{
public:
    TBuf<16>            iDescription;
    TBuf<16>            iUnitTypeShort; //eg. b
    TBuf<16>            iUnitTypeLong;  //eg. bytes
    TInt                iDriveNumber;   //used only for disk drives
    TRgb                iGraphColor;
    CSampleDataArray*   iSampleDataArray;
	}; 
	
typedef CArrayFixSeg<TSampleEntry> CSampleEntryArray;



class CPerfMonEngine : public CActive
	{
public:
	~CPerfMonEngine();
	void ActivateEngineL();
	void DeActivateEngineL();
    void EnableLogging(TBool aEnable);

private:
	void RunL();
	void DoCancel();

protected:
	CPerfMonEngine();
	void ConstructL();
    void LoadSettingsL();
    void SaveSettingsL();
    void OpenLogFile(TBool aOpen);
    void CreateSamplesDataArrayL();
    void UpdateSamplesDataL();
    void AppendLatestSamplesToLogsL();
    void ActivateCPUMonitoringL();
    TBool OpenHandleToNullThread();
    void DeActivateCPUMonitoring();
    TBool CPUTimeSupported();
    TThreadPriority SettingItemToThreadPriority(TInt aIndex);
    void LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TInt& aValue);
    void LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TDes& aValue);
    void LoadDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, TPerfMonSources& aValue);
    void SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TInt& aValue);
    void SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TDes& aValue);
    void SaveDFSValueL(CDictionaryFileStore* aDicFS, const TUid& aUid, const TPerfMonSources& aValue);
    
protected:
    virtual void SendDrawEventToContainersL() = 0;
    virtual void HandleSettingsChangeL();
    
public:
    inline TPerfMonSettings& Settings() { return iSettings; }
    inline CEikonEnv* EikonEnv() { return iEnv; }
    inline RApaLsSession& LsSession() { return iLs; }

    inline CSampleEntryArray* SampleEntryArray() { return iSampleEntryArray; } 

protected:
    RTimer                          iTimer;
    CEikonEnv*                      iEnv;
    TPerfMonSettings                iSettings;
    RApaLsSession                   iLs;
    CSampleEntryArray*              iSampleEntryArray;
    TTime                           iStartTime;

    TInt                            iCurrentCPUMode;

    RThread                         iNullThread;
    RThread                         iCPULoadThread;
	TBool                           iCPULoadCalibrating;
	TInt                            iCPULoadCalibrationCounter;
	TInt64                          iCPULoadMaxValue;
	TInt64                          iCPULoadPreviousValue;    
	TInt64                          iCPULoadCounter;
    TTime                           iPreviousTime;
    
    TBool                           iLogFileInitialized;
    RFile                           iLogFile;
    };
 

#endif
