# $Id: Makefile.seq_c_compat_unit_test.app 206487 2010-09-28 18:16:54Z ucko $
APP = seq_c_compat_unit_test
SRC = seq_c_compat_unit_test

CPPFLAGS = $(BOOST_INCLUDE) $(NCBI_C_INCLUDE) $(ORIG_CPPFLAGS)

LIB = $(SEQ_LIBS) pub medline biblio general \
      xser xxconnect xutil test_boost xncbi
NCBI_C_LIBS = -lncbiobj $(NCBI_C_ncbi)
LIBS = $(NCBI_C_LIBPATH) $(NCBI_C_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

CHECK_CMD =

WATCHERS = ucko

REQUIRES = C-Toolkit
