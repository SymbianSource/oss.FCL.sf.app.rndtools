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

MemSpyThreadDetailModel::MemSpyThreadDetailModel(EngineWrapper &engine, ThreadId threadId, ThreadInfoType type, QObject *parent) :
	QAbstractListModel(parent),
	mThreadInfo(engine.getThreadInfo(threadId, type))
{
}

MemSpyThreadDetailModel::~MemSpyThreadDetailModel()
{
	qDeleteAll(mThreadInfo);
}
	
int MemSpyThreadDetailModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return mThreadInfo.count();
}
	
QVariant MemSpyThreadDetailModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		QStringList lines;
		lines << mThreadInfo.at(index.row())->caption();
		lines << mThreadInfo.at(index.row())->value();
		return lines;
	}
	
	return QVariant();
}

void MemSpyThreadDetailView::initialize(const QVariantMap& params)
{
	MemSpyView::initialize(params);
	
	setTitle(tr("Thread Details"));
	
	ThreadId threadId = qVariantValue<ThreadId>(params["tid"]);
	ThreadInfoType type = static_cast<ThreadInfoType>(qVariantValue<int>(params["type"]));
	
	mListView.setModel(new MemSpyThreadDetailModel(mEngine, threadId, type, this));
}
