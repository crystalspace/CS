# This is a subinclude file used to define the rules needed
# to build the 3D NULL rendering driver -- null3d

# Driver description
DESCRIPTION.null3d = Crystal Space null 3D renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make null3d       Make the $(DESCRIPTION.null3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: null3d
all plugins drivers drivers3d: null3d

null3d:
	$(MAKE_TARGET) MAKE_DLL=yes
null3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/renderer/null

ifeq ($(USE_PLUGINS),yes)
  NULL3D = $(OUTDLL)null3d$(DLL)
  LIB.NULL3D = $(foreach d,$(DEP.NULL3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(NULL3D)
else
  NULL3D = $(OUT)$(LIB_PREFIX)null3d$(LIB)
  DEP.EXE += $(NULL3D)
  SCF.STATIC += null3d
  TO_INSTALL.STATIC_LIBS += $(NULL3D)
endif

INC.NULL3D = $(wildcard plugins/video/renderer/null/*.h) \
  plugins/video/renderer/common/txtmgr.h
SRC.NULL3D = $(wildcard plugins/video/renderer/null/*.cpp) \
  plugins/video/renderer/common/txtmgr.cpp
OBJ.NULL3D = $(addprefix $(OUT),$(notdir $(SRC.NULL3D:.cpp=$O)))
DEP.NULL3D = CSGFX CSUTIL CSSYS CSGEOM CSUTIL
CFG.NULL3D = data/config/null3d.cfg

TO_INSTALL.CONFIG += $(CFG.NULL3D)

MSVC.DSP += NULL3D
DSP.NULL3D.NAME = null3d
DSP.NULL3D.TYPE = plugin
DSP.NULL3D.RESOURCES = $(wildcard plugins/video/renderer/null/*.inc)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: null3d null3dclean

# Chain rules
all: $(NULL3D)
clean: null3dclean

null3d: $(OUTDIRS) $(NULL3D)

$(NULL3D): $(OBJ.NULL3D) $(LIB.NULL3D)
	$(DO.PLUGIN)

null3dclean:
	$(RM) $(NULL3D) $(OBJ.NULL3D)

ifdef DO_DEPEND
dep: $(OUTOS)null3d.dep
$(OUTOS)null3d.dep: $(SRC.NULL3D)
	$(DO.DEP)
else
-include $(OUTOS)null3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
