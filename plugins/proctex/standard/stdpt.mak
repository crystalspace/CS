DESCRIPTION.stdpt = standard procedural textures

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),rootdefines)

# Plugin
PLUGINHELP += \
  $(NEWLINE)@echo $"  make stdpt        Make the $(DESCRIPTION.foobar)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stdpt stdptclean

plugins: stdpt

# These call the target of the same name in the targets section.

# Use this for a plugin target.  (Delete the app target above.)
stdpt:
	$(MAKE_TARGET) MAKE_DLL=yes

# Use this to clean the build detritus from your module.
stdptclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  STDPT = $(OUTDLL)/stdpt$(DLL)
  LIB.STDPT= $(foreach d,$(DEP.STDPT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(STDPT)
else
  FOOBAR = $(OUT)/$(LIBPREFIX)stdpt$(LIB)
  DEP.EXE += $(STDPT)
  SCF.STATIC += stdpt
  TO_INSTALL.STATIC_LIBS += $(STDPT)
endif

# Sections for other types of modules have been omitted from this template
# because the requirements of those differ so much that it would be difficult
# to write a template to suit them all.

# This section defines files used by this module.
DIR.STDPT = plugins/proctex/standard
OUT.STDPT = $(OUT)/$(DIR.STDPT)
INC.STDPT = $(wildcard $(SRCDIR)/$(DIR.STDPT)/*.h)
SRC.STDPT = $(wildcard $(SRCDIR)/$(DIR.STDPT)/*.cpp)
OBJ.STDPT = $(addprefix $(OUT.STDPT)/,$(notdir $(SRC.STDPT:.cpp=$O)))
# Customise the following line.
DEP.STDPT = CSTOOL CSGFX CSUTIL CSSYS CSUTIL

OUTDIRS += $(OUT.STDPT)

# This section is read by the Microsoft Visual C++ project file synthesis tool.
MSVC.DSP += STDPT
DSP.STDPT.NAME = stdpt
DSP.STDPT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#-----------------------------------------------------------------------------#
ifeq ($(MAKESECTION),targets)

.PHONY: stdpt stdptclean stdptcleandep

stdpt: $(OUTDIRS) $(STDPT)
$(STDPT): $(OBJ.STDPT) $(LIB.STDPT)
	$(DO.PLUGIN)

$(OUT.STDPT)/%$O: $(DIR.STDPT)/%.cpp
	$(DO.COMPILE.CPP) $(STDPT.CFLAGS)

clean: stdptclean
stdptclean:
	$(RM) $(STDPT) $(OBJ.STDPT)

# This takes care of creating and including the dependency file.
cleandep: stdptcleandep
stdptcleandep:
	-$(RM) $(OUT.STDPT)/stdpt.dep

ifdef DO_DEPEND
dep: $(OUT.STDPT) $(OUT.STDPT)/stdpt.dep
$(OUT.STDPT)/stdpt.dep: $(SRC.STDPT)
	$(DO.DEPEND)
else
-include $(OUT.STDPT)/stdpt.dep
endif

endif # ifeq ($(MAKESECTION),targets)
