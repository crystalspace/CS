# This is a subinclude file used to define the rules needed
# to build the Empty Displaydriver for GLX 2D driver -- oglempty

# Driver description
DESCRIPTION.oglempty=Empty driver for Crystal Space GL/X 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make oglempty     Make the $(DESCRIPTION.oglempty)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oglempty

all plugins glxdisp: oglempty

oglempty:
	$(MAKE_TARGET) MAKE_DLL=yes

oglemptyclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Local CFLAGS and libraries
#LIBS._OGLEMPTY+=-L$(X11_PATH)/lib -lXext -lX11

# The driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  OGLEMPTY=$(OUTDLL)oglempty$(DLL)
  LIBS.OGLEMPTY=$(LIBS._OGLEMPTY)
#  LIBS.OGLEMPTY=$(LIBS._OGLEMPTY) $(CSUTIL.LIB) $(CSSYS.LIB)
  DEP.OGLEMPTY=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  OGLEMPTY=$(OUT)$(LIB_PREFIX)oglempty$(LIB)
  DEP.EXE+=$(OGLEMPTY)
  LIBS.EXE+=$(LIBS._OGLEMPTY) $(CSUTIL.LIB) $(CSSYS.LIB)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_OGLEMPTY
endif
DESCRIPTION.$(OGLEMPTY) = $(DESCRIPTION.oglempty)
SRC.OGLEMPTY = $(wildcard plugins/video/canvas/openglx/empty/*.cpp)
OBJ.OGLEMPTY = $(addprefix $(OUT),$(notdir $(SRC.OGLEMPTY:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglempty oglemptyclean

# Chain rules
clean: oglemptyclean

oglempty: $(OUTDIRS) $(OGLEMPTY)

$(OUT)%$O: plugins/video/canvas/openglx/empty/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.OGLEMPTY)
 
$(OGLEMPTY): $(OBJ.OGLEMPTY) $(DEP.OGLEMPTY)
	$(DO.PLUGIN) $(LIBS.OGLEMPTY)

oglemptyclean:
	$(RM) $(OGLEMPTY) $(OBJ.OGLEMPTY) $(OUTOS)oglempty.dep
 
ifdef DO_DEPEND
dep: $(OUTOS)oglempty.dep
$(OUTOS)oglempty.dep: $(SRC.OGLEMPTY)
	$(DO.DEP)
else
-include $(OUTOS)oglempty.dep
endif

endif # ifeq ($(MAKESECTION),targets)
