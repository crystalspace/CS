DESCRIPTION.fountain = Fountain mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fountain     Make the $(DESCRIPTION.fountain)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fountain fountclean
plugins meshes all: fountain

fountclean:
	$(MAKE_CLEAN)
fountain:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/fountain/object plugins/mesh/partgen

ifeq ($(USE_PLUGINS),yes)
  FOUNTAIN = $(OUTDLL)fountain$(DLL)
  LIB.FOUNTAIN = $(foreach d,$(DEP.FOUNTAIN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FOUNTAIN)
else
  FOUNTAIN = $(OUT)$(LIB_PREFIX)fountain$(LIB)
  DEP.EXE += $(FOUNTAIN)
  SCF.STATIC += fountain
  TO_INSTALL.STATIC_LIBS += $(FOUNTAIN)
endif

INC.FOUNTAIN = $(wildcard plugins/mesh/fountain/object/*.h plugins/mesh/partgen/*.h)
SRC.FOUNTAIN = $(wildcard plugins/mesh/fountain/object/*.cpp plugins/mesh/partgen/*.cpp)
OBJ.FOUNTAIN = $(addprefix $(OUT),$(notdir $(SRC.FOUNTAIN:.cpp=$O)))
DEP.FOUNTAIN = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += FOUNTAIN
DSP.FOUNTAIN.NAME = fountain
DSP.FOUNTAIN.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: fountain fountclean
fountain: $(OUTDIRS) $(FOUNTAIN)

$(FOUNTAIN): $(OBJ.FOUNTAIN) $(LIB.FOUNTAIN)
	$(DO.PLUGIN)

clean: fountclean
fountclean:
	-$(RM) $(FOUNTAIN) $(OBJ.FOUNTAIN)

ifdef DO_DEPEND
dep: $(OUTOS)fount.dep
$(OUTOS)fount.dep: $(SRC.FOUNTAIN)
	$(DO.DEP)
else
-include $(OUTOS)fount.dep
endif

endif # ifeq ($(MAKESECTION),targets)
