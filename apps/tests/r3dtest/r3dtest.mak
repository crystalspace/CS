# Application description
DESCRIPTION.r3dtest = Test of new renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make r3dtest      Make the $(DESCRIPTION.r3dtest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: r3dtest r3dtestclean

all apps: r3dtest
r3dtest:
	$(MAKE_APP)
r3dtestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/r3dtest

R3DTEST.EXE = r3dtest$(EXE)
INC.R3DTEST = $(wildcard apps/tests/r3dtest/*.h)
SRC.R3DTEST = $(wildcard apps/tests/r3dtest/*.cpp)
OBJ.R3DTEST = $(addprefix $(OUT)/,$(notdir $(SRC.R3DTEST:.cpp=$O)))
DEP.R3DTEST = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.R3DTEST = $(foreach d,$(DEP.R3DTEST),$($d.LIB))

#TO_INSTALL.EXE += $(R3DTEST.EXE)

MSVC.DSP += R3DTEST
DSP.R3DTEST.NAME = r3dtest
DSP.R3DTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.r3dtest r3dtestclean

all: $(R3DTEST.EXE)
build.r3dtest: $(OUTDIRS) $(R3DTEST.EXE)
clean: r3dtestclean

$(R3DTEST.EXE): $(DEP.EXE) $(OBJ.R3DTEST) $(LIB.R3DTEST)
	$(DO.LINK.EXE)

r3dtestclean:
	-$(RMDIR) $(R3DTEST.EXE) $(OBJ.R3DTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/r3dtest.dep
$(OUTOS)/r3dtest.dep: $(SRC.R3DTEST)
	$(DO.DEP)
else
-include $(OUTOS)/r3dtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
