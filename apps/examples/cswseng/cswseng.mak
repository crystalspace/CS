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

CSWSENG.EXE = cswseng$(EXE)
DIR.CSWSENG = apps/examples/cswseng
OUT.CSWSENG = $(OUT)/$(DIR.CSWSENG)
INC.CSWSENG = $(wildcard $(DIR.CSWSENG)/*.h)
SRC.CSWSENG = $(wildcard $(DIR.CSWSENG)/*.cpp)
OBJ.CSWSENG = $(addprefix $(OUT.CSWSENG)/,$(notdir $(SRC.CSWSENG:.cpp=$O)))
DEP.CSWSENG = CSWS CSTOOL CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.CSWSENG = $(foreach d,$(DEP.CSWSENG),$($d.LIB))

#TO_INSTALL.EXE    += $(CSWSENG.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSENG)

MSVC.DSP += CSWSENG
DSP.CSWSENG.NAME = cswseng
DSP.CSWSENG.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.cswseng cswsengclean cswsengcleandep

all: $(CSWSENG.EXE)
build.cswseng: $(OUT.CSWSENG) $(CSWSENG.EXE)
clean: cswsengclean

$(OUT.CSWSENG)/%$O: $(DIR.CSWSENG)/%.cpp
	$(DO.COMPILE.CPP)

$(CSWSENG.EXE): $(DEP.EXE) $(OBJ.CSWSENG) $(LIB.CSWSENG)
	$(DO.LINK.EXE)

$(OUT.CSWSENG):
	$(MKDIRS)

cswsengclean:
	-$(RMDIR) $(CSWSENG.EXE) $(OBJ.CSWSENG)

cleandep: cswsengcleandep
cswsengcleandep:
	-$(RM) $(OUT.CSWSENG)/cswseng.dep

ifdef DO_DEPEND
dep: $(OUT.CSWSENG) $(OUT.CSWSENG)/cswseng.dep
$(OUT.CSWSENG)/cswseng.dep: $(SRC.CSWSENG)
	$(DO.DEPEND)
else
-include $(OUT.CSWSENG)/cswseng.dep
endif

endif # ifeq ($(MAKESECTION),targets)
