# Plug-in module only valid if module is listed in PLUGINS.
ifneq (,$(findstring cspython,$(PLUGINS)))

# Application description
DESCRIPTION.pysimple = Crystal Space Python simple example

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make pysimple     Make the $(DESCRIPTION.pysimple)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pysimple

all apps: pysimple
pysimple:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/pysimp apps/support

PYSIMP.EXE=pysimp$(EXE)
SRC.PYSIMP = $(wildcard apps/pysimp/*.cpp) \
  apps/support/static.cpp
OBJ.PYSIMP = $(addprefix $(OUT),$(notdir $(SRC.PYSIMP:.cpp=$O)))
DESCRIPTION.$(PYSIMP.EXE) = $(DESCRIPTION.pysimple)

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: pysimple pysimpleclean

all: $(PYSIMP.EXE)
pysimple: $(OUTDIRS) $(PYSIMP.EXE)
clean: pysimpleclean

$(PYSIMP.EXE): $(DEP.EXE) $(OBJ.PYSIMP) \
  $(CSPARSER.LIB) $(CSENGINE.LIB) $(CSTERR.LIB) $(CSSCRIPT.LIB) \
  $(CSSFXLDR.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB) $(CSGEOM.LIB) \
  $(CSINPUT.LIB) $(CSOBJECT.LIB) $(CSUTIL.LIB)
	$(DO.LINK.EXE)

pysimpleclean:
	-$(RM) $(PYSIMP.EXE)

ifdef DO_DEPEND
depend: $(OUTOS)pysimp.dep
$(OUTOS)pysimp.dep: $(SRC.PYSIMP)
	$(DO.DEP)
else
-include $(OUTOS)pysimp.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring cspython,$(PLUGINS)))
