# This is a subinclude file used to define the rules needed
# to build the SVGA Displaydriver for GLX 2D driver -- oglsvga

# Driver description
DESCRIPTION.oglsvga=SVGA driver for Crystal Space GL/X 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make oglsvga     Make the $(DESCRIPTION.oglsvga)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oglsvga

all plugins glxdisp: oglsvga

oglsvga:
	$(MAKE_TARGET) MAKE_DLL=yes

oglsvgaclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Local CFLAGS and libraries
#LIBS._oglsvga+=-L$(X11_PATH)/lib -lXext -lX11

# The driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  oglsvga=$(OUTDLL)oglsvga$(DLL)
  LIBS.OGLSVGA=$(LIBS._oglsvga)
  DEP.OGLSVGA=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  oglsvga=$(OUT)$(LIB_PREFIX)oglsvga$(LIB)
  DEP.EXE+=$(oglsvga)
  LIBS.EXE+=$(LIBS._oglsvga) $(CSUTIL.LIB) $(CSSYS.LIB)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_oglsvga
endif
DESCRIPTION.$(oglsvga) = $(DESCRIPTION.OGLSVGA)
SRC.OGLSVGA = $(wildcard libs/cs2d/openglx/svga/*.cpp)
OBJ.OGLSVGA = $(addprefix $(OUT),$(notdir $(SRC.OGLSVGA:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglsvga oglsvgaclean
vpath %.cpp libs/cs2d/openglx/svga

# Chain rules
clean: oglsvgaclean

oglsvga: $(OUTDIRS) $(oglsvga)

$(oglsvga): $(OBJ.OGLSVGA) $(DEP.OGLSVGA)
	$(DO.PLUGIN) $(LIBS.OGLSVGA)

oglsvgaclean:
	$(RM) $(oglsvga) $(OBJ.OGLSVGA)
 
ifdef DO_DEPEND
dep: $(OUTOS)oglsvga.dep
$(OUTOS)oglsvga.dep: $(SRC.OGLSVGA)
	$(DO.DEP)
else
-include $(OUTOS)oglsvga.dep
endif

endif # ifeq ($(MAKESECTION),targets)
