# This is a subinclude file used to define the rules needed
# to build the NEW (Render3d) OpenGL 3D driver -- glrender3d

# Driver description
DESCRIPTION.glrender3d = Crystal Space Render3d OpenGL driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glrender3d   Make the $(DESCRIPTION.glrender3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glrender3d
all plugins drivers drivers3d: glrender3d

glrender3d:
	$(MAKE_TARGET) MAKE_DLL=yes
glrender3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  GLRENDER3D = $(OUTDLL)/glrender3d$(DLL)
  LIB.GLRENDER3D = $(foreach d,$(DEP.GLRENDER3D),$($d.LIB))
  LIB.GLRENDER3D.LFLAGS = $(GL.LFLAGS) $(GLU.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(GLRENDER3D)
else
  GLRENDER3D = $(OUT)/$(LIB_PREFIX)glrender3d$(LIB)
  DEP.EXE += $(GLRENDER3D)
  LIBS.EXE += $(GL.LFLAGS) $(GLU.LFLAGS)
  SCF.STATIC += glrender3d
  TO_INSTALL.STATIC_LIBS += $(GLRENDER3D)
endif

DIR.GLRENDER3D = plugins/video/render3d/opengl
OUT.GLRENDER3D = $(OUT)/$(DIR.GLRENDER3D)
INF.GLRENDER3D = $(SRCDIR)/$(DIR.GLRENDER3D)/glrender3d.csplugin
INC.GLRENDER3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.GLRENDER3D)/*.h \
  plugins/video/render3d/common/txtmgr.h))
SRC.GLRENDER3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.GLRENDER3D)/*.cpp \
  plugins/video/render3d/common/txtmgr.cpp))
OBJ.GLRENDER3D = \
  $(addprefix $(OUT.GLRENDER3D)/,$(notdir $(SRC.GLRENDER3D:.cpp=$O)))
DEP.GLRENDER3D = CSTOOL CSGEOM CSUTIL CSUTIL CSGFX
CFG.GLRENDER3D = $(addprefix $(SRCDIR)/, \
  data/config/render3d/render3d.cfg data/config/render3d/opengl.cfg)

OUTDIRS += $(OUT.GLRENDER3D)

TO_INSTALL.CONFIG += $(CFG.GLRENDER3D)

MSVC.DSP += GLRENDER3D
DSP.GLRENDER3D.NAME = glrender3d
DSP.GLRENDER3D.TYPE = plugin
DSP.GLRENDER3D.RESOURCES = \
  $(wildcard $(SRCDIR)/plugins/video/render3d/opengl/ext/*.inc)
DSP.GLRENDER3D.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glrender3d glrender3dclean glrender3dcleandep

glrender3d: $(OUTDIRS) $(GLRENDER3D)

$(OUT.GLRENDER3D)/%$O: $(SRCDIR)/$(DIR.GLRENDER3D)/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)

$(OUT.GLRENDER3D)/%$O: $(SRCDIR)/plugins/video/render3d/common/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT)

$(GLRENDER3D): $(OBJ.GLRENDER3D) $(LIB.GLRENDER3D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.GLRENDER3D.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: glrender3dclean
glrender3dclean:
	-$(RMDIR) $(GLRENDER3D) $(OBJ.GLRENDER3D) \
	$(OUTDLL)/$(notdir $(INF.GLRENDER3D))

cleandep: glrender3dcleandep
glrender3dcleandep:
	-$(RM) $(OUT.GLRENDER3D)/glrender3d.dep

ifdef DO_DEPEND
dep: $(OUT.GLRENDER3D) $(OUT.GLRENDER3D)/glrender3d.dep
$(OUT.GLRENDER3D)/glrender3d.dep: $(SRC.GLRENDER3D)
	$(DO.DEPEND1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS) \
	$(DO.DEPEND2)
else
-include $(OUT.GLRENDER3D)/glrender3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
