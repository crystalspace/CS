# This is a subinclude file used to define the rules needed
# to build the NEW (Render3d) OpenGL 3D driver -- gl_render3d

# Driver description
DESCRIPTION.gl_render3d = Crystal Space Render3d OpenGL driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make gl3d         Make the $(DESCRIPTION.gl3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gl_render3d
all plugins drivers drivers3d: gl_render3d

gl_render3d:
	$(MAKE_TARGET) MAKE_DLL=yes
gl_render3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifneq (,$(strip $(LIBS.OPENGL.SYSTEM)))
  LIB.gl_render3d.LOCAL += $(LIBS.OPENGL.SYSTEM)
else
  ifdef X11_PATH
    CFLAGS.gl_render3d += -I$(X11_PATH)/include
    LIB.gl_render3d.LOCAL += -L$(X11_PATH)/lib -lXext -lX11
  endif

  ifeq ($(USE_MESA),1)
    ifdef MESA_PATH
      CFLAGS.gl_render3d += -I$(MESA_PATH)/include
      LIB.gl_render3d.LOCAL += -L$(MESA_PATH)/lib
    endif
    LIB.gl_render3d.LOCAL += -lMesaGL
  else
    ifdef OPENGL_PATH
      CFLAGS.gl_render3d += -I$(OPENGL_PATH)/include
      LIB.gl_render3d.LOCAL += -L$(OPENGL_PATH)/lib
    endif
    LIB.gl_render3d.LOCAL += -lGL
  endif
endif

ifeq ($(USE_PLUGINS),yes)
  gl_render3d = $(OUTDLL)/gl_render3d$(DLL)
  LIB.gl_render3d = $(foreach d,$(DEP.gl_render3d),$($d.LIB))
  LIB.gl_render3d.SPECIAL = $(LIB.gl_render3d.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(gl_render3d)
else
  gl_render3d = $(OUT)/$(LIB_PREFIX)gl_render3d$(LIB)
  DEP.EXE += $(gl_render3d)
  LIBS.EXE += $(LIB.gl_render3d.LOCAL)
  SCF.STATIC += gl_render3d
  TO_INSTALL.STATIC_LIBS += $(gl_render3d)
endif

INC.gl_render3d = $(wildcard plugins/video/render3d/opengl/*.h)
SRC.gl_render3d = $(wildcard plugins/video/renderer/opengl/*.cpp)
OBJ.gl_render3d = $(addprefix $(OUT)/,$(notdir $(SRC.gl_render3d:.cpp=$O)))
DEP.gl_render3d = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.gl_render3d = data/config/render3d/render3d.cfg data/config/render3d/opengl.cfg

TO_INSTALL.CONFIG += $(CFG.gl_render3d)

MSVC.DSP += gl_render3d
DSP.gl_render3d.NAME = gl_render3d
DSP.gl_render3d.TYPE = plugin
DSP.gl_render3d.RESOURCES = $(wildcard plugins/video/renderer/opengl/ext/*.inc)
DSP.gl_render3d.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gl_render3d gl_render3dclean

# Chain rules
clean: gl_render3dclean

gl_render3d: $(OUTDIRS) $(gl_render3d)

$(OUT)/%$O: plugins/video/renderer/opengl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.gl_render3d)

$(gl_render3d): $(OBJ.gl_render3d) $(LIB.gl_render3d)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.gl_render3d.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

gl_render3dclean:
	$(RM) $(gl_render3d) $(OBJ.gl_render3d)

ifdef DO_DEPEND
dep: $(OUTOS)/gl_render3d.dep
$(OUTOS)/gl_render3d.dep: $(SRC.gl_render3d)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.gl_render3d) \
	$(DO.DEP2)
else
-include $(OUTOS)/gl_render3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
