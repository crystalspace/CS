# Application description
DESCRIPTION.awstest = Alternate Windowing System test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP+=$(NEWLINE)echo $"  make awstest       Make the $(DESCRIPTION.awstest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: awstest awstestclean

all apps: awstest
awstest:
	$(MAKE_TARGET)
awstestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/awstest apps/support

AWSTEST.EXE = awstest$(EXE)
INC.AWSTEST = $(wildcard apps/awstest/*.h)
SRC.AWSTEST = $(wildcard apps/awstest/*.cpp)
OBJ.AWSTEST = $(addprefix $(OUT),$(notdir $(SRC.AWSTEST:.cpp=$O)))
DEP.AWSTEST = CSTOOL CSUTIL CSSYS CSUTIL CSGEOM CSGFX 
LIB.AWSTEST = $(foreach d,$(DEP.AWSTEST),$($d.LIB))
CFG.AWSTEST = data/config/awstest.cfg

#TO_INSTALL.EXE    += $(CSWSTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.CSWSTEST)

MSVC.DSP += AWSTEST
DSP.AWSTEST.NAME = awstest
DSP.AWSTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: awstest awstestclean

all: $(AWSTEST.EXE)
awstest: $(OUTDIRS) $(AWSTEST.EXE)
clean: awstestclean

$(AWSTEST.EXE): $(DEP.EXE) $(OBJ.AWSTEST) $(LIB.AWSTEST)
	$(DO.LINK.EXE)

awstestclean:
	-$(RM) $(AWSTEST.EXE) $(OBJ.AWSTEST)

ifdef DO_DEPEND
dep: $(OUTOS)awstest.dep
$(OUTOS)awstest.dep: $(SRC.AWSTEST)
	$(DO.DEP)
else
-include $(OUTOS)awstest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
