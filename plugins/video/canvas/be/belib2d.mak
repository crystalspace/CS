# This is a subinclude file used to define the rules needed
# to build the BeOS 2D driver -- be2d

# Driver description
DESCRIPTION.be2d = Crystal Space BeOS software 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make be2d         Make the $(DESCRIPTION.be2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: be2d be2dclean
all softcanvas plugins drivers drivers2d: be2d

be2d:
	$(MAKE_TARGET) MAKE_DLL=yes
be2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/be

# The 2D Belib driver
ifeq ($(USE_PLUGINS),yes)
  BE2D = $(OUTDLL)be2d$(DLL)
  LIB.BE2D = $(foreach d,$(DEP.BE2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BE2D)
else
  BE2D = be2d.a
  DEP.EXE += $(BE2D)
  SCF.STATIC += be2d
  TO_INSTALL.STATIC_LIBS += $(BE2D)
endif

INC.BE2D = $(wildcard plugins/video/canvas/be/*.h   $(INC.COMMON.DRV2D))
SRC.BE2D = $(wildcard plugins/video/canvas/be/*.cpp $(SRC.COMMON.DRV2D))
OBJ.BE2D = $(addprefix $(OUT),$(notdir $(SRC.BE2D:.cpp=$O)))
DEP.BE2D = CSUTIL CSSYS

#MSVC.DSP += BE2D
#DSP.BE2D.NAME = be2d
#DSP.BE2D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/be

.PHONY: be2d be2dclean

# Chain rules
clean: be2dclean

be2d: $(OUTDIRS) $(BE2D)

$(BE2D): $(OBJ.BE2D) $(LIB.BE2D)
	$(DO.PLUGIN)

be2dclean:
	$(RM) $(BE2D) $(OBJ.BE2D)

ifdef DO_DEPEND
dep: $(OUTOS)be2d.dep
$(OUTOS)be2d.dep: $(SRC.BE2D)
	$(DO.DEP)
else
-include $(OUT)be2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
