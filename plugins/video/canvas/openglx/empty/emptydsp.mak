# This is a subinclude file used to define the rules needed
# to build the Empty Displaydriver for GLX 2D driver -- oglempty

# Driver description
DESCRIPTION.oglempty=Empty Displaydriver for Crystal Space GL/X 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make oglempty     Make the $(DESCRIPTION.oglempty)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oglempty

all plugins glxdisp: oglempty

oglempty:
	$(MAKE_TARGET) MAKE_DLL=yes

oglemptyclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Local CFLAGS and libraries
#LIBS._oglempty+=-L$(X11_PATH)/lib -lXext -lX11

# The driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  oglempty=$(OUTDLL)oglempty$(DLL)
  LIBS.oglempty=$(LIBS._oglempty)
#  LIBS.oglempty=$(LIBS._oglempty) $(CSUTIL.LIB) $(CSSYS.LIB)
  DEP.oglempty=$(CSUTIL.LIB) $(CSSYS.LIB)
else
  oglempty=$(OUT)$(LIB_PREFIX)oglempty$(LIB)
  DEP.EXE+=$(oglempty)
  LIBS.EXE+=$(LIBS._oglempty) $(CSUTIL.LIB) $(CSSYS.LIB)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_oglempty
endif
DESCRIPTION.$(oglempty) = $(DESCRIPTION.oglempty)
SRC.oglempty = $(wildcard libs/cs2d/openglx/empty/*.cpp)
OBJ.oglempty = $(addprefix $(OUT),$(notdir $(SRC.oglempty:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglempty oglemptyclean

# Chain rules
clean: oglemptyclean

oglempty: $(OUTDIRS) $(oglempty)

$(OUT)%$O: libs/cs2d/openglx/empty/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.oglempty)
 
$(oglempty): $(OBJ.oglempty) $(DEP.oglempty)
	$(DO.PLUGIN) $(LIBS.oglempty)

oglemptyclean:
	$(RM) $(oglempty) $(OBJ.oglempty)
 
ifdef DO_DEPEND
depend: $(OUTOS)oglempty.dep
$(OUTOS)oglempty.dep: $(SRC.oglempty)
	$(DO.DEP)
else
-include $(OUTOS)oglempty.dep
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
