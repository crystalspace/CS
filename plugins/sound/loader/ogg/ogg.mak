# Plug-in description
DESCRIPTION.csogg = Crystal Space ogg vorbis sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csogg         Make the $(DESCRIPTION.csogg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csogg csoggclean
all plugins drivers snddrivers: csogg

csogg:
	$(MAKE_TARGET) MAKE_DLL=yes
csoggclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/ogg

ifeq ($(USE_PLUGINS),yes)
  CSOGG = $(OUTDLL)sndogg$(DLL)
  LIB.CSOGG = $(foreach d,$(DEP.CSOGG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSOGG)
else
  CSOGG = $(OUT)$(LIB_PREFIX)sndogg$(LIB)
  DEP.EXE += $(CSOGG)
  SCF.STATIC += sndogg
  TO_INSTALL.STATIC_LIBS += $(CSOGG)
endif

INC.CSOGG = $(wildcard plugins/sound/loader/ogg/*.h)
SRC.CSOGG = $(wildcard plugins/sound/loader/ogg/*.cpp)
OBJ.CSOGG = $(addprefix $(OUT),$(notdir $(SRC.CSOGG:.cpp=$O)))
DEP.CSOGG = CSUTIL CSSYS CSUTIL

MSVC.DSP += CSOGG
DSP.CSOGG.NAME = sndogg
DSP.CSOGG.TYPE = plugin
DSP.CSOGG.LIBS = vorbisfile vorbis ogg

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csogg csoggclean

csogg: $(OUTDIRS) $(CSOGG)

$(CSOGG): $(OBJ.CSOGG) $(LIB.CSOGG)
	$(DO.PLUGIN) -lvorbisfile -lvorbis -logg

clean: csoggclean
csoggclean:
	$(RM) $(CSOGG) $(OBJ.CSOGG)

ifdef DO_DEPEND
dep: $(OUTOS)sndogg.dep
$(OUTOS)sndcsogg.dep: $(SRC.CSOGG)
	$(DO.DEP)
else
-include $(OUTOS)sndogg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
