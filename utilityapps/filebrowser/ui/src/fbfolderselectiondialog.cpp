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

#include "fbfolderselectiondialog.h"
#include "fbfolderselectorwrapper.h"

#include <HbListWidget>
#include <HbAction>
#include <HbListWidgetItem>
#include <HbLabel>
#include <HbPushButton>

#include <QGraphicsLinearLayout>
#include <QFileInfo>
#include <QFileIconProvider>

/**
  * Constructor
  */
FbFolderSelectionDialog::FbFolderSelectionDialog(QGraphicsItem *parent) :
        HbDialog(parent),
        mTitle(0),
        mCurrentPath(0),
        mFolderList(0),
        mFolderSelectorWrapper(0),
        mFileIconProvider(0)
{
    init();
}

/**
  * Destructor
  */
FbFolderSelectionDialog::~FbFolderSelectionDialog()
{
    if (mFileIconProvider)
        delete mFileIconProvider;
}

/**
  * Provide currenntly selected folder
  * \return current folder
  */
QString FbFolderSelectionDialog::selectedFolder()
{
    return mFolderSelectorWrapper->currentPath();
}

/**
  * Initialize folder selection dialog
  */
void FbFolderSelectionDialog::init()
{
    setAttribute(Qt::WA_DeleteOnClose);

    mFileIconProvider = new QFileIconProvider();

    createHeading();
    createList();
    createToolBar();
}

/**
  * Create dialog heading widget
  */
void FbFolderSelectionDialog::createHeading()
{
    HbWidget *headingWidget = new HbWidget(this);
    QGraphicsLinearLayout *headingLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    headingWidget->setLayout(headingLayout);

    QGraphicsLinearLayout *titleLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mTitle = new HbLabel(this);
    mCurrentPath = new HbLabel(this);
    mCurrentPath->setPlainText(" ");
    mCurrentPath->setElideMode(Qt::ElideMiddle);
    titleLayout->addItem(mTitle);
    titleLayout->setAlignment( mTitle, Qt::AlignLeft);
    titleLayout->addItem(mCurrentPath);
    titleLayout->setAlignment( mCurrentPath, Qt::AlignLeft);

    HbPushButton *moveUpButton = new HbPushButton(headingWidget);
    moveUpButton->setIcon(HbIcon(QString(":/qtg_indi_status_back.svg")));
    connect(moveUpButton, SIGNAL(pressed()),
            this, SLOT(moveUpPressed()));

    headingLayout->addItem(titleLayout);
    headingLayout->addItem(moveUpButton);
    headingLayout->setAlignment(moveUpButton, Qt::AlignRight);

    setHeadingWidget(headingWidget);
}

/**
  * Create dialog content widget as a list of files
  */
void FbFolderSelectionDialog::createList()
{
    mFolderList = new HbListWidget(this);
    mFolderSelectorWrapper = new FbFolderSelectorWrapper();
    mFolderSelectorWrapper->init();

    refreshView();

    setContentWidget(mFolderList);
    connect(mFolderList, SIGNAL(activated(HbListWidgetItem *)),
            this, SLOT(activated(HbListWidgetItem *)));

    connect(mFolderSelectorWrapper, SIGNAL(FolderSelectionChanged()),
            this, SLOT(refreshView()));
}

/**
  * Create dialog toolbar
  */
void FbFolderSelectionDialog::createToolBar()
{
    HbAction *acceptAction = new HbAction(this);
    acceptAction->setEnabled(false);
    connect(acceptAction, SIGNAL(triggered()),
            this, SLOT(acceptTriggered()));
    addAction(acceptAction);

    HbAction *rejectAction = new HbAction(QString("Cancel"), this);
    addAction(rejectAction);
}

/**
  * Refresh list widget content
  */
void FbFolderSelectionDialog::refreshView()
{
    mFolderList->clear();
    QIcon icon;
    mCurrentPath->setPlainText(mFolderSelectorWrapper->currentPath());
    if (mFolderSelectorWrapper->isDriveListViewActive())
    {
        const QString KSimpleDriveEntry("%d\t%c: <%S>\t\t");
        //TODO icon = mFileIconProvider->icon(QFileIconProvider::Drive);
        icon = mFileIconProvider->icon(QFileIconProvider::File);

        for (TInt i=0; i<mFolderSelectorWrapper->itemCount(); i++) {
            FbDriveEntry driveEntry = mFolderSelectorWrapper->getDriveEntry(i);

            const QString SimpleDriveEntry("%1: <%2>");
            QString diskName = SimpleDriveEntry.arg(driveEntry.driveLetter()).arg(driveEntry.mediaTypeString());

            mFolderList->addItem(icon, diskName);
        }

        if (actions().count() > 1) {
            actions().at(0)->setEnabled(false);
        }
    } else {
        const QString SimpleFileEntry("%1");
        icon = mFileIconProvider->icon(QFileIconProvider::Folder);

        for (TInt i=0; i<mFolderSelectorWrapper->itemCount(); i++) {
            FbFileEntry fileEntry = mFolderSelectorWrapper->getFileEntry(i);

            QString fileName = SimpleFileEntry.arg(fileEntry.name()); ///*<< fileEntry.IconId()*/

            mFolderList->addItem(icon, fileName);
        }

        if (actions().count() > 1) {
            actions().at(0)->setEnabled(true);
        }
    }
}

/**
  * Slot called when list item is activated
  */
void FbFolderSelectionDialog::activated(HbListWidgetItem * item)
{
    int row = mFolderList->row(item);
    if (mFolderSelectorWrapper->isDriveListViewActive()) {
        mFolderSelectorWrapper->moveDownToDirectory(row);
    } else if (row >= 0) {
        mFolderSelectorWrapper->moveDownToDirectory(row);
    }
}

/**
  * Slot handling the move up button to move up one leve in directory hierarchy
  */
void FbFolderSelectionDialog::moveUpPressed()
{
    mFolderSelectorWrapper->moveUpOneLevel();
}

void FbFolderSelectionDialog::acceptTriggered()
{
    accept();
}

// ---------------------------------------------------------------------------

/**
  * Constructor
  */
FbCopyToFolderSelectionDialog::FbCopyToFolderSelectionDialog(QGraphicsItem *parent) :
        FbFolderSelectionDialog(parent)
{
    if (headingWidget()) {
        mTitle->setPlainText(QString("Copy to"));
    }
    if (actions().count() > 1) {
        actions().at(0)->setText(QString("Copy here"));
    }
}

// ---------------------------------------------------------------------------

/**
  * Constructor
  */
FbMoveToFolderSelectionDialog::FbMoveToFolderSelectionDialog(QGraphicsItem *parent) :
        FbFolderSelectionDialog(parent)
{
    if (headingWidget()) {
        mTitle->setPlainText(QString("Move to"));
    }
    if (actions().count() > 1) {
        actions().at(0)->setText(QString("Move here"));
    }
}

// End of file
