# Application description
DESCRIPTION.csfedit = Crystal Space font editor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make csfedit      Make the $(DESCRIPTION.csfedit)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csfedit csfeditclean

all apps: csfedit
csfedit:
	$(MAKE_APP)
csfeditclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/csfedit apps/support

CSFEDIT.EXE=csfedit$(EXE)
INC.CSFEDIT = $(wildcard apps/tools/csfedit/*.h)
SRC.CSFEDIT = $(wildcard apps/tools/csfedit/*.cpp)
OBJ.CSFEDIT = $(addprefix $(OUT)/,$(notdir $(SRC.CSFEDIT:.cpp=$O)))
DEP.CSFEDIT = CSGFX CSWS CSTOOL CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSFEDIT = $(foreach d,$(DEP.CSFEDIT),$($d.LIB))

#TO_INSTALL.EXE += $(CSFEDIT.EXE)

MSVC.DSP += CSFEDIT
DSP.CSFEDIT.NAME = csfedit
DSP.CSFEDIT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.csfedit csfeditclean

all: $(CSFEDIT.EXE)
build.csfedit: $(OUTDIRS) $(CSFEDIT.EXE)
clean: csfeditclean

$(CSFEDIT.EXE): $(DEP.EXE) $(OBJ.CSFEDIT) $(LIB.CSFEDIT)
	$(DO.LINK.EXE)

csfeditclean:
	-$(RM) $(CSFEDIT.EXE) $(OBJ.CSFEDIT)

ifdef DO_DEPEND
dep: $(OUTOS)/csfedit.dep
$(OUTOS)/csfedit.dep: $(SRC.CSFEDIT)
	$(DO.DEP)
else
-include $(OUTOS)/csfedit.dep
endif

endif # ifeq ($(MAKESECTION),targets)
