# This is a subinclude file used to define the rules needed
# to build the a shaderpgrogram plugin for fixed function pipeline -- softshader

# Driver description
DESCRIPTION.softshader = Crystal Space Render3d Shaderplugin for software renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make softshader   Make the $(DESCRIPTION.softshader)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: softshader
all plugins: softshader

softshader:
	$(MAKE_TARGET) MAKE_DLL=yes
softshaderclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/softshader

ifeq ($(USE_PLUGINS),yes)
  SOFTSHADER = $(OUTDLL)/softshader$(DLL)
  LIB.SOFTSHADER = $(foreach d,$(DEP.SOFTSHADER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SOFTSHADER)
else
  SOFTSHADER = $(OUT)/$(LIB_PREFIX)softshader$(LIB)
  DEP.EXE += $(SOFTSHADER)
  LIBS.EXE += $(GL.LFLAGS)
  SCF.STATIC += softshader
  TO_INSTALL.STATIC_LIBS += $(SOFTSHADER)
endif

INF.SOFTSHADER = $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/softshader/softshader.csplugin
INC.SOFTSHADER = \
  $(wildcard $(addprefix $(SRCDIR)/,\
  plugins/video/render3d/shader/shaderplugins/softshader/*.h\
  plugins/video/render3d/shader/common/*.h))
SRC.SOFTSHADER = \
  $(wildcard $(addprefix $(SRCDIR)/,\
  plugins/video/render3d/shader/shaderplugins/softshader/*.cpp\
  plugins/video/render3d/shader/common/*.cpp))
OBJ.SOFTSHADER = $(addprefix $(OUT)/,$(notdir $(SRC.SOFTSHADER :.cpp=$O)))

DEP.SOFTSHADER = CSGEOM CSUTIL CSUTIL CSGFX
CFG.SOFTSHADER =

TO_INSTALL.CONFIG += $(CFG.SOFTSHADER)

MSVC.DSP += SOFTSHADER
DSP.SOFTSHADER.NAME = softshader
DSP.SOFTSHADER.TYPE = plugin
DSP.SOFTSHADER.RESOURCES = 
DSP.SOFTSHADER.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: softshader softshaderclean

# Chain rules
clean: softshaderclean

softshader: $(OUTDIRS) $(SOFTSHADER)

$(OUT)/%$O: $(SRCDIR)/plugins/video/render3d/shader/shaderplugins/softshader/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS)

$(SOFTSHADER): $(OBJ.SOFTSHADER) $(LIB.SOFTSHADER)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(GL.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

softshaderclean:
	-$(RMDIR) $(SOFTSHADER) $(OBJ.SOFTSHADER) $(OUTDLL)/$(notdir $(INF.SOFTSHADER))

ifdef DO_DEPEND
dep: $(OUTOS)/softshader.dep
$(OUTOS)/softshader.dep: $(SRC.SOFTSHADER)
	$(DO.DEP1) \
	-DGL_VERSION_1_1 $(CFLAGS.PIXEL_LAYOUT) $(GL.CFLAGS) \
	$(DO.DEP2)

else
-include $(OUTOS)/softshader.dep
endif

endif # ifeq ($(MAKESECTION),targets)
