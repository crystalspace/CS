# Application description
DESCRIPTION.pview = Crystal Space Picture Viewer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make pview        Make the $(DESCRIPTION.pview)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pview pviewclean

all apps: pview
pview:
	$(MAKE_TARGET)
pviewclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/picview

PVIEW.EXE = picview$(EXE)
INC.PVIEW = $(wildcard apps/tools/picview/*.h)
SRC.PVIEW = $(wildcard apps/tools/picview/*.cpp)
OBJ.PVIEW = $(addprefix $(OUT),$(notdir $(SRC.PVIEW:.cpp=$O)))
DEP.PVIEW = \
  CSWS CSTOOL CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.PVIEW = $(foreach d,$(DEP.PVIEW),$($d.LIB))

#TO_INSTALL.EXE    += $(PVIEW.EXE)
#TO_INSTALL.CONFIG += $(CFG.PVIEW)

MSVC.DSP += PVIEW
DSP.PVIEW.NAME = picview
DSP.PVIEW.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: pview pviewclean

all: $(PVIEW.EXE)
pview: $(OUTDIRS) $(PVIEW.EXE)
clean: pviewclean

$(PVIEW.EXE): $(DEP.EXE) $(OBJ.PVIEW) $(LIB.PVIEW)
	$(DO.LINK.EXE)

pviewclean:
	-$(RM) $(PVIEW.EXE) $(OBJ.PVIEW)

ifdef DO_DEPEND
dep: $(OUTOS)pview.dep
$(OUTOS)pview.dep: $(SRC.PVIEW)
	$(DO.DEP)
else
-include $(OUTOS)pview.dep
endif

endif # ifeq ($(MAKESECTION),targets)
