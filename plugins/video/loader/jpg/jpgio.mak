# Plug-in description
DESCRIPTION.jpgimg = Crystal Space jpg image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make jpgimg       Make the $(DESCRIPTION.jpgimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: jpgimg jpgimgclean
all plugins: jpgimg

jpgimg:
	$(MAKE_TARGET) MAKE_DLL=yes
jpgimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/jpg

ifeq ($(USE_PLUGINS),yes)
  JPGIMG = $(OUTDLL)csjpgimg$(DLL)
  LIB.JPGIMG = $(foreach d,$(DEP.JPGIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(JPGIMG)
else
  JPGIMG = $(OUT)$(LIB_PREFIX)csjpgimg$(LIB)
  DEP.EXE += $(JPGIMG)
  SCF.STATIC += csjpgimg
  TO_INSTALL.STATIC_LIBS += $(JPGIMG)
endif

LIB.JPGIMG.SPECIAL += $(JPG_LIBS)

INC.JPGIMG = $(wildcard plugins/video/loader/jpg/*.h)
SRC.JPGIMG = $(wildcard plugins/video/loader/jpg/*.cpp)

OBJ.JPGIMG = $(addprefix $(OUT),$(notdir $(SRC.JPGIMG:.cpp=$O)))
DEP.JPGIMG = CSUTIL CSSYS CSGFX CSUTIL 

MSVC.DSP += JPGIMG
DSP.JPGIMG.NAME = csjpgimg
DSP.JPGIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: jpgimg jpgimgclean

jpgimg: $(OUTDIRS) $(JPGIMG)

$(JPGIMG): $(OBJ.JPGIMG) $(LIB.JPGIMG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.JPGIMG.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: jpgimgclean
jpgimgclean:
	$(RM) $(JPGIMG) $(OBJ.JPGIMG)

ifdef DO_DEPEND
dep: $(OUTOS)jpgimg.dep
$(OUTOS)jpgimg.dep: $(SRC.JPGIMG)
	$(DO.DEP)
else
-include $(OUTOS)jpgimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

