
DESCRIPTION.netmantest = Crystal Space Network Manager Test App

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION), rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make netmantest   Make the $(DESCRIPTION.netmantest)$"

endif

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION), roottargets)

.PHONY: netmantest netmantestclean

all apps netdrivers: netmantest

clean: netmantestclean

netmantest: netman
	$(MAKE_TARGET)
netmantestclean:
	$(MAKE_CLEAN)

endif

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION), postdefines)

vpath % apps/tests/netmtst

NETMANTEST = netmtst$(EXE)
LIB.NETMANTEST = $(foreach d,$(DEP.NETMANTEST),$($d.LIB))
TO_INSTALL.EXE += $(NETMANTEST)

SRC.NETMANTEST = $(wildcard apps/tests/netmtst/*.cpp)
OBJ.NETMANTEST = $(addprefix $(OUT)/,$(notdir $(SRC.NETMANTEST:.cpp=$O)))
DEP.NETMANTEST = NETMAN CSTOOL CSUTIL CSSYS

MSVC.DSP += NETMANTEST
DSP.NETMANTEST.NAME = netmtst
DSP.NETMANTEST.TYPE = appcon
DSP.NETMANTEST.DEPEND = NETMAN CSTOOL CSUTIL CSSYS

endif

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION), targets)

.PHONY: netmantest netmantestclean

netmantest: $(NETMANTEST)

$(NETMANTEST): $(OBJ.NETMANTEST) $(LIB.NETMANTEST)
	$(DO.LINK.EXE)

$(OBJ.NETMANTEST): $(SRC.NETMANTEST)
	$(DO.COMPILE.CPP)

netmantestclean:
	-$(RMDIR) $(NETMANTEST) $(OBJ.NETMANTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/netmtst.dep
else
-include $(OUTOS)/netmtst.dep
endif

endif

