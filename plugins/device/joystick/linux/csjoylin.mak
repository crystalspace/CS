DESCRIPTION.joylin = Crystal Space Joystick plugin for Linux

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make joylin     Make the $(DESCRIPTION.joylin)$"

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
vpath %.cpp plugins/device/joystick/linux

ifeq ($(USE_PLUGINS),yes)
  JOYLIN = $(OUTDLL)csjoylin$(DLL)
  LIB.JOYLIN = $(foreach d,$(DEP.JOYLIN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(JOYLIN)
else
  JOYLIN = $(OUT)$(LIB_PREFIX)csjoylin$(LIB)
  DEP.EXE += $(JOYLIN)
  SCF.STATIC += csjoylin
  TO_INSTALL.STATIC_LIBS += $(JOYLIN)
endif

INC.JOYLIN = $(wildcard plugins/device/joystick/linux/*.h)
SRC.JOYLIN = $(wildcard plugins/device/joystick/linux/*.cpp)
OBJ.JOYLIN = $(addprefix $(OUT),$(notdir $(SRC.JOYLIN:.cpp=$O)))
DEP.JOYLIN = CSUTIL CSSYS
CFG.JOYLIN = data/config/joystick.cfg

TO_INSTALL.CONFIG += $(CFG.JOYLIN)

MSVC.DSP += JOYLIN
DSP.JOYLIN.NAME = joylin
DSP.JOYLIN.TYPE = plugin
DSP.JOYLIN.LFLAGS = /nodefaultlib:"MSVCRT"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: joylin joylinclean
joylin: $(OUTDIRS) $(JOYLIN)

$(JOYLIN): $(OBJ.JOYLIN) $(LIB.JOYLIN)
	$(DO.PLUGIN) $(LIB.EXTERNAL.JOYLIN)

clean: joylinclean
joylinclean:
	-$(RM) $(JOYLIN) $(OBJ.JOYLIN)

ifdef DO_DEPEND
dep: $(OUTOS)joylin.dep
$(OUTOS)joylin.dep: $(SRC.JOYLIN)
	$(DO.DEP)
else
-include $(OUTOS)joylin.dep
endif

endif # ifeq ($(MAKESECTION),targets)
