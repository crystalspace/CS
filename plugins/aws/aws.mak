#------------------------------------------------------------------------------
# Alternate Window Manager plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.aws = Alternate Windowing System plug-in
DESCRIPTION.awsgen = Flex and Bison generated files (forcibly)
DESCRIPTION.awsinst = Flex and Bison generated files

ifneq (,$(CMD.FLEX))
ifneq (,$(CMD.BISON))
AWS.CAN_GEN = yes
endif
endif

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make aws          Make the $(DESCRIPTION.aws)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: aws awsclean awsgen awsinst awsgenclean
all plugins: aws

aws:
	$(MAKE_TARGET) MAKE_DLL=yes
awsclean:
	$(MAKE_CLEAN)
ifeq ($(AWS.CAN_GEN),yes)
awsgen:
	$(MAKE_TARGET)
awsinst:
	$(MAKE_TARGET)
endif
awsgenclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  AWS = $(OUTDLL)/aws$(DLL)
  LIB.AWS = $(foreach d,$(DEP.AWS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(AWS)
else
  AWS = $(OUT)/$(LIB_PREFIX)aws$(LIB)
  DEP.EXE += $(AWS)
  SCF.STATIC += aws
  TO_INSTALL.STATIC_LIBS += $(AWS)
endif

FLEX.SED = $(OUTDERIVED)/flex.sed
AWS.DERIVED.DIR = $(OUTDERIVED)/aws

DIR.AWS = plugins/aws
OUT.AWS = $(OUT)/$(DIR.AWS)
INF.AWS = $(SRCDIR)/$(DIR.AWS)/aws.csplugin
ifeq ($(DO_MSVCGEN),yes)
INC.AWS = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.AWS)/*.h $(DIR.AWS)/*.hpp include/iaws/*.h))
SRC.AWS = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.AWS)/*.cpp))
else
INC.AWS = $(AWS.DERIVED.DIR)/skinpars.hpp \
  $(filter-out $(SRCDIR)/$(DIR.AWS)/skinpars.hpp,$(wildcard $(addprefix \
  $(SRCDIR)/,$(DIR.AWS)/*.h $(DIR.AWS)/*.hpp include/iaws/*.h)))
SRC.AWS = $(AWS.DERIVED.DIR)/skinlex.cpp $(AWS.DERIVED.DIR)/skinpars.cpp \
  $(filter-out $(addprefix $(SRCDIR)/$(DIR.AWS)/,skinlex.cpp skinpars.cpp), \
  $(wildcard $(SRCDIR)/$(DIR.AWS)/*.cpp))
endif
OBJ.AWS = $(addprefix $(OUT.AWS)/,$(notdir $(SRC.AWS:.cpp=$O)))
DEP.AWS = CSTOOL CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.AWS) $(AWS.DERIVED.DIR)

TO_INSTALL.DATA += $(SRCDIR)/data/awsdef.zip

MSVC.DSP += AWS
DSP.AWS.NAME = aws
DSP.AWS.TYPE = plugin
DSP.AWS.CFLAGS = /D "YY_NEVER_INTERACTIVE"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: aws awsclean awscleandep awsgen awsinst awsgenclean
aws: $(OUTDIRS) $(AWS)

# Auto-generated files (skinlex.cpp and skinpars.cpp) are compiled from within
# $(OUTDERIVED), so we need to use -I to let them know where the AWS headers
# are.  Also silence compiler warnings via -Wno-unused (if available) in
# auto-generated code, over which we have no control.
$(OUT.AWS)/skinpars$O: $(AWS.DERIVED.DIR)/skinpars.cpp
	$(DO.COMPILE.CPP) $(CXXFLAGS.WARNING.NO_UNUSED) \
	$(CFLAGS.I)$(SRCDIR)/$(DIR.AWS)
$(OUT.AWS)/skinlex$O: $(AWS.DERIVED.DIR)/skinlex.cpp
	$(DO.COMPILE.CPP) $(CXXFLAGS.WARNING.NO_UNUSED) \
	$(CFLAGS.I)$(SRCDIR)/$(DIR.AWS)

$(OUT.AWS)/%$O: $(SRCDIR)/$(DIR.AWS)/%.cpp
	$(DO.COMPILE.CPP)

$(AWS): $(OBJ.AWS) $(LIB.AWS)
	$(DO.PLUGIN)

ifneq ($(AWS.CAN_GEN),yes)
$(AWS.DERIVED.DIR)/skinlex.cpp: $(SRCDIR)/$(DIR.AWS)/skinlex.cpp
	-$(RM) $@
	$(CP) $(SRCDIR)/$(DIR.AWS)/skinlex.cpp $@
$(AWS.DERIVED.DIR)/skinpars.cpp: $(SRCDIR)/$(DIR.AWS)/skinpars.cpp
	-$(RM) $@
	$(CP) $(SRCDIR)/$(DIR.AWS)/skinpars.cpp $@
$(AWS.DERIVED.DIR)/skinpars.hpp: $(SRCDIR)/$(DIR.AWS)/skinpars.hpp
	-$(RM) $@
	$(CP) $(SRCDIR)/$(DIR.AWS)/skinpars.hpp $@
else
# Some versions of Flex-generated files want to include <unistd.h> which is not
# normally available on Windows, so we need to protect it.  We also filter out
# CVS `Header' keywords in order to prevent CVS from thinking that the file has
# changed simply because the Header information is different.
# @@@ This macro and Flex generation could be moved to a separate utility file
# since it is not specific to AWS but this is the only client for now.
$(FLEX.SED):
	echo $"s/\([ 	]*#[ 	]*include[ 	][ 	]*<unistd.h>\)/\$">$@
	echo $"#ifndef WIN32\$">>$@
	echo $"\1\$">>$@
	echo $"#endif/$">>$@
	echo $"/$(BUCK)Header:/d$">>$@

$(AWS.DERIVED.DIR)/skinlex.cpp: $(SRCDIR)/$(DIR.AWS)/skinlex.ll $(FLEX.SED)
	$(CMD.FLEX) -L -t $(SRCDIR)/$(DIR.AWS)/skinlex.ll | \
	$(SED) -f $(FLEX.SED) > $@

$(AWS.DERIVED.DIR)/skinpars.cpp: $(SRCDIR)/$(DIR.AWS)/skinpars.yy
	$(CMD.BISON) --no-lines -d -p aws -o $@ $(<)

$(AWS.DERIVED.DIR)/skinpars.hpp: $(AWS.DERIVED.DIR)/skinpars.cpp
	@if [ -f "$(SRCDIR)/$(DIR.AWS)/skinpars.cpp.hpp" ]; then \
	$(MV) $(AWS.DERIVED.DIR)/skinpars.cpp.hpp \
	$(AWS.DERIVED.DIR)/skinpars.hpp; \
	fi

awsgen: $(OUTDIRS) awsgenclean \
  $(AWS.DERIVED.DIR)/skinlex.cpp \
  $(AWS.DERIVED.DIR)/skinpars.cpp \
  $(AWS.DERIVED.DIR)/skinpars.hpp

awsinst: $(OUTDIRS) \
  $(SRCDIR)/$(DIR.AWS)/skinlex.cpp \
  $(SRCDIR)/$(DIR.AWS)/skinpars.cpp \
  $(SRCDIR)/$(DIR.AWS)/skinpars.hpp

$(SRCDIR)/$(DIR.AWS)/skinlex.cpp: $(AWS.DERIVED.DIR)/skinlex.cpp
	-$(RM) $@
	$(CP) $(AWS.DERIVED.DIR)/skinlex.cpp $@
$(SRCDIR)/$(DIR.AWS)/skinpars.cpp: $(AWS.DERIVED.DIR)/skinpars.cpp
	-$(RM) $@
	$(CP) $(AWS.DERIVED.DIR)/skinpars.cpp $@
$(SRCDIR)/$(DIR.AWS)/skinpars.hpp: $(AWS.DERIVED.DIR)/skinpars.hpp
	-$(RM) $@
	$(CP) $(AWS.DERIVED.DIR)/skinpars.hpp $@
endif

clean: awsclean awsgenclean
awsclean:
	-$(RMDIR) $(AWS) $(OBJ.AWS) $(OUTDLL)/$(notdir $(INF.AWS)) $(FLEX.SED)

awsgenclean:
	-$(RM) $(addprefix $(AWS.DERIVED.DIR)/, \
	skinlex.cpp skinpars.cpp skinpars.hpp)

cleandep: awscleandep
awscleandep:
	-$(RM) $(OUT.AWS)/aws.dep

ifdef DO_DEPEND
dep: $(OUT.AWS) $(OUT.AWS)/aws.dep
$(OUT.AWS)/aws.dep: $(SRC.AWS)
	$(DO.DEPEND)
else
-include $(OUT.AWS)/aws.dep
endif

endif # ifeq ($(MAKESECTION),targets)
