# Application description
DESCRIPTION.nettut = Crystal Space network tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make nettut       Make the $(DESCRIPTION.nettut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: nettut nettutclean

all apps: nettut
nettut:
	$(MAKE_APP)
nettutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


NETTUT.EXE = nettut$(EXE.CONSOLE)
DIR.NETTUT = apps/tutorial/nettut
OUT.NETTUT = $(OUT)/$(DIR.NETTUT)
INC.NETTUT = $(wildcard $(DIR.NETTUT)/*.h )
SRC.NETTUT = $(wildcard $(DIR.NETTUT)/*.cpp )
OBJ.NETTUT = $(addprefix $(OUT.NETTUT)/,$(notdir $(SRC.NETTUT:.cpp=$O)))
DEP.NETTUT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.NETTUT = $(foreach d,$(DEP.NETTUT),$($d.LIB))

#TO_INSTALL.EXE += $(NETTUT.EXE)

MSVC.DSP += NETTUT
DSP.NETTUT.NAME = nettut
DSP.NETTUT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.nettut nettutclean nettutcleandep

all: $(NETTUT.EXE)
build.nettut: $(OUT.NETTUT) $(NETTUT.EXE)
clean: nettutclean

$(OUT.NETTUT)/%$O: $(DIR.NETTUT)/%.cpp
	$(DO.COMPILE.CPP)

$(NETTUT.EXE): $(DEP.EXE) $(OBJ.NETTUT) $(LIB.NETTUT)
	$(DO.LINK.CONSOLE.EXE)

$(OUT.NETTUT):
	$(MKDIRS)

nettutclean:
	-$(RMDIR) $(NETTUT.EXE) $(OBJ.NETTUT)

cleandep: nettutcleandep
nettutcleandep:
	-$(RM) $(OUT.NETTUT)/nettut.dep

ifdef DO_DEPEND
dep: $(OUT.NETTUT) $(OUT.NETTUT)/nettut.dep
$(OUT.NETTUT)/nettut.dep: $(SRC.NETTUT)
	$(DO.DEPEND)
else
-include $(OUT.NETTUT)/nettut.dep
endif

endif # ifeq ($(MAKESECTION),targets)
