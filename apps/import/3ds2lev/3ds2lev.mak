ifeq ($(3DS.AVAILABLE),yes)
# Application description
DESCRIPTION.3ds2lev = 3DS to Crystal Space map converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make 3ds2lev      Make the $(DESCRIPTION.3ds2lev)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: 3ds2lev 3ds2levclean

all apps: 3ds2lev
3ds2lev:
	$(MAKE_APP)
3ds2levclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

3DS2LEV.EXE = 3ds2lev$(EXE.CONSOLE)
DIR.3DS2LEV = apps/import/3ds2lev
OUT.3DS2LEV = $(OUT)/$(DIR.3DS2LEV)
INC.3DS2LEV = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.3DS2LEV)/*.h))
SRC.3DS2LEV = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.3DS2LEV)/*.cpp))
OBJ.3DS2LEV = $(addprefix $(OUT.3DS2LEV)/,$(notdir $(SRC.3DS2LEV:.cpp=$O)))
DEP.3DS2LEV = CSGEOM CSUTIL CSUTIL
LIB.3DS2LEV = $(foreach d,$(DEP.3DS2LEV),$($d.LIB))

OUTDIRS += $(OUT.3DS2LEV)

TO_INSTALL.EXE += $(3DS2LEV.EXE)

MSVC.DSP += 3DS2LEV
DSP.3DS2LEV.NAME = 3ds2lev
DSP.3DS2LEV.TYPE = appcon
DSP.3DS2LEV.LIBS = lib3ds-120

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.3ds2lev 3ds2levclean 3ds2levcleandep

build.3ds2lev: $(OUTDIRS) $(3DS2LEV.EXE)
clean: 3ds2levclean

$(OUT.3DS2LEV)/%$O: $(SRCDIR)/$(DIR.3DS2LEV)/%.cpp
	$(DO.COMPILE.CPP) $(3DS.CFLAGS)

$(3DS2LEV.EXE): $(OBJ.3DS2LEV) $(LIB.3DS2LEV)
	$(DO.LINK.CONSOLE.EXE) $(3DS.LFLAGS)

3ds2levclean:
	-$(RM) 3ds2lev.txt
	-$(RMDIR) $(3DS2LEV.EXE) $(OBJ.3DS2LEV)

cleandep: 3ds2levcleandep
3ds2levcleandep:
	-$(RM) $(OUT.3DS2LEV)/3ds2lev.dep

ifdef DO_DEPEND
dep: $(OUT.3DS2LEV) $(OUT.3DS2LEV)/3ds2lev.dep
$(OUT.3DS2LEV)/3ds2lev.dep: $(SRC.3DS2LEV)
	$(DO.DEPEND1) \
	$(3DS.CFLAGS) \
	$(DO.DEPEND2)
else
-include $(OUT.3DS2LEV)/3ds2lev.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifeq ($(3DS.AVAILABLE),yes)
