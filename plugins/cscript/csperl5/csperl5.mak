DESCRIPTION.csperl5 = Crystal Space Perl5 scripting plugin
DESCRIPTION.csperl5distclean = $(DESCRIPTION.csperl5)

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION), rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csperl5      Make the $(DESCRIPTION.csperl5)$"

endif

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION), roottargets)

.PHONY: csperl5 csperl5clean csperl5distclean

all plugins: csperl5

csperl5:
	$(MAKE_TARGET) MAKE_DLL=yes
csperl5clean:
	$(MAKE_CLEAN)
csperl5distclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION), postdefines)

ifeq ($(USE_PLUGINS),yes)
  CSPERL5 = $(OUTDLL)/csperl5$(DLL)
  LIB.CSPERL5 = $(foreach d,$(DEP.CSPERL5),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSPERL5)
else
  CSPERL5 = $(OUT)/$(LIBPREFIX)csperl5$(LIB)
  DEP.EXE += $(CSPERL5)
  SCF.STATIC += csperl5
  TO_INSTALL.STATIC_LIBS += $(CSPERL5)
endif

INC.CSPERL5 = $(wildcard plugins/cscript/csperl5/*.h)
SRC.CSPERL5 = $(wildcard plugins/cscript/csperl5/*.cpp)
OBJ.CSPERL5 = $(addprefix $(OUT)/,$(notdir $(SRC.CSPERL5:.cpp=$O)))
DEP.CSPERL5 = CSGEOM CSSYS CSUTIL CSSYS CSUTIL

PERLXSI.DEP = config.mak
PERLXSI.DIR = $(OUTDERIVED)
PERLXSI.C = $(PERLXSI.DIR)/csperlxs.c
PERLXSI.O = $(OUT)/$(notdir $(PERLXSI.C:.c=$O))

SWIG.I = include/ivaria/cs.i
SWIG.MOD = cspace
SWIG.PERL5.PM = scripts/perl5/$(SWIG.MOD).pm
SWIG.PERL5.C = plugins/cscript/csperl5/cswigpl5.c
SWIG.PERL5.O = $(OUT)/$(notdir $(SWIG.PERL5.C:.c=$O))
SWIG.PERL5.DLL = scripts/perl5/$(SWIG.MOD)$(DLL)
SWIG.PERL5.DOC = scripts/perl5/cs_wrap.doc

CEX.CSPERL5 = perl5.cex
CIN.CSPERL5 = plugins/cscript/csperl5/perl5.cin

MSVC.DSP += MSCSPERL5
DSP.MSCSPERL5.NAME = csperl5
DSP.MSCSPERL5.TYPE = plugin
DSP.MSCSPERL5.CFLAGS = \
  /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NO_STRICT" \
  /D "HAVE_DES_FCRYPT" /D "PERL_IMPLICIT_CONTEXT" \
  /D "PERL_IMPLICIT_SYS" /D "USE_PERLIO" \
  /D "PERL_MSVCRT_READFIX" \
MSCSPERL5 = $(CSPERL5)
LIB.MSCSPERL5 = $(LIB.CSPERL5) $(LIBPREFIX)perl58$(LIB)
INC.MSCSPERL5 = $(INC.CSPERL5)
SRC.MSCSPERL5 = $(SRC.CSPERL5) $(PERLXSI.C)
OBJ.MSCSPERL5 = $(OBJ.CSPERL5) $(PERLXSI.O)
DEP.MSCSPERL5 = $(DEP.CSPERL5)

MSVC.DSP += MSCSPERL5SWIG
DSP.MSCSPERL5SWIG.NAME = csperl5s
DSP.MSCSPERL5SWIG.TYPE = plugin
DSP.MSCSPERL5SWIG.CFLAGS = \
  $(DSP.MSCSPERL5.CFLAGS) /D "PERL_POLLUTE" /D "NO_HANDY_PERL_MACROS"
MSCSPERL5SWIG = $(SWIG.PERL5.DLL)
LIB.MSCSPERL5SWIG = $(LIB.MSCSPERL5)
SRC.MSCSPERL5SWIG = $(SWIG.PERL5.C)
OBJ.MSCSPERL5SWIG = $(SWIG.PERL5.O)
DEP.MSCSPERL5SWIG = $(DEP.CSPERL5)

ifeq ($(PERL5.EXTUTILS.AVAILABLE),yes)
define PERLXSI.CONTENT
$(PERL) -MExtUtils::Embed -e xsinit -- -o $(PERLXSI.C) -std \
  $(PERL5.EXTUTILS.DYNALOADER) cspace
endef
else
define PERLXSI.CONTENT
$(RM) -f $(PERLXSI.C)
@echo '#if defined(__cplusplus) && !defined(PERL_OBJECT)' >> $(PERLXSI.C)
@echo '#  define is_cplusplus' >> $(PERLXSI.C)
@echo '#endif' >> $(PERLXSI.C)
@echo '#ifdef is_cplusplus' >> $(PERLXSI.C)
@echo 'extern "C" {' >> $(PERLXSI.C)
@echo '#endif' >> $(PERLXSI.C)
@echo '#include <EXTERN.h>' >> $(PERLXSI.C)
@echo '#include <perl.h>' >> $(PERLXSI.C)
@echo '#ifdef PERL_OBJECT' >> $(PERLXSI.C)
@echo '#  define NO_XSLOCKS' >> $(PERLXSI.C)
@echo '#  include <XSUB.h>' >> $(PERLXSI.C)
@echo '#  include "win32iop.h"' >> $(PERLXSI.C)
@echo '#  include <fcntl.h>' >> $(PERLXSI.C)
@echo '#  include <perlhost.h>' >> $(PERLXSI.C)
@echo '#endif' >> $(PERLXSI.C)
@echo '#ifdef is_cplusplus' >> $(PERLXSI.C)
@echo '}' >> $(PERLXSI.C)
@echo '#  ifndef EXTERN_C' >> $(PERLXSI.C)
@echo '#    define EXTERN_C extern "C"' >> $(PERLXSI.C)
@echo '#  endif' >> $(PERLXSI.C)
@echo '#else' >> $(PERLXSI.C)
@echo '#  ifndef EXTERN_C' >> $(PERLXSI.C)
@echo '#    define EXTERN_C extern' >> $(PERLXSI.C)
@echo '#  endif' >> $(PERLXSI.C)
@echo '#endif' >> $(PERLXSI.C)
@echo 'EXTERN_C void xs_init (pTHXo);' >> $(PERLXSI.C)
@echo 'EXTERN_C void boot_DynaLoader (pTHXo_ CV* cv);' >> $(PERLXSI.C)
@echo 'EXTERN_C void boot_cspace (pTHXo_ CV* cv);' >> $(PERLXSI.C)
@echo 'EXTERN_C void xs_init(pTHXo)' >> $(PERLXSI.C)
@echo '{' >> $(PERLXSI.C)
@echo '  char *file = __FILE__;' >> $(PERLXSI.C)
@echo '  dXSUB_SYS;' >> $(PERLXSI.C)
@echo '  /* DynaLoader is a special case */' >> $(PERLXSI.C)
@echo '  newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);' >> \
	$(PERLXSI.C)
@echo '  newXS("cspace::bootstrap", boot_cspace, file);' >> $(PERLXSI.C)
@echo '}' >> $(PERLXSI.C)
@echo "Generated $(PERLXSI.C)"
endef
endif

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION), targets)

.PHONY: csperl5 csperl5clean csperl5distclean

csperl5: $(OUTDIRS) $(CSPERL5) $(CSPERL5.PM) $(CEX.PERL5)

$(CSPERL5): $(OBJ.CSPERL5) $(LIB.CSPERL5) $(PERLXSI.O) $(SWIG.PERL5.DLL)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(PERL5.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

$(OUT)/%$O: plugins/cscript/csperl5/%.cpp
	$(DO.COMPILE.CPP) $(PERL5.CFLAGS)

$(PERLXSI.C): $(PERLXSI.DEP) $(PERLXSI.DIR)
	$(PERLXSI.CONTENT)

$(PERLXSI.O): $(PERLXSI.C)
	$(DO.COMPILE.C) $(PERL5.CFLAGS)

ifeq (,$(SWIGBIN))
  SWIGBIN = swig
endif

$(SWIG.PERL5.PM) $(SWIG.PERL5.C): $(SWIG.I)
	$(SWIGBIN) -perl5 -c++ -dascii -Sbefore -shadow -Iinclude \
	-module $(SWIG.MOD) -o $(SWIG.PERL5.C) $(SWIG.I)
	$(MV) plugins/cscript/csperl5/$(SWIG.MOD).pm $(SWIG.PERL5.PM)

$(SWIG.PERL5.O): $(SWIG.PERL5.C)
	$(filter-out -W -Wunused -Wall -Wmost,$(DO.COMPILE.CPP) \
	$(PERL5.CFLAGS) -DPERL_POLLUTE -DNO_HANDY_PERL_MACROS)

$(SWIG.PERL5.DLL): $(SWIG.PERL5.O) $(LIB.CSPERL5)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(PERL5.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

$(CEX.PERL5): $(CIN.PERL5)
	@echo Generating perl5 cs-config extension...
	$(PERL5) $(CIN.PERL5) \
	$"CFLAGS=$(PERL5.CFLAGS)$" $"LFLAGS=$(PERL5.LFLAGS)$" > $@

clean: csperl5clean
distclean: csperl5distclean

csperl5clean:
	$(RM) $(CSPERL5) $(OBJ.CSPERL5) $(PERLXSI.O) \
	$(CSPERL5.PM) $(SWIG.PERL5.O) $(SWIG.PERL5.DLL)

csperl5distclean: csperl5clean
	$(RM) $(PERLXSI.C) $(SWIG.PERL5.DOC)
# If these were not stored in CVS, we would also probably delete them.
#	$(SWIG.PERL5.C) $(SWIG.PERL5.PM)

ifdef DO_DEPEND
dep: $(OUTOS)/csperl5.dep
$(OUTOS)/csperl5.dep: $(SRC.CSPERL5)
	$(DO.DEP1) $(PERL5.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/csperl5.dep
endif

endif
