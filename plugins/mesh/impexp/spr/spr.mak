DESCRIPTION.sprie = SPR Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make sprie        Make the $(DESCRIPTION.sprie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sprie sprieclean
plugins meshes all: sprie

sprieclean:
	$(MAKE_CLEAN)
sprie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/spr

ifeq ($(USE_PLUGINS),yes)
  SPRIE = $(OUTDLL)sprie$(DLL)
  LIB.SPRIE = $(foreach d,$(DEP.SPRIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPRIE)
else
  SPRIE = $(OUT)$(LIB_PREFIX)sprie$(LIB)
  DEP.EXE += $(SPRIE)
  SCF.STATIC += sprie
  TO_INSTALL.STATIC_LIBS += $(SPRIE)
endif

INC.SPRIE = $(wildcard plugins/mesh/impexp/spr/*.h)
SRC.SPRIE = $(wildcard plugins/mesh/impexp/spr/*.cpp)
OBJ.SPRIE = $(addprefix $(OUT),$(notdir $(SRC.SPRIE:.cpp=$O)))
DEP.SPRIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SPRIE
DSP.SPRIE.NAME = sprie
DSP.SPRIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sprie sprieclean
sprie: $(OUTDIRS) $(SPRIE)

$(SPRIE): $(OBJ.SPRIE) $(LIB.SPRIE)
	$(DO.PLUGIN)

clean: sprieclean
sprieclean:
	-$(RM) $(SPRIE) $(OBJ.SPRIE)

ifdef DO_DEPEND
dep: $(OUTOS)sprie.dep
$(OUTOS)sprie.dep: $(SRC.SPRIE)
	$(DO.DEP)
else
-include $(OUTOS)sprie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
