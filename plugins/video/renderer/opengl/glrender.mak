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

ifeq ($(USE_DLL),yes)
all drivers drivers3d: glrender
endif

glrender:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# WARNING: Non-Unix OpenGL renderer do not need the X11 libs;
# TO BE FIXED: For now just comment out the following lines
CFLAGS.GL3D+=-I$(X11_PATH)/include
LIBS._GL3D+=-L$(X11_PATH)/lib -lXext -lX11

ifeq ($(USE_MESA),1)
  ifdef MESA_PATH
    CFLAGS.GL3D+=-I$(MESA_PATH)/include
    LIBS._GL3D+=-L$(MESA_PATH)/lib
  endif
  LIBS._GL3D+=-lMesaGL -lMesaGLU
else
  LIBS._GL3D+=-lGL
endif

# The 3D OpenGL driver
ifeq ($(USE_DLL),yes)
  GL3D=$(OUTDLL)glrender$(DLL)
  LIBS.GL3D=$(LIBS._GL3D)
  DEP.GL3D=$(CSCOM.LIB) $(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  GL3D=$(OUT)$(LIB_PREFIX)glrender$(LIB)
  DEP.EXE+=$(GL3D)
  LIBS.EXE+=$(LIBS._GL3D)
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
	$(DO.LIBRARY) $(LIBS.GL3D)

glclean:
	$(RM) $(GL3D)

glcleanlib:
	$(RM) $(OBJ.GL3D) $(GLX2D)

ifdef DO_DEPEND
$(OUTOS)glrender.dep: $(SRC.GL3D)
	$(DO.DEP)
endif

-include $(OUTOS)glrender.dep

endif # ifeq ($(MAKESECTION),targets)
