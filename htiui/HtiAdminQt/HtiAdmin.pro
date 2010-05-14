#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#

TEMPLATE = app
TARGET = HtiAdmin
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += . /epoc32/include/mw/hb/hbcore /epoc32/include/mw/hb/hbwidgets /epoc32/include/mw/hb/hbtools
LIBS += -lhbcore -lhbwidgets

HEADERS += mainview.h 
SOURCES += main.cpp mainview.cpp 

symbian: { 
    
	LIBS += -lestor -lws32 -lPlatformEnv -limageconversion -leikcore -lfbscli -lapgrfx -lcommonengine -lgdi -lhticfg -lbafl -lecom -lcommdb
	

    HEADERS += htienginewrapper.h chtiadminengine.h 
    SOURCES += htienginewrapper.cpp chtiadminengine.cpp 
    RSS_RULES += "group_name = \"RnD Tools\"";
    TARGET.CAPABILITY = ALL -TCB
    TARGET.UID2 = 0x100039CE
    
    TARGET.UID3 = 0x1020DEB5
    
    
    # TARGET.SID = 0x1020DEB5
    TARGET.VID = 0x101FB657 
        
    # TARGET.EPOCHEAPSIZE = 0x020000 0x800000 // Min 128Kb, Max 16Mb
    # TARGET.EPOCSTACKSIZE = 0x5000
}else {
    error("Only Symbian supported!")
}

