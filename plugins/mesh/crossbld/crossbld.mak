DESCRIPTION.crossbld = Mesh object cross-builder plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make crossbld     Make the $(DESCRIPTION.crossbld)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: crossbld crossbldclean
plugins meshes all: crossbld

crossbldclean:
	$(MAKE_CLEAN)
crossbld:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/crossbld

ifeq ($(USE_PLUGINS),yes)
  CROSSBLD = $(OUTDLL)crossbld$(DLL)
  LIB.CROSSBLD = $(foreach d,$(DEP.CROSSBLD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CROSSBLD)
else
  CROSSBLD = $(OUT)$(LIB_PREFIX)crossbld$(LIB)
  DEP.EXE += $(CROSSBLD)
  SCF.STATIC += crossbld
  TO_INSTALL.STATIC_LIBS += $(CROSSBLD)
endif

INC.CROSSBLD = $(wildcard plugins/mesh/crossbld/*.h)
SRC.CROSSBLD = $(wildcard plugins/mesh/crossbld/*.cpp)
OBJ.CROSSBLD = $(addprefix $(OUT),$(notdir $(SRC.CROSSBLD:.cpp=$O)))
DEP.CROSSBLD = CSGEOM CSTOOL CSUTIL CSSYS CSUTIL CSGEOM

MSVC.DSP += CROSSBLD
DSP.CROSSBLD.NAME = crossbld
DSP.CROSSBLD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: crossbld crossbldclean
crossbld: $(OUTDIRS) $(CROSSBLD)

$(CROSSBLD): $(OBJ.CROSSBLD) $(LIB.CROSSBLD)
	$(DO.PLUGIN)

clean: crossbldclean
crossbldclean:
	-$(RM) $(CROSSBLD) $(OBJ.CROSSBLD)

ifdef DO_DEPEND
dep: $(OUTOS)crossbld.dep
$(OUTOS)crossbld.dep: $(SRC.CROSSBLD)
	$(DO.DEP)
else
-include $(OUTOS)crossbld.dep
endif

endif # ifeq ($(MAKESECTION),targets)
