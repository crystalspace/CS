# This is a subinclude file used to define the rules needed
# to build the simple console plug-in.

# Driver description
DESCRIPTION.simpcon = Simple Crystal Space console plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make simpcon      Make the $(DESCRIPTION.simpcon)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simpcon simpconclean

all plugins: simpcon

simpcon:
	$(MAKE_TARGET) MAKE_DLL=yes
simpconclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/simpcon

ifeq ($(USE_SHARED_PLUGINS),yes)
  SIMPCON=$(OUTDLL)simpcon$(DLL)
  DEP.SIMPCON=$(CSGEOM.LIB) $(CSSYS.LIB) $(CSUTIL.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(SIMPCON)
else
  SIMPCON=$(OUT)$(LIB_PREFIX)simpcon$(LIB)
  DEP.EXE+=$(SIMPCON)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_SIMPCON
  TO_INSTALL.STATIC_LIBS+=$(SIMPCON)
endif
DESCRIPTION.$(SIMPCON) = $(DESCRIPTION.simpcon)
SRC.SIMPCON = $(wildcard plugins/simpcon/*.cpp)
OBJ.SIMPCON = $(addprefix $(OUT),$(notdir $(SRC.SIMPCON:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: simpcon simpconclean

# Chain rules
net: simpcon
clean: simpconclean

simpcon: $(OUTDIRS) $(SIMPCON)

$(SIMPCON): $(OBJ.SIMPCON) $(DEP.SIMPCON)
	$(DO.PLUGIN)

simpconclean:
	$(RM) $(SIMPCON) $(OBJ.SIMPCON) $(OUTOS)simpcon.dep

ifdef DO_DEPEND
dep: $(OUTOS)simpcon.dep
$(OUTOS)simpcon.dep: $(SRC.SIMPCON)
	$(DO.DEP)
else
-include $(OUTOS)simpcon.dep
endif

endif # ifeq ($(MAKESECTION),targets)
