# Plug-in description
DESCRIPTION.motion = Crystal Space skeletal motion plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make motion       Make the $(DESCRIPTION.motion)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: motion motionclean
all plugins: motion

motion:
	$(MAKE_TARGET) MAKE_DLL=yes
motionclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/motion

ifeq ($(USE_PLUGINS),yes)
  MOTION = $(OUTDLL)motion$(DLL)
  LIB.MOTION = $(foreach d,$(DEP.MOTION),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MOTION)
else
  MOTION = $(OUT)$(LIB_PREFIX)motion$(LIB)
  DEP.EXE += $(MOTION)
  SCF.STATIC += motion
  TO_INSTALL.STATIC_LIBS += $(MOTION)
endif

INC.MOTION = $(wildcard plugins/motion/*.h)
SRC.MOTION = $(wildcard plugins/motion/*.cpp)
OBJ.MOTION = $(addprefix $(OUT),$(notdir $(SRC.MOTION:.cpp=$O)))
DEP.MOTION = CSGEOM CSSYS CSUTIL

MSVC.DSP += MOTION
DSP.MOTION.NAME = motion
DSP.MOTION.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: motion motionclean

motion: $(OUTDIRS) $(MOTION)

$(MOTION): $(OBJ.MOTION) $(LIB.MOTION)
	$(DO.PLUGIN)

clean: motionclean
motionclean:
	$(RM) $(MOTION) $(OBJ.MOTION)

ifdef DO_DEPEND
dep: $(OUTOS)motion.dep
$(OUTOS)motion.dep: $(SRC.MOTION)
	$(DO.DEP)
else
-include $(OUTOS)motion.dep
endif

endif # ifeq ($(MAKESECTION),targets)
