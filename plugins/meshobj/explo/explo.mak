DESCRIPTION.explo = Explosion mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make explo        Make the $(DESCRIPTION.explo)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: explo exploclean
plugins all: explo

exploclean:
	$(MAKE_CLEAN)
explo:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/meshobj/explo plugins/meshobj/partgen

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

INC.EXPLO = $(wildcard plugins/meshobj/explo/*.h plugins/meshobj/partgen/*.h)
SRC.EXPLO = $(wildcard plugins/meshobj/explo/*.cpp plugins/meshobj/partgen/*.cpp)
OBJ.EXPLO = $(addprefix $(OUT),$(notdir $(SRC.EXPLO:.cpp=$O)))
DEP.EXPLO = CSGEOM CSUTIL CSSYS

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
	-$(RM) $(EXPLO) $(OBJ.EXPLO) $(OUTOS)explo.dep

ifdef DO_DEPEND
dep: $(OUTOS)explo.dep
$(OUTOS)explo.dep: $(SRC.EXPLO)
	$(DO.DEP)
else
-include $(OUTOS)explo.dep
endif

endif # ifeq ($(MAKESECTION),targets)
