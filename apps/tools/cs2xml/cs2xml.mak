# Application description
DESCRIPTION.cs2xml = Crystal Space World To XML Convertor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make cs2xml       Make the $(DESCRIPTION.cs2xml)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cs2xml cs2xmlclean

all apps: cs2xml
cs2xml:
	$(MAKE_APP)
cs2xmlclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CS2XML.EXE = cs2xml$(EXE.CONSOLE)
DIR.CS2XML = apps/tools/cs2xml
OUT.CS2XML = $(OUT)/$(DIR.CS2XML)
INC.CS2XML = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CS2XML)/*.h ))
SRC.CS2XML = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CS2XML)/*.cpp ))
OBJ.CS2XML = $(addprefix $(OUT.CS2XML)/,$(notdir $(SRC.CS2XML:.cpp=$O)))
DEP.CS2XML = CSTOOL CSGEOM CSTOOL CSGFX CSUTIL CSUTIL
LIB.CS2XML = $(foreach d,$(DEP.CS2XML),$($d.LIB))

OUTDIRS += $(OUT.CS2XML)

TO_INSTALL.EXE += $(CS2XML.EXE)

MSVC.DSP += CS2XML
DSP.CS2XML.NAME = cs2xml
DSP.CS2XML.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.cs2xml cs2xmlclean cs2xmlcleandep

all: $(CS2XML.EXE)
build.cs2xml: $(OUTDIRS) $(CS2XML.EXE)
clean: cs2xmlclean

$(OUT.CS2XML)/%$O: $(SRCDIR)/$(DIR.CS2XML)/%.cpp
	$(DO.COMPILE.CPP)

$(CS2XML.EXE): $(DEP.EXE) $(OBJ.CS2XML) $(LIB.CS2XML)
	$(DO.LINK.CONSOLE.EXE)

cs2xmlclean:
	-$(RM) cs2xml.txt
	-$(RMDIR) $(CS2XML.EXE) $(OBJ.CS2XML)

cleandep: cs2xmlcleandep
cs2xmlcleandep:
	-$(RM) $(OUT.CS2XML)/cs2xml.dep

ifdef DO_DEPEND
dep: $(OUT.CS2XML) $(OUT.CS2XML)/cs2xml.dep
$(OUT.CS2XML)/cs2xml.dep: $(SRC.CS2XML)
	$(DO.DEPEND)
else
-include $(OUT.CS2XML)/cs2xml.dep
endif

endif # ifeq ($(MAKESECTION),targets)
