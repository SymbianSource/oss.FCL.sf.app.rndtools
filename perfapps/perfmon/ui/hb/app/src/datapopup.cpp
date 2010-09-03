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

#include <HbView>
#include <HbEvent>
#include <QFontMetrics>

#include "datapopup.h"
#include "enginewrapper.h"
#include "popupdatacontainer.h"

DataPopup::DataPopup(EngineWrapper &engine) :
        mEngine(engine),
        mPopupCreated(false),
        mPopupVisible(false)
{
    connect(&mEngine, SIGNAL(samplesUpdated()), this, SLOT(updateSamples()));
    connect(&mEngine, SIGNAL(settingsUpdated()), this, SLOT(updateSettings()));
    connect(this, SIGNAL(dataReceived(QVariantMap)), this, SLOT(triggerAction(QVariantMap)));
}

void DataPopup::show()
{
    mPopupVisible = true;
    if (!mPopupCreated &&
        mEngine.sampleEntries().length() &&
        mEngine.settings().dataPopupSources().enabledIndexes().length())
    {
        bool anyData = false;
        foreach (QVariant index, mEngine.settings().dataPopupSources().enabledIndexes())
            anyData = anyData || mEngine.sampleEntries().at(index.toInt()).sampleCount();

        if (anyData) {
            mPopupCreated = HbDeviceDialog::show("com.nokia.rnd.perfmondatapopup/1.0",
                                                 collectParams());
        }
    }
}

void DataPopup::hide()
{
    mPopupVisible = false;
    if (mPopupCreated) {
        mPopupCreated = !cancel();
    }
}

void DataPopup::updateSamples()
{
    updateData();
}

void DataPopup::updateSettings()
{
    // mEngine.settings().dataPopupVisibility can only be changed from
    // main window, so we may assume the window is in foreground
    updateVisibility(true);
    updateData();
}

void DataPopup::updateVisibility(bool foreground)
{
    if (mEngine.settings().dataPopupVisibility() == EDataPopupVisbilityAlwaysOn ||
        (mEngine.settings().dataPopupVisibility() == EDataPopupVisbilityBackgroundOnly && !foreground)) {

        show();
    } else {
        hide();
    }
}

void DataPopup::triggerAction(QVariantMap data)
{
    if (data.contains("mouseEvent") && data["mouseEvent"].toString() == "press") {
        emit clicked();

        // data popup was clicked, move it to other position
        mEngine.settings().setDataPopupLocation(
                EDataPopupLocationBottomMiddle - mEngine.settings().dataPopupLocation());
        mEngine.updateSettings();
    }
}

QVariantMap DataPopup::collectParams() const
{
    QVariantMap result;

    // add location param
    result["location"] = mEngine.settings().dataPopupLocation();

    // add lines param
    QStringList lines;
    QList<SampleEntry> entries = mEngine.sampleEntries();
    int posCounter(ESourceCPU);

    if (mEngine.settings().dataPopupSources().isEnabled(posCounter))
    {
        // loop all CPUs
        for (TInt i = mEngine.CPU0PositionInSamples(); i < mEngine.CPU0PositionInSamples() + mEngine.AmountOfCPUs(); i++)
        {
            // check samples available
            if (entries.at(i).sampleCount() > 0)
            {
                const SampleEntry &entry = entries.at(i);
                const SampleData &sample = entry.sample(0);

                double perc = sample.mSize > 0 ?
                              100. - 100. * sample.mFree / sample.mSize : 0;

                QString text = tr("%1 %2%").arg(entry.description()).
                               arg(perc, 0, 'f', 0);
                lines.append(text);
            }
        }
    }

    posCounter++;

    // draw RAM and Drive values
    for (TInt i = mEngine.RAMPositionInSamples(); i < mEngine.PowerPositionInSamples(); i++)
    {
        // check if this setting has been enabled and it has some data
        if (mEngine.settings().dataPopupSources().isEnabled(posCounter) && entries.at(i).sampleCount() > 0)
        {
            const SampleEntry &entry = entries.at(i);
            const SampleData &sample = entry.sample(0);

            QString text = tr("%1 free %L2%3").arg(entry.description()).
                           arg(sample.mFree).arg(entry.unitShort());
            lines.append(text);
        }

        posCounter++;
    }

    // draw power value
    // check if this setting has been enabled and it has some data
    if (mEngine.settings().dataPopupSources().isEnabled(posCounter) && entries.at(mEngine.PowerPositionInSamples()).sampleCount() > 0)
    {
        const SampleEntry &entry = entries.at(mEngine.PowerPositionInSamples());
        const SampleData &sample = entry.sample(0);

        QString text = tr("%1 %L2%3").arg(entry.description()).
                       arg(sample.mSize - sample.mFree).arg(entry.unitShort());
        lines.append(text);
    }

    result["lines"] = lines;

    return result;
}

void DataPopup::updateData()
{
    if (!mPopupVisible)
        return;

    if (!mPopupCreated) {
        show();
    } else {
        HbDeviceDialog::update(collectParams());
    }
}
