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

vpath %.cpp plugins/video/loader/jpg

LIB.CSJPGIMG.LOCAL += $(JPG_LIBS)

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
	$(DO.DEP)
else
-include $(OUTOS)/jpgimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

