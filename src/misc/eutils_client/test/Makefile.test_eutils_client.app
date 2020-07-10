# $Id: Makefile.test_eutils_client.app 558371 2018-02-26 18:21:23Z ivanov $


APP = test_eutils_client
SRC = test_eutils_client

LIB  = eutils_client xmlwrapp $(SEQ_LIBS) pub medline biblio general xser \
       xcgi $(CONNEXT) xconnect xutil xncbi

LIBS = $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(NETWORK_LIBS) \
	   $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = LIBXML LIBXSLT

CHECK_CMD = test_eutils_client -db pubmed -count dog  /CHECK_NAME=EUtilsCli_Dog

WATCHERS = grichenk
