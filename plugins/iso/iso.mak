#------------------------------------------------------------------------------
# Isometric plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.iso = Crystal Space isometric engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make iso          Make the $(DESCRIPTION.iso)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: iso isoclean
all plugins: iso
iso:
	$(MAKE_TARGET) MAKE_DLL=yes
isoclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/iso

ifeq ($(USE_PLUGINS),yes)
  ISO = $(OUTDLL)iso$(DLL)
  LIB.ISO = $(foreach d,$(DEP.ISO),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ISO)
else
  ISO = $(OUT)$(LIB_PREFIX)iso$(LIB)
  DEP.EXE += $(ISO)
  SCF.STATIC += iso
  TO_INSTALL.STATIC_LIBS += $(ISO)
endif

INC.ISO = $(wildcard plugins/iso/*.h)
SRC.ISO = $(wildcard plugins/iso/*.cpp)
OBJ.ISO = $(addprefix $(OUT),$(notdir $(SRC.ISO:.cpp=$O)))
DEP.ISO = CSUTIL CSSYS CSGEOM CSGFX CSUTIL CSSYS

MSVC.DSP += ISO
DSP.ISO.NAME = iso
DSP.ISO.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: iso isoclean
iso: $(OUTDIRS) $(ISO)

$(ISO): $(OBJ.ISO) $(LIB.ISO)
	$(DO.PLUGIN)

clean: isoclean
isoclean:
	-$(RM) $(ISO) $(OBJ.ISO)

ifdef DO_DEPEND
dep: $(OUTOS)iso.dep
$(OUTOS)iso.dep: $(SRC.ISO)
	$(DO.DEP)
else
-include $(OUTOS)iso.dep
endif

endif # ifeq ($(MAKESECTION),targets)
