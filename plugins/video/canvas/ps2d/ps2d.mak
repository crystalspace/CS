# This is a subinclude file used to define the rules needed
# to build the X-windows 2D driver -- ps2d

# Driver description
DESCRIPTION.ps2d = Crystal Space XLib 2D driver for Line3D

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ps2d         Make the $(DESCRIPTION.ps2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ps2d ps2dclean

all softcanvas plugins drivers drivers2d: ps2d

ps2d:
	$(MAKE_TARGET) MAKE_DLL=yes
ps2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The 2D Xlib driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  PS2LIB2D=$(OUTDLL)ps2d$(DLL)
  LIBS.LOCAL.PS2D=$(LIBS.PS2D)
  DEP.PS2D=$(CSUTIL.LIB) $(CSSYS.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(PS2LIB2D)
else
  PS2LIB2D=$(OUT)$(LIB_PREFIX)ps2d$(LIB)
  DEP.EXE+=$(PS2LIB2D)
  LIBS.EXE+=$(LIBS.PS2D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_PS2D
  TO_INSTALL.STATIC_LIBS+=$(PS2LIB2D)
  SCF.STATIC += ps2d
endif
DESCRIPTION.$(PS2LIB2D) = $(DESCRIPTION.ps2d)
SRC.PS2LIB2D = \
  $(wildcard plugins/video/canvas/ps2d/*.cpp \
  plugins/video/canvas/common/x11-keys.cpp \
  $(SRC.COMMON.DRV2D))
OBJ.PS2LIB2D = $(addprefix $(OUT),$(notdir $(SRC.PS2LIB2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ps2d linelibxclean

# Chain rules
clean: ps2dclean

ps2d: $(OUTDIRS) $(PS2LIB2D)

$(OUT)%$O: plugins/video/canvas/ps2d/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.PS2D)

$(PS2LIB2D): $(OBJ.PS2LIB2D) $(DEP.PS2D)
	$(DO.PLUGIN) $(LIBS.LOCAL.PS2D)

ps2dclean:
	$(RM) $(PS2LIB2D) $(OBJ.PS2LIB2D) $(OUTOS)ps2d.dep

ifdef DO_DEPEND
dep: $(OUTOS)ps2d.dep
$(OUTOS)ps2d.dep: $(SRC.PS2LIB2D)
	$(DO.DEP1) $(CFLAGS.PS2D) $(DO.DEP2)
else
-include $(OUTOS)ps2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
