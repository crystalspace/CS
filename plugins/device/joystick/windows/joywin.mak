DESCRIPTION.joywin = Crystal Space Windows joystick plugin

#------------------------------------------------------------- rootdefines ---#
# Here are defined the makefile target descriptions for the target list.  You
# will usually only want one or two of those shown here depending upon the type
# of your module.  Delete as appropriate.  Be sure to adjust the whitespace
# following the "make joywin" string so that everything lines up correctly when
# you type "make help" at the command line.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make joywin       Make the $(DESCRIPTION.joywin)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
# Here are defined the root makefile targets.  They invoke a secondary instance
# of make which parses the rest of this file.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),roottargets)

.PHONY: joywin joywinclean

# These are all the pseudo-targets that will implicitly call your target.
# Delete the pseudo-targets which are inappropriate for your module.
all \
plugins: joywin

# Use this for a plugin target.  (Delete the app target above.)
joywin:
	$(MAKE_TARGET) MAKE_DLL=yes

# Use this to clean the build detritus from your module.
joywinclean:
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
JOYWIN.CFLAGS =

# This section is only for plugins. It adds the plugin to the list of plugins
# as a dynamic or static library depending on the value of USE_PLUGINS.
ifeq ($(USE_PLUGINS),yes)
  JOYWIN = $(OUTDLL)/joywin$(DLL)
  LIB.JOYWIN = $(foreach d,$(DEP.JOYWIN),$($d.LIB))
  LIB.JOYWIN.SPECIAL = $(LFLAGS.l)dinput
  TO_INSTALL.DYNAMIC_LIBS += $(JOYWIN)
else
  JOYWIN = $(OUT)/$(LIBPREFIX)joywin$(LIB)
  DEP.EXE += $(JOYWIN)
  LIBS.EXE += $(LFLAGS.l)dinput
  SCF.STATIC += joywin
  TO_INSTALL.STATIC_LIBS += $(JOYWIN)
endif

# This section defines files used by this module.
DIR.JOYWIN = plugins/device/joystick/windows
OUT.JOYWIN = $(OUT)/$(DIR.JOYWIN)
INF.JOYWIN = $(SRCDIR)/$(DIR.JOYWIN)/joywin.csplugin
INC.JOYWIN = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.JOYWIN)/*.h))
SRC.JOYWIN = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.JOYWIN)/*.cpp))
OBJ.JOYWIN = $(addprefix $(OUT.JOYWIN)/,$(notdir $(SRC.JOYWIN:.cpp=$O)))
# Customise the following line.
DEP.JOYWIN = CSUTIL

OUTDIRS += $(OUT.JOYWIN)

# This section is read by the Microsoft Visual C++ project file synthesis tool.
MSVC.DSP += JOYWIN
DSP.JOYWIN.NAME = joywin
DSP.JOYWIN.TYPE = plugin
DSP.JOYWIN.CFLAGS = 
DSP.JOYWIN.LFLAGS =
DSP.JOYWIN.LIBS = dinput

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
# Here are defined the targets which are run in a local instance of make,
# protected from similar-named stuff elsewhere in CS.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),targets)

.PHONY: joywin joywinclean joywincleandep

joywin: $(OUTDIRS) $(JOYWIN)
$(JOYWIN): $(OBJ.JOYWIN) $(LIB.JOYWIN)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.JOYWIN.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

# Rule to build joywin object files from joywin source code.
$(OUT.JOYWIN)/%$O: $(SRCDIR)/$(DIR.JOYWIN)/%.cpp
	$(DO.COMPILE.CPP) $(JOYWIN.CFLAGS)

clean: joywinclean
joywinclean:
	-$(RMDIR) $(JOYWIN) $(OBJ.JOYWIN) $(OUTDLL)/$(notdir $(INF.JOYWIN))

# This takes care of creating and including the dependency file.
cleandep: joywincleandep
joywincleandep:
	-$(RM) $(OUT.JOYWIN)/joywin.dep

ifdef DO_DEPEND
dep: $(OUT.JOYWIN) $(OUT.JOYWIN)/joywin.dep
$(OUT.JOYWIN)/joywin.dep: $(SRC.JOYWIN)
	$(DO.DEPEND)
else
-include $(OUT.JOYWIN)/joywin.dep
endif

endif # ifeq ($(MAKESECTION),targets)
