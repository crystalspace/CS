# This is a subinclude file used to define the rules needed
# to build the NULL 2D driver -- null2d

# Driver description
DESCRIPTION.null2d = Crystal Space Null 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make null2d       Make the $(DESCRIPTION.null2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: null2d null2dclean
all plugins drivers drivers2d: null2d

null2d:
	$(MAKE_TARGET) MAKE_DLL=yes

null2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/null2d

ifeq ($(USE_PLUGINS),yes)
  NULL2D = $(OUTDLL)null2d$(DLL)
  LIB.NULL2D = $(foreach d,$(DEP.NULL2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(NULL2D)
else
  NULL2D = $(OUT)$(LIB_PREFIX)null2d$(LIB)
  DEP.EXE += $(NULL2D)
  SCF.STATIC += null2d
  TO_INSTALL.STATIC_LIBS += $(NULL2D)
endif

INC.NULL2D = $(wildcard plugins/video/canvas/null2d/*.h   $(INC.COMMON.DRV2D))
SRC.NULL2D = $(wildcard plugins/video/canvas/null2d/*.cpp $(SRC.COMMON.DRV2D))
OBJ.NULL2D = $(addprefix $(OUT),$(notdir $(SRC.NULL2D:.cpp=$O)))
DEP.NULL2D = CSUTIL CSSYS CSUTIL CSGEOM

MSVC.DSP += NULL2D
DSP.NULL2D.NAME = null2d
DSP.NULL2D.TYPE = plugin
DSP.NULL2D.LIBS = 

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: null2d null2dclean

# Chain rules
all: $(NULL2D)
clean: null2dclean

null2d: $(OUTDIRS) $(NULL2D)

$(NULL2D): $(OBJ.NULL2D) $(LIB.NULL2D)
	$(DO.PLUGIN)

clean: null2dclean
null2dclean:
	$(RM) $(NULL2D) $(OBJ.NULL2D)

ifdef DO_DEPEND
dep: $(OUTOS)null2d.dep
$(OUTOS)null2d.dep: $(SRC.NULL2D)
	$(DO.DEP)
else
-include $(OUTOS)null2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
