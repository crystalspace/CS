# Application description
DESCRIPTION.tbtut = Crystal Space Terrbig Tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make tbtut        Make the $(DESCRIPTION.tbtut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tbtut tbtutclean

all apps: tbtut
tbtut:
	$(MAKE_APP)
tbtutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/tbtut

TBTUT.EXE = tbtut$(EXE)
INC.TBTUT = $(wildcard apps/tutorial/tbtut/*.h)
SRC.TBTUT = $(wildcard apps/tutorial/tbtut/*.cpp)
OBJ.TBTUT = $(addprefix $(OUT)/,$(notdir $(SRC.TBTUT:.cpp=$O)))
DEP.TBTUT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.TBTUT = $(foreach d,$(DEP.TBTUT),$($d.LIB))

#TO_INSTALL.EXE += $(TBTUT.EXE)

MSVC.DSP += TBTUT
DSP.TBTUT.NAME = tbtut
DSP.TBTUT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.tbtut tbtutclean

all: $(TBTUT.EXE)
build.tbtut: $(OUTDIRS) $(TBTUT.EXE)
clean: tbtutclean

$(TBTUT.EXE): $(DEP.EXE) $(OBJ.TBTUT) $(LIB.TBTUT)
	$(DO.LINK.EXE)

tbtutclean:
	-$(RMDIR) $(TBTUT.EXE) $(OBJ.TBTUT)

ifdef DO_DEPEND
dep: $(OUTOS)/tbtut.dep
$(OUTOS)/tbtut.dep: $(SRC.TBTUT)
	$(DO.DEP)
else
-include $(OUTOS)/tbtut.dep
endif

endif # ifeq ($(MAKESECTION),targets)
