#------------------------------------------------------------------------------
# Engine subemakefile
#------------------------------------------------------------------------------

DESCRIPTION.engine = Crystal Space 3D engine plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make engine       Make the $(DESCRIPTION.engine)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: engine engineclean
all plugins: engine

engine:
	$(MAKE_TARGET) MAKE_DLL=yes
engineclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  ENGINE = $(OUTDLL)/engine$(DLL)
  LIB.ENGINE = $(foreach d,$(DEP.ENGINE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ENGINE)
else
  ENGINE = $(OUT)/$(LIB_PREFIX)engine$(LIB)
  DEP.EXE += $(ENGINE)
  SCF.STATIC += engine
  TO_INSTALL.STATIC_LIBS += $(ENGINE)
endif

DIR.ENGINE = plugins/engine/3d
OUT.ENGINE = $(OUT)/$(DIR.ENGINE)
INF.ENGINE = $(SRCDIR)/$(DIR.ENGINE)/engine.csplugin
INC.ENGINE = $(wildcard $(SRCDIR)/$(DIR.ENGINE)/*.h)
SRC.ENGINE = $(wildcard $(SRCDIR)/$(DIR.ENGINE)/*.cpp)
OBJ.ENGINE = $(addprefix $(OUT.ENGINE)/,$(notdir $(SRC.ENGINE:.cpp=$O)))
DEP.ENGINE = CSTOOL CSGFX CSGEOM CSUTIL
CFG.ENGINE = $(SRCDIR)/data/config/engine.cfg \
  $(SRCDIR)/data/shader/or_lighting.xml

OUTDIRS += $(OUT.ENGINE)

TO_INSTALL.CONFIG += $(CFG.ENGINE)

MSVC.DSP += ENGINE
DSP.ENGINE.NAME = engine
DSP.ENGINE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: engine engineclean enginecleandep

engine: $(OUTDIRS) $(ENGINE)

$(OUT.ENGINE)/%$O: $(SRCDIR)/$(DIR.ENGINE)/%.cpp
	$(DO.COMPILE.CPP)

$(ENGINE): $(OBJ.ENGINE) $(LIB.ENGINE)
	$(DO.PLUGIN)

clean: engineclean
engineclean:
	-$(RMDIR) $(ENGINE) $(OBJ.ENGINE) $(OUTDLL)/$(notdir $(INF.ENGINE))

cleandep: enginecleandep
enginecleandep:
	-$(RM) $(OUT.ENGINE)/engine.dep

ifdef DO_DEPEND
dep: $(OUT.ENGINE) $(OUT.ENGINE)/engine.dep
$(OUT.ENGINE)/engine.dep: $(SRC.ENGINE)
	$(DO.DEPEND)
else
-include $(OUT.ENGINE)/engine.dep
endif

endif # ifeq ($(MAKESECTION),targets)
