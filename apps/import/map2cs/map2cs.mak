# Application description
DESCRIPTION.map2cs = Quake map conversion tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make map2cs       Make the $(DESCRIPTION.map2cs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: map2cs map2csclean

all apps: map2cs
map2cs:
	$(MAKE_APP)
map2csclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MAP2CS.EXE = map2cs$(EXE.CONSOLE)
DIR.MAP2CS = apps/import/map2cs
OUT.MAP2CS = $(OUT)/$(DIR.MAP2CS)
INC.MAP2CS = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MAP2CS)/*.h ))
SRC.MAP2CS = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MAP2CS)/*.cpp ))
OBJ.MAP2CS = $(addprefix $(OUT.MAP2CS)/,$(notdir $(SRC.MAP2CS:.cpp=$O)))
DEP.MAP2CS = CSTOOL CSGFX CSUTIL CSUTIL CSGEOM
LIB.MAP2CS = $(foreach d,$(DEP.MAP2CS),$($d.LIB))
CFG.MAP2CS = $(SRCDIR)/data/config/map2cs.cfg

OUTDIRS += $(OUT.MAP2CS)

TO_INSTALL.EXE    += $(MAP2CS.EXE)
TO_INSTALL.CONFIG += $(CFG.MAP2CS)

MSVC.DSP += MAP2CS
DSP.MAP2CS.NAME = map2cs
DSP.MAP2CS.TYPE = appcon
DSP.MAP2CS.LIBS = zlib

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.map2cs map2csclean map2cscleandep

all: $(MAP2CS.EXE)
build.map2cs: $(OUTDIRS) $(MAP2CS.EXE)
clean: map2csclean

$(OUT.MAP2CS)/%$O: $(SRCDIR)/$(DIR.MAP2CS)/%.cpp
	$(DO.COMPILE.CPP)

$(MAP2CS.EXE): $(OBJ.MAP2CS) $(LIB.MAP2CS)
	$(DO.LINK.CONSOLE.EXE) $(ZLIB.LFLAGS)

map2csclean:
	-$(RM) map2cs.txt
	-$(RMDIR) $(MAP2CS.EXE) $(OBJ.MAP2CS)

cleandep: map2cscleandep
map2cscleandep:
	-$(RM) $(OUT.MAP2CS)/map2cs.dep

ifdef DO_DEPEND
dep: $(OUT.MAP2CS) $(OUT.MAP2CS)/map2cs.dep
$(OUT.MAP2CS)/map2cs.dep: $(SRC.MAP2CS)
	$(DO.DEPEND)
else
-include $(OUT.MAP2CS)/map2cs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
