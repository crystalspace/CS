# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring cspython,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.pysimple = Crystal Space Python example

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make pysimple     Make the $(DESCRIPTION.pysimple)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pysimple pysimpleclean

all apps: pysimple
pysimple:
	$(MAKE_TARGET)
pysimpleclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/pysimp apps/support

PYSIMP.EXE = pysimp$(EXE)
INC.PYSIMP = $(wildcard apps/pysimp/*.h)
SRC.PYSIMP = $(wildcard apps/pysimp/*.cpp)
OBJ.PYSIMP = $(addprefix $(OUT),$(notdir $(SRC.PYSIMP:.cpp=$O)))
DEP.PYSIMP = \
  CSPARSER CSTOOL CSENGINE CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.PYSIMP = $(foreach d,$(DEP.PYSIMP),$($d.LIB))

#TO_INSTALL.EXE += $(PYSIMP.EXE)

MSVC.DSP += PYSIMP
DSP.PYSIMP.NAME = pysimp
DSP.PYSIMP.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: pysimple pysimpleclean

all: $(PYSIMP.EXE)
pysimple: $(OUTDIRS) $(PYSIMP.EXE)
clean: pysimpleclean

$(PYSIMP.EXE): $(DEP.EXE) $(OBJ.PYSIMP) $(LIB.PYSIMP)
	$(DO.LINK.EXE)

pysimpleclean:
	-$(RM) $(PYSIMP.EXE) $(OBJ.PYSIMP)

ifdef DO_DEPEND
dep: $(OUTOS)pysimp.dep
$(OUTOS)pysimp.dep: $(SRC.PYSIMP)
	$(DO.DEP)
else
-include $(OUTOS)pysimp.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring cspython,$(PLUGINS) $(PLUGINS.DYNAMIC)))
