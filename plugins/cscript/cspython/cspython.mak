# Plug-in module only valid if module is listed in PLUGINS.
ifneq (,$(findstring cspython,$(PLUGINS)))

# Plugin description
DESCRIPTION.cspython = Crystal Script Python plug-in
DESCRIPTION.cspythonswig = Crystal Script Python SWIG interface

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cspython     Make the $(DESCRIPTION.cspython)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cspython cspythonclean cspythonswig
all plugins: cspython

cspython:
	$(MAKE_TARGET) MAKE_DLL=yes
cspythonclean:
	$(MAKE_CLEAN)
cspythonswig: 
	$(MAKE_TARGET) MAKE_DLL=yes	

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# For custom compiled python with tk/tcl
#TCLTK=-ltk8.0 -ltcl8.0 -L/usr/X11R6/lib -lX11

# For python 2.0+
#LIBUTIL=-lutil

ifneq ($(OS),NEXT)
PTHREAD=-lpthread
endif

ifneq (,$(strip $(PYTHON_INC)))
  CFLAGS.PYTHON += $(CFLAGS.I)$(PYTHON_INC)
endif

CFLAGS.PYTHON += -DSWIG_GLOBAL

ifeq (,$(strip $(LIBS.CSPYTHON.SYSTEM)))
ifneq (,$(strip $(PYTHON_LIB)))
  # PYTHON_LIB points at the Python library directory (which is often
  # /usr/lib/python1.5).  The actual static link library usually resides in a
  # "config" subdirectory of the library directory.  The library's name is the
  # same as the directory, thus in this example, the library would be called
  # "libpython1.5.a".
  LIBS.CSPYTHON.SYSTEM = \
    $(LFLAGS.L)$(PYTHON_LIB)/config $(LFLAGS.L)$(PYTHON_LIB) \
    $(LFLAGS.l)$(notdir $(PYTHON_LIB)) $(TCLTK) $(PTHREAD) $(LIBUTIL)
endif
endif

ifeq ($(USE_PLUGINS),yes)
  CSPYTHON = $(OUTDLL)cspython$(DLL)
  LIB.CSPYTHON = $(foreach d,$(DEP.CSPYTHON),$($d.LIB))
  LIB.CSPYTHON.LOCAL = $(LIBS.CSPYTHON.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(CSPYTHON)
else
  CSPYTHON = $(OUT)$(LIB_PREFIX)cspy$(LIB)
  DEP.EXE += $(CSPYTHON)
  LIBS.EXE += $(LIBS.CSPYTHON.SYSTEM)
  SCF.STATIC += cspython
  TO_INSTALL.STATIC_LIBS += $(CSPYTHON)
endif

SWIG.INTERFACE = include/ivaria/cs.i
SWIG.CSPYTHON = plugins/cscript/cspython/cs_pyth.cpp
SWIG.CSPYTHON.OBJ = $(addprefix $(OUT),$(notdir $(SWIG.CSPYTHON:.cpp=$O)))

TRASH.CSPYTHON = $(wildcard $(addprefix scripts/python/,*.pyc *.pyo))

INC.CSPYTHON = $(wildcard plugins/cscript/cspython/*.h)
SRC.CSPYTHON = \
  $(sort $(wildcard plugins/cscript/cspython/*.cpp) $(SWIG.CSPYTHON))
OBJ.CSPYTHON = $(addprefix $(OUT),$(notdir $(SRC.CSPYTHON:.cpp=$O)))
DEP.CSPYTHON = CSGEOM CSSYS CSUTIL CSSYS CSUTIL

MSVC.DSP += CSPYTHON
DSP.CSPYTHON.NAME = cspython
DSP.CSPYTHON.TYPE = plugin
DSP.CSPYTHON.RESOURCES = $(SWIG.INTERFACE)
DSP.CSPYTHON.CFLAGS = /D "SWIG_GLOBAL"

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cspython cspythonclean cspythonswig csjavaswigclean

all: $(CSPYTHON.LIB)
cspython: $(OUTDIRS) $(CSPYTHON)
clean: cspythonclean

$(SWIG.CSPYTHON.OBJ): $(SWIG.CSPYTHON)
	$(filter-out -W -Wunused -Wall,$(DO.COMPILE.CPP) $(CFLAGS.PYTHON))

$(OUT)%$O: plugins/cscript/cspython/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PYTHON)

$(OUT)%$O: plugins/cscript/cspython/%.c
	$(DO.COMPILE.C) $(CFLAGS.PYTHON)

$(SWIG.CSPYTHON): $(SWIG.INTERFACE)
	swig -python -c++ -shadow -o $(SWIG.CSPYTHON) $(SWIG.INTERFACE)
	mv plugins/cscript/cspython/cspace.py scripts/python/

$(CSPYTHON): $(OBJ.CSPYTHON) $(LIB.CSPYTHON)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSPYTHON.LOCAL) \
	$(DO.PLUGIN.POSTAMBLE)

cspythonclean:
	-$(RM) $(CSPYTHON) $(OBJ.CSPYTHON) $(TRASH.CSPYTHON)

cspythonswig: cspythonswigclean cspython

cspythonswigclean:
	-$(RM) $(CSPYTHON) $(SWIG.CSPYTHON) $(OUT)cs_pyth.cpp

ifdef DO_DEPEND
dep: $(OUTOS)cspython.dep
$(OUTOS)cspython.dep: $(SRC.CSPYTHON)
	$(DO.DEP1) $(CFLAGS.PYTHON) $(DO.DEP2)
else
-include $(OUTOS)cspython.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring cspython,$(PLUGINS)))
