# This is a subinclude file used to define the rules needed
# to build the GL Win32 2D driver -- glwin32

# Driver description
DESCRIPTION.glwin32 = Crystal Space GL/Win32 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glwin32      Make the $(DESCRIPTION.glwin32)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glwin32 glwin32clean
all plugins drivers drivers2d: glwin32

glwin32:
	$(MAKE_TARGET) MAKE_DLL=yes
glwin32clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.GLWIN32 += 

ifndef LIBS.OPENGL.SYSTEM
  LIB.GLWIN32.SYSTEM = -lGL
else
  LIB.GLWIN32.SYSTEM = $(LIBS.OPENGL.SYSTEM)
endif

ifeq ($(USE_PLUGINS),yes)
  GLWIN32 = $(OUTDLL)glwin32$(DLL)
  LIB.GLWIN32 = $(foreach d,$(DEP.GLWIN32),$($d.LIB))
  LIB.GLWIN32.SPECIAL = $(LIB.GLWIN32.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(GLWIN32)
else
  GLWIN32 = $(OUT)$(LIB_PREFIX)glwin32$(LIB)
  DEP.EXE += $(GLWIN32)
  LIBS.EXE += $(LIB.GLWIN32.SYSTEM)
  SCF.STATIC += glwin32
  TO_INSTALL.STATIC_LIBS += $(GLWIN32)
endif

INC.GLWIN32 = $(wildcard plugins/video/canvas/openglwin/*.h \
  $(INC.COMMON.DRV2D.OPENGL) $(INC.COMMON.DRV2D))
SRC.GLWIN32 = $(wildcard plugins/video/canvas/openglwin/*.cpp \
  $(SRC.COMMON.DRV2D.OPENGL) $(SRC.COMMON.DRV2D))
OBJ.GLWIN32 = $(addprefix $(OUT),$(notdir $(SRC.GLWIN32:.cpp=$O)))
DEP.GLWIN32 = CSUTIL CSSYS

MSVC.DSP += GLWIN32
DSP.GLWIN32.NAME = glwin32
DSP.GLWIN32.TYPE = plugin
DSP.GLWIN32.LIBS = opengl32 glu32

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glwin32 glwin32clean

glwin32: $(OUTDIRS) $(GLWIN32)

$(OUT)%$O: plugins/video/canvas/openglwin/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLWIN32)

$(OUT)%$O: plugins/video/canvas/openglcommon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLWIN32)

$(GLWIN32): $(OBJ.GLWIN32) $(LIB.GLWIN32)
	$(DO.PLUGIN) $(LIB.GLWIN32.SPECIAL)

clean: glwin32clean
glwin32clean:
	$(RM) $(GLWIN32) $(OBJ.GLWIN32)

ifdef DO_DEPEND
depend: $(OUTOS)glwin32.dep
$(OUTOS)glwin32.dep: $(SRC.GLWIN32)
	$(DO.DEP) $(CFLAGS.GLWIN32)
else
-include $(OUTOS)glwin32.dep
endif

endif # ifeq ($(MAKESECTION),targets)
