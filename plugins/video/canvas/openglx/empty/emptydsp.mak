# This is a subinclude file used to define the rules needed
# to build the Empty Displaydriver for GLX 2D driver -- oglempty

# Driver description
DESCRIPTION.oglempty = Crystal Space GL/X 2D eMpTy driver

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

ifeq ($(USE_PLUGINS),yes)
  OGLEMPTY = $(OUTDLL)oglempty$(DLL)
  LIB.OGLEMPTY = $(foreach d,$(DEP.OGLEMPTY),$($d.LIB))
  DEP.OGLEMPTY = $(CSUTIL.LIB) $(CSSYS.LIB)
else
  OGLEMPTY = $(OUT)$(LIB_PREFIX)oglempty$(LIB)
  DEP.EXE += $(OGLEMPTY)
  LIBS.EXE += $(CSUTIL.LIB) $(CSSYS.LIB)
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_OGLEMPTY
endif

INC.OGLEMPTY = $(wildcard plugins/video/canvas/openglx/empty/*.cpp)
SRC.OGLEMPTY = $(wildcard plugins/video/canvas/openglx/empty/*.cpp)
OBJ.OGLEMPTY = $(addprefix $(OUT),$(notdir $(SRC.OGLEMPTY:.cpp=$O)))
DEP.OGLEMPTY =

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglempty oglemptyclean

oglempty: $(OUTDIRS) $(OGLEMPTY)

$(OUT)%$O: plugins/video/canvas/openglx/empty/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.OGLEMPTY)
 
$(OGLEMPTY): $(OBJ.OGLEMPTY) $(LIB.OGLEMPTY)
	$(DO.PLUGIN)

clean: oglemptyclean
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
