# Application description
DESCRIPTION.blocks = Crystal Space Blocks game

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make blocks       Make the $(DESCRIPTION.blocks)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: blocks blocksclean

all apps: blocks
blocks:
	$(MAKE_APP)
blocksclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

BLOCKS.EXE = blocks$(EXE)
DIR.BLOCKS = apps/blocks
OUT.BLOCKS = $(OUT)/$(DIR.BLOCKS)
INC.BLOCKS = $(wildcard $(DIR.BLOCKS)/*.h)
SRC.BLOCKS = $(wildcard $(DIR.BLOCKS)/*.cpp)
OBJ.BLOCKS = $(addprefix $(OUT.BLOCKS)/,$(notdir $(SRC.BLOCKS:.cpp=$O)))
DEP.BLOCKS = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.BLOCKS = $(foreach d,$(DEP.BLOCKS),$($d.LIB))
CFG.BLOCKS = data/config/blocks.cfg

TO_INSTALL.EXE    += $(BLOCKS.EXE)
TO_INSTALL.CONFIG += $(CFG.BLOCKS)
TO_INSTALL.DATA   += data/blocks.zip

MSVC.DSP += BLOCKS
DSP.BLOCKS.NAME = blocks
DSP.BLOCKS.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#---------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.blocks blocksclean blockscleandep

all: $(BLOCKS.EXE)
build.blocks: $(OUT.BLOCKS) $(BLOCKS.EXE)
clean: blocksclean

$(OUT.BLOCKS)/%$O: $(DIR.BLOCKS)/%.cpp
	$(DO.COMPILE.CPP)

$(BLOCKS.EXE): $(DEP.EXE) $(OBJ.BLOCKS) $(LIB.BLOCKS)
	$(DO.LINK.EXE)

$(OUT.BLOCKS):
	$(MKDIRS)

blocksclean:
	-$(RMDIR) $(BLOCKS.EXE) $(OBJ.BLOCKS)

cleandep: blockscleandep
blockscleandep:
	-$(RM) $(OUT.BLOCKS)/blocks.dep

ifdef DO_DEPEND
dep: $(OUT.BLOCKS) $(OUT.BLOCKS)/blocks.dep
$(OUT.BLOCKS)/blocks.dep: $(SRC.BLOCKS)
	$(DO.DEPEND)
else
-include $(OUT.BLOCKS)/blocks.dep
endif

endif # ifeq ($(MAKESECTION),targets)
