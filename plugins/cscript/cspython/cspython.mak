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
ifneq (,$(SWIGBIN))
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
ifneq (,$(SWIGBIN))
swigpythgen:
	$(MAKE_TARGET)
swigpythinst:
	$(MAKE_TARGET) DO_SWIGPYTHINST=yes
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
ifneq ($(CSPYTHON_MSVC_EXCLUDE),yes)
SWIG.CSPYTHON = $(SWIG.CSPYTHON.OUTDIR)/cs_pyth.cpp
endif
SWIG.CSPYTHON.CVS = $(SRCDIR)/plugins/cscript/cspython/cs_pyth.cpp
SWIG.CSPYTHON.OBJ = $(addprefix $(OUT)/,$(notdir $(SWIG.CSPYTHON:.cpp=$O)))
SWIG.CSPYTHON.PY = $(SWIG.CSPYTHON.OUTDIR)/cspace.py
SWIG.CSPYTHON.PY.CVS = $(SRCDIR)/scripts/python/cspace.py

TRASH.CSPYTHON = $(wildcard $(addprefix $(SRCDIR)/scripts/python/,*.pyc *.pyo))

INF.CSPYTHON = $(SRCDIR)/plugins/cscript/cspython/cspython.csplugin
INC.CSPYTHON = $(wildcard $(addprefix $(SRCDIR)/,plugins/cscript/cspython/*.h))
SRC.CSPYTHON = $(filter-out $(SRCDIR)/plugins/cscript/cspython/pythmod.cpp, \
  $(sort $(wildcard $(SRCDIR)/plugins/cscript/cspython/*.cpp) $(SWIG.CSPYTHON)))
OBJ.CSPYTHON = $(addprefix $(OUT)/, $(notdir $(SRC.CSPYTHON:.cpp=$O)))
DEP.CSPYTHON = CSTOOL CSGFX CSGEOM CSUTIL CSUTIL

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

SWIGFLAGS=-python -c++ -shadow -I$(SRCDIR)/include
SWIG.CSPYTHON.DEPS=\
	$(SRCDIR)/include/ivaria/pythpre.i \
	$(SRCDIR)/include/ivaria/pythpost.i

ifneq (,$(SWIGBIN))
$(SWIG.CSPYTHON) $(SWIG.CSPYTHON.PY): \
  $(SWIG.CSPYTHON.INTERFACE) $(SWIG.CSPYTHON.DEPS)
	$(SWIGBIN) $(SWIGFLAGS) -o $(SWIG.CSPYTHON) $(SWIG.CSPYTHON.INTERFACE)
	$(SED) '/$(BUCK)Header:/d' < $(SWIG.CSPYTHON) > $(SWIG.CSPYTHON).sed
	$(RM) $(SWIG.CSPYTHON)
	$(MV) $(SWIG.CSPYTHON).sed $(SWIG.CSPYTHON)
else
$(SWIG.CSPYTHON): $(SWIG.CSPYTHON.CVS)
	-$(RM) $(SWIG.CSPYTHON)
	$(CP) $(SWIG.CSPYTHON.CVS) $(SWIG.CSPYTHON)
$(SWIG.CSPYTHON.PY): $(SWIG.CSPYTHON.PY.CVS)
	-$(RM) $(SWIG.CSPYTHON.PY)
	$(CP) $(SWIG.CSPYTHON.PY.CVS) $(SWIG.CSPYTHON.PY)
endif

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
	$(SWIG.CSPYTHON.OUTDIR) $(SRCDIR) $(SRCDIR)/include ./include $(OUT) \
	-- $(OUT) $(PYTHMOD.LIBS.PLATFORM) -- build -q \
	--build-base=$(PYTHMOD.BUILDBASE) install -q \
	--install-lib=$(PYTHMOD.INSTALLDIR)
else
$(PYTHMOD):
	@echo $(DESCRIPTION.pythmod)" not supported: distutils not available!"
endif

pythmodclean:
	-$(RMDIR) $(SWIG.CSPYTHON) $(SWIG.CSPYTHON.PY) $(PYTHMOD) \
	$(PYTHMOD.BUILDBASE) $(PYTHMOD.INSTALLDIR) $(SWIG.CSPYTHON.OUTDIR)

ifeq ($(DO_SWIGPYTHINST),yes)
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

swigpythgen: $(OUTDIRS) swigpythclean $(SWIG.CSPYTHON)

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
