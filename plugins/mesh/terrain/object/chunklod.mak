DESCRIPTION.chunklod = Chunked LOD Terrain

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make chunklod     Make the $(DESCRIPTION.chunklod)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: chunklod chunklodclean
plugins meshes all: chunklod

chunklodclean:
	$(MAKE_CLEAN)
chunklod:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/terrain/object

ifeq ($(USE_PLUGINS),yes)
  CHUNKLOD = $(OUTDLL)/chunklod$(DLL)
  LIB.CHUNKLOD = $(foreach d,$(DEP.CHUNKLOD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CHUNKLOD)
else
  CHUNKLOD = $(OUT)/$(LIB_PREFIX)chunklod$(LIB)
  DEP.EXE += $(CHUNKLOD)
  SCF.STATIC += chunklod
  TO_INSTALL.STATIC_LIBS += $(CHUNKLOD)
endif

INF.CHUNKLOD = $(SRCDIR)/plugins/mesh/terrain/object/chunklod.csplugin
INC.CHUNKLOD = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/terrain/object/chunklod.h))
SRC.CHUNKLOD = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/terrain/object/chunklod.cpp))
OBJ.CHUNKLOD = $(addprefix $(OUT)/,$(notdir $(SRC.CHUNKLOD:.cpp=$O)))
DEP.CHUNKLOD = CSTOOL CSGFX CSGEOM CSUTIL
CFLAGS.CHUNKLOD = $(CFLAGS.I)plugins/mesh/terrain/object

MSVC.DSP += CHUNKLOD
DSP.CHUNKLOD.NAME = chunklod
DSP.CHUNKLOD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: chunklod chunklodclean
chunklod: $(OUTDIRS) $(CHUNKLOD)

clean: chunklodclean
chunklodclean:
	-$(RMDIR) $(CHUNKLOD) $(OBJ.CHUNKLOD) $(OUTDLL)/$(notdir $(INF.CHUNKLOD))

$(OUT)/%$O: $(SRCDIR)/plugins/mesh/terrain/object/chunklod.o
	$(DO.COMPILE.CPP) $(CFLAGS.CHUNKLOD)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)/%$O: %.cpp
	$(DO.COMPILE.CPP)

$(CHUNKLOD): $(OBJ.CHUNKLOD) $(LIB.CHUNKLOD)
	$(DO.PLUGIN)

ifdef DO_DEPEND
dep: $(OUTOS)/chunklod.dep
$(OUTOS)/chunklod.dep: $(SRC.CHUNKLOD)
	$(DO.DEP1) $(CFLAGS.CHUNKLOD) $(DO.DEP2)
else
-include $(OUTOS)/chunklod.dep
endif

endif # ifeq ($(MAKESECTION),targets)
