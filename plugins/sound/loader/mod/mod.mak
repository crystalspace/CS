# Plug-in description
DESCRIPTION.sndmod = Crystal Space mod MikMod sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndmod       Make the $(DESCRIPTION.sndmod)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndmod sndmodclean
all plugins: sndmod

sndmod:
	$(MAKE_TARGET) MAKE_DLL=yes
sndmodclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  SNDMOD = $(OUTDLL)/sndmod$(DLL)
  LIB.SNDMOD = $(foreach d,$(DEP.SNDMOD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDMOD)
else
  SNDMOD = $(OUT)/$(LIB_PREFIX)sndmod$(LIB)
  DEP.EXE += $(SNDMOD)
  SCF.STATIC += sndmod
  TO_INSTALL.STATIC_LIBS += $(SNDMOD)
endif

INC.SNDMOD = $(wildcard plugins/sound/loader/mod/*.h)
SRC.SNDMOD = $(wildcard plugins/sound/loader/mod/*.cpp)
OBJ.SNDMOD = $(addprefix $(OUT)/,$(notdir $(SRC.SNDMOD:.cpp=$O)))
DEP.SNDMOD = CSUTIL CSSYS CSUTIL

MSVC.DSP += SNDMOD
DSP.SNDMOD.NAME = sndmod
DSP.SNDMOD.TYPE = plugin
DSP.SNDMOD.LIBS = mikmod

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndmod sndmodclean
sndmod: $(OUTDIRS) $(SNDMOD)

$(OUT)/%$O: plugins/sound/loader/mod/%.cpp
	$(DO.COMPILE.CPP) $(MIKMOD.CFLAGS)

$(SNDMOD): $(OBJ.SNDMOD) $(LIB.SNDMOD)
	$(DO.PLUGIN) $(MIKMOD.LFLAGS)

clean: sndmodclean
sndmodclean:
	$(RM) $(SNDMOD) $(OBJ.SNDMOD)

ifdef DO_DEPEND
dep: $(OUTOS)/sndmod.dep
$(OUTOS)/sndmod.dep: $(SRC.SNDMOD)
	$(DO.DEP)
else
-include $(OUTOS)/sndmod.dep
endif

endif # ifeq ($(MAKESECTION),targets)
