#------------------------------------------------------------------------------
# Alternate Window Manager plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.aws = Alternate Windowing System plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make aws          Make the $(DESCRIPTION.aws)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: aws awsclean
all plugins: aws

aws:
	$(MAKE_TARGET) MAKE_DLL=yes
awsclean:
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

DIR.AWS = plugins/aws
OUT.AWS = $(OUT)/$(DIR.AWS)
INF.AWS = $(SRCDIR)/$(DIR.AWS)/aws.csplugin
INC.AWS = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.AWS)/*.h include/iaws/*.h))
# We add skinlex.cpp and skinpars.cpp explicitly since they might not be
# present (thus need to be regenerated automatically).  We use $(sort) for its
# side-effect of folding out duplicate entries to ensure that skinlex.cpp and
# skinpars.cpp only appear in the list once each.
SRC.AWS = $(sort $(wildcard $(SRCDIR)/$(DIR.AWS)/*.cpp) \
  $(addprefix $(SRCDIR)/,$(DIR.AWS)/skinlex.cpp $(DIR.AWS)/skinpars.cpp))
OBJ.AWS = $(addprefix $(OUT.AWS)/,$(notdir $(SRC.AWS:.cpp=$O)))
DEP.AWS = CSUTIL CSUTIL CSGEOM CSTOOL CSGFX

OUTDIRS += $(OUT.AWS)

TO_INSTALL.DATA += $(SRCDIR)/data/awsdef.zip

MSVC.DSP += AWS
DSP.AWS.NAME = aws
DSP.AWS.TYPE = plugin
DSP.AWS.CFLAGS = /D "YY_NEVER_INTERACTIVE"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: aws awsclean awscleandep
aws: $(OUTDIRS) $(AWS)

# Work around bizarre preprocessor bug on older MacOS/X installations where it
# fails to search the CS/plugins/aws directory for headers #included by
# skinpars.cpp.  Work around is to add appropriate -I directive.  Also silence
# compiler warnings via -Wno-unused in auto-generated code (skinlex.cpp and
# skinpars.cpp), over which we have no control.  NOTE: It would be cleaner to
# have only two special implicit rules: one for skinlex.cpp and one for
# skinpars.cpp, rather than a catch-all rule for everything in AWS.
# Unfortunately, buggy GNU make on MacOS/X prevents us from doing so because
# it resolves implicit rules incorrectly (seemingly randomly).  For instance,
# if we use a special rule for skinpars.cpp, then GNU make bogusly tries
# building almost every object file in the entire project from the
# skinpars.cpp source.  An attempt to work around this bug by adding a
# catch-all implicit rule for AWS for files other than skinpars.cpp also
# misbehaves incorrectly.  If the catch-all rule is defined after the
# skinpars.cpp rule, then the catch-all rule is ignored.  If it is defined
# before the skinpars.cpp rule, then the skinpars.cpp rule is ignored.  Using
# just the catch-all rule alone for AWS at least allows the module to be
# built, ugly though it is.  @@@ FIXME: Configure script should check if
# compiler recognizes -Wno-unused, rather than blatently assuming that it does.
$(OUT.AWS)/%$O: $(SRCDIR)/$(DIR.AWS)/%.cpp
	$(DO.COMPILE.CPP) -Wno-unused $(CFLAGS.I)$(SRCDIR)/$(DIR.AWS)

$(AWS): $(OBJ.AWS) $(LIB.AWS)
	$(DO.PLUGIN)

ifneq (,$(FLEXBIN))
ifneq (,$(BISONBIN))

$(SRCDIR)/$(DIR.AWS)/skinlex.cpp: \
  $(SRCDIR)/$(DIR.AWS)/skinpars.hpp \
  $(SRCDIR)/$(DIR.AWS)/skinlex.ll \
  $(SRCDIR)/$(DIR.AWS)/skinpars.cpp
	flex -L -S$(SRCDIR)/$(DIR.AWS)/flex.skl \
	-t $(SRCDIR)/$(DIR.AWS)/skinlex.ll > $(SRCDIR)/$(DIR.AWS)/skinlex.cpp

$(SRCDIR)/$(DIR.AWS)/skinpars.cpp: $(SRCDIR)/$(DIR.AWS)/skinpars.yy
	bison --no-lines -d -p aws -o $@ $(<)

$(SRCDIR)/$(DIR.AWS)/skinpars.hpp: $(SRCDIR)/$(DIR.AWS)/skinpars.cpp
	@if [ -f "$(SRCDIR)/$(DIR.AWS)/skinpars.cpp.hpp" ]; then \
	$(MV) $(SRCDIR)/$(DIR.AWS)/skinpars.cpp.hpp \
	$(SRCDIR)/$(DIR.AWS)/skinpars.hpp; \
	fi
endif
endif
clean: awsclean
awsclean:
	-$(RMDIR) $(AWS) $(OBJ.AWS) $(OUTDLL)/$(notdir $(INF.AWS))

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
