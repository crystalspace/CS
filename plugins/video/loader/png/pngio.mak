# Plug-in description
DESCRIPTION.pngimg = Crystal Space png image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make pngimg      Make the $(DESCRIPTION.pngimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pngimg pngimgclean
all plugins drivers: pngimg

pngimg:
	$(MAKE_TARGET) MAKE_DLL=yes
pngimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/png

ifeq ($(USE_PLUGINS),yes)
  PNGIMG = $(OUTDLL)cspngimg$(DLL)
  LIB.PNGIMG = $(foreach d,$(DEP.PNGIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PNGIMG)
else
  PNGIMG = $(OUT)$(LIB_PREFIX)cspngimg$(LIB)
  DEP.EXE += $(PNGIMG)
  SCF.STATIC += cspngimg
  TO_INSTALL.STATIC_LIBS += $(PNGIMG)
endif

LIB.PNGIMG.SPECIAL += $(PNG_LIBS)

INC.PNGIMG = $(wildcard plugins/video/loader/png/*.h)
SRC.PNGIMG = $(wildcard plugins/video/loader/png/*.cpp)

OBJ.PNGIMG = $(addprefix $(OUT),$(notdir $(SRC.PNGIMG:.cpp=$O)))
DEP.PNGIMG = CSUTIL CSSYS CSGFX CSUTIL 

MSVC.DSP += PNGIMG
DSP.PNGIMG.NAME = pngimg
DSP.PNGIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: pngimg pngimgclean

pngimg: $(OUTDIRS) $(PNGIMG)

$(PNGIMG): $(OBJ.PNGIMG) $(LIB.PNGIMG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.PNGIMG.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: pngimgclean
pngimgclean:
	$(RM) $(PNGIMG) $(OBJ.PNGIMG)

ifdef DO_DEPEND
dep: $(OUTOS)pngimg.dep
$(OUTOS)pngimg.dep: $(SRC.PNGIMG)
	$(DO.DEP)
else
-include $(OUTOS)pngimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

