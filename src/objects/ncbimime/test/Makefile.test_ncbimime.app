#################################
# $Id: Makefile.test_ncbimime.app 500429 2016-05-04 16:47:01Z gouriano $
# Author:  Eugene Vasilchenko (vasilche@ncbi.nlm.nih.gov)
#################################

# Build test application "test_ncbimime"
#################################

APP = test_ncbimime
SRC = test_ncbimime
LIB = ncbimime cdd scoremat cn3d mmdb seqset $(SEQ_LIBS) \
        pub medline medlars biblio general \
	xser xutil xncbi

WATCHERS = vasilche
