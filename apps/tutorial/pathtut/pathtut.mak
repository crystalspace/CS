# Application description
DESCRIPTION.pathtut = Crystal Space Path Tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make pathtut      Make the $(DESCRIPTION.pathtut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pathtut pathtutclean

all apps: pathtut
pathtut:
	$(MAKE_APP)
pathtutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/pathtut

PATHTUT.EXE = pathtut$(EXE)
INC.PATHTUT = $(wildcard apps/tutorial/pathtut/*.h)
SRC.PATHTUT = $(wildcard apps/tutorial/pathtut/*.cpp)
OBJ.PATHTUT = $(addprefix $(OUT)/,$(notdir $(SRC.PATHTUT:.cpp=$O)))
DEP.PATHTUT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.PATHTUT = $(foreach d,$(DEP.PATHTUT),$($d.LIB))

#TO_INSTALL.EXE += $(PATHTUT.EXE)

MSVC.DSP += PATHTUT
DSP.PATHTUT.NAME = pathtut
DSP.PATHTUT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.pathtut pathtutclean

all: $(PATHTUT.EXE)
build.pathtut: $(OUTDIRS) $(PATHTUT.EXE)
clean: pathtutclean

$(PATHTUT.EXE): $(DEP.EXE) $(OBJ.PATHTUT) $(LIB.PATHTUT)
	$(DO.LINK.EXE)

pathtutclean:
	-$(RM) $(PATHTUT.EXE) $(OBJ.PATHTUT)

ifdef DO_DEPEND
dep: $(OUTOS)/pathtut.dep
$(OUTOS)/pathtut.dep: $(SRC.PATHTUT)
	$(DO.DEP)
else
-include $(OUTOS)/pathtut.dep
endif

endif # ifeq ($(MAKESECTION),targets)

