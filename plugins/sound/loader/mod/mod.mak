# Plug-in description
DESCRIPTION.csmod = Crystal Space mod MikMod sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csmod        Make the $(DESCRIPTION.csmod)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csmod csmodclean
all plugins drivers snddrivers: csmod

csmod:
	$(MAKE_TARGET) MAKE_DLL=yes
csmodclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/mod

ifeq ($(USE_PLUGINS),yes)
  CSMOD = $(OUTDLL)sndmod$(DLL)
  LIB.CSMOD = $(foreach d,$(DEP.CSMOD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSMOD)
else
  CSMOD = $(OUT)$(LIB_PREFIX)sndmod$(LIB)
  DEP.EXE += $(CSMOD)
  SCF.STATIC += sndmod
  TO_INSTALL.STATIC_LIBS += $(CSMOD)
endif

INC.CSMOD = $(wildcard plugins/sound/loader/mod/*.h)
SRC.CSMOD = $(wildcard plugins/sound/loader/mod/*.cpp)
OBJ.CSMOD = $(addprefix $(OUT),$(notdir $(SRC.CSMOD:.cpp=$O)))
DEP.CSMOD = CSUTIL CSSYS CSUTIL

MSVC.DSP += CSMOD
DSP.CSMOD.NAME = sndmod
DSP.CSMOD.TYPE = plugin
DSP.CSMOD.LIBS = mikmod

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csmod csmodclean

csmod: $(OUTDIRS) $(CSMOD)

$(CSMOD): $(OBJ.CSMOD) $(LIB.CSMOD)
	$(DO.PLUGIN) -lmikmod

clean: csmodclean
csmodclean:
	$(RM) $(CSMOD) $(OBJ.CSMOD)

ifdef DO_DEPEND
dep: $(OUTOS)sndmod.dep
$(OUTOS)sndmod.dep: $(SRC.CSMOD)
	$(DO.DEP)
else
-include $(OUTOS)sndmod.dep
endif

endif # ifeq ($(MAKESECTION),targets)
