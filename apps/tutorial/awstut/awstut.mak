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
	$(MAKE_TARGET)
awstutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/awstut apps/support

AWSTUT.EXE = awstutor$(EXE)
INC.AWSTUT = $(wildcard apps/tutorial/awstut/*.h)
SRC.AWSTUT = $(wildcard apps/tutorial/awstut/*.cpp)
OBJ.AWSTUT = $(addprefix $(OUT),$(notdir $(SRC.AWSTUT:.cpp=$O)))
DEP.AWSTUT = CSTOOL CSUTIL CSSYS CSUTIL CSGEOM CSGFX
LIB.AWSTUT = $(foreach d,$(DEP.AWSTUT),$($d.LIB))
CFG.AWSTUT = data/config/awstut.cfg

#TO_INSTALL.EXE    += $(CSWSTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSTEST)

MSVC.DSP += AWSTUT
DSP.AWSTUT.NAME = awstut
DSP.AWSTUT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: awstut awstutclean

all: $(AWSTUT.EXE)
awstut: $(OUTDIRS) $(AWSTUT.EXE)
clean: awstutclean

$(AWSTUT.EXE): $(DEP.EXE) $(OBJ.AWSTUT) $(LIB.AWSTUT)
	$(DO.LINK.EXE)

awstutclean:
	-$(RM) $(AWSTUT.EXE) $(OBJ.AWSTUT)

ifdef DO_DEPEND
dep: $(OUTOS)awstut.dep
$(OUTOS)awstut.dep: $(SRC.AWSTUT)
	$(DO.DEP)
else
-include $(OUTOS)awstut.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))
