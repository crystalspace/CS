#------------------------------------------------------------------------------
# Isometric loader plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.isoload = Crystal Space isometric world loader plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make isoload          Make the $(DESCRIPTION.isoload)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: isoload isoloadclean
all plugins: isoload
isoload:
	$(MAKE_TARGET) MAKE_DLL=yes
isoloadclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/engine/isoload

ifeq ($(USE_PLUGINS),yes)
  ISO = $(OUTDLL)isoload$(DLL)
  LIB.ISO = $(foreach d,$(DEP.ISO),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ISO)
else
  ISO = $(OUT)$(LIB_PREFIX)isoload$(LIB)
  DEP.EXE += $(ISO)
  SCF.STATIC += isoload
  TO_INSTALL.STATIC_LIBS += $(ISO)
endif

INC.ISO = $(wildcard plugins/engine/iso/*.h)
SRC.ISO = $(wildcard plugins/engine/iso/*.cpp)
OBJ.ISO = $(addprefix $(OUT),$(notdir $(SRC.ISO:.cpp=$O)))
DEP.ISO = CSUTIL CSSYS CSGEOM CSGFX CSUTIL CSSYS

MSVC.DSP += ISO
DSP.ISO.NAME = isoload
DSP.ISO.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: isoload isoloadclean
isoload: $(OUTDIRS) $(ISO)

$(ISO): $(OBJ.ISO) $(LIB.ISO)
	$(DO.PLUGIN)

clean: isoloadclean
isoloadclean:
	-$(RM) $(ISO) $(OBJ.ISO)

ifdef DO_DEPEND
dep: $(OUTOS)isoload.dep
$(OUTOS)isoload.dep: $(SRC.ISO)
	$(DO.DEP)
else
-include $(OUTOS)isoload.dep
endif

endif # ifeq ($(MAKESECTION),targets)
