DESCRIPTION.partphys_simple = Simple Particles Physics

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make partphys_simple     Make the $(DESCRIPTION.partphys_simple)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: partphys_simple partphys_simpleclean
plugins meshes all: partphys_simple

partphys_simpleclean:
	$(MAKE_CLEAN)
partphys_simple:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/particles/physics/simple

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

INF.PARTPHYS_SIMPLE = $(SRCDIR)/plugins/mesh/particles/physics/simple/partphys_simple.csplugin
INC.PARTPHYS_SIMPLE = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/particles/physics/simple/simplephys.h))
SRC.PARTPHYS_SIMPLE = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/particles/physics/simple/simplephys.cpp))
OBJ.PARTPHYS_SIMPLE = $(addprefix $(OUT)/,$(notdir $(SRC.PARTPHYS_SIMPLE:.cpp=$O)))
DEP.PARTPHYS_SIMPLE = CSGFX CSGEOM CSUTIL CSSYS CSUTIL
CFLAGS.PARTPHYS_SIMPLE = $(CFLAGS.I)plugins/mesh/particles/physics/simple

MSVC.DSP += PARTPHYS_SIMPLE
DSP.PARTPHYS_SIMPLE.NAME = partphys_simple
DSP.PARTPHYS_SIMPLE.TYPE = plugin
DSP.PARTPHYS_SIMPLE.CFLAGS = /I "..\..\plugins\mesh\particles\physics\simple"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: partphys_simple partphys_simpleclean
partphys_simple: $(OUTDIRS) $(PARTPHYS_SIMPLE)

clean: partphys_simpleclean
partphys_simpleclean:
	-$(RMDIR) $(PARTPHYS_SIMPLE) $(OBJ.PARTPHYS_SIMPLE) $(OUTDLL)/$(notdir $(INF.PARTPHYS_SIMPLE))

$(OUT)/%$O: $(SRCDIR)/plugins/mesh/particles/physics/simple/simplephys.o
	$(DO.COMPILE.CPP) $(CFLAGS.PARTPHYS_SIMPLE)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)/%$O: %.cpp
	$(DO.COMPILE.CPP)

$(PARTPHYS_SIMPLE): $(OBJ.PARTPHYS_SIMPLE) $(LIB.PARTPHYS_SIMPLE)
	$(DO.PLUGIN)

ifdef DO_DEPEND
dep: $(OUTOS)/partphys_simple.dep
$(OUTOS)/partphys_simple.dep: $(SRC.PARTPHYS_SIMPLE)
	$(DO.DEP1) $(CFLAGS.PARTPHYS_SIMPLE) $(DO.DEP2)
else
-include $(OUTOS)/partphys_simple.dep
endif

endif # ifeq ($(MAKESECTION),targets)
