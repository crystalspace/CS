# Plug-in description
DESCRIPTION.imgplex = Crystal Space imgplex image I/O multiplexer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make imgplex      Make the $(DESCRIPTION.imgplex)$"

PSEUDOHELP += $(NEWLINE) \
  echo $"  make imgplexall   Make imgplex and all graphic format loaders at once$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: imgplex imgplexclean imgplexall
all plugins imgplexall: imgplex

imgplex:
	$(MAKE_TARGET) MAKE_DLL=yes
imgplexclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/loader/mplex

ifeq ($(USE_PLUGINS),yes)
  IMGPLEX = $(OUTDLL)/imgplex$(DLL)
  LIB.IMGPLEX = $(foreach d,$(DEP.IMGPLEX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(IMGPLEX)
else
  IMGPLEX = $(OUT)/$(LIB_PREFIX)csimgplex$(LIB)
  DEP.EXE += $(IMGPLEX)
  SCF.STATIC += imgplex
  TO_INSTALL.STATIC_LIBS += $(IMGPLEX)
endif

INF.IMGPLEX = $(SRCDIR)/plugins/video/loader/mplex/imgplex.csplugin
INC.IMGPLEX = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/mplex/*.h))
SRC.IMGPLEX = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/loader/mplex/*.cpp))
OBJ.IMGPLEX = $(addprefix $(OUT)/,$(notdir $(SRC.IMGPLEX:.cpp=$O)))
DEP.IMGPLEX = CSUTIL CSGFX CSUTIL

MSVC.DSP += IMGPLEX
DSP.IMGPLEX.NAME = imgplex
DSP.IMGPLEX.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: imgplex imgplexclean

imgplex: $(OUTDIRS) $(IMGPLEX)

$(IMGPLEX): $(OBJ.IMGPLEX) $(LIB.IMGPLEX)
	$(DO.PLUGIN)

clean: imgplexclean
imgplexclean:
	-$(RMDIR) $(IMGPLEX) $(OBJ.IMGPLEX) $(OUTDLL)/$(notdir $(INF.IMGPLEX))

ifdef DO_DEPEND
dep: $(OUTOS)/imgplex.dep
$(OUTOS)/imgplex.dep: $(SRC.IMGPLEX)
	$(DO.DEP)
else
-include $(OUTOS)/imgplex.dep
endif

endif # ifeq ($(MAKESECTION),targets)

