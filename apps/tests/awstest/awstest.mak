# Application target only valid if module is listed in PLUGINS.
ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))

# Application description
DESCRIPTION.awstest = Alternate Windowing System test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make awstest      Make the $(DESCRIPTION.awstest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: awstest awstestclean

all apps: awstest
awstest:
	$(MAKE_APP)
awstestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


AWSTEST.EXE = awstest$(EXE)
DIR.AWSTEST = apps/tests/awstest
OUT.AWSTEST = $(OUT)/$(DIR.AWSTEST)
INC.AWSTEST = $(wildcard $(DIR.AWSTEST)/*.h )
SRC.AWSTEST = $(wildcard $(DIR.AWSTEST)/*.cpp )
OBJ.AWSTEST = $(addprefix $(OUT.AWSTEST)/,$(notdir $(SRC.AWSTEST:.cpp=$O)))
DEP.AWSTEST = CSTOOL CSUTIL CSSYS CSUTIL CSGEOM CSGFX
LIB.AWSTEST = $(foreach d,$(DEP.AWSTEST),$($d.LIB))
CFG.AWSTEST = data/config/awstest.cfg

#TO_INSTALL.EXE    += $(CSWSTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSTEST)

MSVC.DSP += AWSTEST
DSP.AWSTEST.NAME = awstest
DSP.AWSTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.awstest awstestclean awstestcleandep

all: $(AWSTEST.EXE)
build.awstest: $(OUT.AWSTEST) $(AWSTEST.EXE)
clean: awstestclean

$(OUT.AWSTEST)/%$O: $(DIR.AWSTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(AWSTEST.EXE): $(DEP.EXE) $(OBJ.AWSTEST) $(LIB.AWSTEST)
	$(DO.LINK.EXE)

$(OUT.AWSTEST):
	$(MKDIRS)

awstestclean:
	-$(RM) awstest.txt
	-$(RMDIR) $(AWSTEST.EXE) $(OBJ.AWSTEST)

cleandep: awstestcleandep
awstestcleandep:
	-$(RM) $(OUT.AWSTEST)/awstest.dep

ifdef DO_DEPEND
dep: $(OUT.AWSTEST) $(OUT.AWSTEST)/awstest.dep
$(OUT.AWSTEST)/awstest.dep: $(SRC.AWSTEST)
	$(DO.DEPEND)
else
-include $(OUT.AWSTEST)/awstest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))
