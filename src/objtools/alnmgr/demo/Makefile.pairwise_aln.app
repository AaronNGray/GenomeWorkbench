# $Id: Makefile.pairwise_aln.app 502200 2016-05-23 14:42:40Z vakatov $

APP = pairwise_aln
SRC = pairwise_aln_app
LIB = submit $(OBJMGR_LIBS)

LIBS = $(CMPRS_LIBS) $(DL_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

WATCHERS = grichenk
