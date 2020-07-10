# $Id: Makefile.test_ncbi_ftp_download.app 561784 2018-04-11 21:22:30Z lavr $

APP = test_ncbi_ftp_download
SRC = test_ncbi_ftp_download
CPPFLAGS = $(CMPRS_INCLUDE) $(ORIG_CPPFLAGS)
LIB = xconnect $(COMPRESS_LIBS) xutil xncbi

LIBS = $(NETWORK_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)
#LINK = purify $(ORIG_LINK)

CHECK_CMD = test_ncbi_ftp_download.sh
CHECK_COPY = test_ncbi_ftp_download.sh

WATCHERS = lavr
