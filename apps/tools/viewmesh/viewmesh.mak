# Application description
DESCRIPTION.vmesh = Crystal Space mesh viewing utility

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make vmesh        Make the $(DESCRIPTION.vmesh)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vmesh vmeshclean

all apps: vmesh
vmesh:
	$(MAKE_TARGET)
vmeshclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/viewmesh

VIEWMESH.EXE = viewmesh$(EXE)
INC.VIEWMESH = $(wildcard apps/tools/viewmesh/*.h)
SRC.VIEWMESH = $(wildcard apps/tools/viewmesh/*.cpp)
OBJ.VIEWMESH = $(addprefix $(OUT),$(notdir $(SRC.VIEWMESH:.cpp=$O)))
DEP.VIEWMESH = CSWS CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.VIEWMESH = $(foreach d,$(DEP.VIEWMESH),$($d.LIB))

#TO_INSTALL.EXE += $(VIEWMESH.EXE)

MSVC.DSP += VIEWMESH
DSP.VIEWMESH.NAME = viewmesh
DSP.VIEWMESH.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: vmesh vmeshclean

all: $(VIEWMESH.EXE)
vmesh: $(OUTDIRS) $(VIEWMESH.EXE)
clean: vmeshclean

$(VIEWMESH.EXE): $(DEP.EXE) $(OBJ.VIEWMESH) $(LIB.VIEWMESH)
	$(DO.LINK.EXE)

vmeshclean:
	-$(RM) $(VIEWMESH.EXE) $(OBJ.VIEWMESH)

ifdef DO_DEPEND
dep: $(OUTOS)viewmesh.dep
$(OUTOS)viewmesh.dep: $(SRC.VIEWMESH)
	$(DO.DEP)
else
-include $(OUTOS)viewmesh.dep
endif

endif # ifeq ($(MAKESECTION),targets)
