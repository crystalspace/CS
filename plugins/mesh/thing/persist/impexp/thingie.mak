DESCRIPTION.thingie = Thing import/export persistance plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make thingie        Make the $(DESCRIPTION.thingie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: thingie thingieclean
plugins meshes all: thingie

thingieclean:
	$(MAKE_CLEAN)
thingie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/thing/persist/impexp

ifeq ($(USE_PLUGINS),yes)
  THINGIE = $(OUTDLL)thingie$(DLL)
  LIB.THINGIE = $(foreach d,$(DEP.THINGIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(THINGIE)
else
  THINGIE = $(OUT)$(LIB_PREFIX)thingie$(LIB)
  DEP.EXE += $(THINGIE)
  SCF.STATIC += thingie
  TO_INSTALL.STATIC_LIBS += $(THINGIE)
endif

INC.THINGIE = $(wildcard plugins/mesh/thing/persist/impexp/*.h)
SRC.THINGIE = $(wildcard plugins/mesh/thing/persist/impexp/*.cpp)
OBJ.THINGIE = $(addprefix $(OUT),$(notdir $(SRC.THINGIE:.cpp=$O)))
DEP.THINGIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += THINGIE
DSP.THINGIE.NAME = thingie
DSP.THINGIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: thingie thingieclean
thingie: $(OUTDIRS) $(THINGIE)

$(THINGIE): $(OBJ.THINGIE) $(LIB.THINGIE)
	$(DO.PLUGIN)

clean: thingieclean
thingieclean:
	-$(RM) $(THINGIE) $(OBJ.THINGIE)

ifdef DO_DEPEND
dep: $(OUTOS)thingie.dep
$(OUTOS)thingie.dep: $(SRC.THINGIE)
	$(DO.DEP)
else
-include $(OUTOS)thingie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
