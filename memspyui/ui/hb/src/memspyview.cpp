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

#include <HbMenu>
#include <HbToolBar>
#include <HbApplication>
#include <HbMessageBox>
#include <HbLabel>

#include "memspyview.h"


MemSpyView::MemSpyView(EngineWrapper &engine, ViewManager &viewManager) : 
	HbView(),
	mEngine(engine),
	mViewManager(viewManager),
	mOutputMenu(0), 
	mOutputGenInfoMenu(0), 
	mOutputHeapInfoMenu(0),
	mOutputStackInfoMenu(0),
	mToolBar(0)
{
	setWidget(&mListView);
}

MemSpyView::~MemSpyView()
{
	delete mToolBar;
	delete mOutputStackInfoMenu;
	delete mOutputHeapInfoMenu;
	delete mOutputGenInfoMenu; 
	delete mOutputMenu;
}

void MemSpyView::initialize(const QVariantMap& params)
{
	if (isRefreshable())
		menu()->addAction(tr("Refresh"));
	
	HbMenu* toolMenu = createToolMenu();
	if (toolMenu)
		menu()->addMenu(toolMenu);
	
	mOutputMenu = menu()->addMenu(tr("Output"));
	mOutputMenu->addActions(createOutputActions());
	mOutputGenInfoMenu = mOutputMenu->addMenu(tr("General Info"));
	mOutputHeapInfoMenu = mOutputMenu->addMenu(tr("Heap Info"));
	mOutputStackInfoMenu = mOutputMenu->addMenu(tr("Stack Info"));
	mOutputGenInfoMenu->addAction(tr("Summary"));
	mOutputGenInfoMenu->addAction(tr("Detailed Info"));
	mOutputGenInfoMenu->addAction(tr("Handle Info"));
	mOutputGenInfoMenu->addAction(tr("Kernel Containters"));
	mOutputHeapInfoMenu->addAction(tr("Detailed Summary"));
	mOutputHeapInfoMenu->addAction(tr("Compact Summary"));
	mOutputHeapInfoMenu->addAction(tr("Cell Listing"));
	mOutputHeapInfoMenu->addAction(tr("Data (Binary)"));
	mOutputStackInfoMenu->addAction(tr("Detailed Summary"));
	mOutputStackInfoMenu->addAction(tr("Compact Summary"));
	mOutputStackInfoMenu->addAction(tr("User Stacks (Binary)"));
	mOutputStackInfoMenu->addAction(tr("Kernel Stacks (Binary)"));
	menu()->addAction(tr("Start Auto Capture"));
	menu()->addAction(tr("Settings ..."));
	menu()->addAction(tr("About ..."), this, SLOT(showAbout()));
	menu()->addAction(tr("Exit"), qApp, SLOT(quit()));
	
	mToolBar = createToolBar();
	if (mToolBar)
		setToolBar(mToolBar);
}

QList<QAction*> MemSpyView::createOutputActions()
{
	return QList<QAction*>();
}

HbMenu* MemSpyView::createToolMenu()
{
	return 0;
}

HbToolBar* MemSpyView::createToolBar()
{
	return 0;
}

bool MemSpyView::isRefreshable() const
{
	return false;
}

void MemSpyView::refresh()
{
	// Empty default implementation
}

void MemSpyView::showAbout()
{
	HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation);
	messageBox->setText("Version 2.0.0 - 23th April 2010. Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved. Licensed under Eclipse Public License v1.0.");
	HbLabel *header = new HbLabel("About MemSpy", messageBox);
	messageBox->setHeadingWidget(header);
	messageBox->setAttribute(Qt::WA_DeleteOnClose);
	messageBox->setTimeout(HbPopup::NoTimeout);
	messageBox->open();
}
