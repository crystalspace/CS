# Target description
DESCRIPTION.csconfig = cs-config compile help script

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make csconfig     Build the $(DESCRIPTION.csconfig)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconfig csconfigclean

all: csconfig
csconfig:
	$(MAKE_TARGET)
csconfigclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSCONFIG.EXE = cs-config
CSCONFIG.TMP = $(OUT)/csconfig.tmp

TO_INSTALL.EXE	+= $(CSCONFIG.EXE)

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconfig csconfigclean $(CSCONFIG.EXE)

all: $(CSCONFIG.EXE)
csconfig: $(OUTDIRS) $(CSCONFIG.EXE)
clean: csconfigclean

verfile:=include/csver.h
vmajor:=`sed -e '/\#define[ 	][ 	]*CS_VERSION_MAJOR/!d' -e '/\#define[ 	][ 	]*CS_VERSION_MAJOR/s/\(\#define[ 	][ 	]*CS_VERSION_MAJOR[ 	][ 	]*"\)\([^\"]*\)"\(.*\)/\2/' < $(verfile)`
vminor:=`sed -e '/\#define[ 	][ 	]*CS_VERSION_MINOR/!d' -e '/\#define[ 	][ 	]*CS_VERSION_MINOR/s/\(\#define[ 	][ 	]*CS_VERSION_MINOR[ 	][ 	]*"\)\([^\"]*\)"\(.*\)/\2/' < $(verfile)`
rdate:=`sed -e '/\#define[ 	][ 	]*CS_RELEASE_DATE/!d' -e '/\#define[ 	][ 	]*CS_RELEASE_DATE/s/\(\#define[ 	][ 	]*CS_RELEASE_DATE[ 	][ 	]*"\)\([^\"]*\)"\(.*\)/\2/' < $(verfile)`

# Create csconfig.tmp for the make variables that need to be transferred.
$(CSCONFIG.EXE):
	@echo Generating cs-config script...
	@if test -x cs-config; then rm cs-config; fi

	@cat scripts/cs-config/cs-config.temppre > cs-config
	@echo "#	WARNING: This script is generated automatically! " >> cs-config
	@echo ""			>> cs-config
	@echo "CRYSTAL=\$${CRYSTAL-$(INSTALL_DIR)}"	>> cs-config
	@echo "prefix=\$${CRYSTAL}"			>> cs-config
	@echo "makeout=\"$(OUT)\""			>> cs-config
	@echo "exec_prefix=\$${prefix}"			>> cs-config
	@echo "version='$(vmajor)'"			>> cs-config
	@echo "longversion='$(vmajor).$(vminor) ($(rdate))'"	>> cs-config
	@echo "includedir=\$${prefix}/include"		>> cs-config
	@echo "if test -r \$${prefix}/lib; then"	>> cs-config
	@echo "  libdir=\$${prefix}/lib"		>> cs-config
	@echo "fi"					>> cs-config
	@echo "syslibs=\"$(LIBS.EXE)\""			>> cs-config
	@echo "common_cflags=\"$(CFLAGS)\""		>> cs-config
	@echo "common_cxxflags=\"$(CXXFLAGS)\""		>> cs-config
	@echo ""					>> cs-config
	@echo "makevars()"				>> cs-config
	@echo "{"					>> cs-config
	@echo "	cat <<EOF"				>> cs-config
	@echo $"# Crystal Space config make variables for $(DESCRIPTION.$(TARGET)).$" >> cs-config
	@echo $"# Note: automatically generated. $" 	>> cs-config
	@echo $"EXE=$(EXE)$" 				>> cs-config
	@echo $"DLL=$(DLL)$" 				>> cs-config
	@echo $"LFLAGS.EXE=$(LFLAGS.EXE)$" 		>> cs-config
	@echo $"DO.SHARED.PLUGIN.PREAMBLE=$(DO.SHARED.PLUGIN.PREAMBLE)$" >> cs-config
	@echo $"DO.SHARED.PLUGIN.POSTAMBLE=$(DO.SHARED.PLUGIN.POSTAMBLE)$" >> cs-config
	@echo $"LIBS.EXE.PLATFORM=$(LIBS.EXE.PLATFORM)$" >> cs-config
ifneq ($(LINK.PLUGIN),)
	@echo $"LINK.PLUGIN=$(LINK.PLUGIN)$" 		>> cs-config
	@echo $"LFLAGS.DLL=$(LFLAGS.DLL) \$$@$" 	>> cs-config
else
	@echo $"LINK.PLUGIN=$" 				>> cs-config
	@echo "LFLAGS.DLL=`echo $(LFLAGS.DLL) | sed -e "s/cs-config/\\\\\\\\$$\\@/"`" 		>> cs-config
endif
	@echo $"PLUGIN.POSTFLAGS=$(PLUGIN.POSTFLAGS)$"	>> cs-config
	@echo "EOF"					>> cs-config
	@echo "}"					>> cs-config
	@echo						>> cs-config
	@cat scripts/cs-config/cs-config.temppost	>> cs-config

	@chmod +x cs-config
	
csconfigclean:
	-$(RM) $(CSCONFIG.EXE) 

endif
