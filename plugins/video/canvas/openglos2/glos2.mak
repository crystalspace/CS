# This is a subinclude file used to define the rules needed
# to build the OS/2 OpenGL 2D driver

# Driver description
DESCRIPTION.glos2 = Crystal Space OS/2 OpenGL 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make glos2        Make the $(DESCRIPTION.glos2)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glos2 glos2clean

ifeq ($(USE_PLUGINS),yes)
all plugins drivers drivers2d: glos2
endif

glos2:
	$(MAKE_TARGET) MAKE_DLL=yes
glos2clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/openglos2 plugins/video/canvas/openglcommon

# The OpenGL library
LIB.GLOS2.SYSTEM += -lopengl

# Resource file for OS/2 OpenGL driver
RES.GLOS2 = $(OUTOS)libGL.res

# The 2D OS/2 OpenGL driver
ifeq ($(USE_PLUGINS),yes)
  GLOS2 = $(OUTDLL)glos2$(DLL)
  LIB.GLOS2 = $(foreach d,$(DEP.GLOS2),$($d.LIB))
  LIB.GLOS2.SPECIAL = $(LIB.GLOS2.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(GLOS2)
else
  GLOS2 = $(OUT)$(LIB_PREFIX)glos2$(LIB)
  DEP.EXE += $(RES.GLOS2) $(GLOS2) $(CSOS2.LIB)
  LIBS.EXE += $(LIB.GLOS2.SYSTEM)
  SCF.STATIC += glos2
  TO_INSTALL.STATIC_LIBS += $(GLOS2)
endif

INC.GLOS2 = $(wildcard plugins/video/canvas/openglos2/*.h \
  $(INC.COMMON.DRV2D.OPENGL) $(INC.COMMON.DRV2D))
SRC.GLOS2 = $(wildcard plugins/video/canvas/openglos2/*.cpp \
  plugins/video/canvas/common/pc-keys.cpp \
  $(SRC.COMMON.DRV2D.OPENGL) $(SRC.COMMON.DRV2D))
OBJ.GLOS2 = $(addprefix $(OUT),$(notdir $(SRC.GLOS2:.cpp=$O)))
DEP.GLOS2 = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glos2 glos2clean

glos2: $(OUTDIRS) $(GLOS2)

$(GLOS2): $(OBJ.GLOS2) $(LIB.GLOS2) $(RES.GLOS2)
	$(DO.PLUGIN) $(LIB.GLOS2.SPECIAL)

$(RES.GLOS2): plugins/video/canvas/openglos2/libGL.rc
	$(RC) $(RCFLAGS) $< $@

clean: glos2clean
glos2clean:
	$(RM) $(GLOS2) $(OBJ.GLOS2)

ifdef DO_DEPEND
dep: $(OUTOS)glos2.dep
$(OUTOS)glos2.dep: $(SRC.GLOS2)
	$(DO.DEP)
else
-include $(OUTOS)glos2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
