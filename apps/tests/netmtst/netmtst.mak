DESCRIPTION.netmantest = Crystal Space network manager test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make netmantest   Make the $(DESCRIPTION.netmantest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: netmantest netmantestclean

all apps: netmantest

netmantest:
	$(MAKE_APP)
netmantestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tests/netmtst

NETMANTEST.EXE = netmtst$(EXE)
SRC.NETMANTEST = $(wildcard apps/tests/netmtst/*.cpp)
OBJ.NETMANTEST = $(addprefix $(OUT)/,$(notdir $(SRC.NETMANTEST:.cpp=$O)))
DEP.NETMANTEST = NETMAN CSTOOL CSUTIL CSSYS
LIB.NETMANTEST = $(foreach d,$(DEP.NETMANTEST),$($d.LIB))

MSVC.DSP += NETMANTEST
DSP.NETMANTEST.NAME = netmtst
DSP.NETMANTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.netmantest netmantestclean

all: $(NETMANTEST.EXE)
build.netmantest: $(OUTDIRS) $(NETMANTEST.EXE)
clean: netmantestclean

$(NETMANTEST.EXE): $(OBJ.NETMANTEST) $(LIB.NETMANTEST)
	$(DO.LINK.EXE)

netmantestclean:
	-$(RMDIR) $(NETMANTEST.EXE) $(OBJ.NETMANTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/netmtst.dep
$(OUTOS)/netmtst.dep: $(SRC.NETMANTEST)
	$(DO.DEP)
else
-include $(OUTOS)/netmtst.dep
endif

endif # ifeq ($(MAKESECTION),targets)
