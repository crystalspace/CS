# Plug-in module only valid if module is listed in PLUGINS.
ifneq (,$(findstring cspython,$(PLUGINS)))

# Plugin description
DESCRIPTION.cspython = Crystal Script Python plug-in
DESCRIPTION.cspymod = Crystal Space Python module
DESCRIPTION.cspythonswig = Crystal Script Python SWIG interface

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cspython     Make the $(DESCRIPTION.cspython)$"
ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
PLUGINHELP += \
  $(NEWLINE)echo $"  make cspymod      Make the $(DESCRIPTION.cspymod)$"
endif

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cspython cspymod cspythonclean cspythonswig
ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
all plugins: cspython cspymod
else
all plugins: cspython
endif

cspython:
	$(MAKE_TARGET) MAKE_DLL=yes
ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
cspymod:
	$(MAKE_TARGET) MAKE_DLL=yes
endif
cspythonclean:
	$(MAKE_CLEAN)
cspythonswig:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PYTHON.CFLAGS += -DSWIG_GLOBAL
ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
  PYTHON.CFLAGS += -DUSE_NEW_CSPYTHON_PLUGIN
endif

ifeq ($(USE_PLUGINS),yes)
  CSPYTHON = $(OUTDLL)/cspython$(DLL)
  LIB.CSPYTHON = $(foreach d,$(DEP.CSPYTHON),$($d.LIB))
  LIB.CSPYTHON.LOCAL = $(PYTHON.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(CSPYTHON)
else
  CSPYTHON = $(OUT)/$(LIB_PREFIX)cspy$(LIB)
  DEP.EXE += $(CSPYTHON)
  LIBS.EXE += $(PYTHON.LFLAGS)
  SCF.STATIC += cspython
  TO_INSTALL.STATIC_LIBS += $(CSPYTHON)
endif

ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
  CSPYMOD = scripts/python/_cspace$(DLL)
  LIB.CSPYMOD = $(LIB.CSPYTHON)
  LIB.CSPYMOD.LOCAL = $(LIB.CSPYTHON.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(CSPYMOD)
endif

TO_INSTALL.EXE += python.cex

ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
SWIG.INTERFACE = include/ivaria/cspace.i
SWIG.INTERFACE.SRC = include/ivaria/cspace.i include/ivaria/pythoncs.i
else
SWIG.INTERFACE = include/ivaria/cs.i
SWIG.INTERFACE.SRC = include/ivaria/cs.i
endif
SWIG.CSPYTHON = plugins/cscript/cspython/cs_pyth.cpp
SWIG.CSPYTHON.OBJ = $(addprefix $(OUT)/,$(notdir $(SWIG.CSPYTHON:.cpp=$O)))

TRASH.CSPYTHON = $(wildcard $(addprefix scripts/python/,*.pyc *.pyo))

INC.CSPYTHON = $(wildcard plugins/cscript/cspython/*.h)
SRC.CSPYTHON = \
  $(filter-out plugins/cscript/cspython/cspymod.cpp, $(sort $(wildcard plugins/cscript/cspython/*.cpp) $(SWIG.CSPYTHON)))
OBJ.CSPYTHON = $(addprefix $(OUT)/, $(notdir $(SRC.CSPYTHON:.cpp=$O)))
DEP.CSPYTHON = CSTOOL CSGEOM CSSYS CSUTIL CSSYS CSUTIL

INC.CSPYMOD =
SRC.CSPYMOD = plugins/cscript/cspython/cspymod.cpp $(SWIG.CSPYTHON)
OBJ.CSPYMOD = $(addprefix $(OUT)/, $(notdir $(SRC.CSPYMOD:.cpp=$O)))
DEP.CSPYMOD = $(DEP.CSPYTHON)

MSVC.DSP += CSPYTHON
DSP.CSPYTHON.NAME = cspython
DSP.CSPYTHON.TYPE = plugin
DSP.CSPYTHON.RESOURCES = $(SWIG.INTERFACE)
DSP.CSPYTHON.CFLAGS = /D "SWIG_GLOBAL"

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cspython cspymod cspythonclean cspythonswig csjavaswigclean

ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
all: $(CSPYTHON.LIB) $(CSPYMOD.LIB)
else
all: $(CSPYTHON.LIB)
endif
cspython: $(OUTDIRS) $(CSPYTHON) python.cex
ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
cspymod: $(OUTDIRS) $(CSPYMOD) python.cex
endif
clean: cspythonclean

$(SWIG.CSPYTHON.OBJ): $(SWIG.CSPYTHON)
	$(filter-out -W -Wunused -Wall -Wmost,$(DO.COMPILE.CPP) $(PYTHON.CFLAGS))

$(OUT)/%$O: plugins/cscript/cspython/%.cpp
	$(DO.COMPILE.CPP) $(PYTHON.CFLAGS)

$(OUT)/%$O: plugins/cscript/cspython/%.c
	$(DO.COMPILE.C) $(PYTHON.CFLAGS)

ifeq (,$(SWIGBIN))
  SWIGBIN = swig
endif

ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
  SWIGFLAGS=-python -c++ -shadow -Iinclude/ -DUSE_NEW_CSPYTHON_PLUGIN
else
  SWIGFLAGS=-python -c++ -docstring -dascii -Sbefore -shadow -Iinclude/
endif

$(SWIG.CSPYTHON): $(SWIG.INTERFACE.SRC)
	$(SWIGBIN) $(SWIGFLAGS) -o $(SWIG.CSPYTHON) $(SWIG.INTERFACE)
	$(MV) plugins/cscript/cspython/cspace.py scripts/python/

python.cex: plugins/cscript/cspython/python.cin
	@echo Generate python cs-config extension...
	@echo $"#!/bin/sh$" > python.cex
	@echo $"# WARNING: This file is generated automatically by cspython.mak$" >> python.cex
	@echo $"PYTH_LIBS="$(LIB.CSPYTHON.LOCAL)"$" >> python.cex
	@echo $"PYTH_CFLAGS="$(PYTHON.CFLAGS)"$"    >> python.cex
	@echo $"PYTH_CXXFLAGS="$(PYTHON.CFLAGS)"$"  >> python.cex
	@echo $"PYTH_DEPS=""$"		            >> python.cex
	@cat plugins/cscript/cspython/python.cin    >> python.cex

$(CSPYTHON): $(OBJ.CSPYTHON) $(LIB.CSPYTHON)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSPYTHON.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)

ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)
$(CSPYMOD): $(OBJ.CSPYMOD) $(LIB.CSPYMOD)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSPYMOD.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)
endif

cspythonclean:
	-$(RM) $(CSPYTHON) $(OBJ.CSPYTHON) $(TRASH.CSPYTHON) python.cex

cspythonswig: cspythonswigclean cspython

cspythonswigclean:
	-$(RM) $(CSPYTHON) $(CSPYMOD) $(SWIG.CSPYTHON) $(OUT)/cs_pyth.cpp

ifeq ($(USE_NEW_CSPYTHON_PLUGIN),yes)

ifdef DO_DEPEND
dep: $(OUTOS)/cspython.dep
$(OUTOS)/cspython.dep: $(SRC.CSPYTHON)
	$(DO.DEP1) $(PYTHON.CFLAGS) $(DO.DEP2)
dep: $(OUTOS)/cspymod.dep
$(OUTOS)/cspymod.dep: $(SRC.CSPYMOD)
	$(DO.DEP1) $(PYTHON.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/cspython.dep
-include $(OUTOS)/cspymod.dep
endif

else

ifdef DO_DEPEND
dep: $(OUTOS)/cspython.dep
$(OUTOS)/cspython.dep: $(SRC.CSPYTHON)
	$(DO.DEP1) $(PYTHON.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/cspython.dep
endif

endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring cspython,$(PLUGINS)))
