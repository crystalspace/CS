DESCRIPTION.rle = Crystal Space RLE codec

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make rle        Make the $(DESCRIPTION.rle)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rle rleclean
plugins all: rle

rle:
	$(MAKE_TARGET) MAKE_DLL=yes
rleclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/format/codecs/rle

ifeq ($(USE_PLUGINS),yes)
  RLE = $(OUTDLL)rlecodec$(DLL)
  LIB.RLE = $(foreach d,$(DEP.RLE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RLE)
else
  RLE = $(OUT)$(LIB_PREFIX)rlecodec$(LIB)
  DEP.EXE += $(RLE)
  SCF.STATIC += rle
  TO_INSTALL.STATIC_LIBS += $(RLE)
endif

INC.RLE = $(wildcard plugins/video/format/codecs/rle/*.h)
SRC.RLE = $(wildcard plugins/video/format/codecs/rle/*.cpp)
OBJ.RLE = $(addprefix $(OUT),$(notdir $(SRC.RLE:.cpp=$O)))
DEP.RLE = CSUTIL CSSYS
CFG.RLE = 

TO_INSTALL.CONFIG += $(CFG.RLE)
TO_INSTALL.DATA += 

MSVC.DSP += RLE
DSP.RLE.NAME = rle
DSP.RLE.TYPE = plugin


endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rle rleclean
rle: $(OUTDIRS) $(RLE)

$(RLE): $(OBJ.RLE) $(LIB.RLE)
	$(DO.PLUGIN) $(LIB.EXTERNAL.RLE)

clean: rleclean
rleclean:
	-$(RM) $(RLE) $(OBJ.RLE)

ifdef DO_DEPEND
dep: $(OUTOS)rle.dep
$(OUTOS)rle.dep: $(SRC.RLE)
	$(DO.DEP)
else
-include $(OUTOS)rle.dep
endif

endif # ifeq ($(MAKESECTION),targets)
