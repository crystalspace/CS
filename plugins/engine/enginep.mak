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

vpath %.cpp plugins/engine

ifeq ($(USE_PLUGINS),yes)
  ENGINE = $(OUTDLL)enginep$(DLL)
  LIB.ENGINE = $(foreach d,$(DEP.ENGINE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ENGINE)
else
  ENGINE = $(OUT)$(LIB_PREFIX)enginep$(LIB)
  DEP.EXE += $(ENGINE)
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_ENGINE
  TO_INSTALL.STATIC_LIBS += $(ENGINE)
endif

INC.ENGINE = $(wildcard plugins/engine/*.h)
SRC.ENGINE = $(wildcard plugins/engine/*.cpp)
OBJ.ENGINE = $(addprefix $(OUT),$(notdir $(SRC.ENGINE:.cpp=$O)))
DEP.ENGINE = CSENGINE CSTERR CSUTIL CSSYS CSGEOM CSOBJECT CSGFXLDR CSUTIL CSSYS

MSVC.DSP += ENGINE
DSP.ENGINE.NAME = enginep
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
	-$(RM) $(ENGINE) $(OBJ.ENGINE) $(OUTOS)engine.dep

ifdef DO_DEPEND
dep: $(OUTOS)engine.dep
$(OUTOS)engine.dep: $(SRC.ENGINE)
	$(DO.DEP)
else
-include $(OUTOS)engine.dep
endif

endif # ifeq ($(MAKESECTION),targets)
