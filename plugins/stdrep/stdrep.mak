# This is a subinclude file used to define the rules needed
# to build the stdrep plug-in.

# Driver description
DESCRIPTION.stdrep = Crystal Space Standard Report Listener Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make stdrep       Make the $(DESCRIPTION.stdrep)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stdrep stdrepclean
all plugins: stdrep

stdrep:
	$(MAKE_TARGET) MAKE_DLL=yes
stdrepclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/stdrep

ifeq ($(USE_PLUGINS),yes)
  STDREP = $(OUTDLL)stdrep$(DLL)
  LIB.STDREP = $(foreach d,$(DEP.STDREP),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(STDREP)
else
  STDREP = $(OUT)$(LIB_PREFIX)stdrep$(LIB)
  DEP.EXE += $(STDREP)
  SCF.STATIC += stdrep
  TO_INSTALL.STATIC_LIBS += $(STDREP)
endif

INC.STDREP = $(wildcard plugins/stdrep/*.h)
SRC.STDREP = $(wildcard plugins/stdrep/*.cpp)
OBJ.STDREP = $(addprefix $(OUT),$(notdir $(SRC.STDREP:.cpp=$O)))
DEP.STDREP = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += STDREP
DSP.STDREP.NAME = stdrep
DSP.STDREP.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: stdrep stdrepclean

stdrep: $(OUTDIRS) $(STDREP)

$(STDREP): $(OBJ.STDREP) $(LIB.STDREP)
	$(DO.PLUGIN)

clean: stdrepclean
stdrepclean:
	$(RM) $(STDREP) $(OBJ.STDREP)

ifdef DO_DEPEND
dep: $(OUTOS)stdrep.dep
$(OUTOS)stdrep.dep: $(SRC.STDREP)
	$(DO.DEP)
else
-include $(OUTOS)stdrep.dep
endif

endif # ifeq ($(MAKESECTION),targets)
