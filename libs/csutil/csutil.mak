#
# This submakefile requires the following variables to be defined by
# each platform-specific makefile:
#
# SRC.SYS_CSUTIL
#   - All system-dependent source files that should be included in csutil
#     library

# Library description
DESCRIPTION.csutil = Crystal Space utility library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csutil       Make the $(DESCRIPTION.csutil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csutil

all libs: csutil
csutil:
	$(MAKE_TARGET)
csutilclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSUTIL.LIB = $(OUT)/$(LIB_PREFIX)csutil$(LIB_SUFFIX)

DIR.CSUTIL = libs/csutil
OUT.CSUTIL = $(OUT)/$(DIR.CSUTIL)

INC.CSUTIL = $(INC.SYS_CSUTIL) \
  $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSUTIL)/*.h include/csutil/*.h))

SRC.CSUTIL.LOCAL = $(wildcard $(addprefix $(SRCDIR)/$(DIR.CSUTIL)/,*.cpp *.c))
ifneq ($(REGEX.AVAILABLE),yes)
  SRC.CSUTIL.LOCAL += $(SRCDIR)/$(DIR.CSUTIL)/generic/regex.c
endif
SRC.CSUTIL = $(SRC.SYS_CSUTIL) $(SRC.CSUTIL.LOCAL)

# Platform-specific makefiles may want to provide their own value for
# OBJ.SYS_CSUTIL (for instance, they might recognize other file types in
# addition to .s, .c, and .cpp), so we set OBJ.SYS_CSUTIL only if not already
# set by the platform-specific makefile.
ifeq (,$(strip $(OBJ.SYS_CSUTIL)))
OBJ.SYS_CSUTIL = $(addprefix $(OUT.CSUTIL)/,$(notdir \
  $(subst .s,$O,$(subst .c,$O,$(SRC.SYS_CSUTIL:.cpp=$O)))))
endif
OBJ.CSUTIL = $(OBJ.SYS_CSUTIL) $(addprefix $(OUT.CSUTIL)/, \
  $(notdir $(patsubst %.c,%$O,$(SRC.CSUTIL.LOCAL:.cpp=$O))))

CFG.CSUTIL = $(SRCDIR)/data/config/mouse.cfg

OUTDIRS += $(OUT.CSUTIL)

TO_INSTALL.CONFIG += $(CFG.CSUTIL)
TO_INSTALL.STATIC_LIBS += $(CSUTIL.LIB)

MSVC.DSP += CSUTIL
DSP.CSUTIL.NAME = csutil
DSP.CSUTIL.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csutil csutilclean csutilcleandep

csutil: $(OUTDIRS) $(CSUTIL.LIB)

$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/%.c
	$(DO.COMPILE.C)

$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/%.cpp
	$(DO.COMPILE.CPP) $(ZLIB.CFLAGS)

$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/generic/%.c
	$(DO.COMPILE.C)

$(OUT.CSUTIL)/%$O: $(SRCDIR)/$(DIR.CSUTIL)/generic/%.cpp
	$(DO.COMPILE.CPP)

$(CSUTIL.LIB): $(OBJ.CSUTIL)
	$(DO.LIBRARY)

clean: csutilclean
csutilclean:
	-$(RM) $(CSUTIL.LIB) $(OBJ.CSUTIL)

cleandep: csutilcleandep
csutilcleandep:
	-$(RM) $(OUT.CSUTIL)/csutil.dep

ifdef DO_DEPEND
dep: $(OUT.CSUTIL) $(OUT.CSUTIL)/csutil.dep
$(OUT.CSUTIL)/csutil.dep: $(SRC.CSUTIL.LOCAL)
	$(DO.DEPEND1) \
	$(ZLIB.CFLAGS) \
	$(CFLAGS.D)CS_CONFIGDIR='"$(CS_CONFIGDIR)"' \
	$(CFLAGS.D)CS_PLUGINDIR'"=$(CS_PLUGINDIR)"' \
	$(DO.DEPEND2)
else
-include $(OUT.CSUTIL)/csutil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
