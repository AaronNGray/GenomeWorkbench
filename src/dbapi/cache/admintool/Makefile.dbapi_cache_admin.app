# $Id: Makefile.dbapi_cache_admin.app 184358 2010-02-26 16:33:57Z ivanov $


APP = dbapi_cache_admin
SRC = dbapi_cache_admin


LIB  = ncbi_xcache_dbapi dbapi ncbi_xdbapi_ftds $(FTDS_LIB) dbapi_driver \
       xutil xncbi

LIBS = $(FTDS_LIBS) $(ICONV_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS)


REQUIRES = FreeTDS

WATCHERS = kuznets


