# Application description
DESCRIPTION.phystut = Crystal Space physics tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make phystut      Make the $(DESCRIPTION.phystut)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: phystut phystutclean

all apps: phystut
phystut:
	$(MAKE_APP)
phystutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tutorial/phystut

PHYSTUT.EXE = phystut$(EXE)
INC.PHYSTUT = $(wildcard apps/tutorial/phystut/*.h)
SRC.PHYSTUT = $(wildcard apps/tutorial/phystut/*.cpp)
OBJ.PHYSTUT = $(addprefix $(OUT)/,$(notdir $(SRC.PHYSTUT:.cpp=$O)))
DEP.PHYSTUT = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.PHYSTUT = $(foreach d,$(DEP.PHYSTUT),$($d.LIB))

TO_INSTALL.EXE += $(PHYSTUT.EXE)

MSVC.DSP += PHYSTUT
DSP.PHYSTUT.NAME = phystut
DSP.PHYSTUT.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.phystut phystutclean

all: $(PHYSTUT.EXE)
build.phystut: $(OUTDIRS) $(PHYSTUT.EXE)
clean: phystutclean

$(PHYSTUT.EXE): $(DEP.EXE) $(OBJ.PHYSTUT) $(LIB.PHYSTUT)
	$(DO.LINK.EXE)

phystutclean:
	-$(RMDIR) $(PHYSTUT.EXE) $(OBJ.PHYSTUT)

ifdef DO_DEPEND
dep: $(OUTOS)/phys.dep
$(OUTOS)/phys.dep: $(SRC.PHYSTUT)
	$(DO.DEP)
else
-include $(OUTOS)/phys.dep
endif

endif # ifeq ($(MAKESECTION),targets)
