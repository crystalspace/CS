# Application description
DESCRIPTION.simpmap = Crystal Space tutorial part three, map loading

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simpmap      Make the $(DESCRIPTION.simpmap)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simpmap simpmapclean

all apps: simpmap
simpmap:
	$(MAKE_APP)
simpmapclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SIMPMAP.EXE=simpmap$(EXE)
DIR.SIMPMAP = apps/tutorial/simpmap
OUT.SIMPMAP = $(OUT)/$(DIR.SIMPMAP)
INC.SIMPMAP = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPMAP)/*.h ))
SRC.SIMPMAP = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPMAP)/*.cpp ))
OBJ.SIMPMAP = $(addprefix $(OUT.SIMPMAP)/,$(notdir $(SRC.SIMPMAP:.cpp=$O)))
DEP.SIMPMAP = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.SIMPMAP = $(foreach d,$(DEP.SIMPMAP),$($d.LIB))

OUTDIRS += $(OUT.SIMPMAP)

#TO_INSTALL.EXE += $(SIMPMAP.EXE)

MSVC.DSP += SIMPMAP
DSP.SIMPMAP.NAME = simpmap
DSP.SIMPMAP.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simpmap simpmapclean simpmapcleandep

build.simpmap: $(OUTDIRS) $(SIMPMAP.EXE)
clean: simpmapclean

$(OUT.SIMPMAP)/%$O: $(SRCDIR)/$(DIR.SIMPMAP)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPMAP.EXE): $(DEP.EXE) $(OBJ.SIMPMAP) $(LIB.SIMPMAP)
	$(DO.LINK.EXE)

simpmapclean:
	-$(RM) simpmap.txt
	-$(RMDIR) $(SIMPMAP.EXE) $(OBJ.SIMPMAP)

cleandep: simpmapcleandep
simpmapcleandep:
	-$(RM) $(OUT.SIMPMAP)/simpmap.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPMAP) $(OUT.SIMPMAP)/simpmap.dep
$(OUT.SIMPMAP)/simpmap.dep: $(SRC.SIMPMAP)
	$(DO.DEPEND)
else
-include $(OUT.SIMPMAP)/simpmap.dep
endif

endif # ifeq ($(MAKESECTION),targets)
