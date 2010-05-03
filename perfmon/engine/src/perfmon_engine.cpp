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
// INCLUDE FILES
#include "perfmon_engine.h"
#include <coeutils.h>
#include <bautils.h>
#include <eikenv.h>
#include <e32hal.h>
#include <u32std.h>
#include <s32file.h>
#include <akntitle.h> 
#include <eikspane.h>
#include <aknnotewrappers.h>  
_LIT(KDefaultLogFilePath, "c:\\data\\PerfMon.log");

const TInt KCalibrationLength = 2;
const TInt KMinimumSamplesLength = 16;
const TInt KCPUTimeMultiplier = 1000000; // used to avoid TReal conversions

const TInt KSettingsDrive = EDriveC;
_LIT(KSettingsFileName, "perfmon_settings.ini");

// --------------------------------------------------------------------------------------------

TInt CPULoadCount(TAny* aInt)
    {
    // increase the counter
    (*(TUint*) aInt)++;
    return 1;
    }

TInt CPULoadNOPThread(TAny* aParam)
    {
    CTrapCleanup* pC = CTrapCleanup::New();
    CActiveScheduler* pS = new CActiveScheduler;
    CActiveScheduler::Install(pS);

    CIdle* idle = CIdle::NewL(CActive::EPriorityStandard);
    TCallBack cb(CPULoadCount, aParam);
    idle->Start(cb);

    pS->Start();

    delete idle;
    delete pS;
    delete pC;

    return 0;
    }

// ===================================== MEMBER FUNCTIONS =====================================

CPerfMonEngine::CPerfMonEngine() :
    CActive(EPriorityUserInput)
    {
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::ConstructL()
    {
    iCurrentCPUMode = ECPUModeNotSet;
    iLogFileInitialized = EFalse;

    iEnv = CEikonEnv::Static();
    User::LeaveIfError(iLs.Connect());

    User::LeaveIfError(iTimer.CreateLocal());
    CActiveScheduler::Add(this);
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::ActivateEngineL()
    {
    // load settings
    TRAP_IGNORE(LoadSettingsL());

    // create data storages for the samples
    CreateSamplesDataArrayL();

    // set default modes
    HandleSettingsChangeL();

    // start sampling data immediately (jump to RunL)    
    iTimer.After(iStatus, 100);
    SetActive();
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::DeActivateEngineL()
    {
    Cancel();

    DeActivateCPUMonitoring();

    // close log file
    OpenLogFile(EFalse);
    }

// --------------------------------------------------------------------------------------------

CPerfMonEngine::~CPerfMonEngine()
    {
    iTimer.Close();

    // clear data storages
    if (iSampleEntryArray)
        {
        for (TInt i = 0; i < iSampleEntryArray->Count(); i++)
            {
            delete iSampleEntryArray->At(i).iSampleDataArray;
            }

        delete iSampleEntryArray;
        }

    iLs.Close();
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::DoCancel()
    {
    iTimer.Cancel();
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::RunL()
    {
    // calculate new values 
    UpdateSamplesDataL();

    // log changes
    AppendLatestSamplesToLogsL();

    // redraw views
    SendDrawEventToContainersL();

    // continue
    iTimer.After(iStatus, iSettings.iHeartBeat * 1000); // convert from milliseconds to microseconds
    SetActive();
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::HandleSettingsChangeL()
    {
    // set priority of the thread
    RThread().SetPriority(SettingItemToThreadPriority(iSettings.iPriority));

    // init cpu monitor if setting has been changed
    if (iCurrentCPUMode != iSettings.iCPUMode)
        {
        DeActivateCPUMonitoring();
        ActivateCPUMonitoringL();
        }

    // close log file
    OpenLogFile(EFalse);

    // enable log file
    if (iSettings.iLoggingEnabled && (iSettings.iLoggingMode
            == ELoggingModeLogFile || iSettings.iLoggingMode
            == ELoggingModeRDebugLogFile))
        OpenLogFile(ETrue);
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::EnableLogging(TBool aEnable)
    {
    if (aEnable)
        {
        if (iSettings.iLoggingMode == ELoggingModeLogFile
                || iSettings.iLoggingMode == ELoggingModeRDebugLogFile)
            OpenLogFile(ETrue);

        iSettings.iLoggingEnabled = ETrue;
        }
    else // disable
        {
        iSettings.iLoggingEnabled = EFalse;
        OpenLogFile(EFalse);
        }
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::OpenLogFile(TBool aOpen)
    {
    // open log file for writing
    if (aOpen)
        {
        if (!iLogFileInitialized)
            {
            TInt err(KErrNone);

            // open the log file for writing
            if (iLogFile.Open(iEnv->FsSession(), iSettings.iLoggingFilePath,
                    EFileWrite) != KErrNone)
                {
                iEnv->FsSession().MkDirAll(iSettings.iLoggingFilePath);
                err = iLogFile.Replace(iEnv->FsSession(),
                        iSettings.iLoggingFilePath, EFileWrite);
                }
            else
                {
                // file opens correctly, seek to the end
                TInt fileSize = 0;
                iLogFile.Size(fileSize);
                err = iLogFile.Seek(ESeekCurrent, fileSize);
                }

            if (err == KErrNone)
                {
                iLogFileInitialized = ETrue;
                }
            else
                {
                // show error
                CAknErrorNote* note = new (ELeave) CAknErrorNote();
                note->ExecuteLD(_L("Unable to create log file, check settings"));
                }
            }
        }

    // close handle to log file
    else
        {
        if (iLogFileInitialized)
            {
            iLogFile.Flush();
            iLogFile.Close();

            iLogFileInitialized = EFalse;
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::ActivateCPUMonitoringL()
    {
    // reset counter variables
    iCPULoadCalibrating = ETrue;
    iCPULoadCalibrationCounter = 0;
    iCPULoadMaxValue = 999999999;
    iCPULoadPreviousValue = 1;
    iCPULoadCounter = 0;

    // use null thread is cpu time is supported and the setting is on
    if (CPUTimeSupported() && iSettings.iCPUMode == ECPUModeCPUTime)
        {
        // try to open handle to null thread
        if (OpenHandleToNullThread())
            {
            // handle is open, get initial value
            TTimeIntervalMicroSeconds time;
            iNullThread.GetCpuTime(time);
            iCPULoadPreviousValue = time.Int64();
            iPreviousTime.HomeTime();

            iCurrentCPUMode = ECPUModeCPUTime;
            return; // cpu time is succesfully in use           
            }
        }

    // otherwise use normal sampling with nops    
    iCurrentCPUMode = ECPUModeNotSet;

    // show a warning if cpu time cannot be taken in use
    if (iSettings.iCPUMode == ECPUModeCPUTime)
        {
        CAknInformationNote* note = new (ELeave) CAknInformationNote();
        note->ExecuteLD(
                _L("CPU Time not supported in this system, using NOPs sampling"));
        }

    // create a thread for CPU load monitoring
    User::LeaveIfError(iCPULoadThread.Create(_L("PerfMonCPULoad"),
            CPULoadNOPThread, 0x1000, 0x1000, 0x100000, &iCPULoadCounter));
    iCPULoadThread.SetPriority(EPriorityLess);
    iCPULoadThread.Resume();

    iCurrentCPUMode = ECPUModeNOPs; // NOPs taken succesfully in use
    }

// --------------------------------------------------------------------------------------------

TBool CPerfMonEngine::OpenHandleToNullThread()
    {
    // find the kernel process and then the null thread
    TFindProcess fp(_L("ekern.exe*"));

    TFullName kernelName;
    if (fp.Next(kernelName) == KErrNone)
        {
        // process found, append null thread identifier
        kernelName.Append(_L("::Null"));

        // find the thread
        TFindThread ft(kernelName);

        TFullName threadName;
        if (ft.Next(threadName) == KErrNone)
            {
            // open instance to the thread
            if (iNullThread.Open(threadName) != KErrNone)
                return EFalse;
            }
        }

    // process not found
    else
        return EFalse;

    // success!
    return ETrue;
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::DeActivateCPUMonitoring()
    {
    if (iCurrentCPUMode == ECPUModeCPUTime)
        {
        iNullThread.Close();
        }

    else if (iCurrentCPUMode == ECPUModeNOPs)
        {
        // kill the cpu load thread
        iCPULoadThread.Kill(0);
        iCPULoadThread.Close();
        }
    }

// --------------------------------------------------------------------------------------------

TBool CPerfMonEngine::CPUTimeSupported()
    {
    TTimeIntervalMicroSeconds time;
    TInt err = RThread().GetCpuTime(time);

    if (err == KErrNone && time.Int64() > 0)
        return ETrue;
    else
        return EFalse;
    }

// --------------------------------------------------------------------------------------------

TThreadPriority CPerfMonEngine::SettingItemToThreadPriority(TInt aIndex)
    {
    TThreadPriority threadPriority = EPriorityNull;

    switch (aIndex)
        {
        case EThreadPriorityTypeMuchLess:
            {
            threadPriority = EPriorityMuchLess;
            break;
            }
        case EThreadPriorityTypeLess:
            {
            threadPriority = EPriorityLess;
            break;
            }
        case EThreadPriorityTypeNormal:
            {
            threadPriority = EPriorityNormal;
            break;
            }
        case EThreadPriorityTypeMore:
            {
            threadPriority = EPriorityMore;
            break;
            }
        case EThreadPriorityTypeMuchMore:
            {
            threadPriority = EPriorityMuchMore;
            break;
            }
        case EThreadPriorityTypeRealTime:
            {
            threadPriority = EPriorityRealTime;
            break;
            }
        case EThreadPriorityTypeAbsoluteVeryLow:
            {
            threadPriority = EPriorityAbsoluteVeryLow;
            break;
            }
        case EThreadPriorityTypeAbsoluteLow:
            {
            threadPriority = EPriorityAbsoluteLow;
            break;
            }
        case EThreadPriorityTypeAbsoluteBackground:
            {
            threadPriority = EPriorityAbsoluteBackground;
            break;
            }
        case EThreadPriorityTypeAbsoluteForeground:
            {
            threadPriority = EPriorityAbsoluteForeground;
            break;
            }
        case EThreadPriorityTypeAbsoluteHigh:
            {
            threadPriority = EPriorityAbsoluteHigh;
            break;
            }

        default:
            {
            User::Panic(_L("Wrong tp index"), 276);
            break;
            }
        }

    return threadPriority;
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::CreateSamplesDataArrayL()
    {
    TInt
            maxSamples =
                    iSettings.iMaxSamples >= KMinimumSamplesLength ? iSettings.iMaxSamples
                            : KMinimumSamplesLength;

    // create the data structure to store all samples
    iSampleEntryArray = new (ELeave) CSampleEntryArray(16);

    // add all source entries
    for (TInt i = 0; i < ESourcesLength; i++)
        {
        TSampleEntry newSampleEntry;

        if (i == ESourceCPU)
            {
            newSampleEntry.iDescription.Copy(_L("CPU"));
            newSampleEntry.iUnitTypeShort.Copy(KNullDesC);
            newSampleEntry.iUnitTypeLong.Copy(KNullDesC);
            newSampleEntry.iDriveNumber = -1;
            newSampleEntry.iGraphColor = KRgbYellow;
            }

        else if (i == ESourceRAM)
            {
            newSampleEntry.iDescription.Copy(_L("RAM"));
            newSampleEntry.iUnitTypeShort.Copy(_L("b"));
            newSampleEntry.iUnitTypeLong.Copy(_L("bytes"));
            newSampleEntry.iDriveNumber = -1;
            newSampleEntry.iGraphColor = KRgbGreen;
            }

        else //drives
            {
            TChar driveLetter = 'C' + i - ESourceC; // C is the first drive

            newSampleEntry.iDescription.Append(driveLetter);
            newSampleEntry.iDescription.Append(_L(":"));
            newSampleEntry.iUnitTypeShort.Copy(_L("b"));
            newSampleEntry.iUnitTypeLong.Copy(_L("bytes"));

            iEnv->FsSession().CharToDrive(driveLetter,
                    newSampleEntry.iDriveNumber);

            newSampleEntry.iGraphColor = KRgbCyan;
            newSampleEntry.iGraphColor.SetGreen(255 - (i - ESourceC) * 30);
            newSampleEntry.iGraphColor.SetRed(i * 30);
            }

        newSampleEntry.iSampleDataArray = new (ELeave) CSampleDataArray(
                maxSamples);

        iSampleEntryArray->AppendL(newSampleEntry);
        }

    // save current time as start time
    iStartTime.HomeTime();
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::UpdateSamplesDataL()
    {
    // reset inactivity timers
    if (iSettings.iKeepBacklightOn)
        User::ResetInactivityTime();

    // get current time
    TTime currentTime;
    currentTime.HomeTime();

    // calculate time difference
    TTimeIntervalMicroSeconds timeDeltaFromPreviousSample =
            currentTime.MicroSecondsFrom(iPreviousTime);

    // remember current time as previous
    iPreviousTime = currentTime;

    // get CPU
    TInt64 cpuLoadDelta(0);
    TInt64 cpuLoadFree(0);
    TInt64 cpuLoadSize(0);
    TInt64 currentCPUValue(0);

    if (iCurrentCPUMode == ECPUModeCPUTime || iCurrentCPUMode == ECPUModeNOPs)
        {
        if (iCurrentCPUMode == ECPUModeCPUTime)
            {
            TTimeIntervalMicroSeconds time;
            iNullThread.GetCpuTime(time);
            currentCPUValue = time.Int64();
            }
        else if (iCurrentCPUMode == ECPUModeNOPs)
            {
            currentCPUValue = iCPULoadCounter;
            }

        // get delta and store the previous value
        cpuLoadDelta = currentCPUValue - iCPULoadPreviousValue;
        iCPULoadPreviousValue = currentCPUValue;

        // velocity = distance / time
        cpuLoadFree = cpuLoadDelta * KCPUTimeMultiplier
                / timeDeltaFromPreviousSample.Int64();

        // detect maximum value
        if (cpuLoadFree > iCPULoadMaxValue)
            iCPULoadMaxValue = cpuLoadFree;

        // check calibration status      
        if (iCPULoadCalibrating)
            {
            iCPULoadCalibrationCounter++;
            cpuLoadSize = cpuLoadFree;

            // check if need to calibrate anymore
            if (iCPULoadCalibrationCounter > KCalibrationLength)
                {
                iCPULoadCalibrating = EFalse;

                // from the samples, get the minimum value, and let it be the max value 
                for (TInt i = 0; i
                        < iSampleEntryArray->At(0).iSampleDataArray->Count(); i++)
                    {
                    TInt64 newCPULoadMaxValue = iCPULoadMaxValue;

                    if (iSampleEntryArray->At(0).iSampleDataArray->At(i).iFree
                            < newCPULoadMaxValue)
                        {
                        newCPULoadMaxValue
                                = iSampleEntryArray->At(0).iSampleDataArray->At(
                                        i).iFree;
                        }

                    iCPULoadMaxValue = newCPULoadMaxValue;
                    }

                // adjust priority of the poller thread
                if (iCurrentCPUMode == ECPUModeNOPs)
                    {
                    iCPULoadThread.SetPriority(EPriorityAbsoluteVeryLow);
                    }
                }

            }
        else
            {
            cpuLoadSize = iCPULoadMaxValue;
            }
        }

    // save cpu sample data    
    TSampleData cpuSample;
    cpuSample.iFree = cpuLoadFree;
    cpuSample.iSize = cpuLoadSize;
    cpuSample.iTimeFromStart = currentTime.MicroSecondsFrom(iStartTime);

    iSampleEntryArray->At(0).iSampleDataArray->InsertL(0, cpuSample);

    // get ram memory
    TMemoryInfoV1Buf ramMemory;
    UserHal::MemoryInfo(ramMemory);

    TSampleData memorySample;
    memorySample.iFree = ramMemory().iFreeRamInBytes;
    memorySample.iSize = ramMemory().iMaxFreeRamInBytes;
    memorySample.iTimeFromStart = currentTime.MicroSecondsFrom(iStartTime);

    iSampleEntryArray->At(1).iSampleDataArray->InsertL(0, memorySample);

    // all drives
    for (TInt i = 2; i < iSampleEntryArray->Count(); i++)
        {
        TSampleData driveSample;

        // get volume info from RFs
        TVolumeInfo volumeInfo;
        if (iEnv->FsSession().Volume(volumeInfo,
                iSampleEntryArray->At(i).iDriveNumber) == KErrNone)
            {
            driveSample.iFree = volumeInfo.iFree;
            driveSample.iSize = volumeInfo.iSize;
            }
        else
            {
            driveSample.iFree = 0;
            driveSample.iSize = 0;
            }

        driveSample.iTimeFromStart = currentTime.MicroSecondsFrom(iStartTime);

        iSampleEntryArray->At(i).iSampleDataArray->InsertL(0, driveSample);
        }

    // compress sample data arrays to save memory
    TInt curLength(iSampleEntryArray->At(0).iSampleDataArray->Count());

    TInt
            maxSamples =
                    iSettings.iMaxSamples >= KMinimumSamplesLength ? iSettings.iMaxSamples
                            : KMinimumSamplesLength;

    if (curLength > maxSamples && curLength % 5 == 0)
        {
        for (TInt i = 0; i < iSampleEntryArray->Count(); i++)
            {
            iSampleEntryArray->At(i).iSampleDataArray->ResizeL(maxSamples); // looses old samples
            iSampleEntryArray->At(i).iSampleDataArray->Compress();
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::AppendLatestSamplesToLogsL()
    {
    if (iSettings.iLoggingEnabled && SampleEntryArray())
        {
        // loop all sources
        for (TInt i = 0; i < SampleEntryArray()->Count(); i++)
            {
            // check if this setting has been enabled and it has some data
            if (iSettings.iLoggingSources.iSrcEnabled[i]
                    && SampleEntryArray()->At(i).iSampleDataArray->Count() > 0)
                {
                // get current sample
                TSampleData& currentSample =
                        SampleEntryArray()->At(i).iSampleDataArray->At(0);

                TBuf<128> buf;
                buf.Append(_L("PERFMON;"));
                buf.Append(SampleEntryArray()->At(i).iDescription);
                buf.Append(_L(";"));
                buf.AppendNum(currentSample.iTimeFromStart.Int64());
                buf.Append(_L(";"));
                buf.AppendNum(currentSample.iFree);
                buf.Append(_L(";"));
                buf.AppendNum(currentSample.iSize);

                // print to RDebug
                if (iSettings.iLoggingMode == ELoggingModeRDebug
                        || iSettings.iLoggingMode == ELoggingModeRDebugLogFile)
                    {
                    RDebug::Print(buf);
                    }

                // print to log file
                if (iSettings.iLoggingMode == ELoggingModeLogFile
                        || iSettings.iLoggingMode == ELoggingModeRDebugLogFile)
                    {
                    buf.Append(_L("\r\n"));

                    TBuf8<128> buf8;
                    buf8.Copy(buf);

                    iLogFile.Write(buf8);
                    }
                }
            }
        }
    }

void CPerfMonEngine::LoadSettingsL()
    {
    // set defaults
    iSettings.iHeartBeat = 600;
    iSettings.iMaxSamples = 64;
    iSettings.iPriority = EThreadPriorityTypeNormal;
    iSettings.iCPUMode = ECPUModeCPUTime;
    iSettings.iKeepBacklightOn = ETrue;

    iSettings.iDataPopupVisibility = EDataPopupVisbilityAlwaysOn;
    // TODO: remove the next line once data popup is working
    iSettings.iDataPopupVisibility = EDataPopupVisbilityAlwaysAlwaysOff;
    iSettings.iDataPopupLocation = EDataPopupLocationTopRight;
    iSettings.iDataPopupSources.SetDefaults1();

    iSettings.iGraphsVerticalBarPeriod = 5;
    iSettings.iGraphsSources.SetDefaults2();

    iSettings.iLoggingMode = ELoggingModeRDebug;
    iSettings.iLoggingFilePath.Copy(KDefaultLogFilePath);
    iSettings.iLoggingSources.SetDefaults2();

    iSettings.iLoggingEnabled = EFalse;

    // make sure that the private path of this app in c-drive exists
    iEnv->FsSession().CreatePrivatePath(KSettingsDrive); // c:\\private\\20011385\\
    
    // handle settings always in the private directory 
    if (iEnv->FsSession().SetSessionToPrivate(KSettingsDrive) == KErrNone)
        {
        const TUid KUidPerfMon =
            {
            0x20011385
            };
        // open or create a dictionary file store
        CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(
                iEnv->FsSession(), KSettingsFileName, KUidPerfMon);

        LoadDFSValueL(settingsStore, KPMSettingHeartBeat, iSettings.iHeartBeat);
        LoadDFSValueL(settingsStore, KPMSettingMaxSamples,
                iSettings.iMaxSamples);
        LoadDFSValueL(settingsStore, KPMSettingPriority, iSettings.iPriority);
        
        // TODO: enable next line when NOPs will be working
        //LoadDFSValueL(settingsStore, KPMSettingCPUMode, iSettings.iCPUMode);
        
        LoadDFSValueL(settingsStore, KPMSettingKeepBackLightOn,
                iSettings.iKeepBacklightOn);

        LoadDFSValueL(settingsStore, KPMSettingDataPopupVisbility,
                iSettings.iDataPopupVisibility);
        LoadDFSValueL(settingsStore, KPMSettingDataPopupLocation,
                iSettings.iDataPopupLocation);
        LoadDFSValueL(settingsStore, KPMSettingDataPopupSources,
                iSettings.iDataPopupSources);

        LoadDFSValueL(settingsStore, KPMSettingGraphsVerticalBarPeriod,
                iSettings.iGraphsVerticalBarPeriod);
        LoadDFSValueL(settingsStore, KPMSettingGraphsSources,
                iSettings.iGraphsSources);

        LoadDFSValueL(settingsStore, KPMSettingLoggingMode,
                iSettings.iLoggingMode);
        LoadDFSValueL(settingsStore, KPMSettingLoggingFilePath,
                iSettings.iLoggingFilePath);
        LoadDFSValueL(settingsStore, KPMSettingLoggingSources,
                iSettings.iLoggingSources);

        CleanupStack::PopAndDestroy(); // settingsStore         
        }
    }

// --------------------------------------------------------------------------------------------

void CPerfMonEngine::SaveSettingsL()
    {
    // handle settings always in c:\\private\\20011385\\
    if (iEnv->FsSession().SetSessionToPrivate( KSettingsDrive ) == KErrNone)
        {
        // delete existing store to make sure that it is clean and not eg corrupted
        if (BaflUtils::FileExists(iEnv->FsSession(), KSettingsFileName))
            {
            iEnv->FsSession().Delete(KSettingsFileName);
            }

        const TUid KUidPerfMon =
            {
            0x20011385
            };
        // create a dictionary file store
        CDictionaryFileStore* settingsStore = CDictionaryFileStore::OpenLC(
                iEnv->FsSession(), KSettingsFileName, KUidPerfMon);

        SaveDFSValueL(settingsStore, KPMSettingHeartBeat, iSettings.iHeartBeat);
        SaveDFSValueL(settingsStore, KPMSettingMaxSamples,
                iSettings.iMaxSamples);
        SaveDFSValueL(settingsStore, KPMSettingPriority, iSettings.iPriority);
        SaveDFSValueL(settingsStore, KPMSettingCPUMode, iSettings.iCPUMode);
        SaveDFSValueL(settingsStore, KPMSettingKeepBackLightOn,
                iSettings.iKeepBacklightOn);

        SaveDFSValueL(settingsStore, KPMSettingDataPopupVisbility,
                iSettings.iDataPopupVisibility);
        SaveDFSValueL(settingsStore, KPMSettingDataPopupLocation,
                iSettings.iDataPopupLocation);
        SaveDFSValueL(settingsStore, KPMSettingDataPopupSources,
                iSettings.iDataPopupSources);

        SaveDFSValueL(settingsStore, KPMSettingGraphsVerticalBarPeriod,
                iSettings.iGraphsVerticalBarPeriod);
        SaveDFSValueL(settingsStore, KPMSettingGraphsSources,
                iSettings.iGraphsSources);

        SaveDFSValueL(settingsStore, KPMSettingLoggingMode,
                iSettings.iLoggingMode);
        SaveDFSValueL(settingsStore, KPMSettingLoggingFilePath,
                iSettings.iLoggingFilePath);
        SaveDFSValueL(settingsStore, KPMSettingLoggingSources,
                iSettings.iLoggingSources);

        settingsStore->CommitL();
        CleanupStack::PopAndDestroy(); // settingsStore             
        }
    }

// ---------------------------------------------------------------------------

void CPerfMonEngine::LoadDFSValueL(CDictionaryFileStore* aDicFS,
        const TUid& aUid, TInt& aValue)
    {
    if (aDicFS->IsPresentL(aUid))
        {
        RDictionaryReadStream in;
        in.OpenLC(*aDicFS, aUid);
        aValue = in.ReadInt16L();
        CleanupStack::PopAndDestroy(); // in        
        }
    }

// ---------------------------------------------------------------------------

void CPerfMonEngine::LoadDFSValueL(CDictionaryFileStore* aDicFS,
        const TUid& aUid, TDes& aValue)
    {
    if (aDicFS->IsPresentL(aUid))
        {
        RDictionaryReadStream in;
        in.OpenLC(*aDicFS, aUid);
        TInt bufLength = in.ReadInt16L(); // get length of descriptor
        in.ReadL(aValue, bufLength); // get the descriptor itself
        CleanupStack::PopAndDestroy(); // in
        }
    }

// ---------------------------------------------------------------------------

void CPerfMonEngine::LoadDFSValueL(CDictionaryFileStore* aDicFS,
        const TUid& aUid, TPerfMonSources& aValue)
    {
    if (aDicFS->IsPresentL(aUid))
        {
        RDictionaryReadStream in;
        in.OpenLC(*aDicFS, aUid);
        TInt bufLength = in.ReadInt16L(); // get length of the array

        if (bufLength < 0 || bufLength > ESourcesLength) // check for validaty
            User::Leave(KErrNotSupported);

        for (TInt i = 0; i < bufLength; i++) // get all items
            aValue.iSrcEnabled[i] = in.ReadInt16L();

        CleanupStack::PopAndDestroy(); // in
        }
    }

// ---------------------------------------------------------------------------

void CPerfMonEngine::SaveDFSValueL(CDictionaryFileStore* aDicFS,
        const TUid& aUid, const TInt& aValue)
    {
    RDictionaryWriteStream out;
    out.AssignLC(*aDicFS, aUid);
    out.WriteInt16L(aValue);
    out.CommitL();
    CleanupStack::PopAndDestroy(); // out
    }

// ---------------------------------------------------------------------------

void CPerfMonEngine::SaveDFSValueL(CDictionaryFileStore* aDicFS,
        const TUid& aUid, const TDes& aValue)
    {
    RDictionaryWriteStream out;
    out.AssignLC(*aDicFS, aUid);
    out.WriteInt16L(aValue.Length()); // write length of the descriptor
    out.WriteL(aValue, aValue.Length()); // write the descriptor itself
    out.CommitL();
    CleanupStack::PopAndDestroy(); // out
    }

// ---------------------------------------------------------------------------

void CPerfMonEngine::SaveDFSValueL(CDictionaryFileStore* aDicFS,
        const TUid& aUid, const TPerfMonSources& aValue)
    {
    RDictionaryWriteStream out;
    out.AssignLC(*aDicFS, aUid);

    out.WriteInt16L(ESourcesLength); // write length of the array

    for (TInt i = 0; i < ESourcesLength; i++) // write all items
        out.WriteInt16L(aValue.iSrcEnabled[i]);

    out.CommitL();
    CleanupStack::PopAndDestroy(); // out
    }

// ---------------------------------------------------------------------------

// End of File
