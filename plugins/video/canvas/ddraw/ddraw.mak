# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.ddraw2d = Crystal Space DirectDraw 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ddraw2d      Make the $(DESCRIPTION.ddraw2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddraw2d ddraw2dclean
all softcanvas plugins drivers drivers2d: ddraw2d

ddraw2d:
	$(MAKE_TARGET) MAKE_DLL=yes
ddraw2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/ddraw plugins/video/canvas/common \
  plugins/video/canvas/directxcommon

ifeq ($(USE_PLUGINS),yes)
  DDRAW2D = $(OUTDLL)/ddraw2d$(DLL)
  LIB.DDRAW2D = $(foreach d,$(DEP.DDRAW2D),$($d.LIB))
  LIB.DDRAW2D.SPECIAL = $(LFLAGS.l)ddraw
  TO_INSTALL.DYNAMIC_LIBS += $(DDRAW2D)
else
  DDRAW2D = $(OUT)/$(LIB_PREFIX)ddraw2d$(LIB)
  DEP.EXE += $(DDRAW2D)
  LIBS.EXE += $(LFLAGS.l)ddraw
  SCF.STATIC += ddraw2d
  TO_INSTALL.STATIC_LIBS += $(DDRAW2D)
endif

INC.DDRAW2D = $(wildcard plugins/video/canvas/ddraw/*.h \
  $(wildcard plugins/video/canvas/directxcommon/*.h $(INC.COMMON.DRV2D))) \
  $(wildcard directx/*.h)
SRC.DDRAW2D = $(wildcard plugins/video/canvas/ddraw/*.cpp \
  $(wildcard plugins/video/canvas/directxcommon/*.cpp $(SRC.COMMON.DRV2D)))
OBJ.DDRAW2D = $(addprefix $(OUT)/,$(notdir $(SRC.DDRAW2D:.cpp=$O)))
DEP.DDRAW2D = CSUTIL CSSYS CSUTIL

MSVC.DSP += DDRAW2D
DSP.DDRAW2D.NAME = ddraw2d
DSP.DDRAW2D.TYPE = plugin
DSP.DDRAW2D.LIBS = ddraw

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ddraw2d ddraw2dclean

ddraw2d: $(OUTDIRS) $(DDRAW2D)

$(DDRAW2D): $(OBJ.DDRAW2D) $(LIB.DDRAW2D)
	$(DO.PLUGIN) $(LIB.DDRAW2D.SPECIAL)

clean: ddraw2dclean
ddraw2dclean:
	$(RM) $(DDRAW2D) $(OBJ.DDRAW2D)

ifdef DO_DEPEND
depend: $(OUTOS)/ddraw2d.dep
$(OUTOS)/ddraw2d.dep: $(SRC.DDRAW2D)
	$(DO.DEP)
else
-include $(OUTOS)/ddraw2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
