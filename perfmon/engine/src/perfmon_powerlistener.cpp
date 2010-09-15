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
#include "perfmon_powerlistener.h"
#include "../../symbian_version.hrh"

#include <centralrepository.h>
#include <hwrm/hwrmpowerdomaincrkeys.h>


#if (SYMBIAN_VERSION_SUPPORT == SYMBIAN_3)

    CPerfMonPowerListener* CPerfMonPowerListener::NewL()
        {
        CPerfMonPowerListener* self = new (ELeave) CPerfMonPowerListener();
        CleanupStack::PushL(self);
        self->ConstructL();
        CleanupStack::Pop();
        return self;
        }

    // --------------------------------------------------------------------------------------------

    CPerfMonPowerListener::CPerfMonPowerListener() :
        iHWRMPower(0),
        iLastPowerAvg(0),
        iMaxPower(0),
        iOriginalMaxReportingPeriod(0)
        {
        }

    // --------------------------------------------------------------------------------------------

    void CPerfMonPowerListener::ConstructL()
        {
        }

    // --------------------------------------------------------------------------------------------

    CPerfMonPowerListener::~CPerfMonPowerListener()
        {
        DeActivate();
        
        if (iHWRMPower != 0)
            {
            delete iHWRMPower;
            iHWRMPower = 0;
            }
        }

    // --------------------------------------------------------------------------------------------

    TBool CPerfMonPowerListener::IsSupported()
        {
        return ETrue;
        }

    // --------------------------------------------------------------------------------------------

    TInt CPerfMonPowerListener::Activate()
        {
        if (iHWRMPower == 0)
            {
            TRAPD(err, iHWRMPower = CHWRMPower::NewL());
            if (err != KErrNone)
                {
                return err;
                }

            // Callbacks to this object
            err = iHWRMPower->SetPowerReportObserver(this);
            if (err != KErrNone)
                {
                return err;
                }
            
            TRAP_IGNORE(GetReportingPeriodL());
            }

        // Set infinite reporting period
        TRAPD(err, SetReportingPeriodL(KReportingDuration));
        if (err != KErrNone)
            {
            return err;
            }

        TRequestStatus status(KRequestPending);

        // Start the power consumption notification
        iHWRMPower->StartAveragePowerReporting(status, KSampleIntervalMultiple);
        User::WaitForRequest(status);

        // Check if monitoring was succesfully started or already ongoing.
        if ((status.Int() != KErrNone) && (status.Int() != KErrAlreadyExists))
            {
            return status.Int();
            }

        return KErrNone;
        }

    // ---------------------------------------------------------------------------

    void CPerfMonPowerListener::DeActivate()
        {
        if (iHWRMPower != 0)
            {
            TRAP_IGNORE(iHWRMPower->StopAveragePowerReportingL());
            }

        // Restore original value to max sampling duration
        TRAP_IGNORE(SetReportingPeriodL(iOriginalMaxReportingPeriod));

        iPowerBuffer.Reset();
        iLastPowerAvg = 0;
        }

    // ---------------------------------------------------------------------------

    TInt CPerfMonPowerListener::GetPower()
        {
        TInt avgPower = 0;
        TInt newValueCount = iPowerBuffer.Count();

        if (newValueCount > 0)
            {
            // Read all new values from buffer and calculate average from them.
            for (int i = 0; i < newValueCount; i++)
                {
                avgPower += iPowerBuffer[i];
                }
            avgPower = avgPower / newValueCount;

            iPowerBuffer.Reset();
            iLastPowerAvg = avgPower;
            }
        else
            {
            avgPower = iLastPowerAvg;
            }

        return avgPower;
        }

    // ---------------------------------------------------------------------------

    void CPerfMonPowerListener::PowerMeasurement(TInt aErrCode, CHWRMPower::TBatteryPowerMeasurementData& aMeasurement)
        {
        if (aErrCode == KErrNone)
            {
            // Store new value to buffer to wait for reading
            TInt value = aMeasurement.iAverageVoltage * aMeasurement.iAverageCurrent;

            // If charger is connected, reported values may be negative.
            if (value < 0)
                {
                value = 0;
                }

            iPowerBuffer.Append(value);

            if ( value > iMaxPower )
                {
                iMaxPower = value;
                }
            }
        // Ignore any errors
        }

    void CPerfMonPowerListener::GetReportingPeriodL()
        {
        CRepository* cenRep = CRepository::NewL(KCRUidPowerSettings);

        CleanupStack::PushL(cenRep);
        User::LeaveIfError(cenRep->Get(KPowerMaxReportingPeriod, iOriginalMaxReportingPeriod));
        CleanupStack::Pop();

        delete cenRep;
        }

    void CPerfMonPowerListener::SetReportingPeriodL(TInt aDuration)
        {
        CRepository* cenRep = CRepository::NewL(KCRUidPowerSettings);

        CleanupStack::PushL(cenRep);
        User::LeaveIfError(cenRep->Set(KPowerMaxReportingPeriod, aDuration));
        CleanupStack::Pop();
        
        delete cenRep;
        }

// SYMBIAN_VERSION_SUPPORT < SYMBIAN_3
#else

    // Stub implementation for older Symbian versions

    CPerfMonPowerListener* CPerfMonPowerListener::NewL()
        {
        CPerfMonPowerListener* self = new (ELeave) CPerfMonPowerListener();
        CleanupStack::PushL(self);
        self->ConstructL();
        CleanupStack::Pop();
        return self;
        }

    CPerfMonPowerListener::CPerfMonPowerListener() :
        iHWRMPower(0),
        iLastPowerAvg(0),
        iMaxPower(0)
        {
        }

    void CPerfMonPowerListener::ConstructL()
        {
        }

    CPerfMonPowerListener::~CPerfMonPowerListener()
        {
        DeActivate();
        }

    TBool CPerfMonPowerListener::IsSupported()
        {
        return EFalse;
        }

    TInt CPerfMonPowerListener::Activate()
        {
        return KErrNotSupported;
        }

    void CPerfMonPowerListener::DeActivate()
        {
        }

    TInt CPerfMonPowerListener::GetPower()
        {
        return 0;
        }

    void CPerfMonPowerListener::PowerMeasurement(TInt aErrCode, CHWRMPower::TBatteryPowerMeasurementData& aMeasurement)
        {
        }

#endif

// End of File
