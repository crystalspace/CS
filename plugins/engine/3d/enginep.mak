#------------------------------------------------------------------------------
# Engine plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.engine = Crystal Space 3D engine plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

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

vpath %.cpp plugins/engine/3d

ifeq ($(USE_PLUGINS),yes)
  ENGINE = $(OUTDLL)engine$(DLL)
  LIB.ENGINE = $(foreach d,$(DEP.ENGINE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ENGINE)
else
  ENGINE = $(OUT)$(LIB_PREFIX)engine$(LIB)
  DEP.EXE += $(ENGINE)
  SCF.STATIC += engine
  TO_INSTALL.STATIC_LIBS += $(ENGINE)
endif

INC.ENGINE = $(wildcard plugins/engine/3d/*.h)
SRC.ENGINE = $(wildcard plugins/engine/3d/*.cpp)
OBJ.ENGINE = $(addprefix $(OUT),$(notdir $(SRC.ENGINE:.cpp=$O)))
DEP.ENGINE = CSENGINE CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS

MSVC.DSP += ENGINE
DSP.ENGINE.NAME = engine
DSP.ENGINE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: engine engineclean
engine: $(OUTDIRS) $(ENGINE)

$(ENGINE): $(OBJ.ENGINE) $(LIB.ENGINE)
	$(DO.PLUGIN)

clean: engineclean
engineclean:
	-$(RM) $(ENGINE) $(OBJ.ENGINE)

ifdef DO_DEPEND
dep: $(OUTOS)engine.dep
$(OUTOS)engine.dep: $(SRC.ENGINE)
	$(DO.DEP)
else
-include $(OUTOS)engine.dep
endif

endif # ifeq ($(MAKESECTION),targets)
