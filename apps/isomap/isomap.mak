# Application description
DESCRIPTION.isomaptst = Crystal Space isomap demo executable

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make isomaptst    Make the $(DESCRIPTION.isomaptst)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: isomaptst isomaptstclean

all apps: isomaptst
isomaptst:
	$(MAKE_TARGET)
isomaptstclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/isomap

ISOMAP.EXE = isomap$(EXE)
INC.ISOMAP = $(wildcard apps/isomap/*.h)
SRC.ISOMAP = $(wildcard apps/isomap/*.cpp)
OBJ.ISOMAP = $(addprefix $(OUT),$(notdir $(SRC.ISOMAP:.cpp=$O)))
DEP.ISOMAP = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL
LIB.ISOMAP = $(foreach d,$(DEP.ISOMAP),$($d.LIB))
#CFG.ISOMAP = data/config/isomap.cfg

TO_INSTALL.EXE    += $(ISOMAP.EXE)
#TO_INSTALL.CONFIG += $(CFG.ISOMAP)

MSVC.DSP += ISOMAP
DSP.ISOMAP.NAME = isomap
DSP.ISOMAP.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: isomaptst isomaptstclean

all: $(ISOMAP.EXE)
isomaptst: $(OUTDIRS) $(ISOMAP.EXE)
clean: isomaptstclean

$(ISOMAP.EXE): $(DEP.EXE) $(OBJ.ISOMAP) $(LIB.ISOMAP)
	$(DO.LINK.EXE)

isomaptstclean:
	-$(RM) $(ISOMAP.EXE) $(OBJ.ISOMAP)

ifdef DO_DEPEND
dep: $(OUTOS)isomap.dep
$(OUTOS)isomap.dep: $(SRC.ISOMAP)
	$(DO.DEP)
else
-include $(OUTOS)isomap.dep
endif

endif # ifeq ($(MAKESECTION),targets)
