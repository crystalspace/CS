DESCRIPTION.explo = Explosion mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make explo        Make the $(DESCRIPTION.explo)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: explo exploclean
plugins meshes all: explo

exploclean:
	$(MAKE_CLEAN)
explo:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/explo plugins/mesh/object/partgen

ifeq ($(USE_PLUGINS),yes)
  EXPLO = $(OUTDLL)explo$(DLL)
  LIB.EXPLO = $(foreach d,$(DEP.EXPLO),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(EXPLO)
else
  EXPLO = $(OUT)$(LIB_PREFIX)explo$(LIB)
  DEP.EXE += $(EXPLO)
  SCF.STATIC += explo
  TO_INSTALL.STATIC_LIBS += $(EXPLO)
endif

INC.EXPLO = $(wildcard plugins/mesh/object/explo/*.h plugins/mesh/object/partgen/*.h)
SRC.EXPLO = $(wildcard plugins/mesh/object/explo/*.cpp plugins/mesh/object/partgen/*.cpp)
OBJ.EXPLO = $(addprefix $(OUT),$(notdir $(SRC.EXPLO:.cpp=$O)))
DEP.EXPLO = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += EXPLO
DSP.EXPLO.NAME = explo
DSP.EXPLO.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: explo exploclean
explo: $(OUTDIRS) $(EXPLO)

$(EXPLO): $(OBJ.EXPLO) $(LIB.EXPLO)
	$(DO.PLUGIN)

clean: exploclean
exploclean:
	-$(RM) $(EXPLO) $(OBJ.EXPLO)

ifdef DO_DEPEND
dep: $(OUTOS)explo.dep
$(OUTOS)explo.dep: $(SRC.EXPLO)
	$(DO.DEP)
else
-include $(OUTOS)explo.dep
endif

endif # ifeq ($(MAKESECTION),targets)
