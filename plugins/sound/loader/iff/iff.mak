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

.PHONY: sndiff sndiffclean
all plugins: sndiff

sndiff:
	$(MAKE_TARGET) MAKE_DLL=yes
sndiffclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/sound/loader/iff $(SRCDIR)/plugins/sound/loader/common

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

INF.SNDIFF = $(SRCDIR)/plugins/sound/loader/iff/sndiff.csplugin
INC.SNDIFF = $(wildcard $(addprefix $(SRCDIR)/, \
  plugins/sound/loader/iff/*.h plugins/sound/loader/common/*.h))
SRC.SNDIFF = $(wildcard $(addprefix $(SRCDIR)/, \
  plugins/sound/loader/iff/*.cpp plugins/sound/loader/common/*.cpp))
OBJ.SNDIFF = $(addprefix $(OUT)/,$(notdir $(SRC.SNDIFF:.cpp=$O)))
DEP.SNDIFF = CSUTIL CSUTIL

MSVC.DSP += SNDIFF
DSP.SNDIFF.NAME = sndiff
DSP.SNDIFF.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndiff sndiffclean

sndiff: $(OUTDIRS) $(SNDIFF)

$(SNDIFF): $(OBJ.SNDIFF) $(LIB.SNDIFF)
	$(DO.PLUGIN)

clean: sndiffclean
sndiffclean:
	-$(RMDIR) $(SNDIFF) $(OBJ.SNDIFF) $(OUTDLL)/$(notdir $(INF.SNDIFF))

ifdef DO_DEPEND
dep: $(OUTOS)/sndiff.dep
$(OUTOS)/sndiff.dep: $(SRC.SNDIFF)
	$(DO.DEP)
else
-include $(OUTOS)/sndiff.dep
endif

endif # ifeq ($(MAKESECTION),targets)
