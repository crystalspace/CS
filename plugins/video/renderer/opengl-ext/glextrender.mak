# This is a subinclude file used to define the rules needed
# to build the OpenGL 3DEXT driver -- gl3d_ext

# Driver description
DESCRIPTION.gl3d_ext = Crystal Space OpenGL 3DEXT renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make gl3d_ext     Make the $(DESCRIPTION.gl3d_ext)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gl3d_ext
all plugins drivers drivers3d: gl3d_ext

gl3d_ext:
	$(MAKE_TARGET) MAKE_DLL=yes
gl3d_extclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifneq (,$(strip $(LIBS.OPENGL.SYSTEM)))
  LIB.GL3D_EXT.LOCAL += $(LIBS.OPENGL.SYSTEM)
else
  ifdef X11_PATH
    CFLAGS.GL3D_EXT += -I$(X11_PATH)/include
    LIB.GL3D_EXT.LOCAL += -L$(X11_PATH)/lib -lXext -lX11
  endif

  ifeq ($(USE_MESA),1)
    ifdef MESA_PATH
      CFLAGS.GL3D_EXT += -I$(MESA_PATH)/include
      LIB.GL3D_EXT.LOCAL += -L$(MESA_PATH)/lib
    endif
    LIB.GL3D_EXT.LOCAL += -lMesaGL
  else
    ifdef OPENGL_PATH
      CFLAGS.GL3D_EXT += -I$(OPENGL_PATH)/include
      LIB.GL3D_EXT.LOCAL += -L$(OPENGL_PATH)/lib
    endif
    LIB.GL3D_EXT.LOCAL += -lGL
  endif
endif

ifeq ($(USE_PLUGINS),yes)
  GL3D_EXT = $(OUTDLL)gl3d_ext$(DLL)
  LIB.GL3D_EXT = $(foreach d,$(DEP.GL3D),$($d.LIB))
  LIB.GL3D_EXT.SPECIAL = $(LIB.GL3D_EXT.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(GL3D_EXT)
else
  GL3D_EXT = $(OUT)$(LIB_PREFIX)gl3d_ext$(LIB)
  DEP.EXE += $(GL3D_EXT)
  LIBS.EXE += $(LIB.GL3D_EXT.LOCAL)
  SCF.STATIC += gl3d_ext
  TO_INSTALL.STATIC_LIBS += $(GL3D_EXT)
endif

INC.GL3D_EXT = $(wildcard plugins/video/renderer/opengl-ext/*.h) \
  plugins/video/renderer/common/txtmgr.h \
  plugins/video/renderer/common/dtmesh.h \
  plugins/video/renderer/common/dpmesh.h \
  plugins/video/renderer/common/vbufmgr.h \
  plugins/video/renderer/common/polybuf.h \
  plugins/video/renderer/common/pixfmt.h
SRC.GL3D_EXT = $(wildcard plugins/video/renderer/opengl-ext/*.cpp) \
  plugins/video/renderer/common/txtmgr.cpp \
  plugins/video/renderer/common/dtmesh.cpp \
  plugins/video/renderer/common/dpmesh.cpp \
  plugins/video/renderer/common/vbufmgr.cpp \
  plugins/video/renderer/common/polybuf.cpp
OBJ.GL3D_EXT = $(addprefix $(OUT),$(notdir $(SRC.GL3D_EXT:.cpp=$O)))
DEP.GL3D_EXT = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.GL3D_EXT = data/config/opengl.cfg

TO_INSTALL.CONFIG += $(CFG.GL3D_EXT)

MSVC.DSP += GL3D_EXT
DSP.GL3D_EXT.NAME = gl3d_ext
DSP.GL3D_EXT.TYPE = plugin
DSP.GL3D_EXT.RESOURCES = $(wildcard plugins/video/renderer/opengl-ext/ext/*.inc)
DSP.GL3D_EXT.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gl3d_ext gl3d_extclean

# Chain rules
clean: gl3d_extclean

gl3d_ext: $(OUTDIRS) $(GL3D_EXT)

$(OUT)%$O: plugins/video/renderer/opengl-ext/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GL3D_EXT)

$(GL3D_EXT): $(OBJ.GL3D_EXT) $(LIB.GL3D_EXT)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.GL3D_EXT.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

gl3d_extclean:
	$(RM) $(GL3D_EXT) $(OBJ.GL3D_EXT)

ifdef DO_DEPEND
dep: $(OUTOS)gl3d_ext.dep
$(OUTOS)gl3d_ext.dep: $(SRC.GL3D_EXT)
	$(DO.DEP1) $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GL3D_EXT) $(DO.DEP2)
else
-include $(OUTOS)gl3d_ext.dep
endif

endif # ifeq ($(MAKESECTION),targets)
