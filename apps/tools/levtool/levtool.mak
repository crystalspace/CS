# Application description
DESCRIPTION.levtool = Crystal Space Level Tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make levtool      Make the $(DESCRIPTION.levtool)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: levtool ltoolclean

all apps: levtool
levtool:
	$(MAKE_APP)
ltoolclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/levtool

LEVTOOL.EXE = levtool$(EXE)
INC.LEVTOOL = $(wildcard apps/tools/levtool/*.h)
SRC.LEVTOOL = $(wildcard apps/tools/levtool/*.cpp)
OBJ.LEVTOOL = $(addprefix $(OUT),$(notdir $(SRC.LEVTOOL:.cpp=$O)))
DEP.LEVTOOL = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.LEVTOOL = $(foreach d,$(DEP.LEVTOOL),$($d.LIB))

TO_INSTALL.EXE    += $(LEVTOOL.EXE)

MSVC.DSP += LEVTOOL
DSP.LEVTOOL.NAME = levtool
DSP.LEVTOOL.TYPE = appcon

#$(LEVTOOL.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.levtool ltoolclean

all: $(LEVTOOL.EXE)
build.levtool: $(OUTDIRS) $(LEVTOOL.EXE)
clean: ltoolclean

$(LEVTOOL.EXE): $(DEP.EXE) $(OBJ.LEVTOOL) $(LIB.LEVTOOL)
	$(DO.LINK.EXE)

ltoolclean:
	-$(RM) $(LEVTOOL.EXE) $(OBJ.LEVTOOL)

ifdef DO_DEPEND
dep: $(OUTOS)levtool.dep
$(OUTOS)levtool.dep: $(SRC.LEVTOOL)
	$(DO.DEP)
else
-include $(OUTOS)levtool.dep
endif

endif # ifeq ($(MAKESECTION),targets)
