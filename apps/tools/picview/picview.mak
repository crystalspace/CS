# Application description
DESCRIPTION.picview = Crystal Space Picture Viewer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make picview      Make the $(DESCRIPTION.picview)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: picview picviewclean

all apps: picview
picview:
	$(MAKE_APP)
picviewclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/picview

PICVIEW.EXE = picview$(EXE)
INC.PICVIEW = $(wildcard apps/tools/picview/*.h)
SRC.PICVIEW = $(wildcard apps/tools/picview/*.cpp)
OBJ.PICVIEW = $(addprefix $(OUT)/,$(notdir $(SRC.PICVIEW:.cpp=$O)))
DEP.PICVIEW = \
  CSWS CSTOOL CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.PICVIEW = $(foreach d,$(DEP.PICVIEW),$($d.LIB))

#TO_INSTALL.EXE    += $(picview.EXE)
#TO_INSTALL.CONFIG += $(CFG.PICVIEW)

MSVC.DSP += PICVIEW
DSP.PICVIEW.NAME = picview
DSP.PICVIEW.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.picview picviewclean

all: $(PICVIEW.EXE)
build.picview: $(OUTDIRS) $(PICVIEW.EXE)
clean: picviewclean

$(PICVIEW.EXE): $(DEP.EXE) $(OBJ.PICVIEW) $(LIB.PICVIEW)
	$(DO.LINK.EXE)

picviewclean:
	-$(RMDIR) $(PICVIEW.EXE) $(OBJ.PICVIEW)

ifdef DO_DEPEND
dep: $(OUTOS)/picview.dep
$(OUTOS)/picview.dep: $(SRC.PICVIEW)
	$(DO.DEP)
else
-include $(OUTOS)/picview.dep
endif

endif # ifeq ($(MAKESECTION),targets)
