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

.PHONY: gl3d gl3dclean
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
  LIB.GL3D.LFLAGS = $(GL.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(GL3D)
else
  GL3D = $(OUT)/$(LIB_PREFIX)gl3d$(LIB)
  DEP.EXE += $(GL3D)
  LIBS.EXE += $(GL.LFLAGS)
  SCF.STATIC += gl3d
  TO_INSTALL.STATIC_LIBS += $(GL3D)
endif

DIR.GL3D = plugins/video/renderer/opengl
OUT.GL3D = $(OUT)/$(DIR.GL3D)
INF.GL3D = $(SRCDIR)/$(DIR.GL3D)/gl3d.csplugin
INC.GL3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.GL3D)/*.h \
  $(wildcard plugins/video/renderer/opengl/effects/*.h) \
  plugins/video/renderer/common/txtmgr.h \
  plugins/video/renderer/common/dtmesh.h \
  plugins/video/renderer/common/dpmesh.h \
  plugins/video/renderer/common/vbufmgr.h \
  plugins/video/renderer/common/polybuf.h \
  plugins/video/renderer/common/pixfmt.h))
SRC.GL3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.GL3D)/*.cpp \
  $(wildcard plugins/video/renderer/opengl/effects/*.cpp) \
  plugins/video/renderer/common/txtmgr.cpp \
  plugins/video/renderer/common/dtmesh.cpp \
  plugins/video/renderer/common/dpmesh.cpp \
  plugins/video/renderer/common/vbufmgr.cpp \
  plugins/video/renderer/common/polybuf.cpp))
OBJ.GL3D = $(addprefix $(OUT.GL3D)/,$(notdir $(SRC.GL3D:.cpp=$O)))
DEP.GL3D = CSGEOM CSUTIL CSUTIL CSGFX
CFG.GL3D = $(addprefix $(SRCDIR)/, \
  data/config/opengl.cfg data/config/glnvgf.cfg data/config/gl3dfx.cfg)

OUTDIRS += $(OUT.GL3D)

TO_INSTALL.CONFIG += $(CFG.GL3D)

MSVC.DSP += GL3D
DSP.GL3D.NAME = gl3d
DSP.GL3D.TYPE = plugin
DSP.GL3D.RESOURCES = \
  $(wildcard $(SRCDIR)/plugins/video/renderer/opengl/ext/*.inc)
DSP.GL3D.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gl3d gl3dclean gl3dcleandep

gl3d: $(OUTDIRS) $(GL3D)

$(OUT.GL3D)/%$O: $(SRCDIR)/$(DIR.GL3D)/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)
        
$(OUT.GL3D)/%$O: $(SRCDIR)/$(DIR.GL3D)/effects/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)

$(OUT.GL3D)/%$O: $(SRCDIR)/plugins/video/renderer/common/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT)

$(GL3D): $(OBJ.GL3D) $(LIB.GL3D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.GL3D.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: gl3dclean
gl3dclean:
	-$(RMDIR) $(GL3D) $(OBJ.GL3D) $(OUTDLL)/$(notdir $(INF.GL3D))

cleandep: gl3dcleandep
gl3dcleandep:
	-$(RM) $(OUT.GL3D)/gl3d.dep

ifdef DO_DEPEND
dep: $(OUT.GL3D) $(OUT.GL3D)/gl3d.dep
$(OUT.GL3D)/gl3d.dep: $(SRC.GL3D)
	$(DO.DEPEND1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS) \
	$(DO.DEPEND2)
else
-include $(OUT.GL3D)/gl3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
