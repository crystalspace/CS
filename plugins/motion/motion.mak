# Driver description
DESCRIPTION.motion = Crystal Space Skeletal Motion plugin

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

ifeq ($(USE_SHARED_PLUGINS),yes)
  MOTION=$(OUTDLL)motion$(DLL)
  DEP.MOTION=$(CSGEOM.LIB) $(CSSYS.LIB) $(CSUTIL.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(MOTION)
else
  MOTION=$(OUT)$(LIB_PREFIX)motion$(LIB)
  DEP.EXE+=$(MOTION)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_MOTION
  TO_INSTALL.STATIC_LIBS+=$(MOTION)
endif
DESCRIPTION.$(MOTION) = $(DESCRIPTION.motion)
SRC.MOTION = $(wildcard plugins/motion/*.cpp)
OBJ.MOTION = $(addprefix $(OUT),$(notdir $(SRC.MOTION:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: motion motionclean

# Chain rules
clean: motionclean

motion: $(OUTDIRS) $(MOTION)

$(MOTION): $(OBJ.MOTION) $(DEP.MOTION)
	$(DO.PLUGIN)

motionclean:
	$(RM) $(MOTION) $(OBJ.MOTION) $(OUTOS)motion.dep

ifdef DO_DEPEND
dep: $(OUTOS)motion.dep
$(OUTOS)motion.dep: $(SRC.MOTION)
	$(DO.DEP)
else
-include $(OUTOS)motion.dep
endif

endif # ifeq ($(MAKESECTION),targets)
