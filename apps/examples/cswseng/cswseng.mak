# Application description
DESCRIPTION.cswse = Crystal Space Example: CSWS And Engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make cswse        Make the $(DESCRIPTION.cswse)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cswse cswseclean

all apps: cswse
cswse:
	$(MAKE_TARGET)
cswseclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/examples/cswseng

CSWSE.EXE = cswseng$(EXE)
INC.CSWSE = $(wildcard apps/examples/cswseng/*.h)
SRC.CSWSE = $(wildcard apps/examples/cswseng/*.cpp)
OBJ.CSWSE = $(addprefix $(OUT),$(notdir $(SRC.CSWSE:.cpp=$O)))
DEP.CSWSE = \
  CSWS CSTOOL CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSWSE = $(foreach d,$(DEP.CSWSE),$($d.LIB))

#TO_INSTALL.EXE    += $(CSWSE.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSE)

MSVC.DSP += CSWSE
DSP.CSWSE.NAME = cswseng
DSP.CSWSE.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cswse cswseclean

all: $(CSWSE.EXE)
cswse: $(OUTDIRS) $(CSWSE.EXE)
clean: cswseclean

$(CSWSE.EXE): $(DEP.EXE) $(OBJ.CSWSE) $(LIB.CSWSE)
	$(DO.LINK.EXE)

cswseclean:
	-$(RM) $(CSWSE.EXE) $(OBJ.CSWSE)

ifdef DO_DEPEND
dep: $(OUTOS)cswse.dep
$(OUTOS)cswse.dep: $(SRC.CSWSE)
	$(DO.DEP)
else
-include $(OUTOS)cswse.dep
endif

endif # ifeq ($(MAKESECTION),targets)
