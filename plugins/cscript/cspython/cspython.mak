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

#TCLTK=-ltk8.0 -ltcl8.0 -L/usr/X11R6/lib -lX11
PTHREAD=-lpthread

CFLAGS.PYTHON += $(CFLAGS.I)$(PYTHON_INC)
LIBS.CSPYTHON += $(LFLAGS.l)$(notdir $(PYTHON_LIB)) \
  $(LFLAGS.L)$(PYTHON_LIB)/config $(TCLTK) $(PTHREAD)

ifeq ($(USE_SHARED_PLUGINS),yes)
  CSPYTHON = $(OUTDLL)cspython$(DLL)
  LIBS.LOCAL.CSPYTHON = $(LIBS.CSPYTHON)
  DEP.CSPYTHON = $(CSGEOM.LIB) $(CSSYS.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  CSPYTHON = $(OUT)$(LIB_PREFIX)cspy$(LIB)
  DEP.EXE += $(CSPYTHON)
  LIBS.EXE += $(LIBS.CSPYTHON)
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_PYTHON
endif
DESCRIPTION.$(CSPYTHON) = $(DESCRIPTION.cspython)

SWIG.CSPYTHON = plugins/cscript/cspython/cs_pyth.cpp
SWIG.INTERFACE = plugins/cscript/common/cs.i

SRC.CSPYTHON = $(wildcard plugins/cscript/cspython/*.cpp) $(SWIG.CSPYTHON)
OBJ.CSPYTHON = $(addprefix $(OUT),$(notdir $(SRC.CSPYTHON:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cspython cspythonclean cspythonswig

all: $(CSPYTHON.LIB)
cspython: $(OUTDIRS) $(CSPYTHON)
clean: cspythonclean

$(OUT)%$O: plugins/cspython/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PYTHON)

$(SWIG.CSPYTHON): $(SWIG.INTERFACE)
	swig -python -c++ -shadow -o $(SWIG.CSPYTHON) $(SWIG.INTERFACE)
	mv plugins/cscript/cspython/cspace.py scripts/python/

$(OUT)%$O: plugins/cscript/cspython/%.cpp
	$(DO.COMPILE.C) -w $(CFLAGS.PYTHON)

$(CSPYTHON): $(OBJ.CSPYTHON) $(DEP.CSPYTHON)
	$(DO.PLUGIN) $(LIBS.LOCAL.CSPYTHON)

cspythonclean:
	-$(RM) $(CSPYTHON) $(OBJ.CSPYTHON) $(OUTOS)cspython.dep

cspythonswig: cspythonswigclean cspython

cspythonswigclean:
	-$(RM) $(CSPYTHON) $(SWIG.CSPYTHON) $(OUT)cs_pyth.cpp

ifdef DO_DEPEND
dep: $(OUTOS)cspython.dep
$(OUTOS)cspython.dep: $(SRC.CSPYTHON)
	$(DO.DEP)
else
-include $(OUTOS)cspython.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring cspython,$(PLUGINS)))
