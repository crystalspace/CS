# This is a subinclude file used to define the rules needed
# to build the rlsstencil plug-in.

# Driver description
DESCRIPTION.rlsstencil = Crystal Space Stencil Shadow renderstep plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make rlsstencil      Make the $(DESCRIPTION.rlsstencil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rlsstencil rlsstencilclean
all plugins: rlsstencil

rlsstencil:
	$(MAKE_TARGET) MAKE_DLL=yes
rlsstencilclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  RLSSTENCIL = $(OUTDLL)/rlsstencil$(DLL)
  LIB.RLSSTENCIL = $(foreach d,$(DEP.RLSSTENCIL),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RLSSTENCIL)
else
  RLSSTENCIL = $(OUT)/$(LIB_PREFIX)rlsstencil$(LIB)
  DEP.EXE += $(RLSSTENCIL)
  SCF.STATIC += rlsstencil
  TO_INSTALL.STATIC_LIBS += $(RLSSTENCIL)
endif

DIR.RLSSTENCIL = plugins/video/render3d/renderloop
INF.RLSSTENCIL = $(SRCDIR)/$(DIR.RLSSTENCIL)/shadow/rlsstencil/rlsstencil.csplugin
INC.RLSSTENCIL = $(wildcard $(addprefix $(SRCDIR)/, \
   $(DIR.RLSSTENCIL)/shadow/rlsstencil/*.h \
  plugins/video/render3d/renderloop/common/*.h))
SRC.RLSSTENCIL = $(wildcard $(addprefix $(SRCDIR)/, \
   $(DIR.RLSSTENCIL)/shadow/rlsstencil/*.cpp \
  plugins/video/render3d/renderloop/common/*.cpp))
OBJ.RLSSTENCIL = $(addprefix $(OUT)/,$(notdir $(SRC.RLSSTENCIL:.cpp=$O)))
DEP.RLSSTENCIL = CSTOOL CSGEOM CSUTIL CSSYS CSUTIL

TO_INSTALL.CONFIG += $(CFG.RLSSTENCIL)

MSVC.DSP += RLSSTENCIL
DSP.RLSSTENCIL.NAME = rlsstencil
DSP.RLSSTENCIL.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rlsstencil rlsstencilclean

rlsstencil: $(OUTDIRS) $(RLSSTENCIL)

$(RLSSTENCIL): $(OBJ.RLSSTENCIL) $(LIB.RLSSTENCIL)
	$(DO.PLUGIN)

$(OUT)/%$O: $(SRCDIR)/$(DIR.RLSSTENCIL)/shadow/rlsstencil/%.cpp
	$(DO.COMPILE.CPP) $(RLSSTENCIL.CFLAGS)

$(OUT)/%$O: $(SRCDIR)/$(DIR.RLSSTENCIL)/common/%.cpp
	$(DO.COMPILE.CPP) $(RLSSTENCIL.CFLAGS)

clean: rlsstencilclean
rlsstencilclean:
	$(RM) $(RLSSTENCIL) $(OBJ.RLSSTENCIL)

ifdef DO_DEPEND
dep: $(OUTOS)/rlsstencil.dep
$(OUTOS)/rlsstencil.dep: $(SRC.RLSSTENCIL)
	$(DO.DEP)
else
-include $(OUTOS)/rlsstencil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
