# Plug-in description
DESCRIPTION.cswalimg = Crystal Space wal image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cswalimg     Make the $(DESCRIPTION.cswalimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cswalimg cswalimgclean
all plugins imgplexall: cswalimg

cswalimg:
	$(MAKE_TARGET) MAKE_DLL=yes
cswalimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/loader/wal

ifeq ($(USE_PLUGINS),yes)
  CSWALIMG = $(OUTDLL)/cswalimg$(DLL)
  LIB.CSWALIMG = $(foreach d,$(DEP.CSWALIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSWALIMG)
else
  WALIMG = $(OUT)/$(LIB_PREFIX)cswalimg$(LIB)
  DEP.EXE += $(CSWALIMG)
  SCF.STATIC += cswalimg
  TO_INSTALL.STATIC_LIBS += $(CSWALIMG)
endif

INF.CSWALIMG = $(SRCDIR)/plugins/video/loader/wal/cswalimg.csplugin
INC.CSWALIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/wal/*.h))
SRC.CSWALIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/wal/*.cpp))

OBJ.CSWALIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSWALIMG:.cpp=$O)))
DEP.CSWALIMG = CSUTIL CSGFX CSUTIL

MSVC.DSP += CSWALIMG
DSP.CSWALIMG.NAME = cswalimg
DSP.CSWALIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cswalimg cswalimgclean

cswalimg: $(OUTDIRS) $(CSWALIMG)

$(CSWALIMG): $(OBJ.CSWALIMG) $(LIB.CSWALIMG)
	$(DO.PLUGIN)

clean: cswalimgclean
cswalimgclean:
	-$(RMDIR) $(CSWALIMG) $(OBJ.CSWALIMG) $(OUTDLL)/$(notdir $(INF.CSWALIMG))

ifdef DO_DEPEND
dep: $(OUTOS)/walimg.dep
$(OUTOS)/walimg.dep: $(SRC.CSWALIMG)
	$(DO.DEP)
else
-include $(OUTOS)/walimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

