# Plug-in description
DESCRIPTION.sndiff = Crystal Space iff sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndiff       Make the $(DESCRIPTION.sndiff)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndiff iffclean
all plugins drivers snddrivers: sndiff

sndiff:
	$(MAKE_TARGET) MAKE_DLL=yes
iffclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/iff plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  SNDIFF = $(OUTDLL)/sndiff$(DLL)
  LIB.SNDIFF = $(foreach d,$(DEP.SNDIFF),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDIFF)
else
  SNDIFF = $(OUT)/$(LIB_PREFIX)sndiff$(LIB)
  DEP.EXE += $(SNDIFF)
  SCF.STATIC += sndiff
  TO_INSTALL.STATIC_LIBS += $(SNDIFF)
endif

INC.SNDIFF = $(wildcard plugins/sound/loader/iff/*.h) \
  $(wildcard plugins/sound/loader/common/*.h)
SRC.SNDIFF = $(wildcard plugins/sound/loader/iff/*.cpp) \
  $(wildcard plugins/sound/loader/common/*.cpp)
OBJ.SNDIFF = $(addprefix $(OUT)/,$(notdir $(SRC.SNDIFF:.cpp=$O)))
DEP.SNDIFF = CSUTIL CSSYS CSUTIL

MSVC.DSP += SNDIFF
DSP.SNDIFF.NAME = sndiff
DSP.SNDIFF.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndiff iffclean

sndiff: $(OUTDIRS) $(SNDIFF)

$(SNDIFF): $(OBJ.SNDIFF) $(LIB.SNDIFF)
	$(DO.PLUGIN)

clean: iffclean
iffclean:
	$(RM) $(SNDIFF) $(OBJ.SNDIFF)

ifdef DO_DEPEND
dep: $(OUTOS)/sndiff.dep
$(OUTOS)/sndiff.dep: $(SRC.SNDIFF)
	$(DO.DEP)
else
-include $(OUTOS)/sndiff.dep
endif

endif # ifeq ($(MAKESECTION),targets)
