# Plug-in description
DESCRIPTION.wav = Crystal Space wav sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make wav          Make the $(DESCRIPTION.wav)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: wav wavclean
all plugins drivers snddrivers: wav

wav:
	$(MAKE_TARGET) MAKE_DLL=yes
wavclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/wav plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  WAV = $(OUTDLL)sndwav$(DLL)
  LIB.WAV = $(foreach d,$(DEP.WAV),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(WAV)
else
  WAV = $(OUT)$(LIB_PREFIX)sndwav$(LIB)
  DEP.EXE += $(WAV)
  SCF.STATIC += sndwav
  TO_INSTALL.STATIC_LIBS += $(WAV)
endif

INC.WAV = $(wildcard plugins/sound/loader/wav/*.h) $(wildcard plugins/sound/loader/common/*.h)
SRC.WAV = $(wildcard plugins/sound/loader/wav/*.cpp) $(wildcard plugins/sound/loader/common/*.cpp)
OBJ.WAV = $(addprefix $(OUT),$(notdir $(SRC.WAV:.cpp=$O)))
DEP.WAV = CSUTIL CSSYS CSUTIL
CFG.WAV = data/config/sound.cfg

TO_INSTALL.CONFIG += $(CFG.WAV)

MSVC.DSP += WAV
DSP.WAV.NAME = sndwav
DSP.WAV.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: wav wavclean

wav: $(OUTDIRS) $(WAV)

$(WAV): $(OBJ.WAV) $(LIB.WAV)
	$(DO.PLUGIN)

clean: wavclean
wavclean:
	$(RM) $(WAV) $(OBJ.WAV)

ifdef DO_DEPEND
dep: $(OUTOS)sndwav.dep
$(OUTOS)sndwav.dep: $(SRC.WAV)
	$(DO.DEP)
else
-include $(OUTOS)sndwav.dep
endif

endif # ifeq ($(MAKESECTION),targets)

