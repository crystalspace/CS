# Plug-in description
DESCRIPTION.imgload = Crystal Space image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make imgload      Make the $(DESCRIPTION.imgload)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: imgload imgloadclean
all plugins drivers: imgload

imgload:
	$(MAKE_TARGET) MAKE_DLL=yes
imgloadclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader

ifeq ($(USE_PLUGINS),yes)
  IMGLOAD = $(OUTDLL)imgload$(DLL)
  LIB.IMGLOAD = $(foreach d,$(DEP.IMGLOAD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(IMGLOAD)
else
  IMGLOAD = $(OUT)$(LIB_PREFIX)imgload$(LIB)
  DEP.EXE += $(IMGLOAD)
  SCF.STATIC += imgload
  TO_INSTALL.STATIC_LIBS += $(IMGLOAD)
endif

INC.IMGLOAD = plugins/video/loader/imgload.h
SRC.IMGLOAD = plugins/video/loader/imgload.cpp

ifeq ($(DO_GIF),yes)
  INC.IMGLOAD += plugins/video/loader/gifimage.h
  SRC.IMGLOAD += plugins/video/loader/gifimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_GIF
endif
ifeq ($(DO_BMP),yes)
  INC.IMGLOAD += plugins/video/loader/bmpimage.h
  SRC.IMGLOAD += plugins/video/loader/bmpimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_BMP
endif
ifeq ($(DO_TGA),yes)
  INC.IMGLOAD += plugins/video/loader/tgaimage.h
  SRC.IMGLOAD += plugins/video/loader/tgaimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_TGA
endif
ifeq ($(DO_PNG),yes)
  INC.IMGLOAD += plugins/video/loader/pngimage.h
  SRC.IMGLOAD += plugins/video/loader/pngimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_PNG
  LIB.IMGLOAD += $(PNG_LIBS)
endif
ifeq ($(DO_JPG),yes)
  INC.IMGLOAD += plugins/video/loader/jpgimage.h
  SRC.IMGLOAD += plugins/video/loader/jpgimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_JPG
  LIB.IMGLOAD += $(JPG_LIBS)
endif
ifeq ($(DO_WAL),yes)
  INC.IMGLOAD += plugins/video/loader/walimage.h plugins/video/loader/walpal.h
  SRC.IMGLOAD += plugins/video/loader/walimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_WAL
endif
ifeq ($(DO_SGI),yes)
  INC.IMGLOAD += plugins/video/loader/sgiimage.h
  SRC.IMGLOAD += plugins/video/loader/sgiimage.cpp
  CFLAGS.IMG_FORMATS += $(CFLAGS.D)DO_SGI
endif

OBJ.IMGLOAD = $(addprefix $(OUT),$(notdir $(SRC.IMGLOAD:.cpp=$O)))
DEP.IMGLOAD = CSUTIL CSSYS CSGFX

MSVC.DSP += IMGLOAD
DSP.IMGLOAD.NAME = imgload
DSP.IMGLOAD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: imgload imgloadclean

imgload: $(OUTDIRS) $(IMGLOAD)

$(OUT)imgload$O: imgload.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.IMG_FORMATS)

$(IMGLOAD): $(OBJ.IMGLOAD) $(LIB.IMGLOAD)
	$(DO.PLUGIN)

clean: imgloadclean
imgloadclean:
	$(RM) $(IMGLOAD) $(OBJ.IMGLOAD)

ifdef DO_DEPEND
dep: $(OUTOS)imgload.dep
$(OUTOS)imgload.dep: $(SRC.IMGLOAD)
	$(DO.DEP)
else
-include $(OUTOS)imgload.dep
endif

endif # ifeq ($(MAKESECTION),targets)

