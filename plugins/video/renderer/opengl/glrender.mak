# This is a subinclude file used to define the rules needed
# to build the OpenGL 3D driver -- gl3d

# Driver description
DESCRIPTION.gl3d = Crystal Space OpenGL 3D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make gl3d         Make the $(DESCRIPTION.gl3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: gl3d

all plugins drivers drivers3d: gl3d

gl3d:
	$(MAKE_TARGET) MAKE_DLL=yes
gl3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Non-Unix OpenGL renderer do not need the X11 libs
ifndef OPENGL.LIBS.DEFINED

LIBS.LOCAL.GL3D+=-L$(X11_PATH)/lib -lXext -lX11

ifeq ($(USE_MESA),1)
  ifdef MESA_PATH
    CFLAGS.GL3D+=-I$(MESA_PATH)/include
    LIBS.LOCAL.GL3D+=-L$(MESA_PATH)/lib
  endif
  LIBS.LOCAL.GL3D+=-lMesaGLU -lMesaGL
else
  LIBS.LOCAL.GL3D+=-lGLU -lGL
endif

CFLAGS.GL3D+=-I$(X11_PATH)/include

endif # OPENGL.LIBS.DEFINED

# The 3D OpenGL driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GL3D=$(OUTDLL)gl3d$(DLL)
  LIBS.GL3D=$(LIBS.LOCAL.GL3D)
  DEP.GL3D=$(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  GL3D=$(OUT)$(LIB_PREFIX)gl3d$(LIB)
  DEP.EXE+=$(GL3D)
  LIBS.EXE+=$(LIBS.LOCAL.GL3D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_OPENGL3D
endif
DESCRIPTION.$(GL3D) = $(DESCRIPTION.gl3d)
SRC.GL3D = $(wildcard libs/cs3d/opengl/*.cpp) \
  libs/cs3d/common/txtmgr.cpp libs/cs3d/common/memheap.cpp \
  libs/cs3d/common/inv_cmap.cpp libs/cs3d/common/imgtools.cpp
OBJ.GL3D = $(addprefix $(OUT),$(notdir $(SRC.GL3D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: gl3d gl3dclean

# Chain rules
clean: gl3dclean

gl3d: $(OUTDIRS) $(GL3D)

$(OUT)%$O: libs/cs3d/opengl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GL3D)
 
$(GL3D): $(OBJ.GL3D) $(DEP.GL3D)
	$(DO.PLUGIN) $(LIBS.GL3D)

gl3dclean:
	$(RM) $(GL3D) $(OBJ.GL3D)

ifdef DO_DEPEND
depend: $(OUTOS)gl3d.dep
$(OUTOS)gl3d.dep: $(SRC.GL3D)
	$(DO.DEP)
else
-include $(OUTOS)gl3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
