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

# We need also the SDL libs
CFLAGS.SDL2D += `sdl-config --cflags`
LIBS.SDL2D += `sdl-config --libs` -ldl

ifeq ($(USE_SHARED_PLUGINS),yes)
  SDL2D = $(OUTDLL)sdl2d$(DLL)
  LIBS.LOCAL.SDL2D = $(LIBS.SDL2D)
  DEP.SDL2D = $(CSUTIL.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(SDL2D)
else
  SDL2D = $(OUT)$(LIB_PREFIX)sdl2d$(LIB)
  DEP.EXE += $(SDL2D)
  LIBS.EXE += $(LIBS.SDL2D)
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_SDL2D
  TO_INSTALL.STATIC_LIBS+=$(SDL2D)
endif
DESCRIPTION.$(SDL2D) = $(DESCRIPTION.sdl2d)
SRC.SDL2D = $(wildcard plugins/video/canvas/sdl/*.cpp $(SRC.COMMON.DRV2D))
OBJ.SDL2D = $(addprefix $(OUT),$(notdir $(SRC.SDL2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sdl2d sdl2dclean

# Chain rules
clean: sdl2dclean

sdl2d: $(OUTDIRS) $(SDL2D)

$(OUT)%$O: plugins/video/canvas/sdl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SDL2D)
 
$(SDL2D): $(OBJ.SDL2D) $(DEP.SDL2D)
	$(DO.PLUGIN) $(LIBS.LOCAL.SDL2D)

sdl2dclean:
	$(RM) $(SDL2D) $(OBJ.SDL2D) $(OUTOS)sdl2d.dep

ifdef DO_DEPEND
dep: $(OUTOS)sdl2d.dep
$(OUTOS)sdl2d.dep: $(SRC.SDL2D)
	$(DO.DEP1) $(CFLAGS.SDL2D) $(DO.DEP2)
else
-include $(OUTOS)sdl2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
