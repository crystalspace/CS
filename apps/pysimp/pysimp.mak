# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring cspython,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.pysimp = Crystal Space Python example

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make pysimp       Make the $(DESCRIPTION.pysimp)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pysimp pysimpclean

all apps: pysimp
pysimp:
	$(MAKE_APP)
pysimpclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PYSIMP.EXE = pysimp$(EXE)
DIR.PYSIMP = apps/pysimp
OUT.PYSIMP = $(OUT)/$(DIR.PYSIMP)
INC.PYSIMP = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PYSIMP)/*.h))
SRC.PYSIMP = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PYSIMP)/*.cpp))
OBJ.PYSIMP = $(addprefix $(OUT.PYSIMP)/,$(notdir $(SRC.PYSIMP:.cpp=$O)))
DEP.PYSIMP = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.PYSIMP = $(foreach d,$(DEP.PYSIMP),$($d.LIB))

OUTDIRS += $(OUT.PYSIMP)

#TO_INSTALL.EXE += $(PYSIMP.EXE)

MSVC.DSP += PYSIMP
DSP.PYSIMP.NAME = pysimp
DSP.PYSIMP.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.pysimp pysimpclean pysimpcleandep

all: $(PYSIMP.EXE)
build.pysimp: $(OUTDIRS) $(PYSIMP.EXE)
clean: pysimpclean

$(OUT.PYSIMP)/%$O: $(SRCDIR)/$(DIR.PYSIMP)/%.cpp
	$(DO.COMPILE.CPP)

$(PYSIMP.EXE): $(DEP.EXE) $(OBJ.PYSIMP) $(LIB.PYSIMP)
	$(DO.LINK.EXE)

pysimpclean:
	-$(RM) pysimp.txt
	-$(RMDIR) $(PYSIMP.EXE) $(OBJ.PYSIMP)

cleandep: pysimpcleandep
pysimpcleandep:
	-$(RM) $(OUT.PYSIMP)/pysimp.dep

ifdef DO_DEPEND
dep: $(OUT.PYSIMP) $(OUT.PYSIMP)/pysimp.dep
$(OUT.PYSIMP)/pysimp.dep: $(SRC.PYSIMP)
	$(DO.DEPEND)
else
-include $(OUT.PYSIMP)/pysimp.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring cspython,$(PLUGINS) $(PLUGINS.DYNAMIC)))
