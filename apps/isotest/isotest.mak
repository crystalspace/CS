# Application description
DESCRIPTION.isotest = Crystal Space isotest demo executable

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make isotest      Make the $(DESCRIPTION.isotest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: isotest isotestclean

all apps: isotest
isotest:
	$(MAKE_APP)
isotestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ISOTEST.EXE = isotest$(EXE)
DIR.ISOTEST = apps/isotest
OUT.ISOTEST = $(OUT)/$(DIR.ISOTEST)
INC.ISOTEST = $(wildcard $(DIR.ISOTEST)/*.h)
SRC.ISOTEST = $(wildcard $(DIR.ISOTEST)/*.cpp)
OBJ.ISOTEST = $(addprefix $(OUT.ISOTEST)/,$(notdir $(SRC.ISOTEST:.cpp=$O)))
DEP.ISOTEST = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL
LIB.ISOTEST = $(foreach d,$(DEP.ISOTEST),$($d.LIB))
#CFG.ISOTEST = data/config/isotest.cfg

TO_INSTALL.EXE    += $(ISOTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.ISOTEST)

MSVC.DSP += ISOTEST
DSP.ISOTEST.NAME = isotest
DSP.ISOTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.isotest isotestclean isotestcleandep

all: $(ISOTEST.EXE)
build.isotest: $(OUT.ISOTEST) $(ISOTEST.EXE)
clean: isotestclean

$(OUT.ISOTEST)/%$O: $(DIR.ISOTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(ISOTEST.EXE): $(DEP.EXE) $(OBJ.ISOTEST) $(LIB.ISOTEST)
	$(DO.LINK.EXE)

$(OUT.ISOTEST):
	$(MKDIRS)

isotestclean:
	-$(RMDIR) $(ISOTEST.EXE) $(OBJ.ISOTEST)

cleandep: isotestcleandep
isotestcleandep:
	-$(RM) $(OUT.ISOTEST)/isotest.dep

ifdef DO_DEPEND
dep: $(OUT.ISOTEST) $(OUT.ISOTEST)/isotest.dep
$(OUT.ISOTEST)/isotest.dep: $(SRC.ISOTEST)
	$(DO.DEPEND)
else
-include $(OUT.ISOTEST)/isotest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
