# Plug-in description
DESCRIPTION.walimg = Crystal Space wal image loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make walimg       Make the $(DESCRIPTION.walimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: walimg walimgclean
all plugins: walimg

walimg:
	$(MAKE_TARGET) MAKE_DLL=yes
walimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/loader/wal

ifeq ($(USE_PLUGINS),yes)
  WALIMG = $(OUTDLL)cswalimg$(DLL)
  LIB.WALIMG = $(foreach d,$(DEP.WALIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(WALIMG)
else
  WALIMG = $(OUT)$(LIB_PREFIX)cswalimg$(LIB)
  DEP.EXE += $(WALIMG)
  SCF.STATIC += cswalimg
  TO_INSTALL.STATIC_LIBS += $(WALIMG)
endif

INC.WALIMG = $(wildcard plugins/video/loader/wal/*.h)
SRC.WALIMG = $(wildcard plugins/video/loader/wal/*.cpp)

OBJ.WALIMG = $(addprefix $(OUT),$(notdir $(SRC.WALIMG:.cpp=$O)))
DEP.WALIMG = CSUTIL CSSYS CSGFX CSUTIL

MSVC.DSP += WALIMG
DSP.WALIMG.NAME = cswalimg
DSP.WALIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: walimg walimgclean

walimg: $(OUTDIRS) $(WALIMG)

$(WALIMG): $(OBJ.WALIMG) $(LIB.WALIMG)
	$(DO.PLUGIN)

clean: walimgclean
walimgclean:
	$(RM) $(WALIMG) $(OBJ.WALIMG)

ifdef DO_DEPEND
dep: $(OUTOS)walimg.dep
$(OUTOS)walimg.dep: $(SRC.WALIMG)
	$(DO.DEP)
else
-include $(OUTOS)walimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)

