# Plug-in description
DESCRIPTION.sndsoft = Crystal Space software sound renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndsoft      Make the $(DESCRIPTION.sndsoft)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndsoft sndsoftclean
all plugins drivers snddrivers: sndsoft

sndsoft:
	$(MAKE_TARGET) MAKE_DLL=yes
sndsoftclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/renderer/software plugins/sound/renderer/common

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
  SNDSOFT = $(OUTDLL)sndsoft$(DLL)
  LIB.SNDSOFT = $(foreach d,$(DEP.SNDSOFT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDSOFT)
else
  SNDSOFT = $(OUT)$(LIB_PREFIX)sndsoft$(LIB)
  DEP.EXE += $(SNDSOFT)
  ifeq ($(OS),WIN32)
    ifeq ($(COMP),GCC)
      LIBS.EXE += $(LIBS.DSOUND)
    else
      LIBS.EXE += $(LIBS.DSOUND)$(LIB)
    endif
  endif
  SCF.STATIC += sndsoft
  TO_INSTALL.STATIC_LIBS += $(SNDSOFT)
endif

INC.SNDSOFT = $(wildcard plugins/sound/renderer/software/*.h) \
  $(wildcard plugins/sound/renderer/common/*.h)
SRC.SNDSOFT = $(wildcard plugins/sound/renderer/software/*.cpp) \
  $(wildcard plugins/sound/renderer/common/*.cpp)
OBJ.SNDSOFT = $(addprefix $(OUT),$(notdir $(SRC.SNDSOFT:.cpp=$O)))
DEP.SNDSOFT = CSUTIL CSGEOM CSSYS

MSVC.DSP += SNDSOFT
DSP.SNDSOFT.NAME = sndsoft
DSP.SNDSOFT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndsoft sndsoftclean

sndsoft: $(OUTDIRS) $(SNDSOFT)

$(SNDSOFT): $(OBJ.SNDSOFT) $(LIB.SNDSOFT)
	$(DO.PLUGIN)

clean: sndsoftclean
sndsoftclean:
	$(RM) $(SNDSOFT) $(OBJ.SNDSOFT)

ifdef DO_DEPEND
dep: $(OUTOS)sndsoft.dep
$(OUTOS)sndsoft.dep: $(SRC.SNDSOFT)
	$(DO.DEP)
else
-include $(OUTOS)sndsoft.dep
endif

endif # ifeq ($(MAKESECTION),targets)
