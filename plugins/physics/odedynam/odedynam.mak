# This is a subinclude file used to define the rules needed
# to build the odedynam plug-in.

# Driver description
DESCRIPTION.odedynam = Crystal Space ODE dynamics plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make odedynam     Make the $(DESCRIPTION.odedynam)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: odedynam odedynamclean
all plugins: odedynam

odedynam:
	$(MAKE_TARGET) MAKE_DLL=yes
odedynamclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/physics/odedynam

ifeq ($(USE_PLUGINS),yes)
  odedynam = $(OUTDLL)odedynam$(DLL)
  LIB.odedynam = $(foreach d,$(DEP.odedynam),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(odedynam)
else
  odedynam = $(OUT)$(LIB_PREFIX)odedynam$(LIB)
  DEP.EXE += $(odedynam)
  SCF.STATIC += odedynam
  TO_INSTALL.STATIC_LIBS += $(odedynam)
endif

INC.odedynam = $(wildcard plugins/physics/odedynam/*.h)
SRC.odedynam = $(wildcard plugins/physics/odedynam/*.cpp)
OBJ.odedynam = $(addprefix $(OUT),$(notdir $(SRC.odedynam:.cpp=$O)))
DEP.odedynam = CSGEOM CSUTIL CSSYS

MSVC.DSP += odedynam
DSP.odedynam.NAME = odedynam
DSP.odedynam.TYPE = plugin
DSP.odedynam.LIBS = ode

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: odedynam odedynamclean

odedynam: $(OUTDIRS) $(odedynam)

$(odedynam): $(OBJ.odedynam) $(LIB.odedynam)
	$(DO.PLUGIN)

clean: odedynamclean
odedynamclean:
	$(RM) $(odedynam) $(OBJ.odedynam)

ifdef DO_DEPEND
dep: $(OUTOS)odedynam.dep
$(OUTOS)odedynam.dep: $(SRC.odedynam)
	$(DO.DEP)
else
-include $(OUTOS)odedynam.dep
endif

endif # ifeq ($(MAKESECTION),targets)
