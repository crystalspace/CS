DESCRIPTION.joylin = Crystal Space Joystick plugin for Linux

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make joylin       Make the $(DESCRIPTION.joylin)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: joylin joylinclean
all plugins: joylin
joylin:
	$(MAKE_TARGET) MAKE_DLL=yes
joylinclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)
vpath %.cpp $(SRCDIR)/plugins/device/joystick/linux

ifeq ($(USE_PLUGINS),yes)
  JOYLIN = $(OUTDLL)/joylin$(DLL)
  LIB.JOYLIN = $(foreach d,$(DEP.JOYLIN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(JOYLIN)
else
  JOYLIN = $(OUT)/$(LIB_PREFIX)joylin$(LIB)
  DEP.EXE += $(JOYLIN)
  SCF.STATIC += joylin
  TO_INSTALL.STATIC_LIBS += $(JOYLIN)
endif

INF.JOYLIN = $(SRCDIR)/plugins/device/joystick/linux/joylin.csplugin
INC.JOYLIN = \
  $(wildcard $(addprefix $(SRCDIR)/,plugins/device/joystick/linux/*.h))
SRC.JOYLIN = \
  $(wildcard $(addprefix $(SRCDIR)/,plugins/device/joystick/linux/*.cpp))
OBJ.JOYLIN = $(addprefix $(OUT)/,$(notdir $(SRC.JOYLIN:.cpp=$O)))
DEP.JOYLIN = CSUTIL CSSYS
CFG.JOYLIN = $(SRCDIR)/data/config/joystick.cfg

TO_INSTALL.CONFIG += $(CFG.JOYLIN)

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: joylin joylinclean
joylin: $(OUTDIRS) $(JOYLIN)

$(JOYLIN): $(OBJ.JOYLIN) $(LIB.JOYLIN)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.EXTERNAL.JOYLIN) \
	$(DO.PLUGIN.POSTAMBLE)

clean: joylinclean
joylinclean:
	-$(RMDIR) $(JOYLIN) $(OBJ.JOYLIN) $(OUTDLL)/$(notdir $(INF.JOYLIN))

ifdef DO_DEPEND
dep: $(OUTOS)/joylin.dep
$(OUTOS)/joylin.dep: $(SRC.JOYLIN)
	$(DO.DEP)
else
-include $(OUTOS)/joylin.dep
endif

endif # ifeq ($(MAKESECTION),targets)
