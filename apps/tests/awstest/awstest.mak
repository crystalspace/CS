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

vpath %.cpp apps/tests/awstest apps/support

AWSTEST.EXE = awstest$(EXE)
INC.AWSTEST = $(wildcard apps/tests/awstest/*.h)
SRC.AWSTEST = $(wildcard apps/tests/awstest/*.cpp)
OBJ.AWSTEST = $(addprefix $(OUT)/,$(notdir $(SRC.AWSTEST:.cpp=$O)))
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

.PHONY: build.awstest awstestclean

all: $(AWSTEST.EXE)
build.awstest: $(OUTDIRS) $(AWSTEST.EXE)
clean: awstestclean

$(AWSTEST.EXE): $(DEP.EXE) $(OBJ.AWSTEST) $(LIB.AWSTEST)
	$(DO.LINK.EXE)

awstestclean:
	-$(RMDIR) $(AWSTEST.EXE) $(OBJ.AWSTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/awstest.dep
$(OUTOS)/awstest.dep: $(SRC.AWSTEST)
	$(DO.DEP)
else
-include $(OUTOS)/awstest.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring aws,$(PLUGINS) $(PLUGINS.DYNAMIC)))
