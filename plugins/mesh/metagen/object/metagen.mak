# This is a subinclude file used to define the rules needed
# to build the metagen plug-in.

# Plug-in description
DESCRIPTION.metagen = Crystal Space MetaGen mesh factory

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make metagen     Make the $(DESCRIPTION.metagen)$"

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

vpath %.cpp plugins/mesh/metagen/object

ifeq ($(USE_SHARED_PLUGINS),yes)
  METAGEN = $(OUTDLL)metagen$(DLL)
  LIB.METAGEN = $(foreach d,$(DEP.METAGEN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(METAGEN)
else
  METAGEN = $(OUT)$(LIB_PREFIX)meta$(LIB)
  DEP.EXE += $(METAGEN)
  SCF.STATIC += metagen
  TO_INSTALL.STATIC_LIBS += $(METAGEN)
endif

INC.METAGEN = $(wildcard plugins/mesh/metagen/object/*.h)
SRC.METAGEN = $(wildcard plugins/mesh/metagen/object/*.cpp)
OBJ.METAGEN = $(addprefix $(OUT),$(notdir $(SRC.METAGEN:.cpp=$O)))
DEP.METAGEN = CSGEOM CSUTIL CSSYS

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
	$(RM) $(METAGEN) $(OBJ.METAGEN)

ifdef DO_DEPEND
dep: $(OUTOS)metagen.dep
$(OUTOS)metagen.dep: $(SRC.METAGEN)
	$(DO.DEP)
else
-include $(OUTOS)metagen.dep
endif

endif # ifeq ($(MAKESECTION),targets)
