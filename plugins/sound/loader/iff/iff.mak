# Plug-in description
DESCRIPTION.iff = Crystal Space iff sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make iff          Make the $(DESCRIPTION.iff)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: iff iffclean
all plugins drivers snddrivers: iff

iff:
	$(MAKE_TARGET) MAKE_DLL=yes
iffclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/iff plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  IFF = $(OUTDLL)sndiff$(DLL)
  LIB.IFF = $(foreach d,$(DEP.IFF),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(IFF)
else
  IFF = $(OUT)$(LIB_PREFIX)sndiff$(LIB)
  DEP.EXE += $(IFF)
  SCF.STATIC += sndiff
  TO_INSTALL.STATIC_LIBS += $(IFF)
endif

INC.IFF = $(wildcard plugins/sound/loader/iff/*.h) $(wildcard plugins/sound/loader/common/*.h)
SRC.IFF = $(wildcard plugins/sound/loader/iff/*.cpp) $(wildcard plugins/sound/loader/common/*.cpp)
OBJ.IFF = $(addprefix $(OUT),$(notdir $(SRC.IFF:.cpp=$O)))
DEP.IFF = CSUTIL CSSYS CSUTIL
CFG.IFF = data/config/sound.cfg

TO_INSTALL.CONFIG += $(CFG.IFF)

MSVC.DSP += IFF
DSP.IFF.NAME = sndiff
DSP.IFF.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: iff iffclean

iff: $(OUTDIRS) $(IFF)

$(IFF): $(OBJ.IFF) $(LIB.IFF)
	$(DO.PLUGIN)

clean: iffclean
iffclean:
	$(RM) $(IFF) $(OBJ.IFF)

ifdef DO_DEPEND
dep: $(OUTOS)sndiff.dep
$(OUTOS)sndiff.dep: $(SRC.IFF)
	$(DO.DEP)
else
-include $(OUTOS)sndiff.dep
endif

endif # ifeq ($(MAKESECTION),targets)

