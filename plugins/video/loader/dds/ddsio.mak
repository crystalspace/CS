# Plug-in description
DESCRIPTION.csddsimg = Crystal Space dds image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csddsimg     Make the $(DESCRIPTION.csddsimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csddsimg csddsimgclean
all plugins imgplexall: csddsimg

csddsimg:
	$(MAKE_TARGET) MAKE_DLL=yes
csddsimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/loader/dds

ifeq ($(USE_PLUGINS),yes)
  CSDDSIMG = $(OUTDLL)/csddsimg$(DLL)
  LIB.CSDDSIMG = $(foreach d,$(DEP.CSDDSIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSDDSIMG)
else
  CSDDSIMG = $(OUT)/$(LIB_PREFIX)csddsimg$(LIB)
  DEP.EXE += $(CSDDSIMG)
  SCF.STATIC += csddsimg
  TO_INSTALL.STATIC_LIBS += $(CSDDSIMG)
endif

INF.CSDDSIMG = $(SRCDIR)/plugins/video/loader/dds/csddsimg.csplugin
INC.CSDDSIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/dds/*.h))
SRC.CSDDSIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/dds/*.cpp))

OBJ.CSDDSIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSDDSIMG:.cpp=$O)))
DEP.CSDDSIMG = CSUTIL CSGFX CSUTIL

MSVC.DSP += CSDDSIMG
DSP.CSDDSIMG.NAME = csddsimg
DSP.CSDDSIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csddsimg csddsimgclean

csddsimg: $(OUTDIRS) $(CSDDSIMG)

$(CSDDSIMG): $(OBJ.CSDDSIMG) $(LIB.CSDDSIMG)
	$(DO.PLUGIN)

clean: csddsimgclean
csddsimgclean:
	-$(RMDIR) $(CSDDSIMG) $(OBJ.CSDDSIMG) $(OUTDLL)/$(notdir $(INF.CSDDSIMG))

ifdef DO_DEPEND
dep: $(OUTOS)/ddsimg.dep
$(OUTOS)/ddsimg.dep: $(SRC.CSDDSIMG)
	$(DO.DEP)
else
-include $(OUTOS)/ddsimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
