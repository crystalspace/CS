# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring freefnt2,$(PLUGINS)))

# Application description
DESCRIPTION.csfgen = TrueType to Crystal Space font converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make csfgen       Make the $(DESCRIPTION.csfgen)$"

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

CSFGEN.EXE = csfgen$(EXE.CONSOLE)
DIR.CSFGEN = apps/tools/csfgen
OUT.CSFGEN = $(OUT)/$(DIR.CSFGEN)
INC.CSFGEN = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSFGEN)/*.h ))
SRC.CSFGEN = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSFGEN)/*.cpp ))
OBJ.CSFGEN = $(addprefix $(OUT.CSFGEN)/,$(notdir $(SRC.CSFGEN:.cpp=$O)))
DEP.CSFGEN = CSTOOL CSUTIL CSGEOM CSUTIL CSGFX
LIB.CSFGEN = $(foreach d,$(DEP.CSFGEN),$($d.LIB))

OUTDIRS += $(OUT.CSFGEN)

TO_INSTALL.EXE += $(CSFGEN.EXE)

MSVC.DSP += CSFGEN
DSP.CSFGEN.NAME = csfgen
DSP.CSFGEN.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.csfgen csfgenclean csfgencleandep

build.csfgen: $(OUTDIRS) $(CSFGEN.EXE)
clean: csfgenclean

$(OUT.CSFGEN)/%$O: $(SRCDIR)/$(DIR.CSFGEN)/%.cpp
	$(DO.COMPILE.CPP)

$(CSFGEN.EXE): $(OBJ.CSFGEN) $(LIB.CSFGEN)
	$(DO.LINK.CONSOLE.EXE)

csfgenclean:
	-$(RM) csfgen.txt
	-$(RMDIR) $(CSFGEN.EXE) $(OBJ.CSFGEN)

cleandep: csfgencleandep
csfgencleandep:
	-$(RM) $(OUT.CSFGEN)/csfgen.dep

ifdef DO_DEPEND
dep: $(OUT.CSFGEN) $(OUT.CSFGEN)/csfgen.dep
$(OUT.CSFGEN)/csfgen.dep: $(SRC.CSFGEN)
	$(DO.DEPEND)
else
-include $(OUT.CSFGEN)/csfgen.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifneq (,$(findstring freefont,$(PLUGINS)))
