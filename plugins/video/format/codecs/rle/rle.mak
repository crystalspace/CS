DESCRIPTION.rlecodec = Crystal Space RLE codec

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make rlecodec     Make the $(DESCRIPTION.rlecodec)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rlecodec rlecodecclean
plugins all: rlecodec

rlecodec:
	$(MAKE_TARGET) MAKE_DLL=yes
rlecodecclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/format/codecs/rle

ifeq ($(USE_PLUGINS),yes)
  RLECODEC = $(OUTDLL)/rlecodec$(DLL)
  LIB.RLECODEC = $(foreach d,$(DEP.RLECODEC),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RLECODEC)
else
  RLECODEC = $(OUT)/$(LIB_PREFIX)rlecodec$(LIB)
  DEP.EXE += $(RLECODEC)
  SCF.STATIC += rlecodec
  TO_INSTALL.STATIC_LIBS += $(RLECODEC)
endif

INF.RLECODEC = $(SRCDIR)/plugins/video/format/codecs/rle/rlecodec.csplugin
INC.RLECODEC = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/format/codecs/rle/*.h))
SRC.RLECODEC = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/format/codecs/rle/*.cpp))
OBJ.RLECODEC = $(addprefix $(OUT)/,$(notdir $(SRC.RLECODEC:.cpp=$O)))
DEP.RLECODEC = CSUTIL CSSYS
CFG.RLECODEC =

TO_INSTALL.CONFIG += $(CFG.RLECODEC)
TO_INSTALL.DATA +=

MSVC.DSP += RLECODEC
DSP.RLECODEC.NAME = rlecodec
DSP.RLECODEC.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rlecodec rlecodecclean
rlecodec: $(OUTDIRS) $(RLECODEC)

$(RLECODEC): $(OBJ.RLECODEC) $(LIB.RLECODEC)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.EXTERNAL.RLECODEC) \
	$(DO.PLUGIN.POSTAMBLE)

clean: rlecodecclean
rlecodecclean:
	-$(RMDIR) $(RLECODEC) $(OBJ.RLECODEC) $(OUTDLL)/$(notdir $(INF.RLECODEC))

ifdef DO_DEPEND
dep: $(OUTOS)/rlecodec.dep
$(OUTOS)/rlecodec.dep: $(SRC.RLECODEC)
	$(DO.DEP)
else
-include $(OUTOS)/rlecodec.dep
endif

endif # ifeq ($(MAKESECTION),targets)
