# Plug-in description
DESCRIPTION.cssgiimg = Crystal Space sgi image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cssgiimg     Make the $(DESCRIPTION.cssgiimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssgiimg sgiimgclean
all plugins: cssgiimg

cssgiimg:
	$(MAKE_TARGET) MAKE_DLL=yes
sgiimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/sgi

ifeq ($(USE_PLUGINS),yes)
  CSSGIIMG = $(OUTDLL)/cssgiimg$(DLL)
  LIB.CSSGIIMG = $(foreach d,$(DEP.CSSGIIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSSGIIMG)
else
  CSSGIIMG = $(OUT)/$(LIB_PREFIX)cssgiimg$(LIB)
  DEP.EXE += $(CSSGIIMG)
  SCF.STATIC += cssgiimg
  TO_INSTALL.STATIC_LIBS += $(CSSGIIMG)
endif

INC.CSSGIIMG = $(wildcard plugins/video/loader/sgi/*.h)
SRC.CSSGIIMG = $(wildcard plugins/video/loader/sgi/*.cpp)

OBJ.CSSGIIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSSGIIMG:.cpp=$O)))
DEP.CSSGIIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += CSSGIIMG
DSP.CSSGIIMG.NAME = cssgiimg
DSP.CSSGIIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssgiimg sgiimgclean

cssgiimg: $(OUTDIRS) $(CSSGIIMG)

$(CSSGIIMG): $(OBJ.CSSGIIMG) $(LIB.CSSGIIMG)
	$(DO.PLUGIN)

clean: sgiimgclean
sgiimgclean:
	$(RM) $(CSSGIIMG) $(OBJ.CSSGIIMG)

ifdef DO_DEPEND
dep: $(OUTOS)/sgiimg.dep
$(OUTOS)/sgiimg.dep: $(SRC.CSSGIIMG)
	$(DO.DEP)
else
-include $(OUTOS)/sgiimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

