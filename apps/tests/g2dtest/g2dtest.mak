# Application description
DESCRIPTION.g2dtst = Crystal Space Graphics Canvas plugin test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make g2dtst       Make the $(DESCRIPTION.g2dtst)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: g2dtst g2dtstclean

all apps: g2dtst
g2dtst:
	$(MAKE_TARGET)
g2dtstclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tests/g2dtest

LIB.G2DTEST = $(CSSYS.LIB) $(CSUTIL.LIB)
G2DTEST.EXE = g2dtest$(EXE)
SRC.G2DTEST += apps/tests/g2dtest/g2dtest.cpp
OBJ.G2DTEST = $(addprefix $(OUT),$(notdir $(SRC.G2DTEST:.cpp=$O)))
TO_INSTALL.EXE+=$(G2DTEST.EXE)

DESCRIPTION.$(G2DTEST.EXE) = $(DESCRIPTION.g2dtst)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: g2dtst g2dtstclean

g2dtst: $(OUTDIRS) $(G2DTEST.EXE)
clean: g2dtstclean

$(G2DTEST.EXE): $(OBJ.G2DTEST) $(LIB.G2DTEST)
	$(DO.LINK.EXE)

g2dtstclean:
	-$(RM) $(G2DTEST.EXE) $(OBJ.G2DTEST) $(OUTOS)g2dtest.dep

ifdef DO_DEPEND
dep: $(OUTOS)g2dtest.dep
$(OUTOS)g2dtest.dep: $(SRC.G2DTEST)
	$(DO.DEP)
else
-include $(OUTOS)g2dtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
