DESCRIPTION.rapid = RAPID collision detection plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make rapid        Make the $(DESCRIPTION.rapid)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rapid rapidclean
plugins all: rapid

rapidclean:
	$(MAKE_CLEAN)
rapid:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/colldet/rapid

ifeq ($(USE_PLUGINS),yes)
  RAPID = $(OUTDLL)rapid$(DLL)
  LIB.RAPID = $(foreach d,$(DEP.RAPID),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RAPID)
else
  RAPID = $(OUT)$(LIB_PREFIX)rapid$(LIB)
  DEP.EXE += $(RAPID)
  SCF.STATIC += rapid
  TO_INSTALL.STATIC_LIBS += $(RAPID)
endif

INC.RAPID = $(wildcard plugins/colldet/rapid/*.h)
SRC.RAPID = $(wildcard plugins/colldet/rapid/*.cpp)
OBJ.RAPID = $(addprefix $(OUT),$(notdir $(SRC.RAPID:.cpp=$O)))
DEP.RAPID = CSGEOM CSUTIL CSSYS

MSVC.DSP += RAPID
DSP.RAPID.NAME = rapid
DSP.RAPID.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rapid rapidclean
rapid: $(OUTDIRS) $(RAPID)

$(RAPID): $(OBJ.RAPID) $(LIB.RAPID)
	$(DO.PLUGIN)

clean: rapidclean
rapidclean:
	-$(RM) $(RAPID) $(OBJ.RAPID)

ifdef DO_DEPEND
dep: $(OUTOS)rapid.dep
$(OUTOS)rapid.dep: $(SRC.RAPID)
	$(DO.DEP)
else
-include $(OUTOS)rapid.dep
endif

endif # ifeq ($(MAKESECTION),targets)
