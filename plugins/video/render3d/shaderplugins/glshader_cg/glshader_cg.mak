# This is a subinclude file used to define the rules needed
# to build the a shaderpgrogram plugin for GLCG -- glshader_cg

# Driver description
DESCRIPTION.glshader_cg = Crystal Space Render3d Shaderplugin for CG

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glshader_cg   Make the $(DESCRIPTION.glshader_cg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glshader_cg
all plugins: glshader_cg

glshader_cg:
	$(MAKE_TARGET) MAKE_DLL=yes
glshader_cgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/render3d/shaderplugins/glshader_cg

ifeq ($(USE_PLUGINS),yes)
  GLSHADER_CG = $(OUTDLL)/glshader_cg$(DLL)
  LIB.GLSHADER_CG = $(foreach d,$(DEP.GLSHADER_CG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GLSHADER_CG)
else
  GLSHADER_CG = $(OUT)/$(LIB_PREFIX)glshader_cg$(LIB)
  DEP.EXE += $(GLSHADER_CG)
  LIBS.EXE += $(CG.LFLAGS)
  SCF.STATIC += glshader_cg
  TO_INSTALL.STATIC_LIBS += $(GLSHADER_CG)
endif

INC.GLSHADER_CG = \
  $(wildcard plugins/video/render3d/shaderplugins/glshader_cg/*.h) 
SRC.GLSHADER_CG = \
  $(wildcard plugins/video/render3d/shaderplugins/glshader_cg/*.cpp)
OBJ.GLSHADER_CG = $(addprefix $(OUT)/,$(notdir $(SRC.GLSHADER_CG:.cpp=$O)))
DEP.GLSHADER_CG = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.GLSHADER_CG =

TO_INSTALL.CONFIG += $(CFG.GLSHADER_CG)

MSVC.DSP += GLSHADER_CG
DSP.GLSHADER_CG.NAME = glshader_cg
DSP.GLSHADER_CG.TYPE = plugin
DSP.GLSHADER_CG.RESOURCES = 
DSP.GLSHADER_CG.LIBS = opengl32 glu32 cg cggl

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glshader_cg glshader_cgclean

# Chain rules
clean: glshader_cgclean

glshader_cg: $(OUTDIRS) $(GLSHADER_CG)

$(OUT)/%$O: plugins/video/render3d/shaderpluginsglshader_cg/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(CG.CFLAGS)

$(GLSHADER_CG): $(OBJ.GLSHADER_CG) $(LIB.GLSHADER_CG)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(CG.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

glshader_cgclean:
	$(RM) $(GLSHADER_CG) $(OBJ.GLSHADER_CG)

ifdef DO_DEPEND
dep: $(OUTOS)/glshader_cg.dep
$(OUTOS)/glshader_cg.dep: $(SRC.GLSHADER_CG)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(CG.CFLAGS) \
	$(DO.DEP2)

else
-include $(OUTOS)/glshader_cg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
