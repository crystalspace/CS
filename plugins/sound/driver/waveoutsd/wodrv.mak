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

.PHONY: sndwaveout wosclean
all plugins drivers snddrivers: sndwaveout

sndwaveout:
	$(MAKE_TARGET) MAKE_DLL=yes
wosclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/driver/waveoutsd

# The WaveOut sound driver
ifeq ($(USE_PLUGINS),yes)
  WOS = $(OUTDLL)sndwaveout$(DLL)
  LIB.WOS = $(foreach d,$(DEP.WOS),$($d.LIB))
  LDFLAGS.WOS = $(LIBS.SOUND.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(WOS)
else
  WOS = $(OUT)$(LIB_PREFIX)sndwaveout$(LIB)
  DEP.EXE += $(WOS)
  LIBS.EXE += $(LIBS.SOUND.SYSTEM)
  SCF.STATIC += sndwaveout
  TO_INSTALL.STATIC_LIBS += $(WOS)
endif

INC.WOS = $(wildcard plugins/sound/driver/waveoutsd/*.h)
SRC.WOS = $(wildcard plugins/sound/driver/waveoutsd/*.cpp)
OBJ.WOS = $(addprefix $(OUT),$(notdir $(SRC.WOS:.cpp=$O)))
DEP.WOS = CSUTIL CSSYS CSUTIL

MSVC.DSP += WOS
DSP.WOS.NAME = sndwaveout
DSP.WOS.TYPE = plugin
DSP.WOS.LIBS = winmm

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndwaveout wosclean

sndwaveout: $(OUTDIRS) $(WOS)

$(WOS): $(OBJ.WOS) $(LIB.WOS)
	$(DO.PLUGIN) $(LDFLAGS.WOS)

clean: wosclean
wosclean:
	$(RM) $(WOS) $(OBJ.WOS)

ifdef DO_DEPEND
dep: $(OUTOS)wos.dep
$(OUTOS)wos.dep: $(SRC.WOS)
	$(DO.DEP)
else
-include $(OUTOS)wos.dep
endif

endif # ifeq ($(MAKESECTION),targets)
