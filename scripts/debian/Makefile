BUILDDIR=$(HOME)/debian/crystal
#DATE=$(shell date '+%Y%m%d')
VERSION=0.94
TAG=R0_94_001
DATE=20020412
MINOR=1

default:
	@echo "=============================================================="
	@echo "You can build from your local copy"
	@echo "Typing:"
	@echo "	make fromlocal"
	@echo "Or from a CVS export (edit DATE in Makefile if you want a particular date)"
	@echo "Typing:"
	@echo "	make fromcvs"
	@echo "=============================================================="

# BUILD FROM CVS EXPORT
fromcvs: start cvsexportdate debfiles debbuild
# BUILD FROM LOCAL COPY
# just binaries
justbin: start copy debfiles debcopy debclean debconfigure debbinary
# don't rebuild all
retry: start copy debfiles debcopy debconfigure debbinary
# full with debuild
fromlocal: start copy debfiles debcopy debbuild

start:
	@echo "Building Crystalspace-$(VERSION).$(DATE)"

# Just to copy from local files
copy: $(BUILDDIR) $(BUILDDIR)/copy-stamp
$(BUILDDIR)/copy-stamp:
	echo "Copying files"
	cd ../../..; find CS | grep -v CVS | cpio -pdumvB $(BUILDDIR)/crystalspace-$(VERSION).$(DATE)
	touch $(BUILDDIR)/copy-stamp

# To anonymous login, do it only one time is needed
cvslogin:
	echo "Login crystalspace CVS archive as anonymous"
	echo "Password is <Enter>"
	cvs -d:pserver:anonymous@cvs.crystal.sourceforge.net:/cvsroot/crystal login

# If you want to export a special date
cvsexportdate: $(BUILDDIR) $(BUILDDIR)/cvsexportdate-stamp
$(BUILDDIR)/cvsexportdate-stamp: 
	echo "Exporting CVS by date $(DATE)"
	cd $(BUILDDIR) && cvs -d:pserver:anonymous@cvs.crystal.sourceforge.net:/cvsroot/crystal export -d crystalspace-$(VERSION).$(DATE) -D $(DATE) crystal
	touch $(BUILDDIR)/cvsexportdate-stamp

# If you want to export a special tag (to make e.g. orig file)
cvsexporttag: $(BUILDDIR) $(BUILDDIR)/cvsexporttag-stamp
$(BUILDDIR)/cvsexporttag-stamp: 
	echo "Exporting CVS by tag $(TAG)"
	cd $(BUILDDIR) && cvs -d:pserver:anonymous@cvs.crystal.sourceforge.net:/cvsroot/crystal export -d crystalspace_$(VERSION) -r $(TAG) crystal
	touch $(BUILDDIR)/cvsexporttag-stamp

# Prepare debian files
debfiles: $(BUILDDIR)/debfile-stamp
$(BUILDDIR)/debfile-stamp:
	echo "Moving debian files"
	mv $(BUILDDIR)/crystalspace-$(VERSION).$(DATE)/CS/scripts/debian/files $(BUILDDIR)/crystalspace-$(VERSION).$(DATE)/debian
	touch $(BUILDDIR)/debfile-stamp

debcopy:
	cd files ; find . | cpio -pdumvB $(BUILDDIR)/crystalspace-$(VERSION).$(DATE)/debian

debclean:
	cd $(BUILDDIR)/crystalspace-$(VERSION).$(DATE); fakeroot debian/rules clean
	
debconfigure:
	cd $(BUILDDIR)/crystalspace-$(VERSION).$(DATE); fakeroot debian/rules configure

debbinary:
	cd $(BUILDDIR)/crystalspace-$(VERSION).$(DATE);dch -v $(VERSION).$(DATE)-$(MINOR) "Build from CVS" ; fakeroot debian/rules binary

debbuild:
	#cd $(BUILDDIR)/crystalspace-$(VERSION).$(DATE);dch -v $(VERSION).$(DATE)-$(MINOR) "Build from CVS" ; debuild -us -uc -v$(VERSION)-$(shell expr $(MINOR) - 1)
	#cd $(BUILDDIR)/crystalspace-$(VERSION).$(DATE);dch -v $(VERSION).$(DATE)-$(MINOR) "Build from CVS" ; debuild -v$(VERSION)-$(shell expr $(MINOR) - 1)
	cd $(BUILDDIR)/crystalspace-$(VERSION).$(DATE); debuild

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -f $(BUILDDIR)/*-stamp
