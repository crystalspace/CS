# This is a subinclude file used to define the rules needed
# to build the metagen plug-in.

# Plug-in description
DESCRIPTION.metagen = MetaGen mesh factory

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make metagen      Make the $(DESCRIPTION.metagen)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: metagen metagenclean
all plugins: metagen

metagen:
	$(MAKE_TARGET) MAKE_DLL=yes
metagenclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/metagen/object

ifeq ($(USE_PLUGINS),yes)
  METAGEN = $(OUTDLL)/metagen$(DLL)
  LIB.METAGEN = $(foreach d,$(DEP.METAGEN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(METAGEN)
else
  METAGEN = $(OUT)/$(LIB_PREFIX)metagen$(LIB)
  DEP.EXE += $(METAGEN)
  SCF.STATIC += metagen
  TO_INSTALL.STATIC_LIBS += $(METAGEN)
endif

INF.METAGEN = $(SRCDIR)/plugins/mesh/metagen/object/metagen.csplugin
INC.METAGEN = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/metagen/object/*.h))
SRC.METAGEN = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/metagen/object/*.cpp))
OBJ.METAGEN = $(addprefix $(OUT)/,$(notdir $(SRC.METAGEN:.cpp=$O)))
DEP.METAGEN = CSGEOM CSUTIL

MSVC.DSP += METAGEN
DSP.METAGEN.NAME = metagen
DSP.METAGEN.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: metagen metagenclean

# Chain rules
net: metagen
clean: metagenclean

metagen: $(OUTDIRS) $(METAGEN)

$(METAGEN): $(OBJ.METAGEN) $(LIB.METAGEN)
	$(DO.PLUGIN)

metagenclean:
	-$(RMDIR) $(METAGEN) $(OBJ.METAGEN) $(OUTDLL)/$(notdir $(INF.METAGEN))

ifdef DO_DEPEND
dep: $(OUTOS)/metagen.dep
$(OUTOS)/metagen.dep: $(SRC.METAGEN)
	$(DO.DEP)
else
-include $(OUTOS)/metagen.dep
endif

endif # ifeq ($(MAKESECTION),targets)
