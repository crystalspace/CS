# Application description
DESCRIPTION.cal3dtocs = Cal3D to Sprite3D converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make cal3dtocs    Make the $(DESCRIPTION.cal3dtocs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cal3dtocs cal3dtocsclean

all apps: cal3dtocs
cal3dtocs:
	$(MAKE_TARGET)
cal3dtocsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/import/caltocs

CALTOCS.EXE = caltocs$(EXE)
SRC.CALTOCS = $(wildcard apps/import/caltocs/*.cpp)
OBJ.CALTOCS = $(addprefix $(OUT),$(notdir $(SRC.CALTOCS:.cpp=$O)))

TO_INSTALL.EXE    += $(CALTOCS.EXE)

MSVC.DSP += CALTOCS
DSP.CALTOCS.NAME = caltocs
DSP.CALTOCS.TYPE = appcon
DSP.CALTOCS.LIBS = cal3d

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cal3dtocs cal3dtocsclean

all: $(CALTOCS.EXE)
cal3dtocs: $(OUTDIRS) $(CALTOCS.EXE)
clean: cal3dtocsclean

$(CALTOCS.EXE): $(OBJ.CALTOCS)
	$(DO.LINK.CONSOLE.EXE) -lcal3d

cal3dtocsclean:
	-$(RM) $(CALTOCS.EXE) $(OBJ.CALTOCS)

ifdef DO_DEPEND
dep: $(OUTOS)caltocs.dep
$(OUTOS)caltocs.dep: $(SRC.CALTOCS)
	$(DO.DEP)
else
-include $(OUTOS)caltocs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
