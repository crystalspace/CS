# This is a subinclude file used to define the rules needed
# to build the OpenGL 3D driver -- gl3d

# Driver description
DESCRIPTION.gl3d = Crystal Space OpenGL 3D renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make gl3d         Make the $(DESCRIPTION.gl3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gl3d
all plugins drivers drivers3d: gl3d

gl3d:
	$(MAKE_TARGET) MAKE_DLL=yes
gl3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifneq (,$(strip $(LIBS.OPENGL.SYSTEM)))
  LIB.GL3D.LOCAL += $(LIBS.OPENGL.SYSTEM)
else
  ifdef X11_PATH
    CFLAGS.GL3D += -I$(X11_PATH)/include
    LIB.GL3D.LOCAL += -L$(X11_PATH)/lib -lXext -lX11
  endif

  ifeq ($(USE_MESA),1)
    ifdef MESA_PATH
      CFLAGS.GL3D += -I$(MESA_PATH)/include
      LIB.GL3D.LOCAL += -L$(MESA_PATH)/lib
    endif
    LIB.GL3D.LOCAL += -lMesaGL
  else
    ifdef OPENGL_PATH
      CFLAGS.GL3D += -I$(OPENGL_PATH)/include
      LIB.GL3D.LOCAL += -L$(OPENGL_PATH)/lib
    endif
    LIB.GL3D.LOCAL += -lGL
  endif
endif

ifeq ($(USE_PLUGINS),yes)
  GL3D = $(OUTDLL)gl3d$(DLL)
  LIB.GL3D = $(foreach d,$(DEP.GL3D),$($d.LIB))
  LIB.GL3D.SPECIAL = $(LIB.GL3D.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(GL3D)
else
  GL3D = $(OUT)$(LIB_PREFIX)gl3d$(LIB)
  DEP.EXE += $(GL3D)
  LIBS.EXE += $(LIB.GL3D.LOCAL)
  SCF.STATIC += gl3d
  TO_INSTALL.STATIC_LIBS += $(GL3D)
endif

INC.GL3D = $(wildcard plugins/video/renderer/opengl/*.h) \
  plugins/video/renderer/common/txtmgr.h \
  plugins/video/renderer/common/dtmesh.h \
  plugins/video/renderer/common/dpmesh.h \
  plugins/video/renderer/common/vbufmgr.h \
  plugins/video/renderer/common/polybuf.h \
  plugins/video/renderer/common/pixfmt.h
SRC.GL3D = $(wildcard plugins/video/renderer/opengl/*.cpp) \
  plugins/video/renderer/common/txtmgr.cpp \
  plugins/video/renderer/common/dtmesh.cpp \
  plugins/video/renderer/common/dpmesh.cpp \
  plugins/video/renderer/common/vbufmgr.cpp \
  plugins/video/renderer/common/polybuf.cpp
OBJ.GL3D = $(addprefix $(OUT),$(notdir $(SRC.GL3D:.cpp=$O)))
DEP.GL3D = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.GL3D = data/config/opengl.cfg

TO_INSTALL.CONFIG += $(CFG.GL3D)

MSVC.DSP += GL3D
DSP.GL3D.NAME = gl3d
DSP.GL3D.TYPE = plugin
DSP.GL3D.RESOURCES = $(wildcard plugins/video/renderer/opengl/ext/*.inc)
DSP.GL3D.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gl3d gl3dclean

# Chain rules
clean: gl3dclean

gl3d: $(OUTDIRS) $(GL3D)

$(OUT)%$O: plugins/video/renderer/opengl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GL3D)

$(GL3D): $(OBJ.GL3D) $(LIB.GL3D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.GL3D.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

gl3dclean:
	$(RM) $(GL3D) $(OBJ.GL3D)

ifdef DO_DEPEND
dep: $(OUTOS)gl3d.dep
$(OUTOS)gl3d.dep: $(SRC.GL3D)
	$(DO.DEP1) $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GL3D) $(DO.DEP2)
else
-include $(OUTOS)gl3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
