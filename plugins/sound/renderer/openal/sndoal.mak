# Plug-in description
DESCRIPTION.sndoal = Crystal Space openal sound renderer

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

vpath %.cpp plugins/sound/renderer/openal plugins/sound/renderer/common

LIB.EXTERNAL.sndoal = -lopenal

# COMP_GCC Linker assumes static libs have extension '.a'.  Mingw/Cygwin both
# use libdsound.a (static lib) as the place from which to get MS DirectSound.
ifeq ($(OS),WIN32)
  ifeq ($(COMP),GCC)
    LIBS.DSOUND += $(LFLAGS.l)dsound
  else
    LIBS.DSOUND += $(LFLAGS.l)dsound$(LIB)
  endif
endif

ifeq ($(USE_PLUGINS),yes)
  SNDOAL = $(OUTDLL)/sndoal$(DLL)
  LIB.SNDOAL = $(foreach d,$(DEP.SNDOAL),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDOAL)
else
  SNDOAL = $(OUT)/$(LIB_PREFIX)sndoal$(LIB)
  DEP.EXE += $(SNDOAL)
  ifeq ($(OS),WIN32)
    ifeq ($(COMP),GCC)
      LIBS.EXE += $(LIBS.DSOUND)
    else
      LIBS.EXE += $(LIBS.DSOUND)$(LIB)
    endif
  endif
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

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndoal sndoalclean

sndoal: $(OUTDIRS) $(SNDOAL)

$(SNDOAL): $(OBJ.SNDOAL) $(LIB.SNDOAL)
	$(DO.PLUGIN) $(LIB.EXTERNAL.sndoal)

clean: sndoalclean
sndoalclean:
	$(RM) $(SNDOAL) $(OBJ.SNDOAL)

ifdef DO_DEPEND
dep: $(OUTOS)/sndoal.dep
$(OUTOS)/sndoal.dep: $(SRC.SNDOAL)
	$(DO.DEP)
else
-include $(OUTOS)/sndoal.dep
endif

endif # ifeq ($(MAKESECTION),targets)
