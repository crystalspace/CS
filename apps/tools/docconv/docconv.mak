# Application description
DESCRIPTION.docconv = Crystal Space Document Converter

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make docconv      Make the $(DESCRIPTION.docconv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: docconv docconvclean

all apps: docconv
docconv:
	$(MAKE_APP)
docconvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

DOCCONV.EXE = docconv$(EXE.CONSOLE)
DIR.DOCCONV = apps/tools/docconv
OUT.DOCCONV = $(OUT)/$(DIR.DOCCONV)
INC.DOCCONV = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.DOCCONV)/*.h ))
SRC.DOCCONV = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.DOCCONV)/*.cpp ))
OBJ.DOCCONV = $(addprefix $(OUT.DOCCONV)/,$(notdir $(SRC.DOCCONV:.cpp=$O)))
DEP.DOCCONV = CSTOOL CSUTIL
LIB.DOCCONV = $(foreach d,$(DEP.DOCCONV),$($d.LIB))

OUTDIRS += $(OUT.DOCCONV)

TO_INSTALL.EXE    += $(DOCCONV.EXE)

MSVC.DSP += DOCCONV
DSP.DOCCONV.NAME = docconv
DSP.DOCCONV.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.docconv docconvclean docconvcleandep

all: $(DOCCONV.EXE)
build.docconv: $(OUTDIRS) $(DOCCONV.EXE)
clean: docconvclean

$(OUT.DOCCONV)/%$O: $(SRCDIR)/$(DIR.DOCCONV)/%.cpp
	$(DO.COMPILE.CPP)

$(DOCCONV.EXE): $(DEP.EXE) $(OBJ.DOCCONV) $(LIB.DOCCONV)
	$(DO.LINK.CONSOLE.EXE)

docconvclean:
	-$(RM) docconv.txt
	-$(RMDIR) $(DOCCONV.EXE) $(OBJ.DOCCONV)

cleandep: docconvcleandep
docconvcleandep:
	-$(RM) $(OUT.DOCCONV)/docconv.dep

ifdef DO_DEPEND
dep: $(OUT.DOCCONV) $(OUT.DOCCONV)/docconv.dep
$(OUT.DOCCONV)/docconv.dep: $(SRC.DOCCONV)
	$(DO.DEPEND)
else
-include $(OUT.DOCCONV)/docconv.dep
endif

endif # ifeq ($(MAKESECTION),targets)
