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

vpath %.cpp apps/blocks apps/support

BLOCKS.EXE = blocks$(EXE)
INC.BLOCKS = $(wildcard apps/blocks/*.h)
SRC.BLOCKS = $(wildcard apps/blocks/*.cpp)
OBJ.BLOCKS = $(addprefix $(OUT)/,$(notdir $(SRC.BLOCKS:.cpp=$O)))
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

.PHONY: build.blocks blocksclean

all: $(BLOCKS.EXE)
build.blocks: $(OUTDIRS) $(BLOCKS.EXE)
clean: blocksclean

$(BLOCKS.EXE): $(DEP.EXE) $(OBJ.BLOCKS) $(LIB.BLOCKS)
	$(DO.LINK.EXE)

blocksclean:
	-$(RMDIR) $(BLOCKS.EXE) $(OBJ.BLOCKS)

ifdef DO_DEPEND
dep: $(OUTOS)/blocks.dep
$(OUTOS)/blocks.dep: $(SRC.BLOCKS)
	$(DO.DEP)
else
-include $(OUTOS)/blocks.dep
endif

endif # ifeq ($(MAKESECTION),targets)
