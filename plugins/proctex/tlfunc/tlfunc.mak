DESCRIPTION.tlfunc = Function texture loader

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)@echo $"  make tlfunc       Make the $(DESCRIPTION.tlfunc)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tlfunc tlfuncclean
all plugins: tlfunc

tlfunc:
	$(MAKE_TARGET) MAKE_DLL=yes
tlfuncclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  TLFUNC = $(OUTDLL)/tlfunc$(DLL)
  LIB.TLFUNC= $(foreach d,$(DEP.TLFUNC),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(TLFUNC)
else
  TLFUNC = $(OUT)/$(LIB_PREFIX)tlfunc$(LIB)
  DEP.EXE += $(TLFUNC)
  SCF.STATIC += tlfunc
  TO_INSTALL.STATIC_LIBS += $(TLFUNC)
endif

DIR.TLFUNC = plugins/proctex/tlfunc
OUT.TLFUNC = $(OUT)/$(DIR.TLFUNC)
INF.TLFUNC = $(SRCDIR)/$(DIR.TLFUNC)/tlfunc.csplugin
INC.TLFUNC = $(wildcard $(SRCDIR)/$(DIR.TLFUNC)/*.h)
SRC.TLFUNC = $(wildcard $(SRCDIR)/$(DIR.TLFUNC)/*.cpp)
OBJ.TLFUNC = $(addprefix $(OUT.TLFUNC)/,$(notdir $(SRC.TLFUNC:.cpp=$O)))
DEP.TLFUNC = CSTOOL CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.TLFUNC)

MSVC.DSP += TLFUNC
DSP.TLFUNC.NAME = tlfunc
DSP.TLFUNC.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),targets)

.PHONY: tlfunc tlfuncclean tlfunccleandep

tlfunc: $(OUTDIRS) $(TLFUNC)

$(OUT.TLFUNC)/%$O: $(SRCDIR)/$(DIR.TLFUNC)/%.cpp
	$(DO.COMPILE.CPP)

$(TLFUNC): $(OBJ.TLFUNC) $(LIB.TLFUNC)
	$(DO.PLUGIN)

clean: tlfuncclean
tlfuncclean:
	-$(RMDIR) $(TLFUNC) $(OBJ.TLFUNC) $(OUTDLL)/$(notdir $(INF.TLFUNC))

cleandep: tlfunccleandep
tlfunccleandep:
	-$(RM) $(OUT.TLFUNC)/tlfunc.dep

ifdef DO_DEPEND
dep: $(OUT.TLFUNC) $(OUT.TLFUNC)/tlfunc.dep
$(OUT.TLFUNC)/tlfunc.dep: $(SRC.TLFUNC)
	$(DO.DEPEND)
else
-include $(OUT.TLFUNC)/tlfunc.dep
endif

endif # ifeq ($(MAKESECTION),targets)
