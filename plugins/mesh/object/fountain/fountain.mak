DESCRIPTION.fount = Fountain mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fount        Make the $(DESCRIPTION.fount)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fount fountclean
plugins all: fount

fountclean:
	$(MAKE_CLEAN)
fount:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/fountain plugins/mesh/object/partgen

ifeq ($(USE_PLUGINS),yes)
  FOUNT = $(OUTDLL)fountain$(DLL)
  LIB.FOUNT = $(foreach d,$(DEP.FOUNT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FOUNT)
else
  FOUNT = $(OUT)$(LIB_PREFIX)fountain$(LIB)
  DEP.EXE += $(FOUNT)
  SCF.STATIC += fount
  TO_INSTALL.STATIC_LIBS += $(FOUNT)
endif

INC.FOUNT = $(wildcard plugins/mesh/object/fountain/*.h plugins/mesh/object/partgen/*.h)
SRC.FOUNT = $(wildcard plugins/mesh/object/fountain/*.cpp plugins/mesh/object/partgen/*.cpp)
OBJ.FOUNT = $(addprefix $(OUT),$(notdir $(SRC.FOUNT:.cpp=$O)))
DEP.FOUNT = CSGEOM CSUTIL CSSYS

MSVC.DSP += FOUNT
DSP.FOUNT.NAME = fountain
DSP.FOUNT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: fount fountclean
fount: $(OUTDIRS) $(FOUNT)

$(FOUNT): $(OBJ.FOUNT) $(LIB.FOUNT)
	$(DO.PLUGIN)

clean: fountclean
fountclean:
	-$(RM) $(FOUNT) $(OBJ.FOUNT) $(OUTOS)fount.dep

ifdef DO_DEPEND
dep: $(OUTOS)fount.dep
$(OUTOS)fount.dep: $(SRC.FOUNT)
	$(DO.DEP)
else
-include $(OUTOS)fount.dep
endif

endif # ifeq ($(MAKESECTION),targets)
