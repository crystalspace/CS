# Plug-in description
DESCRIPTION.csjpgimg = Crystal Space jpg image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csjpgimg     Make the $(DESCRIPTION.csjpgimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csjpgimg csjpgimgclean
all plugins: csjpgimg

csjpgimg:
	$(MAKE_TARGET) MAKE_DLL=yes
csjpgimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

LIB.CSJPGIMG.LOCAL += $(JPEG.LFLAGS)

ifeq ($(USE_PLUGINS),yes)
  CSJPGIMG = $(OUTDLL)/csjpgimg$(DLL)
  LIB.CSJPGIMG = $(foreach d,$(DEP.CSJPGIMG),$($d.LIB))
  LIB.CSJPGIMG.SPECIAL += $(LIB.CSJPGIMG.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(CSJPGIMG)
else
  CSJPGIMG = $(OUT)/$(LIB_PREFIX)csjpgimg$(LIB)
  DEP.EXE += $(CSJPGIMG)
  LIBS.EXE += $(LIB.CSJPGIMG.LOCAL)
  SCF.STATIC += csjpgimg
  TO_INSTALL.STATIC_LIBS += $(CSJPGIMG)
endif

INC.CSJPGIMG = $(wildcard plugins/video/loader/jpg/*.h)
SRC.CSJPGIMG = $(wildcard plugins/video/loader/jpg/*.cpp)

OBJ.CSJPGIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSJPGIMG:.cpp=$O)))
DEP.CSJPGIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += CSJPGIMG
DSP.CSJPGIMG.NAME = csjpgimg
DSP.CSJPGIMG.TYPE = plugin
DSP.CSJPGIMG.LIBS = libjpeg

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csjpgimg csjpgimgclean

csjpgimg: $(OUTDIRS) $(CSJPGIMG)

$(OUT)/%$O: plugins/video/loader/jpg/%.cpp
	$(DO.COMPILE.CPP) $(JPEG.CFLAGS)

$(CSJPGIMG): $(OBJ.CSJPGIMG) $(LIB.CSJPGIMG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSJPGIMG.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: csjpgimgclean
csjpgimgclean:
	$(RM) $(CSJPGIMG) $(OBJ.CSJPGIMG)

ifdef DO_DEPEND
dep: $(OUTOS)/jpgimg.dep
$(OUTOS)/jpgimg.dep: $(SRC.CSJPGIMG)
	$(DO.DEP1) $(JPEG.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/jpgimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
