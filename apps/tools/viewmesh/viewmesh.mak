# Application description
DESCRIPTION.viewmesh = Crystal Space mesh viewing utility

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make viewmesh     Make the $(DESCRIPTION.viewmesh)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: viewmesh viewmeshclean

all apps: viewmesh
viewmesh:
	$(MAKE_APP)
viewmeshclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

VIEWMESH.EXE = viewmesh$(EXE)
DIR.VIEWMESH = apps/tools/viewmesh
OUT.VIEWMESH = $(OUT)/$(DIR.VIEWMESH)
INC.VIEWMESH = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.VIEWMESH)/*.h ))
SRC.VIEWMESH = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.VIEWMESH)/*.cpp ))
OBJ.VIEWMESH = $(addprefix $(OUT.VIEWMESH)/,$(notdir $(SRC.VIEWMESH:.cpp=$O)))
DEP.VIEWMESH = CSWS CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.VIEWMESH = $(foreach d,$(DEP.VIEWMESH),$($d.LIB))

OUTDIRS += $(OUT.VIEWMESH)

TO_INSTALL.EXE += $(VIEWMESH.EXE)

MSVC.DSP += VIEWMESH
DSP.VIEWMESH.NAME = viewmesh
DSP.VIEWMESH.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.viewmesh viewmeshclean viewmeshcleandep

all: $(VIEWMESH.EXE)
build.viewmesh: $(OUTDIRS) $(VIEWMESH.EXE)
clean: viewmeshclean

$(OUT.VIEWMESH)/%$O: $(SRCDIR)/$(DIR.VIEWMESH)/%.cpp
	$(DO.COMPILE.CPP)

$(VIEWMESH.EXE): $(DEP.EXE) $(OBJ.VIEWMESH) $(LIB.VIEWMESH)
	$(DO.LINK.EXE)

viewmeshclean:
	-$(RM) viewmesh.txt
	-$(RMDIR) $(VIEWMESH.EXE) $(OBJ.VIEWMESH)

cleandep: viewmeshcleandep
viewmeshcleandep:
	-$(RM) $(OUT.VIEWMESH)/viewmesh.dep

ifdef DO_DEPEND
dep: $(OUT.VIEWMESH) $(OUT.VIEWMESH)/viewmesh.dep
$(OUT.VIEWMESH)/viewmesh.dep: $(SRC.VIEWMESH)
	$(DO.DEPEND)
else
-include $(OUT.VIEWMESH)/viewmesh.dep
endif

endif # ifeq ($(MAKESECTION),targets)
