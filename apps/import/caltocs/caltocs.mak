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

vpath %.cpp apps/import/caltocs

CALTOCS.EXE = caltocs$(EXE)
SRC.CALTOCS = $(wildcard apps/import/caltocs/*.cpp)
OBJ.CALTOCS = $(addprefix $(OUT)/,$(notdir $(SRC.CALTOCS:.cpp=$O)))

TO_INSTALL.EXE    += $(CALTOCS.EXE)

MSVC.DSP += CALTOCS
DSP.CALTOCS.NAME = caltocs
DSP.CALTOCS.TYPE = appcon
DSP.CALTOCS.LIBS = cal3d

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.caltocs caltocsclean

all: $(CALTOCS.EXE)
build.caltocs: $(OUTDIRS) $(CALTOCS.EXE)
clean: caltocsclean

$(CALTOCS.EXE): $(OBJ.CALTOCS)
	$(DO.LINK.CONSOLE.EXE) -lcal3d -lstdc++

caltocsclean:
	-$(RM) $(CALTOCS.EXE) $(OBJ.CALTOCS)

ifdef DO_DEPEND
dep: $(OUTOS)/caltocs.dep
$(OUTOS)/caltocs.dep: $(SRC.CALTOCS)
	$(DO.DEP)
else
-include $(OUTOS)/caltocs.dep
endif

endif # ifeq ($(MAKESECTION),targets)
endif # ifeq ($(HAS_CAL3D),yes)