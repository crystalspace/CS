# Plug-in description
DESCRIPTION.cswalimg = Crystal Space wal image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cswalimg     Make the $(DESCRIPTION.cswalimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cswalimg walimgclean
all plugins: cswalimg

cswalimg:
	$(MAKE_TARGET) MAKE_DLL=yes
walimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/wal

ifeq ($(USE_PLUGINS),yes)
  CSWALIMG = $(OUTDLL)/cswalimg$(DLL)
  LIB.CSWALIMG = $(foreach d,$(DEP.CSWALIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSWALIMG)
else
  WALIMG = $(OUT)/$(LIB_PREFIX)cswalimg$(LIB)
  DEP.EXE += $(CSWALIMG)
  SCF.STATIC += cswalimg
  TO_INSTALL.STATIC_LIBS += $(CSWALIMG)
endif

INC.CSWALIMG = $(wildcard plugins/video/loader/wal/*.h)
SRC.CSWALIMG = $(wildcard plugins/video/loader/wal/*.cpp)

OBJ.CSWALIMG = $(addprefix $(OUT)/,$(notdir $(SRC.CSWALIMG:.cpp=$O)))
DEP.CSWALIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += CSWALIMG
DSP.CSWALIMG.NAME = cswalimg
DSP.CSWALIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cswalimg walimgclean

cswalimg: $(OUTDIRS) $(CSWALIMG)

$(CSWALIMG): $(OBJ.CSWALIMG) $(LIB.CSWALIMG)
	$(DO.PLUGIN)

clean: walimgclean
walimgclean:
	$(RM) $(CSWALIMG) $(OBJ.CSWALIMG)

ifdef DO_DEPEND
dep: $(OUTOS)/walimg.dep
$(OUTOS)/walimg.dep: $(SRC.CSWALIMG)
	$(DO.DEP)
else
-include $(OUTOS)/walimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

