# This is a subinclude file used to define the rules needed
# to build the bugplug plug-in.

# Driver description
DESCRIPTION.bugplug = Crystal Space Debugger plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make bugplug      Make the $(DESCRIPTION.bugplug)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bugplug bugplugclean
all plugins: bugplug

bugplug:
	$(MAKE_TARGET) MAKE_DLL=yes
bugplugclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/bugplug

ifeq ($(USE_PLUGINS),yes)
  BUGPLUG = $(OUTDLL)bugplug$(DLL)
  LIB.BUGPLUG = $(foreach d,$(DEP.BUGPLUG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BUGPLUG)
else
  BUGPLUG = $(OUT)$(LIB_PREFIX)bugplug$(LIB)
  DEP.EXE += $(BUGPLUG)
  SCF.STATIC += bugplug
  TO_INSTALL.STATIC_LIBS += $(BUGPLUG)
endif

INC.BUGPLUG = $(wildcard plugins/bugplug/*.h)
SRC.BUGPLUG = $(wildcard plugins/bugplug/*.cpp)
OBJ.BUGPLUG = $(addprefix $(OUT),$(notdir $(SRC.BUGPLUG:.cpp=$O)))
DEP.BUGPLUG = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += BUGPLUG
DSP.BUGPLUG.NAME = bugplug
DSP.BUGPLUG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bugplug bugplugclean

bugplug: $(OUTDIRS) $(BUGPLUG)

$(BUGPLUG): $(OBJ.BUGPLUG) $(LIB.BUGPLUG)
	$(DO.PLUGIN)

clean: bugplugclean
bugplugclean:
	$(RM) $(BUGPLUG) $(OBJ.BUGPLUG)

ifdef DO_DEPEND
dep: $(OUTOS)bugplug.dep
$(OUTOS)bugplug.dep: $(SRC.BUGPLUG)
	$(DO.DEP)
else
-include $(OUTOS)bugplug.dep
endif

endif # ifeq ($(MAKESECTION),targets)
