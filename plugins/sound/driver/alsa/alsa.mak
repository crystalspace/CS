# This is a subinclude file used to define the rules needed
# to build the AdvancedLinuxSoundArchitecture driver -- alsadrv

# Driver description
DESCRIPTION.alsadrv = Crystal Space ALSA sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make alsadrv       Make the $(DESCRIPTION.alsadrv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: alsadrv alsadrvclean
all plugins drivers snddrivers: alsadrv

alsadrv:
	$(MAKE_TARGET) MAKE_DLL=yes
alsadrvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/sound/driver/alsa

# The ALSA sound driver
ifeq ($(USE_PLUGINS),yes)
  ALSADRV = $(OUTDLL)/alsadrv$(DLL)
  LIB.ALSADRV = $(foreach d,$(DEP.ALSADRV),$($d.LIB))
  LIB.ALSADRV.LFLAGS = $(ALSA.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(ALSADRV)
else
  ALSADRV = $(OUT)/$(LIB_PREFIX)alsadrv.a
  DEP.EXE += $(ALSADRV)
  SCF.STATIC += alsadrv
  TO_INSTALL.STATIC_LIBS += $(ALSADRV)
endif

INF.ALSADRV = $(SRCDIR)/plugins/sound/driver/alsa/alsadrv.csplugin
INC.ALSADRV = $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/driver/alsa/*.h))
SRC.ALSADRV = $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/driver/alsa/*.cpp))
OBJ.ALSADRV = $(addprefix $(OUT)/,$(notdir $(SRC.ALSADRV:.cpp=$O)))
DEP.ALSADRV = CSUTIL CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: alsadrv alsadrvclean

alsadrv: $(OUTDIRS) $(ALSADRV)

$(ALSADRV): $(OBJ.ALSADRV) $(LIB.ALSADRV)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.ALSADRV.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: alsadrvclean
alsadrvclean:
	-$(RMDIR) $(ALSADRV) $(OBJ.ALSADRV) $(OUTDLL)/$(notdir $(INF.ALSADRV))

ifdef DO_DEPEND
dep: $(OUTOS)/alsa.dep
$(OUTOS)/alsa.dep: $(SRC.ALSADRV)
	$(DO.DEP)
else
-include $(OUTOS)/alsa.dep
endif

endif # ifeq ($(MAKESECTION),targets)
