# This is a subinclude file used to define the rules needed
# to build the X-windows 2D driver -- x2d

# Driver description
DESCRIPTION.x2d = Crystal Space XLib 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make x2d          Make the $(DESCRIPTION.x2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: x2d

ifeq ($(USE_DLL),yes)
all drivers drivers2d: x2d
endif

x2d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need also the X libs
CFLAGS.X2D+=-I$(X11_PATH)/include
LIBS._X2D+=-L$(X11_PATH)/lib -lXext -lX11
 
ifeq ($(DO_SHM),yes)
  CFLAGS.X2D += $(CFLAGS.D)DO_SHM
endif

# The 2D Xlib driver
ifeq ($(USE_DLL),yes)
  XLIB2D=$(OUTDLL)x2d$(DLL)
  LIBS.X2D=$(LIBS._X2D)
  DEP.X2D=$(CSCOM.LIB) $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  XLIB2D=$(OUT)$(LIB_PREFIX)x2d$(LIB)
  DEP.EXE+=$(XLIB2D)
  LIBS.EXE+=$(LIBS._X2D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_X2D
endif
DESCRIPTION.$(XLIB2D) = $(DESCRIPTION.x2d)
SRC.XLIB2D = $(wildcard libs/cs2d/softx/*.cpp $(SRC.COMMON.DRV2D))
OBJ.XLIB2D = $(addprefix $(OUT),$(notdir $(SRC.XLIB2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: x2d libxclean libxcleanlib

# Chain rules
clean: libxclean
cleanlib: libxcleanlib

x2d: $(OUTDIRS) $(XLIB2D)

$(OUT)%$O: libs/cs2d/softx/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.X2D)
 
$(XLIB2D): $(OBJ.XLIB2D) $(DEP.X2D)
	$(DO.LIBRARY) $(LIBS.X2D)

libxclean:
	$(RM) $(XLIB2D)

libxcleanlib:
	$(RM) $(OBJ.XLIB2D) $(XLIB2D)

ifdef DO_DEPEND
$(OUTOS)x2d.dep: $(SRC.XLIB2D)
	$(DO.DEP) $(OUTOS)x2d.dep
endif

-include $(OUTOS)x2d.dep
endif # ifeq ($(MAKESECTION),targets)
