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
  ENGINE = $(OUTDLL)engine$(DLL)
  LIB.ENGINE = $(foreach d,$(DEP.ENGINE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ENGINE)
else
  ENGINE = $(OUT)$(LIB_PREFIX)engine$(LIB)
  DEP.EXE += $(ENGINE)
  SCF.STATIC += engine
  TO_INSTALL.STATIC_LIBS += $(ENGINE)
endif

INC.ENGINE = $(wildcard plugins/engine/*.h)
SRC.ENGINE = $(wildcard plugins/engine/*.cpp)
OBJ.ENGINE = $(addprefix $(OUT),$(notdir $(SRC.ENGINE:.cpp=$O)))
# @@@ Should also include "CSENGINE" but see *Mingw* note below.
DEP.ENGINE = CSGFX CSUTIL CSSYS CSENGINE CSGEOM CSOBJECT CSUTIL CSSYS

MSVC.DSP += ENGINE
DSP.ENGINE.NAME = engine
DSP.ENGINE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: engine engineclean
engine: $(OUTDIRS) $(ENGINE)

# @@@ *Mingw* Dependency should really be "$(OBJ.ENGINE) $(LIB.ENGINE)"; should
# not include $(OBJ.CSENGINE).  Unfortunately, there is a bug in `dllwrap',
# which is employed by Mingw, where it fails to emit symbols into the output
# DLL which are exported from csengine.lib.  Even though the symbols, such as
# engine_GetClassTable(), are properly declared with __declspec(dllexport) in
# csengine.lib, dllwrap fails to emit them into the generated DLL.  To work
# around this problem, instead of linking with CSENGINE.LIB as expected, we
# link individually with the object files from OBJ.CSENGINE.  The real solution
# to this problem, however, is to figure out why dllwrap is misbehaving.
$(ENGINE): $(OBJ.ENGINE) $(OBJ.CSENGINE) $(LIB.ENGINE)
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
