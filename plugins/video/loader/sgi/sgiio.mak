# Plug-in description
DESCRIPTION.sgiimg = Crystal Space sgi image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sgiimg       Make the $(DESCRIPTION.sgiimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sgiimg sgiimgclean
all plugins: sgiimg

sgiimg:
	$(MAKE_TARGET) MAKE_DLL=yes
sgiimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/sgi

ifeq ($(USE_PLUGINS),yes)
  SGIIMG = $(OUTDLL)cssgiimg$(DLL)
  LIB.SGIIMG = $(foreach d,$(DEP.SGIIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SGIIMG)
else
  SGIIMG = $(OUT)$(LIB_PREFIX)cssgiimg$(LIB)
  DEP.EXE += $(SGIIMG)
  SCF.STATIC += cssgiimg
  TO_INSTALL.STATIC_LIBS += $(SGIIMG)
endif

INC.SGIIMG = $(wildcard plugins/video/loader/sgi/*.h)
SRC.SGIIMG = $(wildcard plugins/video/loader/sgi/*.cpp)

OBJ.SGIIMG = $(addprefix $(OUT),$(notdir $(SRC.SGIIMG:.cpp=$O)))
DEP.SGIIMG = CSUTIL CSSYS CSGFX CSUTIL 

MSVC.DSP += SGIIMG
DSP.SGIIMG.NAME = cssgiimg
DSP.SGIIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sgiimg sgiimgclean

sgiimg: $(OUTDIRS) $(SGIIMG)

$(SGIIMG): $(OBJ.SGIIMG) $(LIB.SGIIMG)
	$(DO.PLUGIN)

clean: sgiimgclean
sgiimgclean:
	$(RM) $(SGIIMG) $(OBJ.SGIIMG)

ifdef DO_DEPEND
dep: $(OUTOS)sgiimg.dep
$(OUTOS)sgiimg.dep: $(SRC.SGIIMG)
	$(DO.DEP)
else
-include $(OUTOS)sgiimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

