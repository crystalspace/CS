# Application description
DESCRIPTION.3dslev = 3DS to Crystal Space map convertor

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make 3dslev       Make the $(DESCRIPTION.3dslev)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: 3dslev 3dslevclean

3dslev:
	$(MAKE_TARGET)
3dslevclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/import/3ds2lev

3DS2LEV.EXE = 3ds2lev$(EXE)
INC.3DS2LEV = $(wildcard apps/import/3ds2lev/*.h)
SRC.3DS2LEV = $(wildcard apps/import/3ds2lev/*.cpp)
OBJ.3DS2LEV = $(addprefix $(OUT),$(notdir $(SRC.3DS2LEV:.cpp=$O)))
DEP.3DS2LEV = CSUTIL CSSYS CSUTIL
LIB.3DS2LEV = $(foreach d,$(DEP.3DS2LEV),$($d.LIB))

#TO_INSTALL.EXE += $(3DS2LEV.EXE)

MSVC.DSP += 3DS2LEV
DSP.3DS2LEV.NAME = 3ds2lev
DSP.3DS2LEV.TYPE = appcon
DSP.3DS2LEV.LIBS = lib3ds-101

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: 3dslev 3dslevclean

3dslev: $(OUTDIRS) $(3DS2LEV.EXE)
clean: 3dslevclean

$(3DS2LEV.EXE): $(OBJ.3DS2LEV) $(LIB.3DS2LEV)
	$(DO.LINK.CONSOLE.EXE) -l3ds

3dslevclean:
	-$(RM) $(3DS2LEV.EXE) $(OBJ.3DS2LEV)

ifdef DO_DEPEND
3ds2levdep: $(OUTOS)3ds2lev.dep
#dep: $(OUTOS)3ds2lev.dep
$(OUTOS)3ds2lev.dep: $(SRC.3DS2LEV)
	$(DO.DEP)
else
-include $(OUTOS)3ds2lev.dep
endif

endif # ifeq ($(MAKESECTION),targets)
