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

ifeq ($(USE_PLUGINS),yes)
  GL3D = $(OUTDLL)/gl3d$(DLL)
  LIB.GL3D = $(foreach d,$(DEP.GL3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GL3D)
else
  GL3D = $(OUT)/$(LIB_PREFIX)gl3d$(LIB)
  DEP.EXE += $(GL3D)
  LIBS.EXE += $(GL.LFLAGS)
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
OBJ.GL3D = $(addprefix $(OUT)/,$(notdir $(SRC.GL3D:.cpp=$O)))
DEP.GL3D = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.GL3D = data/config/opengl.cfg data/config/glnvgf.cfg data/config/gl3dfx.cfg

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

$(OUT)/%$O: plugins/video/renderer/opengl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)

$(GL3D): $(OBJ.GL3D) $(LIB.GL3D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(GL.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

gl3dclean:
	$(RM) $(GL3D) $(OBJ.GL3D)

ifdef DO_DEPEND
dep: $(OUTOS)/gl3d.dep
$(OUTOS)/gl3d.dep: $(SRC.GL3D)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS) \
	$(DO.DEP2)
else
-include $(OUTOS)/gl3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
