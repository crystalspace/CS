#-----------------------------------------------------------------------------#
# Submakefile for the Crystal Space Foo Bar
# Modified from the template submakefile
# (C) 1337 Joe Bloggs
#
# Insert license header here
#
#-----------------------------------------------------------------------------#

#
# When finished, add your submakefile's filename to user.mak.
#

DESCRIPTION.foobar = Crystal Space Foo Bar

#------------------------------------------------------------- rootdefines ---#
# Here are defined the makefile target descriptions for the target list.  You
# will usually only want one or two of those shown here depending upon the type
# of your module.  Delete as appropriate.  Be sure to adjust the whitespace
# following the "make foobar" string so that everything lines up correctly when
# you type "make help" at the command line.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),rootdefines)

# Platform type
SYSHELP += \
  $(NEWLINE)@echo $"  make foobar       Make the $(DESCRIPTION.foobar)$"

# Driver
DRIVERHELP += \
  $(NEWLINE)@echo $"  make foobar       Make the $(DESCRIPTION.foobar)$"

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make foobar       Make the $(DESCRIPTION.foobar)$"

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make foobar       Make the $(DESCRIPTION.foobar)$"

# Static library
LIBHELP += \
  $(NEWLINE)@echo $"  make foobar       Make the $(DESCRIPTION.foobar)$"

# Documentation
DOCHELP += \
  $(NEWLINE)@echo $"  make foobar       Make the $(DESCRIPTION.foobar)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
# Here are defined the root makefile targets.  They invoke a secondary instance
# of make which parses the rest of this file.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),roottargets)

.PHONY: foobar foobarclean

# These are all the pseudo-targets that will implicitly call your target.
# Delete the pseudo-targets which are inappropriate for your module.
all apps libs plugins meshes drivers drivers2d drivers3d snddrivers \
netdrivers: foobar

# These call the target of the same name in the targets section.

# Use this for an application target.  (Delete the plugin target below.)
foobar:
	$(MAKE_APP)

# Use this for a plugin target.  (Delete the app target above.)
foobar:
	$(MAKE_TARGET) MAKE_DLL=yes

# Use this to clean the build detritus from your module.
foobarclean:
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

vpath %.cpp path/to/foobar

# If you need to add any special flags, or link with any external libraries,
# do it here. You should use the compiler-independent flag variables
# from the first 40 lines of cs.mak.
# *** WARNING ***
# Doing this is highly discouraged.  If you really need module-specific flags,
# you should define makefile variables specific to this module rather than
# modifying the flags for all modules.
CFLAGS +=
CFLAGS.INCLUDE +=
LFLAGS +=
LIBS +=

# This section is only for plugins. It adds the plugin to the list of plugins
# as a dynamic or static library depending on the value of USE_PLUGINS.
ifeq ($(USE_PLUGINS),yes)
  FOOBAR = $(OUTDLL)/foobar$(DLL)
  LIB.FOOBAR = $(foreach d,$(DEP.FOOBAR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FOOBAR)
else
  FOOBAR = $(OUT)/$(LIBPREFIX)foobar$(LIB)
  DEP.EXE += $(FOOBAR)
  SCF.STATIC += foobar
  TO_INSTALL.STATIC_LIBS += $(FOOBAR)
endif

# This section is only for applications.
FOOBAR.EXE = foobar$(EXE)
LIB.FOOBAR = $(foreach d,$(DEP.FOOBAR),$($d.LIB))
# Only use the following if your application is a useful tool; test apps are
# not considered sufficiently important for installation.
TO_INSTALL.EXE += $(FOOBAR.EXE)

# Sections for other types of modules have been omitted from this template
# because the requirements of those differ so much that it would be difficult
# to write a template to suit them all.

# This section defines files used by this module.
INC.FOOBAR = $(wildcard path/to/foobar/*.h)
SRC.FOOBAR = $(wildcard path/to/foobar/*.cpp)
OBJ.FOOBAR = $(addprefix $(OUT)/,$(notdir $(SRC.FOOBAR:.cpp=$O)))
# Customise the following line.
DEP.FOOBAR = CSTOOL CSGEOM CSUTIL CSSYS CSUTIL
# Omit this if your module does not have an associated configuration file.
CFG.FOOBAR = data/config/foobar.cfg

# If your module should be installed and has associated resources, list them
# here.
TO_INSTALL.CONFIG += $(CFG.FOOBAR)
TO_INSTALL.DATA += data/foobar.zip

# This section is read by the Microsoft Visual C++ project file synthesis tool.
MSVC.DSP += FOOBAR
DSP.FOOBAR.NAME = foobar
# Choose only one of the following.
DSP.FOOBAR.TYPE = appcon
DSP.FOOBAR.TYPE = library
DSP.FOOBAR.TYPE = plugin
# Add any custom flags and external libraries in the following lines.  Use
# MSVC-style syntax (i.e.  /D "HAVE_FOOBAR").  You can safely omit this section
# if your project does not require any special MSVC flags.
DSP.FOOBAR.CFLAGS =
DSP.FOOBAR.LFLAGS =
DSP.FOOBAR.LIBS =

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
# Here are defined the targets which are run in a local instance of make,
# protected from similar-named stuff elsewhere in CS.
#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),targets)

.PHONY: foobar foobarclean

# For applications (omit for plugins).
all: $(FOOBAR.EXE)
build.foobar: $(OUTDIRS) $(FOOBAR.EXE)
$(FOOBAR.EXE): $(DEP.EXE) $(OBJ.FOOBAR) $(LIB.FOOBAR)
	$(DO.LINK.EXE)

# For plugins (omit for applications)
foobar: $(OUTDIRS) $(FOOBAR)
$(FOOBAR): $(OBJ.FOOBAR) $(LIB.FOOBAR)
	$(DO.PLUGIN)

# If your source files have very specialized compilation requirements that are
# not handled by the implicit rules, you may need to create your own rule for
# the files in your module.  If you use this section, you must omit the `vpath'
# directive above.
# *** WARNING ****
# Use of this section is almost never necessary; it should almost always be
# omitted.
$(OUT)/%$O: path/to/foobar/%.cpp
	$(DO.COMPILE.CPP)

# Cleanup generated resources for applications (omit for plugins)
clean: foobarclean
foobarclean:
	-$(RMDIR) $(FOOBAR.EXE) $(OBJ.FOOBAR)

# Cleanup generated resources for plugins (omit for applications)
clean: foobarclean
foobarclean:
	$(RM) $(FOOBAR) $(OBJ.FOOBAR)

# This takes care of creating and including the dependency file.
ifdef DO_DEPEND
dep: $(OUTOS)/foobar.dep
$(OUTOS)/foobar.dep: $(SRC.FOOBAR)
	$(DO.DEP)
else
-include $(OUTOS)/foobar.dep
endif

endif # ifeq ($(MAKESECTION),targets)
