#------------------------------------------------------------------------------#
#                  Submakefile for Crystal Space utility library               #
#------------------------------------------------------------------------------#

# Library description
DESCRIPTION.csgfxldr = Crystal Space graphics image file loader

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csgfxldr     Make the $(DESCRIPTION.csgfxldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csgfxldr

all libs: csgfxldr
csgfxldr:
	$(MAKE_TARGET)
csgfxldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csgfxldr

SRC.CSGFXLDR = libs/csgfxldr/csimage.cpp libs/csgfxldr/imgload.cpp \
  libs/csgfxldr/quantize.cpp

ifeq ($(DO_GIF),yes)
  SRC.CSGFXLDR+=libs/csgfxldr/gifimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_GIF
endif
ifeq ($(DO_BMP),yes)
  SRC.CSGFXLDR+=libs/csgfxldr/bmpimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_BMP
endif
ifeq ($(DO_TGA),yes)
  SRC.CSGFXLDR+=libs/csgfxldr/tgaimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_TGA
endif
ifeq ($(DO_PNG),yes)
  SRC.CSGFXLDR+=libs/csgfxldr/pngimage.cpp libs/csgfxldr/pngsave.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_PNG
  LIBS.EXE+=$(PNG_LIBS)
endif
ifeq ($(DO_JPG),yes)
  SRC.CSGFXLDR+=libs/csgfxldr/jpgimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_JPG
  LIBS.EXE+=$(JPG_LIBS)
endif
ifeq ($(DO_WAL),yes)
  SRC.CSGFXLDR+=libs/csgfxldr/walimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_WAL
endif
ifeq ($(DO_SGI),yes)
  SRC.CSGFXLDR+=libs/csgfxldr/sgiimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_SGI
endif

CSGFXLDR.LIB = $(OUT)$(LIB_PREFIX)csgfxldr$(LIB_SUFFIX)
OBJ.CSGFXLDR = $(addprefix $(OUT),$(notdir $(SRC.CSGFXLDR:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csgfxldr csgfxldrclean

all: $(CSGFXLDR.LIB)
csgfxldr: $(OUTDIRS) $(CSGFXLDR.LIB)
clean: csgfxldrclean

$(OUT)imgload$O: imgload.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.IMG_FORMATS)

$(CSGFXLDR.LIB): $(OBJ.CSGFXLDR)
	$(DO.LIBRARY)

csgfxldrclean:
	-$(RM) $(CSGFXLDR.LIB) $(OBJ.CSGFXLDR)

ifdef DO_DEPEND
depend: $(OUTOS)csgfxldr.dep
$(OUTOS)csgfxldr.dep: $(SRC.CSGFXLDR)
	$(DO.DEP)
else
-include $(OUTOS)csgfxldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
