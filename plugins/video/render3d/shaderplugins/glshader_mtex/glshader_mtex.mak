# This is a subinclude file used to define the rules needed
# to build the a shaderpgrogram plugin for GLMTEX-vp -- glshader_mtex

# Driver description
DESCRIPTION.glshader_mtex = Crystal Space Render3d Shaderplugin for multitexturing

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glshader_mtex   Make the $(DESCRIPTION.glshader_mtex)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glshader_mtex
all plugins: glshader_mtex

glshader_mtex:
	$(MAKE_TARGET) MAKE_DLL=yes
glshader_mtexclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/render3d/shaderplugins/glshader_mtex

ifneq (,$(strip $(LIBS.OPENGL.SYSTEM)))
  LIB.GLSHADER_MTEX.LOCAL += $(LIBS.OPENGL.SYSTEM)
else
  ifdef X11_PATH
    CFLAGS.GLSHADER_MTEX += -I$(X11_PATH)/include
    LIB.GLSHADER_MTEX.LOCAL += -L$(X11_PATH)/lib -lXext -lX11
  endif

  ifeq ($(USE_MESA),1)
    ifdef MESA_PATH
      CFLAGS.GLSHADER_MTEX += -I$(MESA_PATH)/include
      LIB.GLSHADER_MTEX.LOCAL += -L$(MESA_PATH)/lib
    endif
    LIB.GLSHADER_MTEX.LOCAL += -lMesaGL
  else
    ifdef OPENGL_PATH
      CFLAGS.GLSHADER_MTEX += -I$(OPENGL_PATH)/include
      LIB.GLSHADER_MTEX.LOCAL += -L$(OPENGL_PATH)/lib
    endif
    LIB.GLSHADER_MTEX.LOCAL += -lGL
  endif
endif

ifeq ($(USE_PLUGINS),yes)
  GLSHADER_MTEX = $(OUTDLL)/glshader_mtex$(DLL)
  LIB.GLSHADER_MTEX = $(foreach d,$(DEP.GLSHADER_MTEX),$($d.LIB))
  LIB.GLSHADER_MTEX.SPECIAL = $(LIB.GLSHADER_MTEX.LOCAL)
  TO_INSTALL.DYNAMIC_LIBS += $(GLSHADER_MTEX)
else
  GLSHADER_MTEX = $(OUT)/$(LIB_PREFIX)glshader_mtex$(LIB)
  DEP.EXE += $(GLSHADER_MTEX)
  LIBS.EXE += $(LIB.GLSHADER_MTEX.LOCAL)
  SCF.STATIC += glshader_mtex
  TO_INSTALL.STATIC_LIBS += $(GLSHADER_MTEX)
endif

INC.GLSHADER_MTEX = $(wildcard plugins/video/render3d/shaderplugins/glshader_mtex/*.h) 
SRC.GLSHADER_MTEX = $(wildcard plugins/video/render3d/shaderplugins/glshader_mtex/*.cpp)
OBJ.GLSHADER_MTEX = $(addprefix $(OUT)/,$(notdir $(SRC.GLSHADER_MTEX:.cpp=$O)))
DEP.GLSHADER_MTEX = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.GLSHADER_MTEX =

TO_INSTALL.CONFIG += $(CFG.GLSHADER_MTEX)

MSVC.DSP += GLSHADER_MTEX
DSP.GLSHADER_MTEX.NAME = glshader_mtex
DSP.GLSHADER_MTEX.TYPE = plugin
DSP.GLSHADER_MTEX.RESOURCES = 
DSP.GLSHADER_MTEX.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glshader_mtex glshader_mtexclean

# Chain rules
clean: glshader_mtexclean

glshader_mtex: $(OUTDIRS) $(GLSHADER_MTEX)

$(OUT)/%$O: plugins/video/render3d/shaderpluginsglshader_mtex/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GLSHADER_MTEX)

$(GLSHADER_MTEX): $(OBJ.GLSHADER_MTEX) $(LIB.GLSHADER_MTEX)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.GLSHADER_MTEX.SPECIAL) \
	$(DO.PLUGIN.POSTAMBLE)

glshader_mtexclean:
	$(RM) $(GLSHADER_MTEX) $(OBJ.GLSHADER_MTEX)

ifdef DO_DEPEND
dep: $(OUTOS)/glshader_mtex.dep
$(OUTOS)/glshader_mtex.dep: $(SRC.GLSHADER_MTEX)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(CFLAGS.GLSHADER_MTEX) \
	$(DO.DEP2)

else
-include $(OUTOS)/glshader_mtex.dep
endif

endif # ifeq ($(MAKESECTION),targets)
