DESCRIPTION.particlesldr = Particles object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make particlesldr     Make the $(DESCRIPTION.particlesldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: particlesldr particlesldrclean
plugins meshes all: particlesldr

particlesldrclean:
	$(MAKE_CLEAN)
particlesldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/particles/persist/standard

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

INF.PARTICLESLDR = $(SRCDIR)/plugins/mesh/particles/persist/standard/particlesldr.csplugin
INC.PARTICLESLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/particles/persist/standard/*.h))
SRC.PARTICLESLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/particles/persist/standard/*.cpp))
OBJ.PARTICLESLDR = $(addprefix $(OUT)/,$(notdir $(SRC.PARTICLESLDR:.cpp=$O)))
DEP.PARTICLESLDR = CSGEOM CSUTIL CSUTIL

MSVC.DSP += PARTICLESLDR
DSP.PARTICLESLDR.NAME = particlesldr
DSP.PARTICLESLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: particlesldr particlesldrclean
particlesldr: $(OUTDIRS) $(PARTICLESLDR)

$(PARTICLESLDR): $(OBJ.PARTICLESLDR) $(LIB.PARTICLESLDR)
	$(DO.PLUGIN)

clean: particlesldrclean
particlesldrclean:
	-$(RMDIR) $(PARTICLESLDR) $(OBJ.PARTICLESLDR) $(OUTDLL)/$(notdir $(INF.PARTICLESLDR))

ifdef DO_DEPEND
dep: $(OUTOS)/particlesldr.dep
$(OUTOS)/particlesldr.dep: $(SRC.PARTICLESLDR)
	$(DO.DEP)
else
-include $(OUTOS)/particlesldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
