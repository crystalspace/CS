# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring freefont,$(PLUGINS)))

# Application description
DESCRIPTION.csfg = TrueType to Crystal Space font converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make csfg         Make the $(DESCRIPTION.csfg)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csfg csfgclean

all apps: csfg
csfg:
	$(MAKE_TARGET)
csfgclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/csfgen

CSFGEN.EXE = csfgen$(EXE)
INC.CSFGEN = $(wildcard apps/tools/csfgen/*.h)
SRC.CSFGEN = $(wildcard apps/tools/csfgen/*.cpp)
OBJ.CSFGEN = $(addprefix $(OUT),$(notdir $(SRC.CSFGEN:.cpp=$O)))
DEP.CSFGEN = CSSYS CSUTIL CSGEOM CSTOOL
LIB.CSFGEN = $(foreach d,$(DEP.CSFGEN),$($d.LIB))

TO_INSTALL.EXE += $(CSFGEN.EXE)

MSVC.DSP += CSFGEN
DSP.CSFGEN.NAME = csfgen
DSP.CSFGEN.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csfg csfgclean

all: $(CSFGEN.EXE)
csfg: $(OUTDIRS) $(CSFGEN.EXE)
clean: csfgclean

$(CSFGEN.EXE): $(OBJ.CSFGEN) $(LIB.CSFGEN)
	$(DO.LINK.CONSOLE.EXE)

csfgclean:
	-$(RM) $(CSFGEN.EXE) $(OBJ.CSFGEN)

ifdef DO_DEPEND
dep: $(OUTOS)csfg.dep
$(OUTOS)csfg.dep: $(SRC.CSFGEN)
	$(DO.DEP)
else
-include $(OUTOS)csfg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring freefont,$(PLUGINS)))
