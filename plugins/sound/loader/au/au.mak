# Plug-in description
DESCRIPTION.sndau = Crystal Space au sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndau        Make the $(DESCRIPTION.sndau)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndau sndauclean
all plugins drivers snddrivers: sndau

sndau:
	$(MAKE_TARGET) MAKE_DLL=yes
sndauclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/au plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  SNDAU = $(OUTDLL)/sndau$(DLL)
  LIB.SNDAU = $(foreach d,$(DEP.SNDAU),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDAU)
else
  SNDAU = $(OUT)/$(LIB_PREFIX)sndau$(LIB)
  DEP.EXE += $(SNDAU)
  SCF.STATIC += sndau
  TO_INSTALL.STATIC_LIBS += $(SNDAU)
endif

INC.SNDAU = $(wildcard plugins/sound/loader/au/*.h) \
  $(wildcard plugins/sound/loader/common/*.h)
SRC.SNDAU = $(wildcard plugins/sound/loader/au/*.cpp) \
  $(wildcard plugins/sound/loader/common/*.cpp)
OBJ.SNDAU = $(addprefix $(OUT)/,$(notdir $(SRC.SNDAU:.cpp=$O)))
DEP.SNDAU = CSUTIL CSSYS CSUTIL

MSVC.DSP += SNDAU
DSP.SNDAU.NAME = sndau
DSP.SNDAU.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndau sndauclean

sndau: $(OUTDIRS) $(SNDAU)

$(SNDAU): $(OBJ.SNDAU) $(LIB.SNDAU)
	$(DO.PLUGIN)

clean: sndauclean
sndauclean:
	$(RM) $(SNDAU) $(OBJ.SNDAU)

ifdef DO_DEPEND
dep: $(OUTOS)/sndau.dep
$(OUTOS)/sndau.dep: $(SRC.SNDAU)
	$(DO.DEP)
else
-include $(OUTOS)/sndau.dep
endif

endif # ifeq ($(MAKESECTION),targets)
