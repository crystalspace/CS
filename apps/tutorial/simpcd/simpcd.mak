# Application description
DESCRIPTION.simpcd = Crystal Space CD tutorial

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simpcd       Make the $(DESCRIPTION.simpcd)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simpcd simpcdclean

all apps: simpcd
simpcd:
	$(MAKE_APP)
simpcdclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


SIMPCD.EXE = simpcd$(EXE)
DIR.SIMPCD = apps/tutorial/simpcd
OUT.SIMPCD = $(OUT)/$(DIR.SIMPCD)
INC.SIMPCD = $(wildcard $(DIR.SIMPCD)/*.h )
SRC.SIMPCD = $(wildcard $(DIR.SIMPCD)/*.cpp )
OBJ.SIMPCD = $(addprefix $(OUT.SIMPCD)/,$(notdir $(SRC.SIMPCD:.cpp=$O)))
DEP.SIMPCD = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.SIMPCD = $(foreach d,$(DEP.SIMPCD),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPCD.EXE)

MSVC.DSP += SIMPCD
DSP.SIMPCD.NAME = simpcd
DSP.SIMPCD.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simpcd simpcdclean simpcdcleandep

all: $(SIMPCD.EXE)
build.simpcd: $(OUT.SIMPCD) $(SIMPCD.EXE)
clean: simpcdclean

$(OUT.SIMPCD)/%$O: $(DIR.SIMPCD)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPCD.EXE): $(DEP.EXE) $(OBJ.SIMPCD) $(LIB.SIMPCD)
	$(DO.LINK.EXE)

$(OUT.SIMPCD):
	$(MKDIRS)

simpcdclean:
	-$(RM) simpcd.txt
	-$(RMDIR) $(SIMPCD.EXE) $(OBJ.SIMPCD)

cleandep: simpcdcleandep
simpcdcleandep:
	-$(RM) $(OUT.SIMPCD)/simpcd.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPCD) $(OUT.SIMPCD)/simpcd.dep
$(OUT.SIMPCD)/simpcd.dep: $(SRC.SIMPCD)
	$(DO.DEPEND)
else
-include $(OUT.SIMPCD)/simpcd.dep
endif

endif # ifeq ($(MAKESECTION),targets)
