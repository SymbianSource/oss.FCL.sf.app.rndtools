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

#include "memspyprocessview.h"

#include <QVariantMap>

#include "viewmanager.h"

MemSpyProcessModel::MemSpyProcessModel(EngineWrapper &engine, QObject *parent) :
	QAbstractListModel(parent),
	mProcesses(engine.getProcesses())
{
}

MemSpyProcessModel::~MemSpyProcessModel()
{
	qDeleteAll(mProcesses);
}
	
int MemSpyProcessModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return mProcesses.count();
}
	
QVariant MemSpyProcessModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		QStringList lines;
		lines << mProcesses.at(index.row())->name();
		
		return lines;
	}
	
	if (role == Qt::UserRole)
		return mProcesses.at(index.row())->id();
	
	return QVariant();
}
	

void MemSpyProcessView::initialize(const QVariantMap& params)
{
	MemSpyView::initialize(params);
	
	setTitle(tr("Processes"));
	mListView.setModel(new MemSpyProcessModel(mEngine, this));
	
	connect(&mListView, SIGNAL(activated(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
}

void MemSpyProcessView::itemClicked(const QModelIndex& index)
{
	QVariantMap map;
	map.insert("pid", index.data(Qt::UserRole));
    mViewManager.showView(ThreadView, map);
}

