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

#include "fbpropertiesdialog.h"

#include <HbListWidget>
#include <HbAction>
#include <HbLabel>
#include <HbMarqueeItem>
#include <HbColorScheme>

#include <QSizePolicy>
#include <QGraphicsGridLayout>

/**
  * Constructor
  */
FbPropertiesDialog::FbPropertiesDialog(QGraphicsItem *parent) :
        HbDialog(parent),
        mTitle(0),
        mGridLayout(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setDismissPolicy(HbPopup::TapOutside);
    setTimeout(HbPopup::NoTimeout);

    createHeading();
    createContentWidget();
    createToolBar();
}

/**
  * Destructor
  */
FbPropertiesDialog::~FbPropertiesDialog()
{
}

/**
  * Set \a title into heading widget label
  */
void FbPropertiesDialog::setTitle(const QString &title)
{
    mTitle->setText(title);
}

/**
  * Fill content list with property list \a propertyList
  */
void FbPropertiesDialog::setProperties(const QVector<QPair<QString, QString> > &properties)
{
    QString previousProperty("");
    QPair<QString, QString> propertyPair;
    HbLabel *propertyNameLabel;
    HbLabel *propertyValueLabel;
    for (int i(0), ie(properties.count()); i < ie; ++i)
    {
        propertyPair = properties.at(i);
        propertyNameLabel = new HbLabel();
        if (propertyPair.first != previousProperty) {
            propertyNameLabel->setPlainText(propertyPair.first);
            propertyNameLabel->setElideMode(Qt::ElideNone);
            propertyNameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        }
        mGridLayout->addItem(propertyNameLabel, i, 0);
        previousProperty = propertyPair.first;

        propertyValueLabel = new HbLabel();
        propertyValueLabel->setPlainText(propertyPair.second);
        propertyValueLabel->setTextWrapping(Hb::TextWrapAnywhere);
        mGridLayout->addItem(propertyValueLabel, i, 1);
    }
}

/**
  * Create dialog heading widget
  */
void FbPropertiesDialog::createHeading()
{
    mTitle = new HbMarqueeItem(this);
    mTitle->setLoopCount(1);
    connect(this, SIGNAL(aboutToShow()), mTitle, SLOT(startAnimation()));

    // TODO later check if this is needed, default color is magenta, even if black theme used
    QColor col = HbColorScheme::color("qtc_textedit_normal");
    if (col.isValid()) {
       mTitle->setTextColor(col);
    }

    setHeadingWidget(mTitle);
}

/**
  * Create dialog content widget as a grid
  */
void FbPropertiesDialog::createContentWidget()
{
    mGridLayout = new QGraphicsGridLayout();

    QGraphicsWidget *contentWidget = new QGraphicsWidget;
    contentWidget->setLayout(mGridLayout);
    setContentWidget(contentWidget);
}

/**
  * Create dialog toolbar
  */
void FbPropertiesDialog::createToolBar()
{
    HbAction *rejectAction = new HbAction(QString("Cancel"), this);
    addAction(rejectAction);
}

// End of file
