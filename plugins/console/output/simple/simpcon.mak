# This is a subinclude file used to define the rules needed
# to build the simple console plug-in.

# Plug-in description
DESCRIPTION.simpcon = Crystal Space simple output console

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
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

vpath %.cpp plugins/console/output/simple

ifeq ($(USE_PLUGINS),yes)
  SIMPCON = $(OUTDLL)simpcon$(DLL)
  LIB.SIMPCON = $(foreach d,$(DEP.SIMPCON),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SIMPCON)
else
  SIMPCON = $(OUT)$(LIB_PREFIX)simpcon$(LIB)
  DEP.EXE += $(SIMPCON)
  SCF.STATIC += simpcon
  TO_INSTALL.STATIC_LIBS += $(SIMPCON)
endif

INC.SIMPCON = $(wildcard plugins/console/output/simple/*.h)
SRC.SIMPCON = $(wildcard plugins/console/output/simple/*.cpp)
OBJ.SIMPCON = $(addprefix $(OUT),$(notdir $(SRC.SIMPCON:.cpp=$O)))
DEP.SIMPCON = CSSYS CSUTIL CSGEOM

MSVC.DSP += SIMPCON
DSP.SIMPCON.NAME = simpcon
DSP.SIMPCON.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: simpcon simpconclean

simpcon: $(OUTDIRS) $(SIMPCON)

$(SIMPCON): $(OBJ.SIMPCON) $(LIB.SIMPCON)
	$(DO.PLUGIN)

clean: simpconclean
simpconclean:
	$(RM) $(SIMPCON) $(OBJ.SIMPCON)

ifdef DO_DEPEND
dep: $(OUTOS)simpcon.dep
$(OUTOS)simpcon.dep: $(SRC.SIMPCON)
	$(DO.DEP)
else
-include $(OUTOS)simpcon.dep
endif

endif # ifeq ($(MAKESECTION),targets)
