DESCRIPTION.joytest = Crystal Space Joystick test application

#------------------------------------------------------------- rootdefines ---#
# Here are defined the makefile target descriptions for the target list.  You
# will usually only want one or two of those shown here depending upon the type
# of your module.  Delete as appropriate.  Be sure to adjust the whitespace
# following the "make joytest" string so that everything lines up correctly when
# you type "make help" at the command line.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make joytest       Make the $(DESCRIPTION.joytest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
# Here are defined the root makefile targets.  They invoke a secondary instance
# of make which parses the rest of this file.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),roottargets)

.PHONY: joytest joytestclean

# These are all the pseudo-targets that will implicitly call your target.
# Delete the pseudo-targets which are inappropriate for your module.
all \
apps: joytest

joytest:
	$(MAKE_APP)

joytestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
# Here are (re)defined module-dependent variables.  You probably don't need
# this but if you think you do, see the first 40 or so lines of mk/cs.mak.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),defines)

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
# Here are (re)defined module-independent variables.  See mk/cs.mak if you need
# any clarification.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),postdefines)

# If you need to add any special flags, or link with any external libraries,
# do it here.
JOYTEST.CFLAGS =

# This section is only for applications.
JOYTEST.EXE = joytest$(EXE)
LIB.JOYTEST = $(foreach d,$(DEP.JOYTEST),$($d.LIB))

# This section defines files used by this module.
DIR.JOYTEST = apps/tests/joytest
OUT.JOYTEST = $(OUT)/$(DIR.JOYTEST)
INF.JOYTEST = $(SRCDIR)/$(DIR.JOYTEST)/joytest.csplugin
INC.JOYTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.JOYTEST)/*.h))
SRC.JOYTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.JOYTEST)/*.cpp))
OBJ.JOYTEST = $(addprefix $(OUT.JOYTEST)/,$(notdir $(SRC.JOYTEST:.cpp=$O)))
# Customise the following line.
DEP.JOYTEST = CSTOOL CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.JOYTEST)

# This section is read by the Microsoft Visual C++ project file synthesis tool.
MSVC.DSP += JOYTEST
DSP.JOYTEST.NAME = joytest
DSP.JOYTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
# Here are defined the targets which are run in a local instance of make,
# protected from similar-named stuff elsewhere in CS.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),targets)

.PHONY: joytest joytestclean joytestcleandep

build.joytest: $(OUTDIRS) $(JOYTEST.EXE)

$(JOYTEST.EXE): $(DEP.EXE) $(OBJ.JOYTEST) $(LIB.JOYTEST)
	$(DO.LINK.EXE)

# Rule to build joytest object files from joytest source code.
$(OUT.JOYTEST)/%$O: $(SRCDIR)/$(DIR.JOYTEST)/%.cpp
	$(DO.COMPILE.CPP) $(JOYTEST.CFLAGS)

# Cleanup generated resources for applications (omit for plugins)
clean: joytestclean
joytestclean:
	-$(RMDIR) $(JOYTEST.EXE) $(OBJ.JOYTEST) joytest.txt

# This takes care of creating and including the dependency file.
cleandep: joytestcleandep
joytestcleandep:
	-$(RM) $(OUT.JOYTEST)/joytest.dep

ifdef DO_DEPEND
dep: $(OUT.JOYTEST) $(OUT.JOYTEST)/joytest.dep
$(OUT.JOYTEST)/joytest.dep: $(SRC.JOYTEST)
	$(DO.DEPEND)
else
-include $(OUT.JOYTEST)/joytest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
