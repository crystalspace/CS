DESCRIPTION.particles = Particle system plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make particles    Make the $(DESCRIPTION.particles)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: particles particlesclean
all plugins: particles

particles:
	$(MAKE_TARGET) MAKE_DLL=yes
particlesclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  PARTICLES = $(OUTDLL)/particles$(DLL)
  LIB.PARTICLES = $(foreach d,$(DEP.PARTICLES),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PARTICLES)
else
  PARTICLES = $(OUT)/$(LIB_PREFIX)particles$(LIB)
  DEP.EXE += $(PARTICLES)
  SCF.STATIC += particles
  TO_INSTALL.STATIC_LIBS += $(PARTICLES)
endif

DIR.PARTICLES = plugins/mesh/particles/object
OUT.PARTICLES = $(OUT)/$(DIR.PARTICLES)
INF.PARTICLES = $(SRCDIR)/$(DIR.PARTICLES)/particles.csplugin
INC.PARTICLES = $(wildcard $(SRCDIR)/$(DIR.PARTICLES)/*.h)
SRC.PARTICLES = $(wildcard $(SRCDIR)/$(DIR.PARTICLES)/*.cpp)
OBJ.PARTICLES = \
  $(addprefix $(OUT.PARTICLES)/,$(notdir $(SRC.PARTICLES:.cpp=$O)))
DEP.PARTICLES = CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.PARTICLES)

MSVC.DSP += PARTICLES
DSP.PARTICLES.NAME = particles
DSP.PARTICLES.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: particles particlesclean particlescleandep

particles: $(OUTDIRS) $(PARTICLES)

$(OUT.PARTICLES)/%$O: $(SRCDIR)/$(DIR.PARTICLES)/%.cpp
	$(DO.COMPILE.CPP)

$(PARTICLES): $(OBJ.PARTICLES) $(LIB.PARTICLES)
	$(DO.PLUGIN)

clean: particlesclean
particlesclean:
	-$(RMDIR) $(PARTICLES) $(OBJ.PARTICLES) \
	$(OUTDLL)/$(notdir $(INF.PARTICLES))

cleandep: particlescleandep
particlescleandep:
	-$(RM) $(OUT.PARTICLES)/particles.dep

ifdef DO_DEPEND
dep: $(OUT.PARTICLES) $(OUT.PARTICLES)/particles.dep
$(OUT.PARTICLES)/particles.dep: $(SRC.PARTICLES)
	$(DO.DEPEND)
else
-include $(OUT.PARTICLES)/particles.dep
endif

endif # ifeq ($(MAKESECTION),targets)
