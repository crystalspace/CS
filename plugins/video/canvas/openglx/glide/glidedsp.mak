# This is a subinclude file used to define the rules needed
# to build the Glide Displaydriver for GLX 2D driver -- oglglide

# Driver description
DESCRIPTION.oglglide=Glide driver for Crystal Space GL/X 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make oglglide     Make the $(DESCRIPTION.oglglide)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oglglide oglglideclean

all plugins glxdisp: oglglide

oglglide:
	$(MAKE_TARGET) MAKE_DLL=yes

oglglideclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Local CFLAGS and libraries
#LIBS._OGLGLIDE+=-L$(X11_PATH)/lib -lXext -lX11

CFLAGS.OGLGLIDE+=-I/usr/include/glide -I/usr/local/glide/include
LIBS._OGLGLIDE+=-lglide2x

# The driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  OGLGLIDE=$(OUTDLL)oglglide$(DLL)
  LIBS.OGLGLIDE=$(LIBS._OGLGLIDE)
#  LIBS.OGLGLIDE=$(LIBS._OGLGLIDE) $(CSUTIL.LIB) $(CSSYS.LIB)
  DEP.OGLGLIDE=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  OGLGLIDE=$(OUT)$(LIB_PREFIX)oglglide$(LIB)
  DEP.EXE+=$(OGLGLIDE)
  LIBS.EXE+=$(LIBS._OGLGLIDE) $(CSUTIL.LIB) $(CSSYS.LIB)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_OGLGLIDE
endif
DESCRIPTION.$(OGLGLIDE) = $(DESCRIPTION.oglglide)
SRC.OGLGLIDE = $(wildcard plugins/video/canvas/openglx/glide/*.cpp)
OBJ.OGLGLIDE = $(addprefix $(OUT),$(notdir $(SRC.OGLGLIDE:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglglide oglglideclean

# Chain rules
clean: oglglideclean

oglglide: $(OUTDIRS) $(OGLGLIDE)

$(OUT)%$O: plugins/video/canvas/openglx/glide/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.OGLGLIDE)
 
$(OGLGLIDE): $(OBJ.OGLGLIDE) $(DEP.OGLGLIDE)
	$(DO.PLUGIN) $(LIBS.OGLGLIDE)

oglglideclean:
	$(RM) $(OGLGLIDE) $(OBJ.OGLGLIDE) $(OUTOS)oglglide.dep
 
ifdef DO_DEPEND
dep: $(OUTOS)oglglide.dep
$(OUTOS)oglglide.dep: $(SRC.OGLGLIDE)
	$(DO.DEP1) $(CFLAGS.OGLGLIDE) $(DO.DEP2)
else
-include $(OUTOS)oglglide.dep
endif

endif # ifeq ($(MAKESECTION),targets)
