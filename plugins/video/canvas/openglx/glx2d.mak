# This is a subinclude file used to define the rules needed
# to build the GLX 2D driver -- glx2d

# Driver description
DESCRIPTION.glx2d = Crystal Space GL/X 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glx2d        Make the $(DESCRIPTION.glx2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glx2d glx2dclean
all plugins drivers drivers2d: glx2d

glx2d:
	$(MAKE_TARGET) MAKE_DLL=yes
glx2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  GLX2D = $(OUTDLL)/glx2d$(DLL)
  LIB.GLX2D = $(foreach d,$(DEP.GLX2D),$($d.LIB))
  LIB.GLX2D.LFLAGS = $(GL.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(GLX2D)
else
  GLX2D = $(OUT)/$(LIB_PREFIX)glx2d$(LIB)
  DEP.EXE += $(GLX2D)
  LIBS.EXE += $(GL.LFLAGS)
  SCF.STATIC += glx2d
  TO_INSTALL.STATIC_LIBS += $(GLX2D)
endif

INC.GLX2D = $(wildcard plugins/video/canvas/openglx/*.h \
  $(INC.COMMON.DRV2D.OPENGL) $(INC.COMMON.DRV2D))
SRC.GLX2D = $(wildcard plugins/video/canvas/openglx/*.cpp \
  $(SRC.COMMON.DRV2D.OPENGL) $(SRC.COMMON.DRV2D))
OBJ.GLX2D = $(addprefix $(OUT)/,$(notdir $(SRC.GLX2D:.cpp=$O)))
DEP.GLX2D = CSUTIL CSSYS CSGEOM CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glx2d glx2dclean

glx2d: $(OUTDIRS) $(GLX2D)

$(OUT)/%$O: plugins/video/canvas/openglx/%.cpp
	$(DO.COMPILE.CPP) $(GL.CFLAGS)

$(OUT)/%$O: plugins/video/canvas/openglcommon/%.cpp
	$(DO.COMPILE.CPP) $(GL.CFLAGS)

$(GLX2D): $(OBJ.GLX2D) $(LIB.GLX2D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.GLX2D.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: glx2dclean
glx2dclean:
	$(RM) $(GLX2D) $(OBJ.GLX2D)

ifdef DO_DEPEND
dep: $(OUTOS)/glx2d.dep
$(OUTOS)/glx2d.dep: $(SRC.GLX2D)
	$(DO.DEP1) $(GL.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/glx2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
