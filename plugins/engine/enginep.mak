#---------------------------
# Engine plugin submakefile
#---------------------------

DESCRIPTION.engine = Crystal Space 3D Engine Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make engine       Make the $(DESCRIPTION.engine)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: engine engineclean
plugins all: engine
engineclean:
	$(MAKE_CLEAN)
engine:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SRC.ENGINE = $(wildcard plugins/engine/*.cpp)
OBJ.ENGINE = $(addprefix $(OUT),$(notdir $(SRC.ENGINE:.cpp=$O)))
LIB.ENGINE = $(CSENGINE.LIB) $(CSTERR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB) \
  $(CSGEOM.LIB) $(CSOBJECT.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)

ifeq ($(USE_SHARED_PLUGINS),yes)
ENGINE=$(OUTDLL)enginep$(DLL)
DEP.ENGINE=$(LIB.ENGINE)
LDFLAGS.ENGINE=$(LIBS.EXE)
TO_INSTALL.DYNAMIC_LIBS += $(ENGINE)
else
ENGINE=$(OUT)$(LIB_PREFIX)enginep$(LIB)
DEP.EXE+=$(ENGINE)
CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_ENGINE
TO_INSTALL.STATIC_LIBS += $(ENGINE)
endif

vpath %.cpp $(sort $(dir $(SRC.ENGINE)))

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: engine engineclean
engine: $(OUTDIRS) $(ENGINE)

$(ENGINE): $(OBJ.ENGINE) $(DEP.ENGINE)
	$(DO.PLUGIN) $(LDFLAGS.ENGINE)

clean: engineclean
engineclean:
	-$(RM) $(ENGINE) $(OBJ.ENGINE) $(OUTOS)engine.dep

ifdef DO_DEPEND
dep: $(OUTOS)engine.dep
$(OUTOS)engine.dep: $(SRC.ENGINE)
	$(DO.DEP)
else
-include $(OUTOS)engine.dep
endif

endif # ifeq ($(MAKESECTION),targets)
