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


ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=\epoc32\release\$(PLATFORM)\$(CFG)\z
else
ZDIR=\epoc32\data\z
endif

TARGETDIR=$(ZDIR)\resource\apps
ICONTARGETFILENAME=$(TARGETDIR)\filebrowser_extraicons.mif

HEADERDIR=\epoc32\include
HEADERFILENAME=$(HEADERDIR)\filebrowser_extraicons.mbg


do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN :
	@if exist $(ICONTARGETFILENAME) erase $(ICONTARGETFILENAME)
	@if exist $(HEADERFILENAME) erase $(HEADERFILENAME)

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE : $(ICONTARGETFILENAME)

$(ICONTARGETFILENAME) (HEADERFILENAME) : ..\icons\qgn_indi_tb_filebrowser_folder_parent.svg ..\icons\qgn_indi_tb_filebrowser_find.svg ..\icons\qgn_indi_tb_filebrowser_selection.svg ..\icons\qgn_indi_tb_filebrowser_selection_active.svg
	mifconv $(ICONTARGETFILENAME) /h$(HEADERFILENAME) \
		/c8,8 ..\icons\qgn_indi_tb_filebrowser_folder_parent.svg \
		/c8,8 ..\icons\qgn_indi_tb_filebrowser_find.svg \
		/c8,8 ..\icons\qgn_indi_tb_filebrowser_selection.svg \
		/c8,8 ..\icons\qgn_indi_tb_filebrowser_selection_active.svg

FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(HEADERFILENAME) && \
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing
