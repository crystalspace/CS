#------------------------------------------------------------------------------#
# Submakefile for the Crystal Space Foo Bar
# Modified from the template submakefile
# (C) 1337 Joe Bloggs
#
# Insert license header here
#
#------------------------------------------------------------------------------#

#
# When finished, add your submakefile's filename to subs.mak or user.mak.
#

DESCRIPTION.foobar = Crystal Space Foo Bar

#-------------------------------------------------------------- rootdefines ---#
# Here are defined the makefile target descriptions for the target list.
# You will usually only want one or two of the six shown here,
# depending on what type of thing your module is. Delete as appropriate.
#------------------------------------------------------------------------------#
ifeq ($(MAKESECTION), rootdefines)

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

endif

#-------------------------------------------------------------- roottargets ---#
# Here are defined the root makefile targets.
# They start another instance of make which parses the rest of this file.
#------------------------------------------------------------------------------#
ifeq ($(MAKESECTION), roottargets)

.PHONY: foobar foobarclean

# These are all the pseudo-targets that will implicitly call your target.
# Delete as appropriate.
all apps libs plugins meshes drivers drivers2d drivers3d snddrivers netdrivers: foobar

clean: foobarclean

# These call the target of the same name in the targets section.
foobar: dependencies
	$(MAKE_TARGET) MAKE_DLL=yes
foobarclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------------ defines ---#
# Here are (re)defined platform-dependent variables.
# You probably don't need this,
# but if you think you do, see the first 40 lines of cs.mak.
#------------------------------------------------------------------------------#
ifeq ($(MAKESECTION), defines)

endif

#-------------------------------------------------------------- postdefines ---#
# Here are (re)defined platform-independent variables.
# See cs.mak if you need any clarification.
#------------------------------------------------------------------------------#
ifeq ($(MAKESECTION), postdefines)

# If you need to add any special flags, or link with any external libraries,
# do it here. You should use the compiler-independent flag variables
# from the first 40 lines of cs.mak.
CFLAGS +=
CFLAGS.INCLUDE +=
LFLAGS +=
LIBS +=

vpath % path/to/foobar

# This section is only for plugins. It adds the plugin to the list of plugins
# as a dynamic or static library depending on the value of USE_PLGUINS.
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

# This section is only for applications. It adds it to the list of applications.
FOOBAR = foobar$(EXE)
LIB.FOOBAR = $(foreach d,$(DEP.FOOBAR),$($d.LIB))
TO_INSTALL.EXE += $(FOOBAR)

# I have left out sections for other types of modules from this template,
# because the requirements of those differ so much, it would be difficult to
# write a template that would suit them all.

# This section defines files used by this module.
INC.FOOBAR = $(wildcard path/to/foobar/*.h)
SRC.FOOBAR = $(wildcard path/to/foobar/*.cpp)
OBJ.FOOBAR = $(addprefix $(OUT)/,$(notdir $(SRC.FOOBAR:.cpp=$O)))
# Customise the following line.
DEP.FOOBAR = CSUTIL CSSYS CSUTIL

# This section is read by the Microsoft Visual C++ project file maker utility.
MSVC.DSP += FOOBAR
DSP.FOOBAR.NAME = foobar
# Delete as appropriate from the following line.
DSP.FOOBAR.TYPE = plugin appcon
# Customise the following line.
DSP.FOOBAR.DEPEND = CSUTIL CSSYS CSUTIL
# Add any custom flags and external libraries in the following lines.
# Use MSVC-style flags.
DSP.FOOBAR.CFLAGS =
DSP.FOOBAR.LFLAGS =
DSP.FOOBAR.LIBS =

endif

#------------------------------------------------------------------ targets ---#
# Here are defined the targets which are run in a local instance of make,
# protected from similar-named stuff elsewhere in CS.
#------------------------------------------------------------------------------#
ifeq ($(MAKESECTION), targets)

.PHONY: foobar foobarclean

foobar: $(FOOBAR)

# Delete one of the two indented `action' lines as appropriate.
$(FOOBAR): $(OBJ.FOOBAR) $(LIB.FOOBAR)
	$(DO.PLUGIN)
	$(DO.LINK.EXE)

# This takes care of compiling the source code files
# You may have to customize this if you use anything other than standard C++
$(OUT)/%$O: path/to/foobar/%.cpp
	$(DO.COMPILE.CPP)

# Takes care of the clean target to delete built files.
foobarclean:
	-$(RMDIR) $(FOOBAR) $(OBJ.FOOBAR)

# This takes care of creating and including the dependency file.
ifdef DO_DEPEND
dep: $(OUTOS)/foobar.dep
else
-include $(OUTOS)/foobar.dep
endif

endif

