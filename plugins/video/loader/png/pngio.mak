# Plug-in description
DESCRIPTION.cspngimg = Crystal Space png image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cspngimg     Make the $(DESCRIPTION.cspngimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cspngimg pngimgclean
all plugins: cspngimg

cspngimg:
	$(MAKE_TARGET) MAKE_DLL=yes
pngimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/png

LIB.CSPNGIMG.LOCAL += $(PNG_LIBS) $(Z_LIBS)

ifeq ($(USE_PLUGINS),yes)
  CSPNGIMG = $(OUTDLL)/cspngimg$(DLL)
  LIB.CSPNGIMG = $(foreach d,$(DEP.CSPNGIMG),$($d.LIB))
  LIB.CSPNGIMG.SPECIAL += $(LIB.CSPNGIMG.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(CSPNGIMG)
else
  CSPNGIMG = $(OUT)/$(LIB_PREFIX)cspngimg$(LIB)
  DEP.EXE += $(CSPNGIMG)
  LIBS.EXE += $(LIB.CSPNGIMG.LOCAL)
  SCF.STATIC += cspngimg
  TO_INSTALL.STATIC_LIBS += $(CSPNGIMG)
endif

INC.CSPNGIMG = $(wildcard plugins/video/loader/png/*.h)
SRC.CSPNGIMG = $(wildcard plugins/video/loader/png/*.cpp)

OBJ.CSPNGIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSPNGIMG:.cpp=$O)))
DEP.CSPNGIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += CSPNGIMG
DSP.CSPNGIMG.NAME = cspngimg
DSP.CSPNGIMG.TYPE = plugin
#DSP.CSPNGIMG.LIBS = libpng
DSP.CSPNGIMG.LIBS = png

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cspngimg pngimgclean

cspngimg: $(OUTDIRS) $(CSPNGIMG)

$(CSPNGIMG): $(OBJ.CSPNGIMG) $(LIB.CSPNGIMG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSPNGIMG.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: pngimgclean
pngimgclean:
	$(RM) $(CSPNGIMG) $(OBJ.CSPNGIMG)

ifdef DO_DEPEND
dep: $(OUTOS)/pngimg.dep
$(OUTOS)/pngimg.dep: $(SRC.CSPNGIMG)
	$(DO.DEP)
else
-include $(OUTOS)/pngimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

