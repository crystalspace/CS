# Application description
DESCRIPTION.simplept = Crystal Space procedural textures tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simplept     Make the $(DESCRIPTION.simplept)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simplept simpleptclean

all apps: simplept
simplept:
	$(MAKE_APP)
simpleptclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SIMPLEPT.EXE = simplept$(EXE)
DIR.SIMPLEPT = apps/tutorial/simplept
OUT.SIMPLEPT = $(OUT)/$(DIR.SIMPLEPT)
INC.SIMPLEPT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLEPT)/*.h ))
SRC.SIMPLEPT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLEPT)/*.cpp ))
OBJ.SIMPLEPT = $(addprefix $(OUT.SIMPLEPT)/,$(notdir $(SRC.SIMPLEPT:.cpp=$O)))
DEP.SIMPLEPT = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.SIMPLEPT = $(foreach d,$(DEP.SIMPLEPT),$($d.LIB))

OUTDIRS += $(OUT.SIMPLEPT)

#TO_INSTALL.EXE += $(SIMPLEPT.EXE)

MSVC.DSP += SIMPLEPT
DSP.SIMPLEPT.NAME = simplept
DSP.SIMPLEPT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simplept simpleptclean simpleptcleandep

build.simplept: $(OUTDIRS) $(SIMPLEPT.EXE)
clean: simpleptclean

$(OUT.SIMPLEPT)/%$O: $(SRCDIR)/$(DIR.SIMPLEPT)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPLEPT.EXE): $(DEP.EXE) $(OBJ.SIMPLEPT) $(LIB.SIMPLEPT)
	$(DO.LINK.EXE)

simpleptclean:
	-$(RM) simplept.txt
	-$(RMDIR) $(SIMPLEPT.EXE) $(OBJ.SIMPLEPT)

cleandep: simpleptcleandep
simpleptcleandep:
	-$(RM) $(OUT.SIMPLEPT)/simplept.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPLEPT) $(OUT.SIMPLEPT)/simplept.dep
$(OUT.SIMPLEPT)/simplept.dep: $(SRC.SIMPLEPT)
	$(DO.DEPEND)
else
-include $(OUT.SIMPLEPT)/simplept.dep
endif

endif # ifeq ($(MAKESECTION),targets)
