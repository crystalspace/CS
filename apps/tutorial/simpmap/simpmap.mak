# Application description
DESCRIPTION.simpmap = Crystal Space tutorial part three, map loading

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make simpmap      Make the $(DESCRIPTION.simpmap)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simpmap tutmapclean

all apps: simpmap
simpmap:
	$(MAKE_APP)
tutmapclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/simpmap

SIMPMAP.EXE=simpmap$(EXE)
INC.SIMPMAP = $(wildcard apps/tutorial/simpmap/*.h)
SRC.SIMPMAP = $(wildcard apps/tutorial/simpmap/*.cpp)
OBJ.SIMPMAP = $(addprefix $(OUT),$(notdir $(SRC.SIMPMAP:.cpp=$O)))
DEP.SIMPMAP = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.SIMPMAP = $(foreach d,$(DEP.SIMPMAP),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPMAP.EXE)

MSVC.DSP += SIMPMAP
DSP.SIMPMAP.NAME = simpmap
DSP.SIMPMAP.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simpmap tutmapclean

all: $(SIMPMAP.EXE)
build.simpmap: $(OUTDIRS) $(SIMPMAP.EXE)
clean: tutmapclean

$(SIMPMAP.EXE): $(DEP.EXE) $(OBJ.SIMPMAP) $(LIB.SIMPMAP)
	$(DO.LINK.EXE)

tutmapclean:
	-$(RM) $(SIMPMAP.EXE) $(OBJ.SIMPMAP)

ifdef DO_DEPEND
dep: $(OUTOS)simpmap.dep
$(OUTOS)simpmap.dep: $(SRC.SIMPMAP)
	$(DO.DEP)
else
-include $(OUTOS)simpmap.dep
endif

endif # ifeq ($(MAKESECTION),targets)
