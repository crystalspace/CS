# Library description
DESCRIPTION.csengine = Crystal Space 3D engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csengine     Make the $(DESCRIPTION.csengine)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csengine

all libs: csengine
csengine:
	$(MAKE_TARGET)
csengineclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSENGINE.LIB = $(OUT)/$(LIB_PREFIX)csengine$(LIB_SUFFIX)
DIR.CSENGINE = libs/csengine
OUT.CSENGINE = $(OUT)/$(DIR.CSENGINE)
INC.CSENGINE = \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSENGINE)/*.h include/csengine/*.h))
SRC.CSENGINE = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSENGINE)/*.cpp \
  $(DIR.CSENGINE)/*/*.cpp))
OBJ.CSENGINE = $(addprefix $(OUT.CSENGINE)/,$(notdir $(SRC.CSENGINE:.cpp=$O)))
CFG.CSENGINE = $(SRCDIR)/data/config/engine.cfg

OUTDIRS += $(OUT.CSENGINE)

TO_INSTALL.CONFIG += $(CFG.CSENGINE)
TO_INSTALL.STATIC_LIBS += $(CSENGINE.LIB)

MSVC.DSP += CSENGINE
DSP.CSENGINE.NAME = csengine
DSP.CSENGINE.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csengine csengineclean csenginecleandep

csengine: $(OUTDIRS) $(CSENGINE.LIB)

$(OUT.CSENGINE)/%$O: $(SRCDIR)/$(DIR.CSENGINE)/%.cpp
	$(DO.COMPILE.CPP)

$(OUT.CSENGINE)/%$O: $(SRCDIR)/$(DIR.CSENGINE)/basic/%.cpp
	$(DO.COMPILE.CPP)

$(OUT.CSENGINE)/%$O: $(SRCDIR)/$(DIR.CSENGINE)/light/%.cpp
	$(DO.COMPILE.CPP)

$(OUT.CSENGINE)/%$O: $(SRCDIR)/$(DIR.CSENGINE)/objects/%.cpp
	$(DO.COMPILE.CPP)

$(CSENGINE.LIB): $(OBJ.CSENGINE)
	$(DO.LIBRARY)

clean: csengineclean
csengineclean:
	-$(RM) $(CSENGINE.LIB) $(OBJ.CSENGINE)

cleandep: csenginecleandep
csenginecleandep:
	-$(RM) $(OUT.CSENGINE)/csengine.dep

ifdef DO_DEPEND
dep: $(OUT.CSENGINE) $(OUT.CSENGINE)/csengine.dep
$(OUT.CSENGINE)/csengine.dep: $(SRC.CSENGINE)
	$(DO.DEPEND)
else
-include $(OUT.CSENGINE)/csengine.dep
endif

endif # ifeq ($(MAKESECTION),targets)
