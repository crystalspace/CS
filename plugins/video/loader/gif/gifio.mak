# Plug-in description
DESCRIPTION.gifimg = Crystal Space gif image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make gifimg      Make the $(DESCRIPTION.gifimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gifimg gifimgclean
all plugins drivers: gifimg

gifimg:
	$(MAKE_TARGET) MAKE_DLL=yes
gifimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/gif

ifeq ($(USE_PLUGINS),yes)
  GIFIMG = $(OUTDLL)csgifimg$(DLL)
  LIB.GIFIMG = $(foreach d,$(DEP.GIFIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GIFIMG)
else
  GIFIMG = $(OUT)$(LIB_PREFIX)csgifimg$(LIB)
  DEP.EXE += $(GIFIMG)
  SCF.STATIC += csgifimg
  TO_INSTALL.STATIC_LIBS += $(GIFIMG)
endif

INC.GIFIMG = $(wildcard plugins/video/loader/gif/*.h)
SRC.GIFIMG = $(wildcard plugins/video/loader/gif/*.cpp)

OBJ.GIFIMG = $(addprefix $(OUT),$(notdir $(SRC.GIFIMG:.cpp=$O)))
DEP.GIFIMG = CSUTIL CSSYS CSGFX CSUTIL 

MSVC.DSP += GIFIMG
DSP.GIFIMG.NAME = gifimg
DSP.GIFIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gifimg gifimgclean

gifimg: $(OUTDIRS) $(GIFIMG)

$(GIFIMG): $(OBJ.GIFIMG) $(LIB.GIFIMG)
	$(DO.PLUGIN)

clean: gifimgclean
gifimgclean:
	$(RM) $(GIFIMG) $(OBJ.GIFIMG)

ifdef DO_DEPEND
dep: $(OUTOS)gifimg.dep
$(OUTOS)gifimg.dep: $(SRC.GIFIMG)
	$(DO.DEP)
else
-include $(OUTOS)gifimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

