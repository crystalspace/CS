DESCRIPTION.partphys_simple = Simple particle physics

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
$(NEWLINE)echo $"  make partphys_simple$" \
$(NEWLINE)echo $"                    Make the $(DESCRIPTION.partphys_simple)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: partphys_simple partphys_simpleclean
all plugins: partphys_simple

partphys_simple:
	$(MAKE_TARGET) MAKE_DLL=yes
partphys_simpleclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  PARTPHYS_SIMPLE = $(OUTDLL)/partphys_simple$(DLL)
  LIB.PARTPHYS_SIMPLE = $(foreach d,$(DEP.PARTPHYS_SIMPLE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PARTPHYS_SIMPLE)
else
  PARTPHYS_SIMPLE = $(OUT)/$(LIB_PREFIX)partphys_simple$(LIB)
  DEP.EXE += $(PARTPHYS_SIMPLE)
  SCF.STATIC += partphys_simple
  TO_INSTALL.STATIC_LIBS += $(PARTPHYS_SIMPLE)
endif

DIR.PARTPHYS_SIMPLE = plugins/mesh/particles/physics/simple
OUT.PARTPHYS_SIMPLE = $(OUT)/$(DIR.PARTPHYS_SIMPLE)
INF.PARTPHYS_SIMPLE = $(SRCDIR)/$(DIR.PARTPHYS_SIMPLE)/partphys_simple.csplugin
INC.PARTPHYS_SIMPLE = $(wildcard $(SRCDIR)/$(DIR.PARTPHYS_SIMPLE)/*.h)
SRC.PARTPHYS_SIMPLE = $(wildcard $(SRCDIR)/$(DIR.PARTPHYS_SIMPLE)/*.cpp)
OBJ.PARTPHYS_SIMPLE = \
  $(addprefix $(OUT.PARTPHYS_SIMPLE)/,$(notdir $(SRC.PARTPHYS_SIMPLE:.cpp=$O)))
DEP.PARTPHYS_SIMPLE = CSGEOM CSUTIL

OUTDIRS += $(OUT.PARTPHYS_SIMPLE)

MSVC.DSP += PARTPHYS_SIMPLE
DSP.PARTPHYS_SIMPLE.NAME = partphys_simple
DSP.PARTPHYS_SIMPLE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: partphys_simple partphys_simpleclean partphys_simplecleandep

partphys_simple: $(OUTDIRS) $(PARTPHYS_SIMPLE)

$(OUT.PARTPHYS_SIMPLE)/%$O: $(SRCDIR)/$(DIR.PARTPHYS_SIMPLE)/%.cpp
	$(DO.COMPILE.CPP)

$(PARTPHYS_SIMPLE): $(OBJ.PARTPHYS_SIMPLE) $(LIB.PARTPHYS_SIMPLE)
	$(DO.PLUGIN)

clean: partphys_simpleclean
partphys_simpleclean:
	-$(RMDIR) $(PARTPHYS_SIMPLE) $(OBJ.PARTPHYS_SIMPLE) \
	$(OUTDLL)/$(notdir $(INF.PARTPHYS_SIMPLE))

cleandep: partphys_simplecleandep
partphys_simplecleandep:
	-$(RM) $(OUT.PARTPHYS_SIMPLE)/partphys_simple.dep

ifdef DO_DEPEND
dep: $(OUT.PARTPHYS_SIMPLE) $(OUT.PARTPHYS_SIMPLE)/partphys_simple.dep
$(OUT.PARTPHYS_SIMPLE)/partphys_simple.dep: $(SRC.PARTPHYS_SIMPLE)
	$(DO.DEPEND)
else
-include $(OUT.PARTPHYS_SIMPLE)/partphys_simple.dep
endif

endif # ifeq ($(MAKESECTION),targets)
