# Application description
DESCRIPTION.isomap = Crystal Space isomap demo executable

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make isomap       Make the $(DESCRIPTION.isomap)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: isomap isomapclean

all apps: isomap
isomap:
	$(MAKE_APP)
isomapclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ISOMAP.EXE = isomap$(EXE)
DIR.ISOMAP = apps/isomap
OUT.ISOMAP = $(OUT)/$(DIR.ISOMAP)
INC.ISOMAP = $(wildcard $(DIR.ISOMAP)/*.h)
SRC.ISOMAP = $(wildcard $(DIR.ISOMAP)/*.cpp)
OBJ.ISOMAP = $(addprefix $(OUT.ISOMAP)/,$(notdir $(SRC.ISOMAP:.cpp=$O)))
DEP.ISOMAP = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL
LIB.ISOMAP = $(foreach d,$(DEP.ISOMAP),$($d.LIB))
#CFG.ISOMAP = data/config/isomap.cfg

TO_INSTALL.EXE    += $(ISOMAP.EXE)
#TO_INSTALL.CONFIG += $(CFG.ISOMAP)
TO_INSTALL.DATA   += data/isomap/world

MSVC.DSP += ISOMAP
DSP.ISOMAP.NAME = isomap
DSP.ISOMAP.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.isomap isomapclean isomapcleandep

all: $(ISOMAP.EXE)
build.isomap: $(OUT.ISOMAP) $(ISOMAP.EXE)
clean: isomapclean

$(OUT.ISOMAP)/%$O: $(DIR.ISOMAP)/%.cpp
	$(DO.COMPILE.CPP)

$(ISOMAP.EXE): $(DEP.EXE) $(OBJ.ISOMAP) $(LIB.ISOMAP)
	$(DO.LINK.EXE)

$(OUT.ISOMAP):
	$(MKDIRS)

isomapclean:
	-$(RMDIR) $(ISOMAP.EXE) $(OBJ.ISOMAP)

cleandep: isomapcleandep
isomapcleandep:
	-$(RM) $(OUT.ISOMAP)/isomap.dep

ifdef DO_DEPEND
dep: $(OUT.ISOMAP) $(OUT.ISOMAP)/isomap.dep
$(OUT.ISOMAP)/isomap.dep: $(SRC.ISOMAP)
	$(DO.DEPEND)
else
-include $(OUT.ISOMAP)/isomap.dep
endif

endif # ifeq ($(MAKESECTION),targets)
