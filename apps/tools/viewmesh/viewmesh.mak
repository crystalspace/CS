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

vpath %.cpp apps/tools/viewmesh

VIEWMESH.EXE = viewmesh$(EXE)
INC.VIEWMESH = $(wildcard apps/tools/viewmesh/*.h)
SRC.VIEWMESH = $(wildcard apps/tools/viewmesh/*.cpp)
OBJ.VIEWMESH = $(addprefix $(OUT)/,$(notdir $(SRC.VIEWMESH:.cpp=$O)))
DEP.VIEWMESH = CSWS CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.VIEWMESH = $(foreach d,$(DEP.VIEWMESH),$($d.LIB))

TO_INSTALL.EXE += $(VIEWMESH.EXE)

MSVC.DSP += VIEWMESH
DSP.VIEWMESH.NAME = viewmesh
DSP.VIEWMESH.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.viewmesh viewmeshclean

all: $(VIEWMESH.EXE)
build.viewmesh: $(OUTDIRS) $(VIEWMESH.EXE)
clean: viewmeshclean

$(VIEWMESH.EXE): $(DEP.EXE) $(OBJ.VIEWMESH) $(LIB.VIEWMESH)
	$(DO.LINK.EXE)

viewmeshclean:
	-$(RMDIR) $(VIEWMESH.EXE) $(OBJ.VIEWMESH)

ifdef DO_DEPEND
dep: $(OUTOS)/viewmesh.dep
$(OUTOS)/viewmesh.dep: $(SRC.VIEWMESH)
	$(DO.DEP)
else
-include $(OUTOS)/viewmesh.dep
endif

endif # ifeq ($(MAKESECTION),targets)
