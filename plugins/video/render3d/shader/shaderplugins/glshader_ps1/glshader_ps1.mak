# This is a subinclude file used to define the rules needed
# to build the a shaderpgrogram plugin for PS1 FP emulation -- glshader_ps1

# Driver description
DESCRIPTION.glshader_ps1 = Shader for PS 1.x Emulation

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glshader_ps1 Make the $(DESCRIPTION.glshader_ps1)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glshader_ps1
all plugins: glshader_ps1

glshader_ps1:
	$(MAKE_TARGET) MAKE_DLL=yes
glshader_ps1clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/glshader_ps1

ifeq ($(USE_PLUGINS),yes)
  GLSHADER_PS1 = $(OUTDLL)/glshader_ps1$(DLL)
  LIB.GLSHADER_PS1 = $(foreach d,$(DEP.GLSHADER_PS1),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GLSHADER_PS1)
else
  GLSHADER_PS1 = $(OUT)/$(LIB_PREFIX)glshader_ps1$(LIB)
  DEP.EXE += $(GLSHADER_PS1)
  LIBS.EXE += $(GL.LFLAGS)
  SCF.STATIC += glshader_ps1
  TO_INSTALL.STATIC_LIBS += $(GLSHADER_PS1)
endif

INF.GLSHADER_PS1 = $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/glshader_ps1/glshader_ps1.csplugin
INC.GLSHADER_PS1 = \
  $(wildcard $(addprefix $(SRCDIR)/, \
  plugins/video/render3d/shader/shaderplugins/glshader_ps1/*.h \
  plugins/video/render3d/shader/common/*.h))
SRC.GLSHADER_PS1 = \
  $(wildcard $(addprefix $(SRCDIR)/, \
  plugins/video/render3d/shader/shaderplugins/glshader_ps1/*.cpp \
  plugins/video/render3d/shader/common/*.cpp))
OBJ.GLSHADER_PS1 = $(addprefix $(OUT)/,$(notdir $(SRC.GLSHADER_PS1:.cpp=$O)))
DEP.GLSHADER_PS1 = CSGFX CSGEOM CSUTIL
CFG.GLSHADER_PS1 =

TO_INSTALL.CONFIG += $(CFG.GLSHADER_PS1)

MSVC.DSP += GLSHADER_PS1
DSP.GLSHADER_PS1.NAME = glshader_ps1
DSP.GLSHADER_PS1.TYPE = plugin
DSP.GLSHADER_PS1.RESOURCES = 
DSP.GLSHADER_PS1.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glshader_ps1 glshader_ps1clean

# Chain rules
clean: glshader_ps1clean

glshader_ps1: $(OUTDIRS) $(GLSHADER_PS1)

$(OUT)/%$O: $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/glshader_ps1/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)

$(GLSHADER_PS1): $(OBJ.GLSHADER_PS1) $(LIB.GLSHADER_PS1)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(GL.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

glshader_ps1clean:
	-$(RMDIR) $(GLSHADER_PS1) $(OBJ.GLSHADER_PS1) $(OUTDLL)/$(notdir $(INF.GLSHADER_PS1))

ifdef DO_DEPEND
dep: $(OUTOS)/glshader_ps1.dep
$(OUTOS)/glshader_ps1.dep: $(SRC.GLSHADER_PS1)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS) \
	$(DO.DEP2)

else
-include $(OUTOS)/glshader_ps1.dep
endif

endif # ifeq ($(MAKESECTION),targets)
