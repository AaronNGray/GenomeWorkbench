# $Id: Makefile.sdbapi.mk 565385 2018-06-11 17:58:09Z ucko $

SDBAPI_LIB  = sdbapi ncbi_xdbapi_ftds $(FTDS_LIB) \
              ncbi_xdbapi_ftds95 $(FTDS95_LIB) \
              ncbi_xdbapi_ftds100 $(FTDS100_LIB) \
              dbapi dbapi_util_blobstore dbapi_driver \
              $(XCONNEXT) $(COMPRESS_LIBS)
SDBAPI_LIBS = $(FTDS95_LIBS) $(FTDS100_LIBS) $(CMPRS_LIBS)
