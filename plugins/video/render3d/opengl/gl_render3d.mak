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

ifneq (,$(strip $(LIBS.OPENGL.SYSTEM)))
  LIB.GLRENDER3D.LOCAL += $(LIBS.OPENGL.SYSTEM)
else
  ifdef X11_PATH
    CFLAGS.GLRENDER3D += -I$(X11_PATH)/include
    LIB.GLRENDER3D.LOCAL += -L$(X11_PATH)/lib -lXext -lX11
  endif

  ifeq ($(USE_MESA),1)
    ifdef MESA_PATH
      CFLAGS.GLRENDER3D += -I$(MESA_PATH)/include
      LIB.GLRENDER3D.LOCAL += -L$(MESA_PATH)/lib
    endif
    LIB.GLRENDER3D.LOCAL += -lMesaGL
  else
    ifdef OPENGL_PATH
      CFLAGS.GLRENDER3D += -I$(OPENGL_PATH)/include
      LIB.GLRENDER3D.LOCAL += -L$(OPENGL_PATH)/lib
    endif
    LIB.GLRENDER3D.LOCAL += -lGL
  endif
endif

ifeq ($(USE_PLUGINS),yes)
  GLRENDER3D = $(OUTDLL)/glrender3d$(DLL)
  LIB.GLRENDER3D = $(foreach d,$(DEP.GLRENDER3D),$($d.LIB))
  LIB.GLRENDER3D.SPECIAL = $(LIB.GLRENDER3D.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(GLRENDER3D)
else
  GLRENDER3D = $(OUT)/$(LIB_PREFIX)glrender3d$(LIB)
  DEP.EXE += $(GLRENDER3D)
  LIBS.EXE += $(LIB.GLRENDER3D.LOCAL)
  SCF.STATIC += glrender3d
  TO_INSTALL.STATIC_LIBS += $(GLRENDER3D)
endif

INC.GLRENDER3D = $(wildcard plugins/video/render3d/opengl/*.h) \
  plugins/video/render3d/common/txtmgr.h
SRC.GLRENDER3D = $(wildcard plugins/video/render3d/opengl/*.cpp) \
  plugins/video/render3d/common/txtmgr.cpp
OBJ.GLRENDER3D = $(addprefix $(OUT)/,$(notdir $(SRC.GLRENDER3D:.cpp=$O)))
DEP.GLRENDER3D = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.GLRENDER3D = data/config/render3d/render3d.cfg data/config/render3d/opengl.cfg

TO_INSTALL.CONFIG += $(CFG.GLRENDER3D)

MSVC.DSP += GLRENDER3D
DSP.GLRENDER3D.NAME = glrender3d
DSP.GLRENDER3D.TYPE = plugin
DSP.GLRENDER3D.RESOURCES = $(wildcard plugins/video/render3d/opengl/ext/*.inc)
DSP.GLRENDER3D.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glrender3d glrender3dclean

# Chain rules
clean: glrender3dclean

glrender3d: $(OUTDIRS) $(GLRENDER3D)

$(OUT)/%$O: plugins/video/render3d/opengl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GLRENDER3D)

$(GLRENDER3D): $(OBJ.GLRENDER3D) $(LIB.GLRENDER3D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.GLRENDER3D.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

glrender3dclean:
	$(RM) $(GLRENDER3D) $(OBJ.GLRENDER3D)

ifdef DO_DEPEND
dep: $(OUTOS)/glrender3d.dep
$(OUTOS)/glrender3d.dep: $(SRC.GLRENDER3D)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GLRENDER3D) \
	$(DO.DEP2)
else
-include $(OUTOS)/glrender3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
