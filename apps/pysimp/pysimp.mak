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

vpath %.cpp apps/pysimp apps/support

PYSIMP.EXE = pysimp$(EXE)
INC.PYSIMP = $(wildcard apps/pysimp/*.h)
SRC.PYSIMP = $(wildcard apps/pysimp/*.cpp)
OBJ.PYSIMP = $(addprefix $(OUT)/,$(notdir $(SRC.PYSIMP:.cpp=$O)))
DEP.PYSIMP = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.PYSIMP = $(foreach d,$(DEP.PYSIMP),$($d.LIB))

#TO_INSTALL.EXE += $(PYSIMP.EXE)

MSVC.DSP += PYSIMP
DSP.PYSIMP.NAME = pysimp
DSP.PYSIMP.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.pysimp pysimpclean

all: $(PYSIMP.EXE)
build.pysimp: $(OUTDIRS) $(PYSIMP.EXE)
clean: pysimpclean

$(PYSIMP.EXE): $(DEP.EXE) $(OBJ.PYSIMP) $(LIB.PYSIMP)
	$(DO.LINK.EXE)

pysimpclean:
	-$(RMDIR) $(PYSIMP.EXE) $(OBJ.PYSIMP)

ifdef DO_DEPEND
dep: $(OUTOS)/pysimp.dep
$(OUTOS)/pysimp.dep: $(SRC.PYSIMP)
	$(DO.DEP)
else
-include $(OUTOS)/pysimp.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring cspython,$(PLUGINS) $(PLUGINS.DYNAMIC)))
