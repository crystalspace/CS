# Application description
DESCRIPTION.csxmlconv = Crystal Space World To XML Convertor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make csxmlconv    Make the $(DESCRIPTION.csxmlconv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csxmlconv csxmlconvclean

all apps: csxmlconv
csxmlconv:
	$(MAKE_TARGET)
csxmlconvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/cs2xml

CSXMLCONV.EXE = cs2xml$(EXE)
INC.CSXMLCONV = $(wildcard apps/tools/cs2xml/*.h)
SRC.CSXMLCONV = $(wildcard apps/tools/cs2xml/*.cpp)
OBJ.CSXMLCONV = $(addprefix $(OUT),$(notdir $(SRC.CSXMLCONV:.cpp=$O)))
DEP.CSXMLCONV = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.CSXMLCONV = $(foreach d,$(DEP.CSXMLCONV),$($d.LIB))

TO_INSTALL.EXE    += $(CSXMLCONV.EXE)

MSVC.DSP += CSXMLCONV
DSP.CSXMLCONV.NAME = cs2xml
DSP.CSXMLCONV.TYPE = appcon

#$(CSXMLCONV.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csxmlconv csxmlconvclean

all: $(CSXMLCONV.EXE)
csxmlconv: $(OUTDIRS) $(CSXMLCONV.EXE)
clean: csxmlconvclean

$(CSXMLCONV.EXE): $(DEP.EXE) $(OBJ.CSXMLCONV) $(LIB.CSXMLCONV)
	$(DO.LINK.EXE)

csxmlconvclean:
	-$(RM) $(CSXMLCONV.EXE) $(OBJ.CSXMLCONV)

ifdef DO_DEPEND
dep: $(OUTOS)cs2xml.dep
$(OUTOS)cs2xml.dep: $(SRC.CSXMLCONV)
	$(DO.DEP)
else
-include $(OUTOS)cs2xml.dep
endif

endif # ifeq ($(MAKESECTION),targets)
