# This is a subinclude file used to define the rules needed
# to build the a shaderpgrogram plugin for fixed function
# pipeline -- glshader_fixed

# Driver description
DESCRIPTION.glshader_fixed = Shader for fixed function pipeline

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glshader_fixed$" \
  $(NEWLINE)echo $"                    Make the $(DESCRIPTION.glshader_fixed)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glshader_fixed
all plugins: glshader_fixed

glshader_fixed:
	$(MAKE_TARGET) MAKE_DLL=yes
glshader_fixedclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/glshader_fixed

ifeq ($(USE_PLUGINS),yes)
  GLSHADER_FIXED = $(OUTDLL)/glshader_fixed$(DLL)
  LIB.GLSHADER_FIXED = $(foreach d,$(DEP.GLSHADER_FIXED),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GLSHADER_FIXED)
else
  GLSHADER_FIXED = $(OUT)/$(LIB_PREFIX)glshader_fixed$(LIB)
  DEP.EXE += $(GLSHADER_FIXED)
  LIBS.EXE += $(GL.LFLAGS)
  SCF.STATIC += glshader_fixed
  TO_INSTALL.STATIC_LIBS += $(GLSHADER_FIXED)
endif

INF.GLSHADER_FIXED = $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/glshader_fixed/glshader_fixed.csplugin
INC.GLSHADER_FIXED = \
  $(wildcard $(addprefix $(SRCDIR)/,\
  plugins/video/render3d/shader/shaderplugins/glshader_fixed/*.h\
  plugins/video/render3d/shader/shaderplugins/common/*.h \
  plugins/video/render3d/shader/common/*.h))
SRC.GLSHADER_FIXED = \
  $(wildcard $(addprefix $(SRCDIR)/,\
  plugins/video/render3d/shader/shaderplugins/glshader_fixed/*.cpp\
  plugins/video/render3d/shader/shaderplugins/common/*.cpp \
  plugins/video/render3d/shader/common/*.cpp))
OBJ.GLSHADER_FIXED = $(addprefix $(OUT)/,$(notdir $(SRC.GLSHADER_FIXED:.cpp=$O)))
DEP.GLSHADER_FIXED = CSGFX CSGEOM CSUTIL
CFG.GLSHADER_FIXED =

TO_INSTALL.CONFIG += $(CFG.GLSHADER_FIXED)

MSVC.DSP += GLSHADER_FIXED
DSP.GLSHADER_FIXED.NAME = glshader_fixed
DSP.GLSHADER_FIXED.TYPE = plugin
DSP.GLSHADER_FIXED.RESOURCES = 
DSP.GLSHADER_FIXED.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glshader_fixed glshader_fixedclean

# Chain rules
clean: glshader_fixedclean

glshader_fixed: $(OUTDIRS) $(GLSHADER_FIXED)

$(OUT)/%$O: $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/glshader_fixed/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)

$(OUT)/%$O: $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/common/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT)

$(GLSHADER_FIXED): $(OBJ.GLSHADER_FIXED) $(LIB.GLSHADER_FIXED)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(GL.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

glshader_fixedclean:
	-$(RMDIR) $(GLSHADER_FIXED) $(OBJ.GLSHADER_FIXED) $(OUTDLL)/$(notdir $(INF.GLSHADER_FIXED))

ifdef DO_DEPEND
dep: $(OUTOS)/glshader_fixed.dep
$(OUTOS)/glshader_fixed.dep: $(SRC.GLSHADER_FIXED)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS) \
	$(DO.DEP2)

else
-include $(OUTOS)/glshader_fixed.dep
endif

endif # ifeq ($(MAKESECTION),targets)
