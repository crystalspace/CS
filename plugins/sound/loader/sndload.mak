# Plug-in description
DESCRIPTION.sndload = Crystal Space sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndload      Make the $(DESCRIPTION.sndload)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndload sndloadclean
all plugins drivers snddrivers: sndload

sndload:
	$(MAKE_TARGET) MAKE_DLL=yes
sndloadclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader

ifeq ($(USE_PLUGINS),yes)
  SNDLOAD = $(OUTDLL)sndload$(DLL)
  LIB.SNDLOAD = $(foreach d,$(DEP.SNDLOAD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDLOAD)
else
  SNDLOAD = $(OUT)$(LIB_PREFIX)sndload$(LIB)
  DEP.EXE += $(SNDLOAD)
  SCF.STATIC += sndload
  TO_INSTALL.STATIC_LIBS += $(SNDLOAD)
endif

INC.SNDLOAD = $(wildcard plugins/sound/loader/*.h)
SRC.SNDLOAD = $(wildcard plugins/sound/loader/*.cpp)
OBJ.SNDLOAD = $(addprefix $(OUT),$(notdir $(SRC.SNDLOAD:.cpp=$O)))
DEP.SNDLOAD = CSUTIL CSSYS CSUTIL
CFG.SNDLOAD = data/config/sound.cfg

TO_INSTALL.CONFIG += $(CFG.SNDLOAD)

MSVC.DSP += SNDLOAD
DSP.SNDLOAD.NAME = sndload
DSP.SNDLOAD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndload sndloadclean

sndload: $(OUTDIRS) $(SNDLOAD)

$(SNDLOAD): $(OBJ.SNDLOAD) $(LIB.SNDLOAD)
	$(DO.PLUGIN)

clean: sndloadclean
sndloadclean:
	$(RM) $(SNDLOAD) $(OBJ.SNDLOAD)

ifdef DO_DEPEND
dep: $(OUTOS)sndload.dep
$(OUTOS)sndload.dep: $(SRC.SNDLOAD)
	$(DO.DEP)
else
-include $(OUTOS)sndload.dep
endif

endif # ifeq ($(MAKESECTION),targets)

#------------------------------------------------------------------- config ---#
ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)

ifeq ($(DO_AIFF),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_AIFF$">>volatile.tmp
endif
ifeq ($(DO_IFF),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_IFF$">>volatile.tmp
endif
ifeq ($(DO_WAV),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_WAV$">>volatile.tmp
endif
ifeq ($(DO_AU),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_AU$">>volatile.tmp
endif

endif # ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)
