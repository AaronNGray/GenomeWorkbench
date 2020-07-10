# $Id: Makefile.unit_test_eutils.app 544871 2017-08-28 19:43:34Z ucko $

APP = unit_test_eutils
SRC = unit_test_eutils

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = eutils_client xmlwrapp $(SEQ_LIBS) pub medline biblio general xser \
      xcgi $(CONNEXT) xconnect xutil test_boost xncbi

LIBS = $(LIBXSLT_STATIC_LIBS) \
	   $(LIBXML_STATIC_LIBS) \
	   $(NETWORK_LIBS) $(ICONV_LIBS) $(DL_LIBS) $(ORIG_LIBS)

REQUIRES = Boost.Test.Included LIBXML LIBXSLT

CHECK_CMD =

WATCHERS = kotliaro
