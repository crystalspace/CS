DESCRIPTION.partphys_ode = ODE based particle physics

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
$(NEWLINE)echo $"  make partphys_ode$" \
$(NEWLINE)echo $"                    Make the $(DESCRIPTION.partphys_ode)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: partphys_ode partphys_odeclean
all meshes plugins: partphys_ode

partphys_ode:
	$(MAKE_TARGET) MAKE_DLL=yes
partphys_odeclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  PARTPHYS_ODE = $(OUTDLL)/partphys_ode$(DLL)
  LIB.PARTPHYS_ODE = $(foreach d,$(DEP.PARTPHYS_ODE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PARTPHYS_ODE)
else
  PARTPHYS_ODE = $(OUT)/$(LIB_PREFIX)partphys_ode$(LIB)
  DEP.EXE += $(PARTPHYS_ODE)
  SCF.STATIC += partphys_ode
  TO_INSTALL.STATIC_LIBS += $(PARTPHYS_ODE)
endif

DIR.PARTPHYS_ODE = plugins/mesh/particles/physics/simple
OUT.PARTPHYS_ODE = $(OUT)/$(DIR.PARTPHYS_ODE)
INF.PARTPHYS_ODE = $(SRCDIR)/$(DIR.PARTPHYS_ODE)/partphys_ode.csplugin
INC.PARTPHYS_ODE = $(wildcard $(SRCDIR)/$(DIR.PARTPHYS_ODE)/*.h)
SRC.PARTPHYS_ODE = $(wildcard $(SRCDIR)/$(DIR.PARTPHYS_ODE)/*.cpp)
OBJ.PARTPHYS_ODE = \
  $(addprefix $(OUT.PARTPHYS_ODE)/,$(notdir $(SRC.PARTPHYS_ODE:.cpp=$O)))
DEP.PARTPHYS_ODE = CSGEOM CSUTIL

OUTDIRS += $(OUT.PARTPHYS_ODE)

MSVC.DSP += PARTPHYS_ODE
DSP.PARTPHYS_ODE.NAME = partphys_ode
DSP.PARTPHYS_ODE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: partphys_ode partphys_odeclean partphys_odecleandep

partphys_ode: $(OUTDIRS) $(PARTPHYS_ODE)

$(OUT.PARTPHYS_ODE)/%$O: $(SRCDIR)/$(DIR.PARTPHYS_ODE)/%.cpp
	$(DO.COMPILE.CPP)

$(PARTPHYS_ODE): $(OBJ.PARTPHYS_ODE) $(LIB.PARTPHYS_ODE)
	$(DO.PLUGIN)

clean: partphys_odeclean
partphys_odeclean:
	-$(RMDIR) $(PARTPHYS_ODE) $(OBJ.PARTPHYS_ODE) \
	$(OUTDLL)/$(notdir $(INF.PARTPHYS_ODE))

cleandep: partphys_odecleandep
partphys_odecleandep:
	-$(RM) $(OUT.PARTPHYS_ODE)/partphys_ode.dep

ifdef DO_DEPEND
dep: $(OUT.PARTPHYS_ODE) $(OUT.PARTPHYS_ODE)/partphys_ode.dep
$(OUT.PARTPHYS_ODE)/partphys_ode.dep: $(SRC.PARTPHYS_ODE)
	$(DO.DEPEND)
else
-include $(OUT.PARTPHYS_ODE)/partphys_ode.dep
endif

endif # ifeq ($(MAKESECTION),targets)
