# Plug-in description
DESCRIPTION.sndwav = Crystal Space wav sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndwav       Make the $(DESCRIPTION.sndwav)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndwav sndwavclean
all plugins: sndwav

sndwav:
	$(MAKE_TARGET) MAKE_DLL=yes
sndwavclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/sound/loader/wav $(SRCDIR)/plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  SNDWAV = $(OUTDLL)/sndwav$(DLL)
  LIB.SNDWAV = $(foreach d,$(DEP.SNDWAV),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDWAV)
else
  SNDWAV = $(OUT)/$(LIB_PREFIX)sndwav$(LIB)
  DEP.EXE += $(SNDWAV)
  SCF.STATIC += sndwav
  TO_INSTALL.STATIC_LIBS += $(SNDWAV)
endif

INF.SNDWAV = $(SRCDIR)/plugins/sound/loader/wav/sndwav.csplugin
INC.SNDWAV = $(wildcard $(addprefix $(SRCDIR)/, \
  plugins/sound/loader/wav/*.h plugins/sound/loader/common/*.h))
SRC.SNDWAV = $(wildcard $(addprefix $(SRCDIR)/, \
  plugins/sound/loader/wav/*.cpp plugins/sound/loader/common/*.cpp))
OBJ.SNDWAV = $(addprefix $(OUT)/,$(notdir $(SRC.SNDWAV:.cpp=$O)))
DEP.SNDWAV = CSUTIL CSUTIL

MSVC.DSP += SNDWAV
DSP.SNDWAV.NAME = sndwav
DSP.SNDWAV.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndwav sndwavclean

sndwav: $(OUTDIRS) $(SNDWAV)

$(SNDWAV): $(OBJ.SNDWAV) $(LIB.SNDWAV)
	$(DO.PLUGIN)

clean: sndwavclean
sndwavclean:
	-$(RMDIR) $(SNDWAV) $(OBJ.SNDWAV) $(OUTDLL)/$(notdir $(INF.SNDWAV))

ifdef DO_DEPEND
dep: $(OUTOS)/sndwav.dep
$(OUTOS)/sndwav.dep: $(SRC.SNDWAV)
	$(DO.DEP)
else
-include $(OUTOS)/sndwav.dep
endif

endif # ifeq ($(MAKESECTION),targets)
