# Application target only valid if associated module is listed in PLUGINS.
ifneq (,$(findstring video/format,$(PLUGINS)))

# Application description
DESCRIPTION.vid = Crystal Space video example

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make vid          Make the $(DESCRIPTION.vid)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vid vidclean

all apps: vid
vid:
	$(MAKE_TARGET)
vidclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/video

CSVID.EXE=csvid$(EXE)
INC.CSVID = $(wildcard apps/video/*.h)
SRC.CSVID = $(wildcard apps/video/*.cpp)
OBJ.CSVID = $(addprefix $(OUT),$(notdir $(SRC.CSVID:.cpp=$O)))
DEP.CSVID = CSPARSER CSFX CSENGINE CSFX CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSVID = $(foreach d,$(DEP.CSVID),$($d.LIB))

#TO_INSTALL.EXE += $(CSVID.EXE)

MSVC.DSP += CSVID
DSP.CSVID.NAME = csvid
DSP.CSVID.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: vid vidclean

all: $(CSVID.EXE)
vid: $(OUTDIRS) $(CSVID.EXE)
clean: vidclean

$(CSVID.EXE): $(DEP.EXE) $(OBJ.CSVID) $(LIB.CSVID)
	$(DO.LINK.EXE)

vidclean:
	-$(RM) $(CSVID.EXE) $(OBJ.CSVID)

ifdef DO_DEPEND
dep: $(OUTOS)csvid.dep
$(OUTOS)csvid.dep: $(SRC.CSVID)
	$(DO.DEP)
else
-include $(OUTOS)csvid.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring video/format,$(PLUGINS)))
