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

#include <QAction>
#include <QStringListModel>
#include <HbMenu>

#include "memspythreaddetailview.h"

void MemSpyThreadDetailView::initialize(const QVariantMap& params)
{
	MemSpyView::initialize(params);
	
	setTitle(tr("Thread Details"));
	
	mThreadId = qVariantValue<ThreadId>(params["tid"]);
	
	QStringList lines = QStringList() << tr("General") << tr("Heap") << tr("Stack") 
			<< tr("Chunks") << tr("Code Segments") << tr("Open Files") << tr("Active Objects")
			<< tr("Handles to other Threads") << tr("Handles to other Processes") 
			<< tr("Servers Running in Thread") << tr("Client <-> Server connections")
			<< tr("Semaphores") << tr("References this Thread") << tr("References this Process")
			<< tr("Mutexes") << tr("Timers") << tr("Logical DD Channels") 
			<< tr("Change Notifiers") << tr("Undertakers") << tr("Logical Device Drivers")
			<< tr("Physical Device Drivers") << tr("Memory Tracking");
	
	mListView.setModel(new QStringListModel(lines, this));
}

HbMenu* MemSpyThreadDetailView::createToolMenu()
{
	HbMenu* menu = new HbMenu(tr("Thread"));
	mPriorityMenu = menu->addMenu("Change Priority");
	
	mPriorityMenu->addAction(tr("Abs. Very Low"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Low Normal"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Low"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Background Normal"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Background"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Foreground Normal"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Foreground"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. High Normal"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. High"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 1"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 2"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 3"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 4"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 5"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 6"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 7"), this, SLOT(changePriority()));
	mPriorityMenu->addAction(tr("Abs. Real Time 8"), this, SLOT(changePriority()));
	
	return menu;
}
	
void MemSpyThreadDetailView::changePriority()
{
	QAction *s = qobject_cast<QAction*>(sender());
	int index = mPriorityMenu->actions().indexOf(s);
	
	ThreadPriority priorities[] = {
		ThreadPriorityAbsoluteVeryLow,
		ThreadPriorityAbsoluteLowNormal,
		ThreadPriorityAbsoluteLow,
		ThreadPriorityAbsoluteBackgroundNormal,
		ThreadPriorityAbsoluteBackground,
		ThreadPriorityAbsoluteForegroundNormal,
		ThreadPriorityAbsoluteForeground,
		ThreadPriorityAbsoluteHighNormal,
		ThreadPriorityAbsoluteHigh,
		ThreadPriorityAbsoluteRealTime1,
		ThreadPriorityAbsoluteRealTime2,
		ThreadPriorityAbsoluteRealTime3,
		ThreadPriorityAbsoluteRealTime4,
		ThreadPriorityAbsoluteRealTime5,
		ThreadPriorityAbsoluteRealTime6,
		ThreadPriorityAbsoluteRealTime7, 
		ThreadPriorityAbsoluteRealTime8 };
	
	mEngine.setThreadPriority(mThreadId, priorities[index]);
}

