# Application target only valid if associated module is listed in PLUGINS.
ifneq (,$(findstring video/format,$(PLUGINS)))

# Application description
DESCRIPTION.csvid = Crystal Space video example

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make csvid        Make the $(DESCRIPTION.csvid)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csvid csvidclean

all apps: csvid
csvid:
	$(MAKE_APP)
csvidclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSVID.EXE=csvid$(EXE)
DIR.CSVID = apps/video
OUT.CSVID = $(OUT)/$(DIR.CSVID)
INC.CSVID = $(wildcard $(DIR.CSVID)/*.h)
SRC.CSVID = $(wildcard $(DIR.CSVID)/*.cpp)
OBJ.CSVID = $(addprefix $(OUT.CSVID)/,$(notdir $(SRC.CSVID:.cpp=$O)))
DEP.CSVID = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSVID = $(foreach d,$(DEP.CSVID),$($d.LIB))

#TO_INSTALL.EXE += $(CSVID.EXE)

MSVC.DSP += CSVID
DSP.CSVID.NAME = csvid
DSP.CSVID.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.csvid csvidclean csvidcleandep

all: $(CSVID.EXE)
build.csvid: $(OUT.CSVID) $(CSVID.EXE)
clean: csvidclean

$(OUT.CSVID)/%$O: $(DIR.CSVID)/%.cpp
	$(DO.COMPILE.CPP)

$(CSVID.EXE): $(DEP.EXE) $(OBJ.CSVID) $(LIB.CSVID)
	$(DO.LINK.EXE)

$(OUT.CSVID):
	$(MKDIRS)

csvidclean:
	-$(RMDIR) $(CSVID.EXE) $(OBJ.CSVID)

cleandep: csvidcleandep
csvidcleandep:
	-$(RM) $(OUT.CSVID)/csvid.dep

ifdef DO_DEPEND
dep: $(OUT.CSVID) $(OUT.CSVID)/csvid.dep
$(OUT.CSVID)/csvid.dep: $(SRC.CSVID)
	$(DO.DEPEND)
else
-include $(OUT.CSVID)/csvid.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring video/format,$(PLUGINS)))
