# Plug-in description
DESCRIPTION.sndplex = Crystal Space sndplex sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndplex      Make the $(DESCRIPTION.sndplex)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndplex sndplexclean
all plugins drivers snddrivers: sndplex

sndplex:
	$(MAKE_TARGET) MAKE_DLL=yes
sndplexclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/mplex plugins/sound/loader/common

ifeq ($(USE_PLUGINS),yes)
  SNDPLEX = $(OUTDLL)sndplex$(DLL)
  LIB.SNDPLEX = $(foreach d,$(DEP.SNDPLEX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDPLEX)
else
  SNDPLEX = $(OUT)$(LIB_PREFIX)sndplex$(LIB)
  DEP.EXE += $(SNDPLEX)
  SCF.STATIC += sndplex
  TO_INSTALL.STATIC_LIBS += $(SNDPLEX)
endif

INC.SNDPLEX = $(wildcard plugins/sound/loader/mplex/*.h) $(wildcard plugins/sound/loader/common/*.h)
SRC.SNDPLEX = $(wildcard plugins/sound/loader/mplex/*.cpp) $(wildcard plugins/sound/loader/common/*.cpp)
OBJ.SNDPLEX = $(addprefix $(OUT),$(notdir $(SRC.SNDPLEX:.cpp=$O)))
DEP.SNDPLEX = CSUTIL CSSYS CSUTIL
CFG.SNDPLEX = data/config/sound.cfg

TO_INSTALL.CONFIG += $(CFG.SNDPLEX)

MSVC.DSP += SNDPLEX
DSP.SNDPLEX.NAME = sndplex
DSP.SNDPLEX.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndplex sndplexclean

sndplex: $(OUTDIRS) $(SNDPLEX)

$(SNDPLEX): $(OBJ.SNDPLEX) $(LIB.SNDPLEX)
	$(DO.PLUGIN)

clean: sndplexclean
sndplexclean:
	$(RM) $(SNDPLEX) $(OBJ.SNDPLEX)

ifdef DO_DEPEND
dep: $(OUTOS)sndplex.dep
$(OUTOS)sndplex.dep: $(SRC.SNDPLEX)
	$(DO.DEP)
else
-include $(OUTOS)sndplex.dep
endif

endif # ifeq ($(MAKESECTION),targets)

