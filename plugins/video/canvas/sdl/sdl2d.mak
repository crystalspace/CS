# This is a subinclude file used to define the rules needed
# to build the SDL 2D driver -- sdl2d

# Driver description
DESCRIPTION.sdl2d = Crystal Space SDL 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make sdl2d        Make the $(DESCRIPTION.sdl2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sdl2d sdl2dclean
all plugins drivers drivers2d: sdl2d

sdl2d:
	$(MAKE_TARGET) MAKE_DLL=yes

sdl2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.SDL2D += $(SDL.CFLAGS)
LIB.SDL2D.SYSTEM += $(SDL.LFLAGS) $(SDL.LIBS)

ifeq ($(USE_PLUGINS),yes)
  SDL2D = $(OUTDLL)/sdl2d$(DLL)
  LIB.SDL2D = $(foreach d,$(DEP.SDL2D),$($d.LIB))
  LIB.SDL2D.SPECIAL = $(LIB.SDL2D.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(SDL2D)
else
  SDL2D = $(OUT)/$(LIB_PREFIX)sdl2d$(LIB)
  DEP.EXE += $(SDL2D)
  LIBS.EXE += $(LIB.SDL2D.SYSTEM)
  SCF.STATIC += sdl2d
  TO_INSTALL.STATIC_LIBS += $(SDL2D)
endif

INF.SDL2D = $(SRCDIR)/plugins/video/canvas/sdl/sdl2d.csplugin
INC.SDL2D = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/canvas/sdl/*.h   $(INC.COMMON.DRV2D)))
SRC.SDL2D = $(wildcard $(addprefix $(SRCDIR)/,plugins/video/canvas/sdl/*.cpp $(SRC.COMMON.DRV2D)))
OBJ.SDL2D = $(addprefix $(OUT)/,$(notdir $(SRC.SDL2D:.cpp=$O)))
DEP.SDL2D = CSUTIL CSUTIL CSGEOM

MSVC.DSP += SDL2D
DSP.SDL2D.NAME = sdl2d
DSP.SDL2D.TYPE = plugin
DSP.SDL2D.LIBS = SDL SDLmain

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sdl2d sdl2dclean

sdl2d: $(OUTDIRS) $(SDL2D)

$(OUT)/%$O: $(SRCDIR)/plugins/video/canvas/sdl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SDL2D)

$(SDL2D): $(OBJ.SDL2D) $(LIB.SDL2D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.SDL2D.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

clean: sdl2dclean
sdl2dclean:
	-$(RMDIR) $(SDL2D) $(OBJ.SDL2D) $(OUTDLL)/$(notdir $(INF.SDL2D))

ifdef DO_DEPEND
dep: $(OUTOS)/sdl2d.dep
$(OUTOS)/sdl2d.dep: $(SRC.SDL2D)
	$(DO.DEP1) $(CFLAGS.SDL2D) $(DO.DEP2)
else
-include $(OUTOS)/sdl2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
