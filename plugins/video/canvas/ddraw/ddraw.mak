# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.csddraw = Crystal Space DirectDraw 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make csddraw      Make the $(DESCRIPTION.csddraw)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csddraw ddrawclean
all softcanvas plugins drivers drivers2d: csddraw

csddraw:
	$(MAKE_TARGET) MAKE_DLL=yes
ddrawclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/ddraw plugins/video/canvas/common \
  plugins/video/canvas/directxcommon

ifeq ($(USE_PLUGINS),yes)
  CSDDRAW = $(OUTDLL)csddraw$(DLL)
  LIB.CSDDRAW = $(foreach d,$(DEP.CSDDRAW),$($d.LIB))
  LIB.CSDDRAW.SPECIAL = $(LFLAGS.l)ddraw
  TO_INSTALL.DYNAMIC_LIBS += $(CSDDRAW)
else
  DDRAW = $(OUT)$(LIB_PREFIX)csddraw$(LIB)
  DEP.EXE += $(CSDDRAW)
  LIBS.EXE += $(LFLAGS.l)ddraw
  SCF.STATIC += csddraw
  TO_INSTALL.STATIC_LIBS += $(CSDDRAW)
endif

INC.CSDDRAW = $(wildcard plugins/video/canvas/ddraw/*.h \
  $(wildcard plugins/video/canvas/directxcommon/*.h $(INC.COMMON.DRV2D)))
SRC.CSDDRAW = $(wildcard plugins/video/canvas/ddraw/*.cpp \
  $(wildcard plugins/video/canvas/directxcommon/*.cpp $(SRC.COMMON.DRV2D)))
OBJ.CSDDRAW = $(addprefix $(OUT),$(notdir $(SRC.CSDDRAW:.cpp=$O)))
DEP.CSDDRAW = CSUTIL CSSYS CSUTIL

MSVC.DSP += CSDDRAW
DSP.CSDDRAW.NAME = csddraw
DSP.CSDDRAW.TYPE = plugin
DSP.CSDDRAW.LIBS = ddraw

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csddraw ddrawclean

csddraw: $(OUTDIRS) $(DDRAW)

$(CSDDRAW): $(OBJ.CSDDRAW) $(LIB.CSDDRAW)
	$(DO.PLUGIN) $(LIB.CSDDRAW.SPECIAL)

clean: ddrawclean
ddrawclean:
	$(RM) $(CSDDRAW) $(OBJ.CSDDRAW)

ifdef DO_DEPEND
depend: $(OUTOS)ddraw.dep
$(OUTOS)ddraw.dep: $(SRC.CSDDRAW)
	$(DO.DEP)
else
-include $(OUTOS)ddraw.dep
endif

endif # ifeq ($(MAKESECTION),targets)
