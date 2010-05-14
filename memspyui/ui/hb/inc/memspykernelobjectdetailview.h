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

#ifndef MEMSPYKERNELOBJECTDETAILVIEW_H_
#define MEMSPYKERNELOBJECTDETAILVIEW_H_

#include "memspyview.h"

class MemSpyKernelObjectDetailView : public MemSpyView
{
	Q_OBJECT
	
public:
	MemSpyKernelObjectDetailView(EngineWrapper &engine, ViewManager &viewManager) 
		: MemSpyView(engine, viewManager) {}
	
protected:
	virtual void initialize(const QVariantMap& params);
};

#endif /* MEMSPYKERNELOBJECTDETAILVIEW_H_ */
