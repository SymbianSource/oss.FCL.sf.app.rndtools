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

#ifndef MEMSPYKERNELOBJECTTYPESVIEW_H_
#define MEMSPYKERNELOBJECTTYPESVIEW_H_

#include "memspyview.h"
#include "enginewrapper.h"

class MemSpyKernelObjectTypeModel : public QAbstractListModel
{
public:
	MemSpyKernelObjectTypeModel(EngineWrapper &engine, QObject *parent = 0);
	
	~MemSpyKernelObjectTypeModel();
	
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	
private:
	QList<MemSpyKernelObjectType*> mObjectTypes;
};

class MemSpyKernelObjectTypeView : public MemSpyView
{
	Q_OBJECT
	
public:
	MemSpyKernelObjectTypeView(EngineWrapper &engine, ViewManager &viewManager) 
		: MemSpyView(engine, viewManager) {}

protected:
	virtual void initialize(const QVariantMap& params);

private slots:
	void itemClicked(const QModelIndex& index);
};

#endif /* MEMSPYKERNELOBJECTTYPESVIEW_H_ */
