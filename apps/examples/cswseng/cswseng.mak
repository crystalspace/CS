# Application description
DESCRIPTION.cswseng = Crystal Space Example: CSWS And Engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make cswseng      Make the $(DESCRIPTION.cswseng)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cswseng cswsengclean

all apps: cswseng
cswseng:
	$(MAKE_APP)
cswsengclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/examples/cswseng

CSWSENG.EXE = cswseng$(EXE)
INC.CSWSENG = $(wildcard apps/examples/cswseng/*.h)
SRC.CSWSENG = $(wildcard apps/examples/cswseng/*.cpp)
OBJ.CSWSENG = $(addprefix $(OUT),$(notdir $(SRC.CSWSENG:.cpp=$O)))
DEP.CSWSENG = \
  CSWS CSTOOL CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSWSENG = $(foreach d,$(DEP.CSWSENG),$($d.LIB))

#TO_INSTALL.EXE    += $(CSWSENG.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSENG)

MSVC.DSP += CSWSENG
DSP.CSWSENG.NAME = cswseng
DSP.CSWSENG.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.cswseng cswsengclean

all: $(CSWSENG.EXE)
build.cswseng: $(OUTDIRS) $(CSWSENG.EXE)
clean: cswsengclean

$(CSWSENG.EXE): $(DEP.EXE) $(OBJ.CSWSENG) $(LIB.CSWSENG)
	$(DO.LINK.EXE)

cswsengclean:
	-$(RM) $(CSWSENG.EXE) $(OBJ.CSWSENG)

ifdef DO_DEPEND
dep: $(OUTOS)cswseng.dep
$(OUTOS)cswseng.dep: $(SRC.CSWSENG)
	$(DO.DEP)
else
-include $(OUTOS)cswseng.dep
endif

endif # ifeq ($(MAKESECTION),targets)
