DESCRIPTION.cube = Cube mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make cube         Make the $(DESCRIPTION.cube)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cube cubeclean
plugins all: cube

cubeclean:
	$(MAKE_CLEAN)
cube:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/meshobj/cube

ifeq ($(USE_PLUGINS),yes)
  CUBE = $(OUTDLL)cube$(DLL)
  LIB.CUBE = $(foreach d,$(DEP.CUBE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CUBE)
else
  CUBE = $(OUT)$(LIB_PREFIX)cube$(LIB)
  DEP.EXE += $(CUBE)
  SCF.STATIC += cube
  TO_INSTALL.STATIC_LIBS += $(CUBE)
endif

INC.CUBE = $(wildcard plugins/meshobj/cube/*.h)
SRC.CUBE = $(wildcard plugins/meshobj/cube/*.cpp)
OBJ.CUBE = $(addprefix $(OUT),$(notdir $(SRC.CUBE:.cpp=$O)))
DEP.CUBE = CSGEOM CSUTIL CSSYS

MSVC.DSP += CUBE
DSP.CUBE.NAME = cube
DSP.CUBE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cube cubeclean
cube: $(OUTDIRS) $(CUBE)

$(CUBE): $(OBJ.CUBE) $(LIB.CUBE)
	$(DO.PLUGIN)

clean: cubeclean
cubeclean:
	-$(RM) $(CUBE) $(OBJ.CUBE) $(OUTOS)cube.dep

ifdef DO_DEPEND
dep: $(OUTOS)cube.dep
$(OUTOS)cube.dep: $(SRC.CUBE)
	$(DO.DEP)
else
-include $(OUTOS)cube.dep
endif

endif # ifeq ($(MAKESECTION),targets)
