DESCRIPTION.freefont = Crystal Space FreeType font server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make freefont     Make the $(DESCRIPTION.freefont)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: freefont freefontclean
plugins all: freefont
freefontclean:
	$(MAKE_CLEAN)
freefont:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SRC.FREEFONT = $(wildcard plugins/font/server/freefont/*.cpp)
OBJ.FREEFONT = $(addprefix $(OUT),$(notdir $(SRC.FREEFONT:.cpp=$O)))
LIB.FREEFONT = $(CSUTIL.LIB) $(CSSYS.LIB)

LIB.EXTERNAL.FREEFONT = -lttf
CFLAGS.FREEFONT = -I/usr/local/include/freetype

ifeq ($(USE_SHARED_PLUGINS),yes)
FREEFONT=$(OUTDLL)freefont$(DLL)
DEP.FREEFONT=$(LIB.FREEFONT)
else
FREEFONT=$(OUT)$(LIB_PREFIX)freefont$(LIB)
DEP.EXE+=$(FREEFONT)
CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_FREEFONT
endif

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: freefont freefontclean
freefont: $(OUTDIRS) $(FREEFONT)

$(OUT)%$O: plugins/font/server/freefont/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.FREEFONT) 

$(FREEFONT): $(OBJ.FREEFONT) $(DEP.FREEFONT)
	$(DO.PLUGIN) $(LIB.EXTERNAL.FREEFONT)

clean: freefontclean
freefontclean:
	-$(RM) $(FREEFONT) $(OBJ.FREEFONT) $(OUTOS)freefont.dep

ifdef DO_DEPEND
dep: $(OUTOS)freefont.dep
$(OUTOS)freefont.dep: $(SRC.FREEFONT)
	$(DO.DEP1) $(CFLAGS.FREEFONT) $(DO.DEP2)
else
-include $(OUTOS)freefont.dep
endif

endif # ifeq ($(MAKESECTION),targets)
