#------------------------------------------------------------------------------
# Movierecorder subemakefile
#------------------------------------------------------------------------------

DESCRIPTION.movierecorder = Crystal Space realtime movie recorder

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make movierecorder$" \
  $(NEWLINE)echo $"                    Make the $(DESCRIPTION.movierecorder)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: movierecorder movierecorderclean
all plugins: movierecorder

movierecorder:
	$(MAKE_TARGET) MAKE_DLL=yes
movierecorderclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  MOVIERECORDER = $(OUTDLL)/movierecorder$(DLL)
  LIB.MOVIERECORDER = $(foreach d,$(DEP.MOVIERECORDER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MOVIERECORDER)
else
  MOVIERECORDER = $(OUT)/$(LIB_PREFIX)movierecorder$(LIB)
  DEP.EXE += $(MOVIERECORDER)
  SCF.STATIC += movierecorder
  TO_INSTALL.STATIC_LIBS += $(MOVIERECORDER)
endif

DIR.MOVIERECORDER = plugins/movierecorder
OUT.MOVIERECORDER = $(OUT)/$(DIR.MOVIERECORDER)
INF.MOVIERECORDER = $(SRCDIR)/$(DIR.MOVIERECORDER)/movierecorder.csplugin
INC.MOVIERECORDER = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MOVIERECORDER)/*.h))
SRC.MOVIERECORDER = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MOVIERECORDER)/*.cpp) )
OBJ.MOVIERECORDER = \
  $(addprefix $(OUT.MOVIERECORDER)/,$(notdir $(SRC.MOVIERECORDER:.cpp=$O)))
DEP.MOVIERECORDER = CSTOOL CSUTIL
CFG.MOVIERECORDER = $(SRCDIR)/data/config/movierecorder.cfg

OUTDIRS += $(OUT.MOVIERECORDER)

TO_INSTALL.CONFIG += $(CFG.MOVIERECORDER)

MSVC.DSP += MOVIERECORDER
DSP.MOVIERECORDER.NAME = movierecorder
DSP.MOVIERECORDER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: movierecorder movierecorderclean movierecordercleandep

movierecorder: $(OUTDIRS) $(MOVIERECORDER)

$(OUT.MOVIERECORDER)/%$O: $(SRCDIR)/$(DIR.MOVIERECORDER)/%.cpp
	$(DO.COMPILE.CPP)

$(MOVIERECORDER): $(OBJ.MOVIERECORDER) $(LIB.MOVIERECORDER)
	$(DO.PLUGIN)

clean: movierecorderclean
movierecorderclean:
	-$(RMDIR) $(MOVIERECORDER) $(OBJ.MOVIERECORDER) $(OUTDLL)/$(notdir $(INF.MOVIERECORDER))

cleandep: movierecordercleandep
movierecordercleandep:
	-$(RM) $(OUT.MOVIERECORDER)/movierecorder.dep

ifdef DO_DEPEND
dep: $(OUT.MOVIERECORDER) $(OUT.MOVIERECORDER)/movierecorder.dep
$(OUT.MOVIERECORDER)/movierecorder.dep: $(SRC.MOVIERECORDER)
	$(DO.DEPEND)
else
-include $(OUT.MOVIERECORDER)/movierecorder.dep
endif

endif # ifeq ($(MAKESECTION),targets)
