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

#include "valuedatacontainer.h"
#include "enginewrapper.h"

#include <QGraphicsLinearLayout>
#include <QPalette>
#include <QPainter>
#include <HbColorScheme>

const int leftMargin = 2;

ValueDataContainer::ValueDataContainer(const EngineWrapper& engine, QGraphicsItem *parent) :
    DataContainer(engine, parent)
{
    HbFontSpec spec(HbFontSpec::Secondary);
    mFont = spec.font();
}

void ValueDataContainer::paint (QPainter *painter,
    const QStyleOptionGraphicsItem *option,
    QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // set proper font and prepare font metrics for text height calculation
    painter->setFont(mFont);

    QColor col = HbColorScheme::color("qtc_textedit_normal");
    if(col.isValid())
        painter->setPen(col);
    
    QFontMetricsF metrics(mFont);

    QList<SampleEntry> entries = engine().sampleEntries();
    int c = 1;

    for (int i=0; i<entries.length(); i++)
    {
        const SampleEntry &entry = entries.at(i);

        if (entry.sampleCount() == 0)
            continue;

        const SampleData &sample = entry.sample(0);

        if (i == 0)
        {
            // CPU
            double perc = sample.mSize > 0 ?
                          100. - 100. * sample.mFree / sample.mSize : 0;


            QString text = tr("%1 %2%").arg(entry.description()).
                           arg(perc, 0, 'f', 0);
            painter->drawText(QPointF(leftMargin, c * metrics.height()), text);
            c++;
        }
        else if (sample.mSize > 0)
        {
            // RAM and Drives
            QString text = tr("%1 free %L2%3").arg(entry.description()).
                           arg(sample.mFree).arg(entry.unitShort());
            painter->drawText(QPointF(leftMargin, c * metrics.height()), text);
            c++;

            text = tr("%1 size %L2%3").arg(entry.description()).
                   arg(sample.mSize).arg(entry.unitShort());
            painter->drawText(QPointF(leftMargin, c * metrics.height()), text);
            c++;
        }
    }
}
