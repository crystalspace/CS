# This is a subinclude file used to define the rules needed
# to build the OpenGL 3D driver -- glrender

# Driver description
DESCRIPTION.glrender = Crystal Space OpenGL 3D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make glrender     Make the $(DESCRIPTION.glrender)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glrender

all drivers drivers3d: glrender

glrender:
	$(MAKE_TARGET) MAKE_DLL=yes

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
  LIBS.LOCAL.GL3D+=-lMesaGL -lMesaGLU
else
  LIBS.LOCAL.GL3D+=-lGL
endif

CFLAGS.GL3D+=-I$(X11_PATH)/include

endif # OPENGL.LIBS.DEFINED

# The 3D OpenGL driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GL3D=$(OUTDLL)glrender$(DLL)
  LIBS.GL3D=$(LIBS.LOCAL.GL3D)
  DEP.GL3D=$(CSCOM.LIB) $(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  GL3D=$(OUT)$(LIB_PREFIX)glrender$(LIB)
  DEP.EXE+=$(GL3D)
  LIBS.EXE+=$(LIBS.LOCAL.GL3D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_OPENGL3D
endif
DESCRIPTION.$(GL3D) = $(DESCRIPTION.glrender)
SRC.GL3D = $(wildcard libs/cs3d/opengl/*.cpp) \
  libs/cs3d/common/txtmgr.cpp libs/cs3d/common/memheap.cpp \
  libs/cs3d/common/inv_cmap.cpp libs/cs3d/common/imgtools.cpp
OBJ.GL3D = $(addprefix $(OUT),$(notdir $(SRC.GL3D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glrender glclean glcleanlib

# Chain rules
clean: glclean
cleanlib: glcleanlib

glrender: $(OUTDIRS) $(GL3D)

$(OUT)%$O: libs/cs3d/opengl/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GL3D)
 
$(GL3D): $(OBJ.GL3D) $(DEP.GL3D)
	$(DO.PLUGIN) $(LIBS.GL3D)

glclean:
	$(RM) $(GL3D)

glcleanlib:
	$(RM) $(OBJ.GL3D) $(GLX2D)

ifdef DO_DEPEND
depend: $(OUTOS)glrender.dep
$(OUTOS)glrender.dep: $(SRC.GL3D)
	$(DO.DEP)
else
-include $(OUTOS)glrender.dep
endif

endif # ifeq ($(MAKESECTION),targets)
