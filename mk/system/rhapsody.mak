#==============================================================================
#
#	Copyright (C)1999 by Eric Sunshine <sunshine@sunshineco.com>
#
# The contents of this file are copyrighted by Eric Sunshine.  This work is
# distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  You may distribute this file provided that this
# copyright notice is retained.  Send comments to <sunshine@sunshineco.com>.
#
#==============================================================================
NEXT.FLAVOR=RHAPSODY
NEXT.DESCRIPTION=Rhapsody-DR2
NEXT.ARCHS=i386 ppc
NEXT.SOURCE_DIRS=rhapsody openstep
NEXT.INCLUDE_DIRS=-FAppKit -FFoundation
NEXT.CFLAGS=-Wmost
NEXT.LIBS=-framework AppKit -framework Foundation

include mk/system/next.mak
