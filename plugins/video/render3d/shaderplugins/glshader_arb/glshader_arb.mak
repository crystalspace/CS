# This is a subinclude file used to define the rules needed
# to build the a shaderpgrogram plugin for GLARB-vp -- glshader_arb

# Driver description
DESCRIPTION.glshader_arb = Crystal Space Render3d Shaderplugin for ARB_vp

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glshader_arb   Make the $(DESCRIPTION.glshader_arb)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glshader_arb
all plugins: glshader_arb

glshader_arb:
	$(MAKE_TARGET) MAKE_DLL=yes
glshader_arbclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/render3d/shaderplugins/glshader_arb

ifeq ($(USE_PLUGINS),yes)
  GLSHADER_ARB = $(OUTDLL)/glshader_arb$(DLL)
  LIB.GLSHADER_ARB = $(foreach d,$(DEP.GLSHADER_ARB),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GLSHADER_ARB)
else
  GLSHADER_ARB = $(OUT)/$(LIB_PREFIX)glshader_arb$(LIB)
  DEP.EXE += $(GLSHADER_ARB)
  LIBS.EXE += $(GL.LFLAGS)
  SCF.STATIC += glshader_arb
  TO_INSTALL.STATIC_LIBS += $(GLSHADER_ARB)
endif

INC.GLSHADER_ARB = \
  $(wildcard plugins/video/render3d/shaderplugins/glshader_arb/*.h) 
SRC.GLSHADER_ARB = \
  $(wildcard plugins/video/render3d/shaderplugins/glshader_arb/*.cpp)
OBJ.GLSHADER_ARB = $(addprefix $(OUT)/,$(notdir $(SRC.GLSHADER_ARB:.cpp=$O)))
DEP.GLSHADER_ARB = CSGEOM CSUTIL CSSYS CSUTIL CSGFX
CFG.GLSHADER_ARB =

TO_INSTALL.CONFIG += $(CFG.GLSHADER_ARB)

MSVC.DSP += GLSHADER_ARB
DSP.GLSHADER_ARB.NAME = glshader_arb
DSP.GLSHADER_ARB.TYPE = plugin
DSP.GLSHADER_ARB.RESOURCES = 
DSP.GLSHADER_ARB.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glshader_arb glshader_arbclean

# Chain rules
clean: glshader_arbclean

glshader_arb: $(OUTDIRS) $(GLSHADER_ARB)

$(OUT)/%$O: plugins/video/render3d/shaderpluginsglshader_arb/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)

$(GLSHADER_ARB): $(OBJ.GLSHADER_ARB) $(LIB.GLSHADER_ARB)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(GL.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

glshader_arbclean:
	$(RM) $(GLSHADER_ARB) $(OBJ.GLSHADER_ARB)

ifdef DO_DEPEND
dep: $(OUTOS)/glshader_arb.dep
$(OUTOS)/glshader_arb.dep: $(SRC.GLSHADER_ARB)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS) \
	$(DO.DEP2)

else
-include $(OUTOS)/glshader_arb.dep
endif

endif # ifeq ($(MAKESECTION),targets)
