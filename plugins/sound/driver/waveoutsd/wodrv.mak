# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.sndwaveout = Crystal Space WaveOut sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make sndwaveout   Make the $(DESCRIPTION.sndwaveout)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndwaveout sndwaveoutclean
all plugins drivers snddrivers: sndwaveout

sndwaveout:
	$(MAKE_TARGET) MAKE_DLL=yes
sndwaveoutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/sound/driver/waveoutsd

# The WaveOut sound driver
ifeq ($(USE_PLUGINS),yes)
  SNDWAVEOUT = $(OUTDLL)/sndwaveout$(DLL)
  LIB.SNDWAVEOUT = $(foreach d,$(DEP.SNDWAVEOUT),$($d.LIB))
  LDFLAGS.SNDWAVEOUT = $(LIBS.SOUND.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(SNDWAVEOUT)
else
  SNDWAVEOUT = $(OUT)/$(LIB_PREFIX)sndwaveout$(LIB)
  DEP.EXE += $(SNDWAVEOUT)
  LIBS.EXE += $(LIBS.SOUND.SYSTEM)
  SCF.STATIC += sndwaveout
  TO_INSTALL.STATIC_LIBS += $(SNDWAVEOUT)
endif

INF.SNDWAVEOUT = $(SRCDIR)/plugins/sound/driver/waveoutsd/sndwaveout.csplugin
INC.SNDWAVEOUT = \
  $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/driver/waveoutsd/*.h))
SRC.SNDWAVEOUT = \
  $(wildcard $(addprefix $(SRCDIR)/,plugins/sound/driver/waveoutsd/*.cpp))
OBJ.SNDWAVEOUT = $(addprefix $(OUT)/,$(notdir $(SRC.SNDWAVEOUT:.cpp=$O)))
DEP.SNDWAVEOUT = CSUTIL CSSYS CSUTIL

MSVC.DSP += SNDWAVEOUT
DSP.SNDWAVEOUT.NAME = sndwaveout
DSP.SNDWAVEOUT.TYPE = plugin
DSP.SNDWAVEOUT.LIBS = winmm

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndwaveout sndwaveoutclean

sndwaveout: $(OUTDIRS) $(SNDWAVEOUT)

$(SNDWAVEOUT): $(OBJ.SNDWAVEOUT) $(LIB.SNDWAVEOUT)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LDFLAGS.SNDWAVEOUT) \
	$(DO.PLUGIN.POSTAMBLE)

clean: sndwaveoutclean
sndwaveoutclean:
	-$(RMDIR) $(SNDWAVEOUT) $(OBJ.SNDWAVEOUT) $(OUTDLL)/$(notdir $(INF.SNDWAVEOUT))

ifdef DO_DEPEND
dep: $(OUTOS)/sndwaveout.dep
$(OUTOS)/sndwaveout.dep: $(SRC.SNDWAVEOUT)
	$(DO.DEP)
else
-include $(OUTOS)/sndwaveout.dep
endif

endif # ifeq ($(MAKESECTION),targets)
