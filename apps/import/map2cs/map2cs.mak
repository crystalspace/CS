# Application description
DESCRIPTION.mapconv = Quake map conversion tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make mapconv      Make the $(DESCRIPTION.mapconv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mapconv mapconvclean

all apps: mapconv
mapconv:
	$(MAKE_TARGET)
mapconvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/import/map2cs

MAP2CS.EXE = map2cs$(EXE)
INC.MAP2CS = $(wildcard apps/import/map2cs/*.h)
SRC.MAP2CS = $(wildcard apps/import/map2cs/*.cpp)
OBJ.MAP2CS = $(addprefix $(OUT),$(notdir $(SRC.MAP2CS:.cpp=$O)))
DEP.MAP2CS = CSGFX CSUTIL CSSYS CSUTIL CSGEOM
LIB.MAP2CS = $(foreach d,$(DEP.MAP2CS),$($d.LIB))
CFG.MAP2CS = data/config/map2cs.cfg

TO_INSTALL.EXE    += $(MAP2CS.EXE)
TO_INSTALL.CONFIG += $(CFG.MAP2CS)

MSVC.DSP += MAP2CS
DSP.MAP2CS.NAME = map2cs
DSP.MAP2CS.TYPE = appcon
DSP.MAP2CS.LIBS = zlib

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mapconv mapconvclean

all: $(MAP2CS.EXE)
mapconv: $(OUTDIRS) $(MAP2CS.EXE)
clean: mapconvclean

$(MAP2CS.EXE): $(OBJ.MAP2CS) $(LIB.MAP2CS)
	$(DO.LINK.CONSOLE.EXE)

mapconvclean:
	-$(RM) $(MAP2CS.EXE) $(OBJ.MAP2CS)

ifdef DO_DEPEND
dep: $(OUTOS)map2cs.dep
$(OUTOS)map2cs.dep: $(SRC.MAP2CS)
	$(DO.DEP)
else
-include $(OUTOS)map2cs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
