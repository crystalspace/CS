DESCRIPTION.rlsstd = Crystal Space standard render loop steps

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make rlsstd       Make the $(DESCRIPTION.rlsstd)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rlsstd rlsstdclean

all plugins: rlsstd

rlsstd:
	$(MAKE_TARGET) MAKE_DLL=yes

rlsstdclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  RLSSTD = $(OUTDLL)/rlsstd$(DLL)
  LIB.RLSSTD = $(foreach d,$(DEP.RLSSTD),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RLSSTD)
else
  RLSSTD = $(OUT)/$(LIBPREFIX)rlsstd$(LIB)
  DEP.EXE += $(RLSSTD)
  SCF.STATIC += rlsstd
  TO_INSTALL.STATIC_LIBS += $(RLSSTD)
endif

DIR.RLSSTD = plugins/video/render3d/renderloop/stdsteps
OUT.RLSSTD = $(OUT)/$(DIR.RLSSTD)
INF.RLSSTD = $(SRCDIR)/$(DIR.RLSSTD)/rlsstd.csplugin
INC.RLSSTD = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/*.h)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/parserenderstep.h)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/basesteptype.h)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/basestepfactory.h)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/basesteploader.h)) 
SRC.RLSSTD = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/*.cpp)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/parserenderstep.cpp)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/basesteptype.cpp)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/basestepfactory.cpp)) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.RLSSTD)/../common/basesteploader.cpp)) 
OBJ.RLSSTD = $(addprefix $(OUT.RLSSTD)/,$(notdir $(SRC.RLSSTD:.cpp=$O)))
DEP.RLSSTD = CSSYS CSUTIL CSGEOM

OUTDIRS += $(OUT.RLSSTD)

MSVC.DSP += RLSSTD
DSP.RLSSTD.NAME = rlsstd
DSP.RLSSTD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rlsstd rlsstdclean rlsstdcleandep

rlsstd: $(OUTDIRS) $(RLSSTD)
$(RLSSTD): $(OBJ.RLSSTD) $(LIB.RLSSTD)
	$(DO.PLUGIN)

$(OUT.RLSSTD)/%$O: $(SRCDIR)/$(DIR.RLSSTD)/%.cpp
	$(DO.COMPILE.CPP) $(RLSSTD.CFLAGS)

$(OUT.RLSSTD)/%$O: $(SRCDIR)/$(DIR.RLSSTD)/../common/%.cpp
	$(DO.COMPILE.CPP) $(RLSSTD.CFLAGS)

clean: rlsstdclean
rlsstdclean:
	-$(RMDIR) $(RLSSTD) $(OBJ.RLSSTD) $(OUTDLL)/$(notdir $(INF.RLSSTD))

cleandep: rlsstdcleandep
rlsstdcleandep:
	-$(RM) $(OUT.RLSSTD)/rlsstd.dep

ifdef DO_DEPEND
dep: $(OUT.RLSSTD) $(OUT.RLSSTD)/rlsstd.dep
$(OUT.RLSSTD)/rlsstd.dep: $(SRC.RLSSTD)
	$(DO.DEPEND)
else
-include $(OUT.RLSSTD)/rlsstd.dep
endif

endif # ifeq ($(MAKESECTION),targets)
