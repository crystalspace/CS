# Application description
DESCRIPTION.tbtut = Crystal Space Terrbig Tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tbtut        Make the $(DESCRIPTION.tbtut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tbtut tbtutclean

all apps: tbtut
tbtut:
	$(MAKE_APP)
tbtutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

TBTUT.EXE = tbtut$(EXE)
DIR.TBTUT = apps/tutorial/tbtut
OUT.TBTUT = $(OUT)/$(DIR.TBTUT)
INC.TBTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.TBTUT)/*.h ))
SRC.TBTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.TBTUT)/*.cpp ))
OBJ.TBTUT = $(addprefix $(OUT.TBTUT)/,$(notdir $(SRC.TBTUT:.cpp=$O)))
DEP.TBTUT = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.TBTUT = $(foreach d,$(DEP.TBTUT),$($d.LIB))

OUTDIRS += $(OUT.TBTUT)

#TO_INSTALL.EXE += $(TBTUT.EXE)

MSVC.DSP += TBTUT
DSP.TBTUT.NAME = tbtut
DSP.TBTUT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.tbtut tbtutclean tbtutcleandep

build.tbtut: $(OUTDIRS) $(TBTUT.EXE)
clean: tbtutclean

$(OUT.TBTUT)/%$O: $(SRCDIR)/$(DIR.TBTUT)/%.cpp
	$(DO.COMPILE.CPP)

$(TBTUT.EXE): $(DEP.EXE) $(OBJ.TBTUT) $(LIB.TBTUT)
	$(DO.LINK.EXE)

tbtutclean:
	-$(RM) tbtut.txt
	-$(RMDIR) $(TBTUT.EXE) $(OBJ.TBTUT)

cleandep: tbtutcleandep
tbtutcleandep:
	-$(RM) $(OUT.TBTUT)/tbtut.dep

ifdef DO_DEPEND
dep: $(OUT.TBTUT) $(OUT.TBTUT)/tbtut.dep
$(OUT.TBTUT)/tbtut.dep: $(SRC.TBTUT)
	$(DO.DEPEND)
else
-include $(OUT.TBTUT)/tbtut.dep
endif

endif # ifeq ($(MAKESECTION),targets)
