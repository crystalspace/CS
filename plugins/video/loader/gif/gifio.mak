# Plug-in description
DESCRIPTION.csgifimg = Crystal Space gif image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csgifimg     Make the $(DESCRIPTION.csgifimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csgifimg gifimgclean
all plugins: csgifimg

csgifimg:
	$(MAKE_TARGET) MAKE_DLL=yes
gifimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/gif

ifeq ($(USE_PLUGINS),yes)
  CSGIFIMG = $(OUTDLL)/csgifimg$(DLL)
  LIB.CSGIFIMG = $(foreach d,$(DEP.CSGIFIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSGIFIMG)
else
  CSGIFIMG = $(OUT)/$(LIB_PREFIX)csgifimg$(LIB)
  DEP.EXE += $(CSGIFIMG)
  SCF.STATIC += csgifimg
  TO_INSTALL.STATIC_LIBS += $(CSGIFIMG)
endif

INC.CSGIFIMG = $(wildcard plugins/video/loader/gif/*.h)
SRC.CSGIFIMG = $(wildcard plugins/video/loader/gif/*.cpp)

OBJ.CSGIFIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSGIFIMG:.cpp=$O)))
DEP.CSGIFIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += CSGIFIMG
DSP.CSGIFIMG.NAME = csgifimg
DSP.CSGIFIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csgifimg gifimgclean

csgifimg: $(OUTDIRS) $(CSGIFIMG)

$(CSGIFIMG): $(OBJ.CSGIFIMG) $(LIB.CSGIFIMG)
	$(DO.PLUGIN)

clean: gifimgclean
gifimgclean:
	$(RM) $(CSGIFIMG) $(OBJ.CSGIFIMG)

ifdef DO_DEPEND
dep: $(OUTOS)/gifimg.dep
$(OUTOS)/gifimg.dep: $(SRC.CSGIFIMG)
	$(DO.DEP)
else
-include $(OUTOS)/gifimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

