ifeq ($(HAS_CAL3D),yes)
# Application description
DESCRIPTION.caltocs = Cal3D to Sprite3D converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make caltocs      Make the $(DESCRIPTION.caltocs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: caltocs caltocsclean

all apps: caltocs
caltocs:
	$(MAKE_APP)
caltocsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


CALTOCS.EXE = caltocs$(EXE.CONSOLE)
DIR.CALTOCS = apps/import/caltocs
OUT.CALTOCS = $(OUT)/$(DIR.CALTOCS)
SRC.CALTOCS = $(wildcard $(DIR.CALTOCS)/*.cpp )
OBJ.CALTOCS = $(addprefix $(OUT.CALTOCS)/,$(notdir $(SRC.CALTOCS:.cpp=$O)))
DEP.CALTOCS = CSSYS CSUTIL

TO_INSTALL.EXE    += $(CALTOCS.EXE)

MSVC.DSP += CALTOCS
DSP.CALTOCS.NAME = caltocs
DSP.CALTOCS.TYPE = appcon
DSP.CALTOCS.LIBS = cal3d

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.caltocs caltocsclean caltocscleandep

all: $(CALTOCS.EXE)
build.caltocs: $(OUT.CALTOCS) $(CALTOCS.EXE)
clean: caltocsclean

$(OUT.CALTOCS)/%$O: $(DIR.CALTOCS)/%.cpp
	$(DO.COMPILE.CPP)

$(CALTOCS.EXE): $(OBJ.CALTOCS)
	$(DO.LINK.CONSOLE.EXE) -lcal3d -lstdc++

$(OUT.CALTOCS):
	$(MKDIRS)

caltocsclean:
	-$(RM) caltocs.txt
	-$(RMDIR) $(CALTOCS.EXE) $(OBJ.CALTOCS)

cleandep: caltocscleandep
caltocscleandep:
	-$(RM) $(OUT.CALTOCS)/caltocs.dep

ifdef DO_DEPEND
dep: $(OUT.CALTOCS) $(OUT.CALTOCS)/caltocs.dep
$(OUT.CALTOCS)/caltocs.dep: $(SRC.CALTOCS)
	$(DO.DEPEND)
else
-include $(OUT.CALTOCS)/caltocs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifeq ($(HAS_CAL3D),yes)
