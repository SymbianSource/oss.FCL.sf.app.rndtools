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

#include "memspymainview.h"
#include "viewManager.h"

#include <QStringListModel>
#include <QDebug>

void MemSpyMainView::initialize(const QVariantMap& params)
{
	MemSpyView::initialize(params);
	
	setTitle("MemSpy");
	QStringList items = QStringList() 
			<< tr("Processes & Threads") 
			<< tr("Kernel Objects"); 
			//<< tr("Kernel Heap");
	mListView.setModel(new QStringListModel(items, this));
    
	QObject::connect(&mListView, SIGNAL(released(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
}

void MemSpyMainView::itemClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	ViewIndex indexes[] = { ProcessView, KernelObjectTypeView, KernelObjectTypeView };
    mViewManager.showView(indexes[index.row()]);
}

