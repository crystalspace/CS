# This is a subinclude file used to define the rules needed
# to build the OpenSoundSystem driver -- ossdrv

# Driver description
DESCRIPTION.wos = Crystal Space WaveOut sound driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make wos          Make the $(DESCRIPTION.wos)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: wos wosclean
all plugins drivers snddrivers: wos

wos:
	$(MAKE_TARGET) MAKE_DLL=yes
wosclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/driver/waveoutsd

# The WaveOut sound driver
ifeq ($(USE_PLUGINS),yes)
  SNDWOS = $(OUTDLL)sndwaveout$(DLL)
  LIB.SNDWOS = $(foreach d,$(DEP.SNDWOS),$($d.LIB))
  LDFLAGS.WOS = $(LIBS.SOUND.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(SNDWOS)
else
  SNDWOS = $(OUT)$(LIB_PREFIX)sndwaveout$(LIB)
  DEP.EXE += $(SNDWOS)
  SCF.STATIC += sndwaveout
  TO_INSTALL.STATIC_LIBS += $(SNDWOS)
endif

INC.SNDWOS = $(wildcard plugins/sound/driver/waveoutsd/*.h)
SRC.SNDWOS = $(wildcard plugins/sound/driver/waveoutsd/*.cpp)
OBJ.SNDWOS = $(addprefix $(OUT),$(notdir $(SRC.SNDWOS:.cpp=$O)))
DEP.SNDWOS = CSUTIL CSSYS CSUTIL

MSVC.DSP += SNDWOS
DSP.SNDWOS.NAME = sndwaveout
DSP.SNDWOS.TYPE = plugin
DSP.SNDWOS.LIBS = winmm

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: wos wosclean

wos: $(OUTDIRS) $(SNDWOS)

$(SNDWOS): $(OBJ.SNDWOS) $(LIB.SNDWOS)
	$(DO.PLUGIN) $(LDFLAGS.WOS)

clean: wosclean
wosclean:
	$(RM) $(SNDWOS) $(OBJ.SNDWOS)

ifdef DO_DEPEND
dep: $(OUTOS)wos.dep
$(OUTOS)wos.dep: $(SRC.SNDWOS)
	$(DO.DEP)
else
-include $(OUTOS)wos.dep
endif

endif # ifeq ($(MAKESECTION),targets)
