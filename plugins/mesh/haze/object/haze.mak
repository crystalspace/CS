DESCRIPTION.haze = Haze mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make haze         Make the $(DESCRIPTION.haze)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: haze hazeclean
plugins meshes all: haze

hazeclean:
	$(MAKE_CLEAN)
haze:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/haze/object

ifeq ($(USE_PLUGINS),yes)
  HAZE = $(OUTDLL)haze$(DLL)
  LIB.HAZE = $(foreach d,$(DEP.HAZE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(HAZE)
else
  HAZE = $(OUT)$(LIB_PREFIX)haze$(LIB)
  DEP.EXE += $(HAZE)
  SCF.STATIC += haze
  TO_INSTALL.STATIC_LIBS += $(HAZE)
endif

INC.HAZE = $(wildcard plugins/mesh/haze/object/*.h)
SRC.HAZE = $(wildcard plugins/mesh/haze/object/*.cpp)
OBJ.HAZE = $(addprefix $(OUT),$(notdir $(SRC.HAZE:.cpp=$O)))
DEP.HAZE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += HAZE
DSP.HAZE.NAME = haze
DSP.HAZE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: haze hazeclean
haze: $(OUTDIRS) $(HAZE)

$(HAZE): $(OBJ.HAZE) $(LIB.HAZE)
	$(DO.PLUGIN)

clean: hazeclean
hazeclean:
	-$(RM) $(HAZE) $(OBJ.HAZE)

ifdef DO_DEPEND
dep: $(OUTOS)haze.dep
$(OUTOS)haze.dep: $(SRC.HAZE)
	$(DO.DEP)
else
-include $(OUTOS)haze.dep
endif

endif # ifeq ($(MAKESECTION),targets)
