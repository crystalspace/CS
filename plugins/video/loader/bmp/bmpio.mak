# Plug-in description
DESCRIPTION.bmpimg = Crystal Space bmp image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make bmpimg      Make the $(DESCRIPTION.bmpimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bmpimg bmpimgclean
all plugins drivers: bmpimg

bmpimg:
	$(MAKE_TARGET) MAKE_DLL=yes
bmpimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/bmp

ifeq ($(USE_PLUGINS),yes)
  BMPIMG = $(OUTDLL)csbmpimg$(DLL)
  LIB.BMPIMG = $(foreach d,$(DEP.BMPIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BMPIMG)
else
  BMPIMG = $(OUT)$(LIB_PREFIX)csbmpimg$(LIB)
  DEP.EXE += $(BMPIMG)
  SCF.STATIC += csbmpimg
  TO_INSTALL.STATIC_LIBS += $(BMPIMG)
endif

INC.BMPIMG = $(wildcard plugins/video/loader/bmp/*.h)
SRC.BMPIMG = $(wildcard plugins/video/loader/bmp/*.cpp)

OBJ.BMPIMG = $(addprefix $(OUT),$(notdir $(SRC.BMPIMG:.cpp=$O)))
DEP.BMPIMG = CSUTIL CSSYS CSGFX CSUTIL 

MSVC.DSP += BMPIMG
DSP.BMPIMG.NAME = csbmpimg
DSP.BMPIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bmpimg bmpimgclean

bmpimg: $(OUTDIRS) $(BMPIMG)

$(BMPIMG): $(OBJ.BMPIMG) $(LIB.BMPIMG)
	$(DO.PLUGIN)

clean: bmpimgclean
bmpimgclean:
	$(RM) $(BMPIMG) $(OBJ.BMPIMG)

ifdef DO_DEPEND
dep: $(OUTOS)bmpimg.dep
$(OUTOS)bmpimg.dep: $(SRC.BMPIMG)
	$(DO.DEP)
else
-include $(OUTOS)bmpimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

