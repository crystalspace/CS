# Plug-in module only valid if module is listed in PLUGINS.
ifneq (,$(findstring cspython,$(PLUGINS)))

# Plugin description
DESCRIPTION.cspython = Crystal Script Python plug-in
DESCRIPTION.pythmod = Crystal Space Python module
DESCRIPTION.swigpythgen = SWIG Python files (forcibly)
DESCRIPTION.swigpythinst = SWIG-generated Python files
DESCRIPTION.swigpyth = SWIG-generated Python files
DESCRIPTION.cspythonmaintainer = SWIG-generated Python files

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cspython     Make the $(DESCRIPTION.cspython)$"
ifneq (,$(CMD.SWIG))
PSEUDOHELP += \
  $(NEWLINE)echo $"  make swigpythgen  Make the $(DESCRIPTION.swigpythgen)$" \
  $(NEWLINE)echo $"  make swigpythinst Install $(DESCRIPTION.swigpythinst)$"
endif
ifneq ($(MAKE_PYTHON_MODULE),no)
PLUGINHELP += \
  $(NEWLINE)echo $"  make pythmod      Make the $(DESCRIPTION.pythmod)$"
endif

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cspython pythmod cspythonclean swigpythinst swigpythgen swigpythclean \
  cspythonmaintainerclean
ifneq ($(MAKE_PYTHON_MODULE),no)
all: cspython pythmod
plugins: cspython
else
all plugins: cspython
endif

cspython:
	$(MAKE_TARGET) MAKE_DLL=yes CSPYTHON_MSVC_EXCLUDE=no
cspythonclean:
	$(MAKE_CLEAN)
ifneq ($(MAKE_PYTHON_MODULE),no)
pythmod:
	$(MAKE_TARGET)
pythmodclean:
	$(MAKE_CLEAN)
endif
ifneq (,$(CMD.SWIG))
swigpythgen:
	$(MAKE_TARGET)
swigpythinst:
	$(MAKE_TARGET)
endif
swigpythclean:
	$(MAKE_CLEAN)
cspythonmaintainerclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

PYTHON.CFLAGS += -DSWIG_GLOBAL

PYTHMOD.LFLAGS.PLATFORM += \
  $(foreach l,$(PYTHMOD.LIBS.PLATFORM),$(LFLAGS.l)$l) $(LIBBFD.LFLAGS)

ifeq ($(USE_PLUGINS),yes)
  CSPYTHON = $(OUTDLL)/cspython$(DLL)
  LIB.CSPYTHON = $(foreach d,$(DEP.CSPYTHON),$($d.LIB))
  LIB.CSPYTHON.LOCAL = $(PYTHON.LFLAGS) $(PYTHMOD.LFLAGS.PLATFORM)
  TO_INSTALL.DYNAMIC_LIBS += $(CSPYTHON)
else
  CSPYTHON = $(OUT)/$(LIB_PREFIX)cspython$(LIB)
  DEP.EXE += $(CSPYTHON)
  LIBS.EXE += $(PYTHON.LFLAGS) $(PYTHMOD.LFLAGS.PLATFORM)
  SCF.STATIC += cspython
  TO_INSTALL.STATIC_LIBS += $(CSPYTHON)
endif

PYTHMOD.BUILDBASE=$(OUT)/python
PYTHMOD.INSTALLDIR=$(OUTPROC)/python
PYTHMOD = $(PYTHMOD.INSTALLDIR)/_cspace$(DLL)
LIB.PYTHMOD = $(LIB.CSPYTHON)
LIB.PYTHMOD.LOCAL = $(LIB.CSPYTHON.LOCAL)

ifneq ($(MAKE_PYTHON_MODULE),no)
TO_INSTALL.DYNAMIC_LIBS += $(PYTHMOD)
endif

TO_INSTALL.EXE += python.cex

SWIG.CSPYTHON.OUTDIR = $(OUTDERIVED)/python
SWIG.CSPYTHON.INTERFACE = $(SRCDIR)/include/ivaria/cspace.i
SWIG.CSPYTHON = $(SWIG.CSPYTHON.OUTDIR)/cs_pyth.cpp
SWIG.CSPYTHON.CVS = $(SRCDIR)/plugins/cscript/cspython/cs_pyth.cpp
SWIG.CSPYTHON.OBJ = $(addprefix $(OUT)/,$(notdir $(SWIG.CSPYTHON:.cpp=$O)))
SWIG.CSPYTHON.PY = $(SWIG.CSPYTHON.OUTDIR)/cspace.py
SWIG.CSPYTHON.PY.CVS = $(SRCDIR)/scripts/python/cspace.py

TRASH.CSPYTHON = $(wildcard $(addprefix $(SRCDIR)/scripts/python/,*.pyc *.pyo))

INF.CSPYTHON = $(SRCDIR)/plugins/cscript/cspython/cspython.csplugin
INC.CSPYTHON = $(wildcard $(addprefix $(SRCDIR)/,plugins/cscript/cspython/*.h))
ifeq ($(DO_MSVCGEN),yes)
SRC.CSPYTHON = $(filter-out $(SRCDIR)/plugins/cscript/cspython/pythmod.cpp, \
  $(wildcard $(SRCDIR)/plugins/cscript/cspython/*.cpp))
else
SRC.CSPYTHON = $(SWIG.CSPYTHON) $(filter-out \
  $(SWIG.CSPYTHON.CVS) $(SRCDIR)/plugins/cscript/cspython/pythmod.cpp, \
  $(wildcard $(SRCDIR)/plugins/cscript/cspython/*.cpp))
endif
OBJ.CSPYTHON = $(addprefix $(OUT)/, $(notdir $(SRC.CSPYTHON:.cpp=$O)))
DEP.CSPYTHON = CSTOOL CSGFX CSGEOM CSUTIL

INC.PYTHMOD =
SRC.PYTHMOD = $(SRCDIR)/plugins/cscript/cspython/pythmod.cpp $(SWIG.CSPYTHON)
OBJ.PYTHMOD = $(addprefix $(OUT)/, $(notdir $(SRC.PYTHMOD:.cpp=$O)))
DEP.PYTHMOD = $(DEP.CSPYTHON)

OUTDIRS += $(SWIG.CSPYTHON.OUTDIR) $(PYTHMOD.BUILDBASE) $(PYTHMOD.INSTALLDIR)

MSVC.DSP += CSPYTHON
DSP.CSPYTHON.NAME = cspython
DSP.CSPYTHON.TYPE = plugin
DSP.CSPYTHON.RESOURCES = $(SWIG.CSPYTHON.INTERFACE)
DSP.CSPYTHON.CFLAGS = /D "SWIG_GLOBAL"

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cspython pythmod cspythonclean swigpythinst swigpythgen swigpythclean \
  cspythonmaintainerclean

ifneq ($(MAKE_PYTHON_MODULE),no)
all: $(CSPYTHON.LIB) $(PYTHMOD.LIB)
else
all: $(CSPYTHON.LIB)
endif
cspython: $(OUTDIRS) $(CSPYTHON) python.cex
ifneq ($(MAKE_PYTHON_MODULE),no)
pythmod: $(OUTDIRS) $(SWIG.CSPYTHON.PY) $(PYTHMOD)
endif
clean: cspythonclean pythmodclean
maintainerclean: cspythonmaintainerclean

$(SWIG.CSPYTHON.OBJ): $(SWIG.CSPYTHON)
	$(filter-out -W -Wunused -Wall -Wmost,$(DO.COMPILE.CPP) $(PYTHON.CFLAGS))

$(OUT)/%$O: $(SRCDIR)/plugins/cscript/cspython/%.cpp
	$(DO.COMPILE.CPP) $(PYTHON.CFLAGS)

$(OUT)/%$O: $(SRCDIR)/plugins/cscript/cspython/%.c
	$(DO.COMPILE.C) $(PYTHON.CFLAGS)

SWIG.FLAGS.CSPYTHON=-python -c++ -shadow -I$(SRCDIR)/include
SWIG.CSPYTHON.DEPS=\
	$(SRCDIR)/include/ivaria/pythpre.i \
	$(SRCDIR)/include/ivaria/pythpost.i

ifeq (,$(CMD.SWIG))
$(SWIG.CSPYTHON): $(SWIG.CSPYTHON.CVS)
	-$(RM) $(SWIG.CSPYTHON)
	$(CP) $(SWIG.CSPYTHON.CVS) $(SWIG.CSPYTHON)
$(SWIG.CSPYTHON.PY): $(SWIG.CSPYTHON.PY.CVS)
	-$(RM) $(SWIG.CSPYTHON.PY)
	$(CP) $(SWIG.CSPYTHON.PY.CVS) $(SWIG.CSPYTHON.PY)
else
$(SWIG.CSPYTHON) $(SWIG.CSPYTHON.PY): \
  $(SWIG.CSPYTHON.INTERFACE) $(SWIG.CSPYTHON.DEPS)
	$(CMD.SWIG) $(SWIG.FLAGS) $(SWIG.FLAGS.CSPYTHON) -o $(SWIG.CSPYTHON) \
	$(SWIG.CSPYTHON.INTERFACE)
	$(SED) '/$(BUCK)Header:/d' < $(SWIG.CSPYTHON) > $(SWIG.CSPYTHON).sed
	$(RM) $(SWIG.CSPYTHON)
	$(MV) $(SWIG.CSPYTHON).sed $(SWIG.CSPYTHON)

swigpythgen: $(OUTDIRS) swigpythclean $(SWIG.CSPYTHON)

swigpythinst: $(OUTDIRS) $(SWIG.CSPYTHON.CVS) $(SWIG.CSPYTHON.PY.CVS)

$(SWIG.CSPYTHON.CVS): $(SWIG.CSPYTHON)
	-$(RM) $@
	$(CP) $(SWIG.CSPYTHON) $@

$(SWIG.CSPYTHON.PY.CVS): $(SWIG.CSPYTHON.PY)
	-$(RM) $@
	$(CP) $(SWIG.CSPYTHON.PY) $@
endif

cspythonclean: swigpythclean
	-$(RMDIR) $(CSPYTHON) $(OBJ.CSPYTHON) \
	$(OUTDLL)/$(notdir $(INF.CSPYTHON)) $(TRASH.CSPYTHON) python.cex \
	$(SWIG.CSPYTHON) $(SWIG.CSPYTHON.PY) $(SWIG.CSPYTHON.OUTDIR)

python.cex: $(SRCDIR)/plugins/cscript/cspython/python.cin
	@echo Generate python cs-config extension...
	@echo $"#!/bin/sh$" > python.cex
	@echo $"# WARNING: This file is generated automatically by cspython.mak$" >> python.cex
	@echo $"PYTH_LIBS="$(LIB.CSPYTHON.LOCAL)"$" >> python.cex
	@echo $"PYTH_CFLAGS="$(PYTHON.CFLAGS)"$"    >> python.cex
	@echo $"PYTH_CXXFLAGS="$(PYTHON.CFLAGS)"$"  >> python.cex
	@echo $"PYTH_DEPS=""$"		            >> python.cex
	@cat $(SRCDIR)/plugins/cscript/cspython/python.cin >> python.cex

$(CSPYTHON): $(OBJ.CSPYTHON) $(LIB.CSPYTHON)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSPYTHON.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)

ifeq ($(PYTHON.DISTUTILS),yes)
$(PYTHMOD): $(SRC.PYTHMOD) $(LIB.PYTHMOD)
	$(PYTHON) $(SRCDIR)/plugins/cscript/cspython/pythmod_setup.py \
	$(CXX) \
	$(SWIG.CSPYTHON.OUTDIR) \
	$(SRCDIR) \
	$(SRCDIR)/include ./include $(OUT) -- \
	$(OUT) -- \
	$(patsubst $(LIB_PREFIX)%,%,$(patsubst %$(LIB_SUFFIX),%, \
	$(notdir $(LIB.CSPYTHON)))) $(PYTHMOD.LIBS.PLATFORM) -- \
	$(CFLAGS.SYSTEM.MANDATORY) -- \
	$(PYTHMOD.LFLAGS.PLATFORM) -- \
	build -q --build-base=$(PYTHMOD.BUILDBASE) \
	install -q --install-lib=$(PYTHMOD.INSTALLDIR)
else
$(PYTHMOD):
	@echo $(DESCRIPTION.pythmod)" not supported: distutils not available!"
endif

pythmodclean:
	-$(RMDIR) $(SWIG.CSPYTHON) $(SWIG.CSPYTHON.PY) $(PYTHMOD) \
	$(PYTHMOD.BUILDBASE) $(PYTHMOD.INSTALLDIR) $(SWIG.CSPYTHON.OUTDIR)

swigpythclean:
	-$(RM) $(SWIG.CSPYTHON) $(SWIG.CSPYTHON.PY)

cspythonmaintainerclean: cspythonclean
	-$(RM) $(SWIG.CSPYTHON.CVS) $(SWIG.CSPYTHON.PY.CVS)

ifdef DO_DEPEND
dep: $(OUTOS)/cspython.dep
$(OUTOS)/cspython.dep: $(SRC.CSPYTHON)
	$(DO.DEP1) $(PYTHON.CFLAGS) $(DO.DEP2)
ifneq ($(MAKE_PYTHON_MODULE),no)
dep: $(OUTOS)/pythmod.dep
$(OUTOS)/pythmod.dep: $(SRC.PYTHMOD)
	$(DO.DEP1) $(PYTHON.CFLAGS) $(DO.DEP2)
endif
else # ifdef DO_DEPEND
-include $(OUTOS)/cspython.dep
ifneq ($(MAKE_PYTHON_MODULE),no)
-include $(OUTOS)/pythmod.dep
endif
endif # ifdef/else DO_DEPEND

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring cspython,$(PLUGINS)))
