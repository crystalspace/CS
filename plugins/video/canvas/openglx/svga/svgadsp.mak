# This is a subinclude file used to define the rules needed
# to build the SVGA Displaydriver for GLX 2D driver -- oglsvga

# Driver description
DESCRIPTION.oglsvga=SVGA driver for Crystal Space GL/X 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make oglsvga     Make the $(DESCRIPTION.oglsvga)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oglsvga oglsvgaclean

all plugins glxdisp: oglsvga

oglsvga:
	$(MAKE_TARGET) MAKE_DLL=yes

oglsvgaclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Local CFLAGS and libraries
#LIBS._OGLSVGA+=-L$(X11_PATH)/lib -lXext -lX11 $(X11_EXTRA_LIBS)

# The driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  OGLSVGA=$(OUTDLL)oglsvga$(DLL)
  LIBS.OGLSVGA=$(LIBS._OGLSVGA)
  DEP.OGLSVGA=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  OGLSVGA=$(OUT)$(LIB_PREFIX)oglsvga$(LIB)
  DEP.EXE+=$(OGLSVGA)
  LIBS.EXE+=$(LIBS._OGLSVGA) $(CSUTIL.LIB) $(CSSYS.LIB)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_OGLSVGA
endif
DESCRIPTION.$(OGLSVGA) = $(DESCRIPTION.oglsvga)
SRC.OGLSVGA = $(wildcard plugins/video/canvas/openglx/svga/*.cpp)
OBJ.OGLSVGA = $(addprefix $(OUT),$(notdir $(SRC.OGLSVGA:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglsvga oglsvgaclean
vpath %.cpp plugins/video/canvas/openglx/svga

# Chain rules
clean: oglsvgaclean

oglsvga: $(OUTDIRS) $(OGLSVGA)

$(OGLSVGA): $(OBJ.OGLSVGA) $(DEP.OGLSVGA)
	$(DO.PLUGIN) $(LIBS.OGLSVGA)

oglsvgaclean:
	$(RM) $(OGLSVGA) $(OBJ.OGLSVGA) $(OUTOS)oglsvga.dep
 
ifdef DO_DEPEND
dep: $(OUTOS)oglsvga.dep
$(OUTOS)oglsvga.dep: $(SRC.OGLSVGA)
	$(DO.DEP)
else
-include $(OUTOS)oglsvga.dep
endif

endif # ifeq ($(MAKESECTION),targets)
