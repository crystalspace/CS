# This is a subinclude file used to define the rules needed
# to build the GLBE 2D driver -- glbe2d

# Driver description
DESCRIPTION.glbe2d = Crystal Space GL/Be 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make glbe2d       Make the $(DESCRIPTION.glbe2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glbe2d

all drivers drivers2d: glbe2d

glbe2d:
	$(MAKE_TARGET) MAKE_DLL=yes
glbe2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need also the GL libs
CFLAGS.GLBE2D+=-I/boot/home/develop/headers/be/opengl

# The 2D GLBe driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLBE2D=$(OUTDLL)glbe2d$(DLL)
  DEP.BE2D = $(CSCOM.LIB) $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
  LIBS.GLBE2D=-lGL
else
  GLBE2D=$(OUT)$(LIB_PREFIX)glbe2d$(LIB)
  DEP.EXE+=$(GLBE2D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_GLBE2D
  LIBS.EXE+=-lGL
endif
DESCRIPTION.$(GLBE2D) = $(DESCRIPTION.glbe2d)
SRC.GLBE2D = $(wildcard libs/cs2d/openglbe/*.cpp \
  $(SRC.COMMON.DRV2D.OPENGL) $(SRC.COMMON.DRV2D))
OBJ.GLBE2D = $(addprefix $(OUT),$(notdir $(SRC.GLBE2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glbe2d glbeclean

# Chain rules
clean: glbeclean

glbe2d: $(OUTDIRS) $(GLBE2D)

$(OUT)%$O: libs/cs2d/openglbe/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLBE2D)

$(GLBE2D): $(OBJ.GLBE2D) $(DEP.BE2D)
	$(DO.PLUGIN) $(LIBS.GLBE2D)

glbeclean:
	$(RM) $(GLBE2D) $(OBJ.GLBE2D)

ifdef DO_DEPEND
depend: $(OUTOS)glbe2d.dep
$(OUTOS)glbe2d.dep: $(SRC.GLBE2D)
	$(DO.DEP)
else
-include $(OUTOS)glbe2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
