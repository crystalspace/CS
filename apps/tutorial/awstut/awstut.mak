# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.awstut = Alternate Windowing System Tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make awstut       Make the $(DESCRIPTION.awstut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: awstut awstutclean

all apps: awstut
awstut:
	$(MAKE_APP)
awstutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

AWSTUT.EXE = awstut$(EXE)
DIR.AWSTUT = apps/tutorial/awstut
OUT.AWSTUT = $(OUT)/$(DIR.AWSTUT)
INC.AWSTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.AWSTUT)/*.h ))
SRC.AWSTUT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.AWSTUT)/*.cpp ))
OBJ.AWSTUT = $(addprefix $(OUT.AWSTUT)/,$(notdir $(SRC.AWSTUT:.cpp=$O)))
DEP.AWSTUT = CSTOOL CSUTIL CSUTIL CSGEOM CSGFX
LIB.AWSTUT = $(foreach d,$(DEP.AWSTUT),$($d.LIB))
CFG.AWSTUT = 

OUTDIRS += $(OUT.AWSTUT)

#TO_INSTALL.EXE    += $(CSWSTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSTEST)

MSVC.DSP += AWSTUT
DSP.AWSTUT.NAME = awstut
DSP.AWSTUT.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.awstut awstutclean awstutcleandep

build.awstut: $(OUTDIRS) $(AWSTUT.EXE)
clean: awstutclean

$(OUT.AWSTUT)/%$O: $(SRCDIR)/$(DIR.AWSTUT)/%.cpp
	$(DO.COMPILE.CPP)

$(AWSTUT.EXE): $(DEP.EXE) $(OBJ.AWSTUT) $(LIB.AWSTUT)
	$(DO.LINK.EXE)

awstutclean:
	-$(RM) awstut.txt
	-$(RMDIR) $(AWSTUT.EXE) $(OBJ.AWSTUT)

cleandep: awstutcleandep
awstutcleandep:
	-$(RM) $(OUT.AWSTUT)/awstut.dep

ifdef DO_DEPEND
dep: $(OUT.AWSTUT) $(OUT.AWSTUT)/awstut.dep
$(OUT.AWSTUT)/awstut.dep: $(SRC.AWSTUT)
	$(DO.DEPEND)
else
-include $(OUT.AWSTUT)/awstut.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))
