DESCRIPTION.particles = Particle System

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make particles     Make the $(DESCRIPTION.particles)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: particles particlesclean
plugins meshes all: particles

particlesclean:
	$(MAKE_CLEAN)
particles:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/particles/object

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

INF.PARTICLES = $(SRCDIR)/plugins/mesh/particles/object/particles.csplugin
INC.PARTICLES = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/particles/object/particles.h))
SRC.PARTICLES = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/particles/object/particles.cpp))
OBJ.PARTICLES = $(addprefix $(OUT)/,$(notdir $(SRC.PARTICLES:.cpp=$O)))
DEP.PARTICLES = CSGFX CSGEOM CSUTIL CSSYS CSUTIL
CFLAGS.PARTICLES = $(CFLAGS.I)plugins/mesh/particles/object

MSVC.DSP += PARTICLES
DSP.PARTICLES.NAME = particles
DSP.PARTICLES.TYPE = plugin
DSP.PARTICLES.CFLAGS = /I "..\..\plugins\mesh\particles\object"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: particles particlesclean
particles: $(OUTDIRS) $(PARTICLES)

clean: particlesclean
particlesclean:
	-$(RMDIR) $(PARTICLES) $(OBJ.PARTICLES) $(OUTDLL)/$(notdir $(INF.PARTICLES))

$(OUT)/%$O: $(SRCDIR)/plugins/mesh/particles/object/particles.o
	$(DO.COMPILE.CPP) $(CFLAGS.PARTICLES)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)/%$O: %.cpp
	$(DO.COMPILE.CPP)

$(PARTICLES): $(OBJ.PARTICLES) $(LIB.PARTICLES)
	$(DO.PLUGIN)

ifdef DO_DEPEND
dep: $(OUTOS)/particles.dep
$(OUTOS)/particles.dep: $(SRC.PARTICLES)
	$(DO.DEP1) $(CFLAGS.PARTICLES) $(DO.DEP2)
else
-include $(OUTOS)/particles.dep
endif

endif # ifeq ($(MAKESECTION),targets)
