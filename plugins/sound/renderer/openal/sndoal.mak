# Plug-in description
DESCRIPTION.sndoal = Crystal Space OpenAL sound renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndoal       Make the $(DESCRIPTION.sndoal)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndoal sndoalclean
all plugins drivers snddrivers: sndoal

sndoal:
	$(MAKE_TARGET) MAKE_DLL=yes
sndoalclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/renderer/common

ifeq ($(USE_PLUGINS),yes)
  SNDOAL = $(OUTDLL)/sndoal$(DLL)
  LIB.SNDOAL = $(foreach d,$(DEP.SNDOAL),$($d.LIB))
  LIB.SNDOAL.LFLAGS = $(OPENAL.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(SNDOAL)
else
  SNDOAL = $(OUT)/$(LIB_PREFIX)sndoal$(LIB)
  DEP.EXE += $(SNDOAL)
  LIBS.EXE += $(OPENAL.LFLAGS)
  SCF.STATIC += sndoal
  TO_INSTALL.STATIC_LIBS += $(SNDOAL)
endif

INC.SNDOAL = $(wildcard plugins/sound/renderer/openal/*.h) \
  $(wildcard plugins/sound/renderer/common/*.h)
SRC.SNDOAL = $(wildcard plugins/sound/renderer/openal/*.cpp) \
  $(wildcard plugins/sound/renderer/common/*.cpp)
OBJ.SNDOAL = $(addprefix $(OUT)/,$(notdir $(SRC.SNDOAL:.cpp=$O)))
DEP.SNDOAL = CSUTIL CSGEOM CSSYS CSUTIL

MSVC.DSP += SNDOAL
DSP.SNDOAL.NAME = sndoal
DSP.SNDOAL.TYPE = plugin
DSP.SNDOAL.LIBS = openal32 alut

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndoal sndoalclean

sndoal: $(OUTDIRS) $(SNDOAL)

$(OUT)/%$O: plugins/sound/renderer/openal/%.cpp
	$(DO.COMPILE.CPP) $(OPENAL.CFLAGS)

$(SNDOAL): $(OBJ.SNDOAL) $(LIB.SNDOAL)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(OPENAL.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: sndoalclean
sndoalclean:
	$(RM) $(SNDOAL) $(OBJ.SNDOAL)

ifdef DO_DEPEND
dep: $(OUTOS)/sndoal.dep
$(OUTOS)/sndoal.dep: $(SRC.SNDOAL)
	$(DO.DEP1) \
	$(OPENAL.CFLAGS) \
	$(DO.DEP2)
else
-include $(OUTOS)/sndoal.dep
endif

endif # ifeq ($(MAKESECTION),targets)
