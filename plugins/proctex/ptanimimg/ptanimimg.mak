DESCRIPTION.ptanimimg = Crystal Space animated image procedural texture

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)@echo $"  make ptanimimg    Make the $(DESCRIPTION.ptanimimg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ptanimimg ptanimimgclean
all plugins: ptanimimg

ptanimimg:
	$(MAKE_TARGET) MAKE_DLL=yes
ptanimimgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  PTANIMIMG = $(OUTDLL)/ptanimimg$(DLL)
  LIB.PTANIMIMG= $(foreach d,$(DEP.PTANIMIMG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PTANIMIMG)
else
  PTANIMIMG = $(OUT)/$(LIBPREFIX)ptanimimg$(LIB)
  DEP.EXE += $(PTANIMIMG)
  SCF.STATIC += ptanimimg
  TO_INSTALL.STATIC_LIBS += $(PTANIMIMG)
endif

DIR.PTANIMIMG = plugins/proctex/ptanimimg
OUT.PTANIMIMG = $(OUT)/$(DIR.PTANIMIMG)
INF.PTANIMIMG = $(SRCDIR)/$(DIR.PTANIMIMG)/ptanimimg.csplugin
INC.PTANIMIMG = $(wildcard $(SRCDIR)/$(DIR.PTANIMIMG)/*.h)
SRC.PTANIMIMG = $(wildcard $(SRCDIR)/$(DIR.PTANIMIMG)/*.cpp)
OBJ.PTANIMIMG = $(addprefix $(OUT.PTANIMIMG)/,$(notdir $(SRC.PTANIMIMG:.cpp=$O)))
DEP.PTANIMIMG = CSTOOL CSGFX CSUTIL CSUTIL

OUTDIRS += $(OUT.PTANIMIMG)

MSVC.DSP += PTANIMIMG
DSP.PTANIMIMG.NAME = ptanimimg
DSP.PTANIMIMG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),targets)

.PHONY: ptanimimg ptanimimgclean ptanimimgcleandep

ptanimimg: $(OUTDIRS) $(PTANIMIMG)

$(OUT.PTANIMIMG)/%$O: $(SRCDIR)/$(DIR.PTANIMIMG)/%.cpp
	$(DO.COMPILE.CPP)

$(PTANIMIMG): $(OBJ.PTANIMIMG) $(LIB.PTANIMIMG)
	$(DO.PLUGIN)

clean: ptanimimgclean
ptanimimgclean:
	-$(RMDIR) $(PTANIMIMG) $(OBJ.PTANIMIMG) $(OUTDLL)/$(notdir $(INF.PTANIMIMG))

cleandep: ptanimimgcleandep
ptanimimgcleandep:
	-$(RM) $(OUT.PTANIMIMG)/ptanimimg.dep

ifdef DO_DEPEND
dep: $(OUT.PTANIMIMG) $(OUT.PTANIMIMG)/ptanimimg.dep
$(OUT.PTANIMIMG)/ptanimimg.dep: $(SRC.PTANIMIMG)
	$(DO.DEPEND)
else
-include $(OUT.PTANIMIMG)/ptanimimg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
