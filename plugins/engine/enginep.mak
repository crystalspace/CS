DESCRIPTION.enginep = Crystal Space Engine Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make enginep      Make the $(DESCRIPTION.enginep)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: enginep enginepclean
plugins all: enginep
enginepclean:
	$(MAKE_CLEAN)
enginep:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SRC.ENGINEP = $(wildcard plugins/engine/*.cpp)
OBJ.ENGINEP = $(addprefix $(OUT),$(notdir $(SRC.ENGINEP:.cpp=$O)))
LIB.ENGINEP = $(CSENGINE.LIB) $(CSTERR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB) \
  $(CSGEOM.LIB) $(CSOBJECT.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)

ifeq ($(USE_SHARED_PLUGINS),yes)
ENGINEP=$(OUTDLL)enginep$(DLL)
DEP.ENGINEP=$(LIB.ENGINEP)
LDFLAGS.ENGINEP=$(LIBS.EXE)
TO_INSTALL.DYNAMIC_LIBS += $(ENGINEP)
else
ENGINEP=$(OUT)$(LIB_PREFIX)enginep$(LIB)
DEP.EXE+=$(ENGINEP)
CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_ENGINEP
TO_INSTALL.STATIC_LIBS += $(ENGINEP)
endif

vpath %.cpp $(sort $(dir $(SRC.ENGINEP)))

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: enginep enginepclean
enginep: $(OUTDIRS) $(ENGINEP)

$(ENGINEP): $(OBJ.ENGINEP) $(DEP.ENGINEP)
	$(DO.PLUGIN) $(LDFLAGS.ENGINEP)

clean: enginepclean
enginepclean:
	-$(RM) $(ENGINEP) $(OBJ.ENGINEP) $(OUTOS)enginep.dep

ifdef DO_DEPEND
dep: $(OUTOS)enginep.dep
$(OUTOS)enginep.dep: $(SRC.ENGINEP)
	$(DO.DEP)
else
-include $(OUTOS)enginep.dep
endif

endif # ifeq ($(MAKESECTION),targets)
