# Application description
DESCRIPTION.demosky = Crystal Space sky demo

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make demosky      Make the $(DESCRIPTION.demosky)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: demosky demoskyclean

all apps: demosky
demosky:
	$(MAKE_APP)
demoskyclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/demosky apps/support

DEMOSKY.EXE=demosky$(EXE)
INC.DEMOSKY = $(wildcard apps/demosky/*.h)
SRC.DEMOSKY = $(wildcard apps/demosky/*.cpp)
OBJ.DEMOSKY = $(addprefix $(OUT)/,$(notdir $(SRC.DEMOSKY:.cpp=$O)))
DEP.DEMOSKY = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.DEMOSKY = $(foreach d,$(DEP.DEMOSKY),$($d.LIB))

#TO_INSTALL.EXE += $(DEMOSKY.EXE)

MSVC.DSP += DEMOSKY
DSP.DEMOSKY.NAME = demosky
DSP.DEMOSKY.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.demosky demoskyclean

all: $(DEMOSKY.EXE)
build.demosky: $(OUTDIRS) $(DEMOSKY.EXE)
clean: demoskyclean

$(DEMOSKY.EXE): $(DEP.EXE) $(OBJ.DEMOSKY) $(LIB.DEMOSKY)
	$(DO.LINK.EXE)

demoskyclean:
	-$(RM) $(DEMOSKY.EXE) $(OBJ.DEMOSKY)

ifdef DO_DEPEND
dep: $(OUTOS)/demosky.dep
$(OUTOS)/demosky.dep: $(SRC.DEMOSKY)
	$(DO.DEP)
else
-include $(OUTOS)/demosky.dep
endif

endif # ifeq ($(MAKESECTION),targets)
