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
  SNDSOFT = $(OUTDLL)/sndsoft$(DLL)
  LIB.SNDSOFT = $(foreach d,$(DEP.SNDSOFT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDSOFT)
else
  SNDSOFT = $(OUT)/$(LIB_PREFIX)sndsoft$(LIB)
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

DIR.SNDSOFT = plugins/sound/renderer/software
OUT.SNDSOFT = $(OUT)/$(DIR.SNDSOFT)
INF.SNDSOFT = $(SRCDIR)/$(DIR.SNDSOFT)/sndsoft.csplugin
INC.SNDSOFT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SNDSOFT)/*.h \
  plugins/sound/renderer/common/*.h))
SRC.SNDSOFT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SNDSOFT)/*.cpp \
  plugins/sound/renderer/common/*.cpp))
OBJ.SNDSOFT = $(addprefix $(OUT.SNDSOFT)/,$(notdir $(SRC.SNDSOFT:.cpp=$O)))
DEP.SNDSOFT = CSUTIL CSGEOM CSUTIL

OUTDIRS += $(OUT.SNDSOFT)

MSVC.DSP += SNDSOFT
DSP.SNDSOFT.NAME = sndsoft
DSP.SNDSOFT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndsoft sndsoftclean sndsoftcleandep

sndsoft: $(OUTDIRS) $(SNDSOFT)

$(OUT.SNDSOFT)/%$O: $(SRCDIR)/$(DIR.SNDSOFT)/%.cpp
	$(DO.COMPILE.CPP)

$(OUT.SNDSOFT)/%$O: $(SRCDIR)/plugins/sound/renderer/common/%.cpp
	$(DO.COMPILE.CPP)

$(SNDSOFT): $(OBJ.SNDSOFT) $(LIB.SNDSOFT)
	$(DO.PLUGIN)

clean: sndsoftclean
sndsoftclean:
	-$(RMDIR) $(SNDSOFT) $(OBJ.SNDSOFT) $(OUTDLL)/$(notdir $(INF.SNDSOFT))

cleandep: sndsoftcleandep
sndsoftcleandep:
	-$(RM) $(OUT.SNDSOFT)/sndsoft.dep

ifdef DO_DEPEND
dep: $(OUT.SNDSOFT) $(OUT.SNDSOFT)/sndsoft.dep
$(OUT.SNDSOFT)/sndsoft.dep: $(SRC.SNDSOFT)
	$(DO.DEPEND)
else
-include $(OUT.SNDSOFT)/sndsoft.dep
endif

endif # ifeq ($(MAKESECTION),targets)
