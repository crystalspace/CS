# Application description
DESCRIPTION.cs2xml = Crystal Space World To XML Convertor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make cs2xml       Make the $(DESCRIPTION.cs2xml)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cs2xml csxmlconvclean

all apps: cs2xml
cs2xml:
	$(MAKE_APP)
csxmlconvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/cs2xml

CS2XML.EXE = cs2xml$(EXE)
INC.CS2XML = $(wildcard apps/tools/cs2xml/*.h)
SRC.CS2XML = $(wildcard apps/tools/cs2xml/*.cpp)
OBJ.CS2XML = $(addprefix $(OUT)/,$(notdir $(SRC.CS2XML:.cpp=$O)))
DEP.CS2XML = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.CS2XML = $(foreach d,$(DEP.CS2XML),$($d.LIB))

TO_INSTALL.EXE    += $(CS2XML.EXE)

MSVC.DSP += CS2XML
DSP.CS2XML.NAME = cs2xml
DSP.CS2XML.TYPE = appcon

#$(CS2XML.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.cs2xml csxmlconvclean

all: $(CS2XML.EXE)
build.cs2xml: $(OUTDIRS) $(CS2XML.EXE)
clean: csxmlconvclean

$(CS2XML.EXE): $(DEP.EXE) $(OBJ.CS2XML) $(LIB.CS2XML)
	$(DO.LINK.EXE)

csxmlconvclean:
	-$(RM) $(CS2XML.EXE) $(OBJ.CS2XML)

ifdef DO_DEPEND
dep: $(OUTOS)/cs2xml.dep
$(OUTOS)/cs2xml.dep: $(SRC.CS2XML)
	$(DO.DEP)
else
-include $(OUTOS)/cs2xml.dep
endif

endif # ifeq ($(MAKESECTION),targets)
