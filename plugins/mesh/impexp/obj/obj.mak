DESCRIPTION.objie = OBJ Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make objie        Make the $(DESCRIPTION.objie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: objie objieclean
plugins meshes all: objie

objieclean:
	$(MAKE_CLEAN)
objie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/obj

ifeq ($(USE_PLUGINS),yes)
  OBJIE = $(OUTDLL)objie$(DLL)
  LIB.OBJIE = $(foreach d,$(DEP.OBJIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(OBJIE)
else
  OBJIE = $(OUT)$(LIB_PREFIX)objie$(LIB)
  DEP.EXE += $(OBJIE)
  SCF.STATIC += objie
  TO_INSTALL.STATIC_LIBS += $(OBJIE)
endif

INC.OBJIE = $(wildcard plugins/mesh/impexp/obj/*.h)
SRC.OBJIE = $(wildcard plugins/mesh/impexp/obj/*.cpp)
OBJ.OBJIE = $(addprefix $(OUT),$(notdir $(SRC.OBJIE:.cpp=$O)))
DEP.OBJIE = CSGEOM CSUTIL CSSYS CSUTIL CSTOOL CSUTIL CSGEOM

MSVC.DSP += OBJIE
DSP.OBJIE.NAME = objie
DSP.OBJIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: objie objieclean
objie: $(OUTDIRS) $(OBJIE)

$(OBJIE): $(OBJ.OBJIE) $(LIB.OBJIE)
	$(DO.PLUGIN)

clean: objieclean
objieclean:
	-$(RM) $(OBJIE) $(OBJ.OBJIE)

ifdef DO_DEPEND
dep: $(OUTOS)objie.dep
$(OUTOS)objie.dep: $(SRC.OBJIE)
	$(DO.DEP)
else
-include $(OUTOS)objie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
