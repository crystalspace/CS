# Plug-in description
DESCRIPTION.cstgaimg = Crystal Space tga image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cstgaimg     Make the $(DESCRIPTION.cstgaimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cstgaimg cstgaimgclean
all plugins imgplexall: cstgaimg

cstgaimg:
	$(MAKE_TARGET) MAKE_DLL=yes
cstgaimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/loader/tga

ifeq ($(USE_PLUGINS),yes)
  CSTGAIMG = $(OUTDLL)/cstgaimg$(DLL)
  LIB.CSTGAIMG = $(foreach d,$(DEP.CSTGAIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSTGAIMG)
else
  CSTGAIMG = $(OUT)/$(LIB_PREFIX)cstgaimg$(LIB)
  DEP.EXE += $(CSTGAIMG)
  SCF.STATIC += cstgaimg
  TO_INSTALL.STATIC_LIBS += $(CSTGAIMG)
endif

INF.CSTGAIMG = $(SRCDIR)/plugins/video/loader/tga/cstgaimg.csplugin
INC.CSTGAIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/tga/*.h))
SRC.CSTGAIMG = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/tga/*.cpp))

OBJ.CSTGAIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSTGAIMG:.cpp=$O)))
DEP.CSTGAIMG = CSUTIL CSGFX CSUTIL

MSVC.DSP += CSTGAIMG
DSP.CSTGAIMG.NAME = cstgaimg
DSP.CSTGAIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cstgaimg cstgaimgclean

cstgaimg: $(OUTDIRS) $(CSTGAIMG)

$(CSTGAIMG): $(OBJ.CSTGAIMG) $(LIB.CSTGAIMG)
	$(DO.PLUGIN)

clean: cstgaimgclean
cstgaimgclean:
	-$(RMDIR) $(CSTGAIMG) $(OBJ.CSTGAIMG) $(OUTDLL)/$(notdir $(INF.CSTGAIMG))

ifdef DO_DEPEND
dep: $(OUTOS)/tgaimg.dep
$(OUTOS)/tgaimg.dep: $(SRC.CSTGAIMG)
	$(DO.DEP)
else
-include $(OUTOS)/tgaimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

