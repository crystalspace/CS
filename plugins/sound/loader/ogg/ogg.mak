# Plug-in description
DESCRIPTION.sndogg = Crystal Space ogg vorbis sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndogg       Make the $(DESCRIPTION.sndogg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndogg sndoggclean
all plugins drivers snddrivers: sndogg

sndogg:
	$(MAKE_TARGET) MAKE_DLL=yes
sndoggclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/ogg

ifeq ($(USE_PLUGINS),yes)
  SNDOGG = $(OUTDLL)/sndogg$(DLL)
  LIB.SNDOGG = $(foreach d,$(DEP.SNDOGG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDOGG)
else
  SNDOGG = $(OUT)/$(LIB_PREFIX)sndogg$(LIB)
  DEP.EXE += $(SNDOGG)
  SCF.STATIC += sndogg
  TO_INSTALL.STATIC_LIBS += $(SNDOGG)
endif

INC.SNDOGG = $(wildcard plugins/sound/loader/ogg/*.h)
SRC.SNDOGG = $(wildcard plugins/sound/loader/ogg/*.cpp)
OBJ.SNDOGG = $(addprefix $(OUT)/,$(notdir $(SRC.SNDOGG:.cpp=$O)))
DEP.SNDOGG = CSUTIL CSSYS CSUTIL

MSVC.DSP += SNDOGG
DSP.SNDOGG.NAME = sndogg
DSP.SNDOGG.TYPE = plugin
DSP.SNDOGG.LIBS = vorbisfile vorbis ogg

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndogg sndoggclean

sndogg: $(OUTDIRS) $(SNDOGG)

$(SNDOGG): $(OBJ.SNDOGG) $(LIB.SNDOGG)
	$(DO.PLUGIN) -lvorbisfile -lvorbis -logg

clean: sndoggclean
sndoggclean:
	$(RM) $(SNDOGG) $(OBJ.SNDOGG)

ifdef DO_DEPEND
dep: $(OUTOS)/sndogg.dep
$(OUTOS)/sndogg.dep: $(SRC.SNDOGG)
	$(DO.DEP)
else
-include $(OUTOS)/sndogg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
