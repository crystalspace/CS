DESCRIPTION.stdpt = Standard procedural textures

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)@echo $"  make stdpt        Make the $(DESCRIPTION.stdpt)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stdpt stdptclean
all plugins: stdpt

stdpt:
	$(MAKE_TARGET) MAKE_DLL=yes
stdptclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  STDPT = $(OUTDLL)/stdpt$(DLL)
  LIB.STDPT= $(foreach d,$(DEP.STDPT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(STDPT)
else
  STDPT = $(OUT)/$(LIB_PREFIX)stdpt$(LIB)
  DEP.EXE += $(STDPT)
  SCF.STATIC += stdpt
  TO_INSTALL.STATIC_LIBS += $(STDPT)
endif

DIR.STDPT = plugins/proctex/standard
OUT.STDPT = $(OUT)/$(DIR.STDPT)
INF.STDPT = $(SRCDIR)/$(DIR.STDPT)/stdpt.csplugin
INC.STDPT = $(wildcard $(SRCDIR)/$(DIR.STDPT)/*.h)
SRC.STDPT = $(wildcard $(SRCDIR)/$(DIR.STDPT)/*.cpp)
OBJ.STDPT = $(addprefix $(OUT.STDPT)/,$(notdir $(SRC.STDPT:.cpp=$O)))
DEP.STDPT = CSTOOL CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.STDPT)

MSVC.DSP += STDPT
DSP.STDPT.NAME = stdpt
DSP.STDPT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),targets)

.PHONY: stdpt stdptclean stdptcleandep

stdpt: $(OUTDIRS) $(STDPT)

$(OUT.STDPT)/%$O: $(SRCDIR)/$(DIR.STDPT)/%.cpp
	$(DO.COMPILE.CPP)

$(STDPT): $(OBJ.STDPT) $(LIB.STDPT)
	$(DO.PLUGIN)

clean: stdptclean
stdptclean:
	-$(RMDIR) $(STDPT) $(OBJ.STDPT) $(OUTDLL)/$(notdir $(INF.STDPT))

cleandep: stdptcleandep
stdptcleandep:
	-$(RM) $(OUT.STDPT)/stdpt.dep

ifdef DO_DEPEND
dep: $(OUT.STDPT) $(OUT.STDPT)/stdpt.dep
$(OUT.STDPT)/stdpt.dep: $(SRC.STDPT)
	$(DO.DEPEND)
else
-include $(OUT.STDPT)/stdpt.dep
endif

endif # ifeq ($(MAKESECTION),targets)
