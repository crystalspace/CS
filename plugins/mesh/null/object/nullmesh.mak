DESCRIPTION.nullmesh = General mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make nullmesh     Make the $(DESCRIPTION.nullmesh)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: nullmesh nullmeshclean
plugins meshes all: nullmesh

nullmeshclean:
	$(MAKE_CLEAN)
nullmesh:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/null/object

ifeq ($(USE_PLUGINS),yes)
  NULLMESH = $(OUTDLL)/nullmesh$(DLL)
  LIB.NULLMESH = $(foreach d,$(DEP.NULLMESH),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(NULLMESH)
else
  NULLMESH = $(OUT)/$(LIB_PREFIX)nullmesh$(LIB)
  DEP.EXE += $(NULLMESH)
  SCF.STATIC += nullmesh
  TO_INSTALL.STATIC_LIBS += $(NULLMESH)
endif

INC.NULLMESH = $(wildcard plugins/mesh/null/object/*.h)
SRC.NULLMESH = $(wildcard plugins/mesh/null/object/*.cpp)
OBJ.NULLMESH = $(addprefix $(OUT)/,$(notdir $(SRC.NULLMESH:.cpp=$O)))
DEP.NULLMESH = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += NULLMESH
DSP.NULLMESH.NAME = nullmesh
DSP.NULLMESH.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: nullmesh nullmeshclean
nullmesh: $(OUTDIRS) $(NULLMESH)

$(NULLMESH): $(OBJ.NULLMESH) $(LIB.NULLMESH)
	$(DO.PLUGIN)

clean: nullmeshclean
nullmeshclean:
	-$(RM) $(NULLMESH) $(OBJ.NULLMESH)

ifdef DO_DEPEND
dep: $(OUTOS)/nullmesh.dep
$(OUTOS)/nullmesh.dep: $(SRC.NULLMESH)
	$(DO.DEP)
else
-include $(OUTOS)/nullmesh.dep
endif

endif # ifeq ($(MAKESECTION),targets)
