# Application description
DESCRIPTION.phystut = Crystal Space physics tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make phystut      Make the $(DESCRIPTION.phystut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: phystut phystutclean

all apps: phystut
phystut:
	$(MAKE_APP)
phystutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


PHYSTUT.EXE = phystut$(EXE)
DIR.PHYSTUT = apps/tutorial/phystut
OUT.PHYSTUT = $(OUT)/$(DIR.PHYSTUT)
INC.PHYSTUT = $(wildcard $(DIR.PHYSTUT)/*.h )
SRC.PHYSTUT = $(wildcard $(DIR.PHYSTUT)/*.cpp )
OBJ.PHYSTUT = $(addprefix $(OUT.PHYSTUT)/,$(notdir $(SRC.PHYSTUT:.cpp=$O)))
DEP.PHYSTUT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.PHYSTUT = $(foreach d,$(DEP.PHYSTUT),$($d.LIB))

TO_INSTALL.EXE += $(PHYSTUT.EXE)

MSVC.DSP += PHYSTUT
DSP.PHYSTUT.NAME = phystut
DSP.PHYSTUT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.phystut phystutclean phystutcleandep

all: $(PHYSTUT.EXE)
build.phystut: $(OUT.PHYSTUT) $(PHYSTUT.EXE)
clean: phystutclean

$(OUT.PHYSTUT)/%$O: $(DIR.PHYSTUT)/%.cpp
	$(DO.COMPILE.CPP)

$(PHYSTUT.EXE): $(DEP.EXE) $(OBJ.PHYSTUT) $(LIB.PHYSTUT)
	$(DO.LINK.EXE)

$(OUT.PHYSTUT):
	$(MKDIRS)

phystutclean:
	-$(RM) phystut.txt
	-$(RMDIR) $(PHYSTUT.EXE) $(OBJ.PHYSTUT)

cleandep: phystutcleandep
phystutcleandep:
	-$(RM) $(OUT.PHYSTUT)/phystut.dep

ifdef DO_DEPEND
dep: $(OUT.PHYSTUT) $(OUT.PHYSTUT)/phystut.dep
$(OUT.PHYSTUT)/phystut.dep: $(SRC.PHYSTUT)
	$(DO.DEPEND)
else
-include $(OUT.PHYSTUT)/phystut.dep
endif

endif # ifeq ($(MAKESECTION),targets)
