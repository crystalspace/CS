# Plug-in description
DESCRIPTION.csjngimg = Crystal Space jng image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csjngimg     Make the $(DESCRIPTION.csjngimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csjngimg csjngimgclean
all plugins: csjngimg

csjngimg:
	$(MAKE_TARGET) MAKE_DLL=yes
csjngimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# @@@ FIXME: In the future, auto-detect this.
ifeq (,$(MNG.LFLAGS))
  MNG.LFLAGS = $(LFLAGS.l)mng
endif

LIB.CSJNGIMG.LOCAL += $(MNG.LFLAGS) $(ZLIB.LFLAGS) $(JPEG.LFLAGS)

ifeq ($(USE_PLUGINS),yes)
  CSJNGIMG = $(OUTDLL)/csjngimg$(DLL)
  LIB.CSJNGIMG = $(foreach d,$(DEP.CSJNGIMG),$($d.LIB))
  LIB.CSJNGIMG.SPECIAL += $(LIB.CSJNGIMG.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(CSJNGIMG)
else
  CSJNGIMG = $(OUT)/$(LIB_PREFIX)csjngimg$(LIB)
  DEP.EXE += $(CSJNGIMG)
  LIBS.EXE += $(LIB.CSJNGIMG.LOCAL)
  SCF.STATIC += csjngimg
  TO_INSTALL.STATIC_LIBS += $(CSJNGIMG)
endif

INC.CSJNGIMG = $(wildcard plugins/video/loader/jng/*.h)
SRC.CSJNGIMG = $(wildcard plugins/video/loader/jng/*.cpp)

OBJ.CSJNGIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSJNGIMG:.cpp=$O)))
DEP.CSJNGIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += CSJNGIMG
DSP.CSJNGIMG.NAME = csjngimg
DSP.CSJNGIMG.TYPE = plugin
#DSP.CSJNGIMG.LIBS = libmng libz libjpeg
DSP.CSJNGIMG.LIBS = libmng zlib libjpeg

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csjngimg csjngimgclean

csjngimg: $(OUTDIRS) $(CSJNGIMG)

$(OUT)/%$O: plugins/video/loader/jng/%.cpp
	$(DO.COMPILE.CPP) $(MNG.CFLAGS) $(JPEG.CFLAGS) $(ZLIB.CFLAGS)

$(CSJNGIMG): $(OBJ.CSJNGIMG) $(LIB.CSJNGIMG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.CSJNGIMG.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: csjngimgclean
csjngimgclean:
	$(RM) $(CSJNGIMG) $(OBJ.CSJNGIMG)

ifdef DO_DEPEND
dep: $(OUTOS)/jngimg.dep
$(OUTOS)/jngimg.dep: $(SRC.CSJNGIMG)
	$(DO.DEP1) $(MNG.CFLAGS) $(JPEG.CFLAGS) $(ZLIB.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/jngimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

