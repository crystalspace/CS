# This is a subinclude file used to define the rules needed
# to build the GlideX 2D driver -- glidex2d

# Driver description
DESCRIPTION.glidex2d = Crystal Space Glide/X 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make glidex2d        Make the $(DESCRIPTION.glidex2d)$"

ifeq ($(DO_SHM)$(findstring DO_SHM,$(MAKE_VOLATILE_H)),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DO_SHM$">>$@
endif

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glidex2d

all drivers drivers2d: glidex2d

glidex2d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# local CFLAGS
CFLAGS.GLIDEX2D+=-L$(X11_PATH)/include -I/usr/local/glide/include
LIBS._GLIDEX2D+=-L$(X11_PATH)/lib -lXext -lX11 -lglide2x  

# The 2D GlideX driver
ifeq ($(USE_DLL),yes)
  GLIDEX2D=glidex2d$(DLL)
  LIBS.GLIDEX2D=$(LIBS._GLIDEX2D)
else
  GLIDEX2D=$(OUT)$(LIB_PREFIX)glidex2d$(LIB)
  DEP.EXE+=$(GLIDEX2D)
  LIBS.EXE+=$(LIBS._GLIDEX2D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_GLIDEX2D
endif
DESCRIPTION.$(GLIDEX2D) = $(DESCRIPTION.glidex2d)
SRC.GLIDEX2D = $(wildcard libs/cs2d/unxglide2/*.cpp $(SRC.COMMON.DRV2D))
OBJ.GLIDEX2D = $(addprefix $(OUT),$(notdir $(SRC.GLIDEX2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glidex2d glidexclean glidexcleanlib

# Chain rules
clean: glidexclean
cleanlib: glidexcleanlib

glidex2d: $(OUTDIRS) $(GLIDEX2D)

$(OUT)%$O: libs/cs2d/unxglide2/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDEX2D)
 
$(GLIDEX2D): $(OBJ.GLIDEX2D) $(CSCOM.LIB) $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
	$(DO.LIBRARY) $(LIBS.GLIDEX2D)

glidexclean:
	$(RM) $(GLIDEX2D)

glidexcleanlib:
	$(RM) $(OBJ.GLIDEX2D) $(GLIDEX2D)

ifdef DO_DEPEND
$(OUTOS)glidex2d.dep: $(SRC.GLIDEX2D)
	$(DO.DEP)
endif

-include $(OUTOS)glidex2d.dep

endif # ifeq ($(MAKESECTION),targets)
