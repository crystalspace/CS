DESCRIPTION.csfont = Crystal Space default font server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csfont       Make the $(DESCRIPTION.csfont)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csfont csfontclean
plugins all: csfont
csfontclean:
	$(MAKE_CLEAN)
csfont:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SRC.CSFONT = $(wildcard plugins/font/server/csfont/*.cpp)
OBJ.CSFONT = $(addprefix $(OUT),$(notdir $(SRC.CSFONT:.cpp=$O)))
LIB.CSFONT = $(CSUTIL.LIB) $(CSSYS.LIB)

ifeq ($(USE_SHARED_PLUGINS),yes)
CSFONT=$(OUTDLL)csfont$(DLL)
DEP.CSFONT=$(LIB.CSFONT)
else
CSFONT=$(OUT)$(LIB_PREFIX)csfont$(LIB)
DEP.EXE+=$(CSFONT)
CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_CSFONT
endif

vpath %.cpp $(sort $(dir $(SRC.CSFONT)))

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csfont csfontclean
csfont: $(OUTDIRS) $(CSFONT)

$(CSFONT): $(OBJ.CSFONT) $(DEP.CSFONT)
	$(DO.PLUGIN)

clean: csfontclean
csfontclean:
	-$(RM) $(CSFONT) $(OBJ.CSFONT) $(OUTOS)csfont.dep

ifdef DO_DEPEND
dep: $(OUTOS)csfont.dep
$(OUTOS)csfont.dep: $(SRC.CSFONT)
	$(DO.DEP)
else
-include $(OUTOS)csfont.dep
endif

endif # ifeq ($(MAKESECTION),targets)
