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

#include "memspykernelobjectdetailview.h"

void MemSpyKernelObjectDetailView::initialize(const QVariantMap& params)
{
	MemSpyView::initialize(params);
	
	setTitle("Details");
	QStringList items = params.value("details").toStringList();
	mListView.setModel(new QStringListModel(items, this));
}
