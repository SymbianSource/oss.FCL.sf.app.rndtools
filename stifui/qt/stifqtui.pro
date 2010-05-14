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
# Description: QT project file.              
#

HEADERS += inc\version.h \
    inc\frmmain.h \
    inc\istfqtuicontroller.h \
    inc\stfqtuicontroller.h \
    inc\cstfcase.h \
    inc\cstfmodule.h \
    inc\istfqtuimodel.h \
    inc\stfqtuimodel.h \
    inc\dlgoutput.h \
    inc\uisetting.h \
    inc\dlgsetting.h \
   
SOURCES += src\frmmain.cpp \
    src\main.cpp \
    src\stfqtuimodel.cpp \
    src\stfqtuicontroller.cpp \
    src\dlgoutput.cpp \
    src\uisetting.cpp \
    src\dlgsetting.cpp \
    
RESOURCES += 
symbian { 
    TARGET.UID3 = 0x2002BCA0
    TARGET.EPOCALLOWDLLDATA = 1
    //HEADERS += ../../../inc/.
    INCLUDEPATH += /epoc32/include/mw
    INCLUDEPATH += /epoc32/include/platform
    INCLUDEPATH += /epoc32/include/platform/stifinternal
    INCLUDEPATH += /epoc32/include/domain/osextensions
    INCLUDEPATH += /epoc32/include/domain/osextensions/stif
    HEADERS += inc\stifexecutor.h
    SOURCES += src\stifexecutor.cpp
    LIBS += -leuser \
        -lefsrv \
        -lstiftestinterface \
        -lstiftfwif \
        -lstiftestengine \
        -lecons \
        -lhal \
        -lflogger
    TARGET.CAPABILITY = AllFiles \
        CommDD
    
    # Export headers to SDK Epoc32/include directory
    deploy.path = $$EPOCROOT
    exportheaders.sources = $$PUBLIC_HEADERS
    exportheaders.path = epoc32/include
    for(header, exportheaders.sources)
    :BLD_INF_RULES.prj_exports += "$$header $$deploy.path$$exportheaders.path/$$basename(header)"
}
