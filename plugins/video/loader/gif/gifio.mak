# Plug-in description
DESCRIPTION.csgifimg = Crystal Space gif image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csgifimg     Make the $(DESCRIPTION.csgifimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csgifimg csgifimgclean
all plugins imgplexall: csgifimg

csgifimg:
	$(MAKE_TARGET) MAKE_DLL=yes
csgifimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/loader/gif

ifeq ($(USE_PLUGINS),yes)
  CSGIFIMG = $(OUTDLL)/csgifimg$(DLL)
  LIB.CSGIFIMG = $(foreach d,$(DEP.CSGIFIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSGIFIMG)
else
  CSGIFIMG = $(OUT)/$(LIB_PREFIX)csgifimg$(LIB)
  DEP.EXE += $(CSGIFIMG)
  SCF.STATIC += csgifimg
  TO_INSTALL.STATIC_LIBS += $(CSGIFIMG)
endif

INF.CSGIFIMG = $(SRCDIR)/plugins/video/loader/gif/csgifimg.csplugin
INC.CSGIFIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/gif/*.h))
SRC.CSGIFIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/gif/*.cpp))

OBJ.CSGIFIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSGIFIMG:.cpp=$O)))
DEP.CSGIFIMG = CSUTIL CSGFX CSUTIL

MSVC.DSP += CSGIFIMG
DSP.CSGIFIMG.NAME = csgifimg
DSP.CSGIFIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csgifimg csgifimgclean

csgifimg: $(OUTDIRS) $(CSGIFIMG)

$(CSGIFIMG): $(OBJ.CSGIFIMG) $(LIB.CSGIFIMG)
	$(DO.PLUGIN)

clean: csgifimgclean
csgifimgclean:
	-$(RMDIR) $(CSGIFIMG) $(OBJ.CSGIFIMG) $(OUTDLL)/$(notdir $(INF.CSGIFIMG))

ifdef DO_DEPEND
dep: $(OUTOS)/gifimg.dep
$(OUTOS)/gifimg.dep: $(SRC.CSGIFIMG)
	$(DO.DEP)
else
-include $(OUTOS)/gifimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

