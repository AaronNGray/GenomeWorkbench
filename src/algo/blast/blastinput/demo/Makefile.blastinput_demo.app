# $Id: Makefile.blastinput_demo.app 601536 2020-02-10 15:02:01Z boratyng $

APP = blastinput_demo
SRC = blastinput_demo

CPPFLAGS = $(ORIG_CPPFLAGS) $(BLAST_THIRD_PARTY_INCLUDE)

ENTREZ_LIBS = entrez2cli entrez2

LIB_ = $(BLAST_INPUT_LIBS) $(ENTREZ_LIBS) $(BLAST_LIBS) $(OBJMGR_LIBS)
LIB = $(LIB_:%=%$(STATIC))

LIBS =  $(GENBANK_THIRD_PARTY_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(ORIG_LIBS)
LDFLAGS = $(FAST_LDFLAGS) 

REQUIRES = objects -Cygwin

WATCHERS = madden camacho
