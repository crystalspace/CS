# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw/DX 8 2D driver

# Driver description
DESCRIPTION.ddraw8 = Crystal Space DirectDraw/DX 8 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ddraw8       Make the $(DESCRIPTION.ddraw8)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddraw8 ddraw8clean
all plugins drivers drivers2d: ddraw8

ddraw8:
	$(MAKE_TARGET) MAKE_DLL=yes
ddraw8clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/ddraw8 plugins/video/canvas/common \
  plugins/video/canvas/directxcommon

ifeq ($(USE_PLUGINS),yes)
  DDRAW8 = $(OUTDLL)ddraw8$(DLL)
  LIB.DDRAW8 = $(foreach d,$(DEP.DDRAW8),$($d.LIB))
  LIB.DDRAW8.SPECIAL = $(LFLAGS.l)ddraw $(LFLAGS.l)dxguid
  TO_INSTALL.DYNAMIC_LIBS += $(DDRAW8)
else
  DDRAW8 = $(OUT)$(LIB_PREFIX)ddraw8$(LIB)
  DEP.EXE += $(DDRAW8)
  LIBS.EXE += $(LFLAGS.l)ddraw8
  SCF.STATIC += ddraw8
  TO_INSTALL.STATIC_LIBS += $(DDRAW8)
endif

INC.DDRAW8 = $(wildcard plugins/video/canvas/ddraw8/*.h \
  $(wildcard plugins/video/canvas/directxcommon/*.h $(INC.COMMON.DRV2D)))
SRC.DDRAW8 = $(wildcard plugins/video/canvas/ddraw8/*.cpp \
  $(wildcard plugins/video/canvas/directxcommon/*.cpp $(SRC.COMMON.DRV2D)))
OBJ.DDRAW8 = $(addprefix $(OUT),$(notdir $(SRC.DDRAW8:.cpp=$O)))
DEP.DDRAW8 = CSUTIL CSSYS CSUTIL

MSVC.DSP += DDRAW8
DSP.DDRAW8.NAME = ddraw8
DSP.DDRAW8.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ddraw8 ddraw8clean

ddraw8: $(OUTDIRS) $(DDRAW8)

$(DDRAW8): $(OBJ.DDRAW8) $(LIB.DDRAW8)
	$(DO.PLUGIN) $(LIB.DDRAW8.SPECIAL)

clean: ddraw8clean
ddraw8clean:
	$(RM) $(DDRAW8) $(OBJ.DDRAW8)

ifdef DO_DEPEND
depend: $(OUTOS)ddraw8.dep
$(OUTOS)ddraw8.dep: $(SRC.DDRAW8)
	$(DO.DEP)
else
-include $(OUTOS)ddraw8.dep
endif

endif # ifeq ($(MAKESECTION),targets)
