# Plug-in description
DESCRIPTION.csbmpimg = Crystal Space bmp image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csbmpimg     Make the $(DESCRIPTION.csbmpimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csbmpimg csbmpimgclean
all plugins imgplexall: csbmpimg

csbmpimg:
	$(MAKE_TARGET) MAKE_DLL=yes
csbmpimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/loader/bmp

ifeq ($(USE_PLUGINS),yes)
  CSBMPIMG = $(OUTDLL)/csbmpimg$(DLL)
  LIB.CSBMPIMG = $(foreach d,$(DEP.CSBMPIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSBMPIMG)
else
  CSBMPIMG = $(OUT)/$(LIB_PREFIX)csbmpimg$(LIB)
  DEP.EXE += $(CSBMPIMG)
  SCF.STATIC += csbmpimg
  TO_INSTALL.STATIC_LIBS += $(CSBMPIMG)
endif

INF.CSBMPIMG = $(SRCDIR)/plugins/video/loader/bmp/csbmpimg.csplugin
INC.CSBMPIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/bmp/*.h))
SRC.CSBMPIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/bmp/*.cpp))

OBJ.CSBMPIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSBMPIMG:.cpp=$O)))
DEP.CSBMPIMG = CSUTIL CSGFX CSUTIL

MSVC.DSP += CSBMPIMG
DSP.CSBMPIMG.NAME = csbmpimg
DSP.CSBMPIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csbmpimg csbmpimgclean

csbmpimg: $(OUTDIRS) $(CSBMPIMG)

$(CSBMPIMG): $(OBJ.CSBMPIMG) $(LIB.CSBMPIMG)
	$(DO.PLUGIN)

clean: csbmpimgclean
csbmpimgclean:
	-$(RMDIR) $(CSBMPIMG) $(OBJ.CSBMPIMG) $(OUTDLL)/$(notdir $(INF.CSBMPIMG))

ifdef DO_DEPEND
dep: $(OUTOS)/bmpimg.dep
$(OUTOS)/bmpimg.dep: $(SRC.CSBMPIMG)
	$(DO.DEP)
else
-include $(OUTOS)/bmpimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

