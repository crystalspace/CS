# This is a subinclude file used to define the rules needed
# to build the SVGA Displaydriver for GLX 2D driver -- oglsvga

# Driver description
DESCRIPTION.oglsvga=SVGA Displaydriver for Crystal Space GL/X 2D driver

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
  LIBS.oglsvga=$(LIBS._oglsvga)
#  LIBS.oglsvga=$(LIBS._oglsvga) $(CSUTIL.LIB) $(CSSYS.LIB)
  DEP.oglsvga=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  oglsvga=$(OUT)$(LIB_PREFIX)oglsvga$(LIB)
  DEP.EXE+=$(oglsvga)
  LIBS.EXE+=$(LIBS._oglsvga) $(CSUTIL.LIB) $(CSSYS.LIB)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_oglsvga
endif
DESCRIPTION.$(oglsvga) = $(DESCRIPTION.oglsvga)
SRC.oglsvga = $(wildcard libs/cs2d/openglx/svga/*.cpp)
OBJ.oglsvga = $(addprefix $(OUT),$(notdir $(SRC.oglsvga:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglsvga oglsvgaclean

# Chain rules
clean: oglsvgaclean

oglsvga: $(OUTDIRS) $(oglsvga)

$(OUT)%$O: libs/cs2d/openglx/svga/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.oglsvga)
 
$(oglsvga): $(OBJ.oglsvga) $(DEP.oglsvga)
	$(DO.PLUGIN) $(LIBS.oglsvga)

oglsvgaclean:
	$(RM) $(oglsvga) $(OBJ.oglsvga)
 
ifdef DO_DEPEND
depend: $(OUTOS)oglsvga.dep
$(OUTOS)oglsvga.dep: $(SRC.oglsvga)
	$(DO.DEP)
else
-include $(OUTOS)oglsvga.dep
endif

endif # ifeq ($(MAKESECTION),targets)

#------------------------------------------------------------------- config ---#
ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)

# Default value for DO_SHM
ifndef DO_SHM
  DO_SHM = yes
endif

ifeq ($(DO_SHM)$(findstring DO_SHM,$(MAKE_VOLATILE_H)),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_SHM$">>volatile.tmp
endif

endif # ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)
