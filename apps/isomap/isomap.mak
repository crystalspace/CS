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
	$(MAKE_TARGET)
isomapclean:
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

.PHONY: isomap isomapclean

all: $(ISOMAP.EXE)
isomap: $(OUTDIRS) $(ISOMAP.EXE)
clean: isomapclean

$(ISOMAP.EXE): $(DEP.EXE) $(OBJ.ISOMAP) $(LIB.ISOMAP)
	$(DO.LINK.EXE)

isomapclean:
	-$(RM) $(ISOMAP.EXE) $(OBJ.ISOMAP)

ifdef DO_DEPEND
dep: $(OUTOS)isomap.dep
$(OUTOS)isomap.dep: $(SRC.ISOMAP)
	$(DO.DEP)
else
-include $(OUTOS)isomap.dep
endif

endif # ifeq ($(MAKESECTION),targets)
