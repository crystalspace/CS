DESCRIPTION.particlesldr = Particles object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make particlesldr  Make the $(DESCRIPTION.particlesldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: particlesldr particlesldrclean
all plugins: particlesldr

particlesldr:
	$(MAKE_TARGET) MAKE_DLL=yes
particlesldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  PARTICLESLDR = $(OUTDLL)/particlesldr$(DLL)
  LIB.PARTICLESLDR = $(foreach d,$(DEP.PARTICLESLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PARTICLESLDR)
else
  PARTICLESLDR = $(OUT)/$(LIB_PREFIX)particlesldr$(LIB)
  DEP.EXE += $(PARTICLESLDR)
  SCF.STATIC += particlesldr
  TO_INSTALL.STATIC_LIBS += $(PARTICLESLDR)
endif

DIR.PARTICLESLDR = plugins/mesh/particles/persist/standard
OUT.PARTICLESLDR = $(OUT)/$(DIR.PARTICLESLDR)
INF.PARTICLESLDR = $(SRCDIR)/$(DIR.PARTICLESLDR)/particlesldr.csplugin
INC.PARTICLESLDR = $(wildcard $(SRCDIR)/$(DIR.PARTICLESLDR)/*.h)
SRC.PARTICLESLDR = $(wildcard $(SRCDIR)/$(DIR.PARTICLESLDR)/*.cpp)
OBJ.PARTICLESLDR = \
  $(addprefix $(OUT.PARTICLESLDR)/,$(notdir $(SRC.PARTICLESLDR:.cpp=$O)))
DEP.PARTICLESLDR = CSGEOM CSUTIL

OUTDIRS += $(OUT.PARTICLESLDR)

MSVC.DSP += PARTICLESLDR
DSP.PARTICLESLDR.NAME = particlesldr
DSP.PARTICLESLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: particlesldr particlesldrclean particlesldrcleandep

particlesldr: $(OUTDIRS) $(PARTICLESLDR)

$(OUT.PARTICLESLDR)/%$O: $(SRCDIR)/$(DIR.PARTICLESLDR)/%.cpp
	$(DO.COMPILE.CPP)

$(PARTICLESLDR): $(OBJ.PARTICLESLDR) $(LIB.PARTICLESLDR)
	$(DO.PLUGIN)

clean: particlesldrclean
particlesldrclean:
	-$(RMDIR) $(PARTICLESLDR) $(OBJ.PARTICLESLDR) \
	$(OUTDLL)/$(notdir $(INF.PARTICLESLDR))

cleandep: particlesldrcleandep
particlesldrcleandep:
	-$(RM) $(OUT.PARTICLESLDR)/particlesldr.dep

ifdef DO_DEPEND
dep: $(OUT.PARTICLESLDR) $(OUT.PARTICLESLDR)/particlesldr.dep
$(OUT.PARTICLESLDR)/particlesldr.dep: $(SRC.PARTICLESLDR)
	$(DO.DEPEND)
else
-include $(OUT.PARTICLESLDR)/particlesldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
