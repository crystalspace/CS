# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring freefnt2,$(PLUGINS)))

# Application description
DESCRIPTION.csfgen = TrueType to Crystal Space font converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make csfgen         Make the $(DESCRIPTION.csfgen)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csfgen csfgenclean

all apps: csfgen
csfgen:
	$(MAKE_APP)
csfgenclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/csfgen

CSFGEN.EXE = csfgen$(EXE.CONSOLE)
INC.CSFGEN = $(wildcard apps/tools/csfgen/*.h)
SRC.CSFGEN = $(wildcard apps/tools/csfgen/*.cpp)
OBJ.CSFGEN = $(addprefix $(OUT)/,$(notdir $(SRC.CSFGEN:.cpp=$O)))
DEP.CSFGEN = CSTOOL CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSFGEN = $(foreach d,$(DEP.CSFGEN),$($d.LIB))

TO_INSTALL.EXE += $(CSFGEN.EXE)

MSVC.DSP += CSFGEN
DSP.CSFGEN.NAME = csfgen
DSP.CSFGEN.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.csfgen csfgenclean

all: $(CSFGEN.EXE)
build.csfgen: $(OUTDIRS) $(CSFGEN.EXE)
clean: csfgenclean

$(CSFGEN.EXE): $(OBJ.CSFGEN) $(LIB.CSFGEN)
	$(DO.LINK.CONSOLE.EXE)

csfgenclean:
	-$(RMDIR) $(CSFGEN.EXE) $(OBJ.CSFGEN)

ifdef DO_DEPEND
dep: $(OUTOS)/csfgen.dep
$(OUTOS)/csfgen.dep: $(SRC.CSFGEN)
	$(DO.DEP)
else
-include $(OUTOS)/csfgen.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring freefont,$(PLUGINS)))
