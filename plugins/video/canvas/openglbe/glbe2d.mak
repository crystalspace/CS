# This is a subinclude file used to define the rules needed
# to build the GLBE 2D driver -- glbe2d

# Driver description
DESCRIPTION.glbe2d = Crystal Space BeOS OpenGL 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glbe2d       Make the $(DESCRIPTION.glbe2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glbe2d glbe2dclean
all plugins drivers drivers2d: glbe2d

glbe2d:
	$(MAKE_TARGET) MAKE_DLL=yes
glbe2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.GLBE2D += -I/boot/home/develop/headers/be/opengl

ifeq ($(USE_PLUGINS),yes)
  GLBE2D = $(OUTDLL)glbe2d$(DLL)
  LIB.GLBE2D = $(foreach d,$(DEP.GLBE2D),$($d.LIB))
  LIBS.GLBE2D = -lGL
  TO_INSTALL.DYNAMIC_LIBS += $(GLBE2D)
else
  GLBE2D = $(OUT)$(LIB_PREFIX)glbe2d$(LIB)
  DEP.EXE += $(GLBE2D)
  LIBS.EXE += -lGL
  SCF.STATIC += glbe2d
  TO_INSTALL.STATIC_LIBS += $(GLBE2D)
endif

INC.GLBE2D = $(wildcard plugins/video/canvas/openglbe/*.h \
  $(INC.COMMON.DRV2D.OPENGL) $(INC.COMMON.DRV2D))
SRC.GLBE2D = $(wildcard plugins/video/canvas/openglbe/*.cpp \
  $(SRC.COMMON.DRV2D.OPENGL) $(SRC.COMMON.DRV2D))
OBJ.GLBE2D = $(addprefix $(OUT),$(notdir $(SRC.GLBE2D:.cpp=$O)))
DEP.BE2D = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glbe2d glbeclean

glbe2d: $(OUTDIRS) $(GLBE2D)

$(OUT)%$O: plugins/video/canvas/openglbe/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLBE2D)

$(OUT)%$O: plugins/video/canvas/openglcommon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLBE2D)

$(GLBE2D): $(OBJ.GLBE2D) $(LIB.BE2D)
	$(DO.PLUGIN) $(LIBS.GLBE2D)

clean: glbeclean
glbeclean:
	$(RM) $(GLBE2D) $(OBJ.GLBE2D)

ifdef DO_DEPEND
dep: $(OUTOS)glbe2d.dep
$(OUTOS)glbe2d.dep: $(SRC.GLBE2D)
	$(DO.DEP1) $(CFLAGS.GLBE2D) $(DO.DEP2)
else
-include $(OUTOS)glbe2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
