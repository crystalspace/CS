DESCRIPTION.snow = Snow mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make snow         Make the $(DESCRIPTION.snow)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: snow snowclean
plugins meshes all: snow

snowclean:
	$(MAKE_CLEAN)
snow:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/snow/object plugins/mesh/partgen

ifeq ($(USE_PLUGINS),yes)
  SNOW = $(OUTDLL)snow$(DLL)
  LIB.SNOW = $(foreach d,$(DEP.SNOW),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNOW)
else
  SNOW = $(OUT)$(LIB_PREFIX)snow$(LIB)
  DEP.EXE += $(SNOW)
  SCF.STATIC += snow
  TO_INSTALL.STATIC_LIBS += $(SNOW)
endif

INC.SNOW = $(wildcard plugins/mesh/snow/object/*.h plugins/mesh/partgen/*.h)
SRC.SNOW = $(wildcard plugins/mesh/snow/object/*.cpp plugins/mesh/partgen/*.cpp)
OBJ.SNOW = $(addprefix $(OUT),$(notdir $(SRC.SNOW:.cpp=$O)))
DEP.SNOW = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SNOW
DSP.SNOW.NAME = snow
DSP.SNOW.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: snow snowclean
snow: $(OUTDIRS) $(SNOW)

$(SNOW): $(OBJ.SNOW) $(LIB.SNOW)
	$(DO.PLUGIN)

clean: snowclean
snowclean:
	-$(RM) $(SNOW) $(OBJ.SNOW)

ifdef DO_DEPEND
dep: $(OUTOS)snow.dep
$(OUTOS)snow.dep: $(SRC.SNOW)
	$(DO.DEP)
else
-include $(OUTOS)snow.dep
endif

endif # ifeq ($(MAKESECTION),targets)
