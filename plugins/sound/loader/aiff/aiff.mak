# Plug-in description
DESCRIPTION.aiff = Crystal Space aiff sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make aiff         Make the $(DESCRIPTION.aiff)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: aiff aiffclean
all plugins drivers snddrivers: aiff

aiff:
	$(MAKE_TARGET) MAKE_DLL=yes
aiffclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/aiff plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  AIFF = $(OUTDLL)sndaiff$(DLL)
  LIB.AIFF = $(foreach d,$(DEP.AIFF),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(AIFF)
else
  AIFF = $(OUT)$(LIB_PREFIX)sndaiff$(LIB)
  DEP.EXE += $(AIFF)
  SCF.STATIC += sndaiff
  TO_INSTALL.STATIC_LIBS += $(AIFF)
endif

INC.AIFF = $(wildcard plugins/sound/loader/aiff/*.h) \
  $(wildcard plugins/sound/loader/common/*.h)
SRC.AIFF = $(wildcard plugins/sound/loader/aiff/*.cpp) \
  $(wildcard plugins/sound/loader/common/*.cpp)
OBJ.AIFF = $(addprefix $(OUT),$(notdir $(SRC.AIFF:.cpp=$O)))
DEP.AIFF = CSUTIL CSSYS CSUTIL

MSVC.DSP += AIFF
DSP.AIFF.NAME = sndaiff
DSP.AIFF.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: aiff aiffclean

aiff: $(OUTDIRS) $(AIFF)

$(AIFF): $(OBJ.AIFF) $(LIB.AIFF)
	$(DO.PLUGIN)

clean: aiffclean
aiffclean:
	$(RM) $(AIFF) $(OBJ.AIFF)

ifdef DO_DEPEND
dep: $(OUTOS)sndaiff.dep
$(OUTOS)sndaiff.dep: $(SRC.AIFF)
	$(DO.DEP)
else
-include $(OUTOS)sndaiff.dep
endif

endif # ifeq ($(MAKESECTION),targets)
