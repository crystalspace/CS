# Application description
DESCRIPTION.pathtut = Crystal Space Path Tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make pathtut      Make the $(DESCRIPTION.pathtut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pathtut pathtutclean

all apps: pathtut
pathtut:
	$(MAKE_APP)
pathtutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


PATHTUT.EXE = pathtut$(EXE)
DIR.PATHTUT = apps/tutorial/pathtut
OUT.PATHTUT = $(OUT)/$(DIR.PATHTUT)
INC.PATHTUT = $(wildcard $(DIR.PATHTUT)/*.h )
SRC.PATHTUT = $(wildcard $(DIR.PATHTUT)/*.cpp )
OBJ.PATHTUT = $(addprefix $(OUT.PATHTUT)/,$(notdir $(SRC.PATHTUT:.cpp=$O)))
DEP.PATHTUT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.PATHTUT = $(foreach d,$(DEP.PATHTUT),$($d.LIB))

#TO_INSTALL.EXE += $(PATHTUT.EXE)

MSVC.DSP += PATHTUT
DSP.PATHTUT.NAME = pathtut
DSP.PATHTUT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.pathtut pathtutclean pathtutcleandep

all: $(PATHTUT.EXE)
build.pathtut: $(OUT.PATHTUT) $(PATHTUT.EXE)
clean: pathtutclean

$(OUT.PATHTUT)/%$O: $(DIR.PATHTUT)/%.cpp
	$(DO.COMPILE.CPP)

$(PATHTUT.EXE): $(DEP.EXE) $(OBJ.PATHTUT) $(LIB.PATHTUT)
	$(DO.LINK.EXE)

$(OUT.PATHTUT):
	$(MKDIRS)

pathtutclean:
	-$(RMDIR) $(PATHTUT.EXE) $(OBJ.PATHTUT)

cleandep: pathtutcleandep
pathtutcleandep:
	-$(RM) $(OUT.PATHTUT)/pathtut.dep

ifdef DO_DEPEND
dep: $(OUT.PATHTUT) $(OUT.PATHTUT)/pathtut.dep
$(OUT.PATHTUT)/pathtut.dep: $(SRC.PATHTUT)
	$(DO.DEPEND)
else
-include $(OUT.PATHTUT)/pathtut.dep
endif

endif # ifeq ($(MAKESECTION),targets)

