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

#include <QStringListModel>

#include "memspykernelobjecttypeview.h"
#include "viewmanager.h"

MemSpyKernelObjectTypeModel::MemSpyKernelObjectTypeModel(EngineWrapper &engine, QObject *parent) :
	QAbstractListModel(parent),
	mObjectTypes(engine.getKernelObjectTypes())
{
}

MemSpyKernelObjectTypeModel::~MemSpyKernelObjectTypeModel()
{
	qDeleteAll(mObjectTypes);
}
	
int MemSpyKernelObjectTypeModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return mObjectTypes.count();
}
	
QVariant MemSpyKernelObjectTypeModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		QStringList lines;
		lines << mObjectTypes.at(index.row())->name();
		
		return lines;
	}
	
	if (role == Qt::UserRole)
		return mObjectTypes.at(index.row())->id();
	
	return QVariant();
}

void MemSpyKernelObjectTypeView::initialize(const QVariantMap& params)
{
	MemSpyView::initialize(params);
	
	setTitle(tr("Kernel Objects"));
	
	QStringList list = QStringList() << "Thread" << "Process" << "Chunk" << "Library" <<
			"Semaphore" << "Mutex" << "Timer" << "Server" << "Session" << "Logical Device" <<
			"Physical Device" << "Logical Channel" << "Change Notifier" << "Undertaker" <<
			"Message Queue" << "Property Ref." << "Conditional Var.";
			
	//mListView.setModel(new MemSpyKernelObjectTypeModel(mEngine, this));
	mListView.setModel(new QStringListModel(list, this));
	
	connect(&mListView, SIGNAL(activated(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
}

void MemSpyKernelObjectTypeView::itemClicked(const QModelIndex& index)
{
	QVariantMap map;
	map.insert("type", index.row());
    mViewManager.showView(KernelObjectView, map);
}

