# This is a subinclude file used to define the rules needed
# to build the GL Win32 2D driver -- glwin32

# Driver description
DESCRIPTION.glwin32 = Crystal Space GL/Win32 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make glwin32      Make the $(DESCRIPTION.glwin32)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glwin32

all plugins drivers drivers2d: glwin32

glwin32:
	$(MAKE_TARGET) MAKE_DLL=yes
glwin32clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need also the GL libs
CFLAGS.GLWIN32+=

# The 2D GLBe driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLWIN32=$(OUTDLL)glwin32$(DLL)
  DEP.BE2D = $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
  LIBS.GLWIN32=-lGL
else
  GLWIN32=$(OUT)$(LIB_PREFIX)glwin32$(LIB)
  DEP.EXE+=$(GLWIN32)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_GLWIN32
  LIBS.EXE+=-lGL
endif
DESCRIPTION.$(GLWIN32) = $(DESCRIPTION.glwin32)
SRC.GLWIN32 = $(wildcard libs/cs2d/openglwin/*.cpp \
  $(SRC.COMMON.DRV2D.OPENGL) $(SRC.COMMON.DRV2D))
OBJ.GLWIN32 = $(addprefix $(OUT),$(notdir $(SRC.GLWIN32:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glwin32 glbeclean

# Chain rules
clean: glbeclean

glwin32: $(OUTDIRS) $(GLWIN32)

$(OUT)%$O: libs/cs2d/openglwin/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLWIN32)
$(OUT)%$O: libs/cs2d/openglcommon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLWIN32)

$(GLWIN32): $(OBJ.GLWIN32) $(DEP.BE2D)
	$(DO.PLUGIN) $(LIBS.GLWIN32)

glbeclean:
	$(RM) $(GLWIN32) $(OBJ.GLWIN32)

ifdef DO_DEPEND
depend: $(OUTOS)glwin32.dep
$(OUTOS)glwin32.dep: $(SRC.GLWIN32)
	$(DO.DEP) $(CFLAGS.GLWIN32)
else
-include $(OUTOS)glwin32.dep
endif

endif # ifeq ($(MAKESECTION),targets)
