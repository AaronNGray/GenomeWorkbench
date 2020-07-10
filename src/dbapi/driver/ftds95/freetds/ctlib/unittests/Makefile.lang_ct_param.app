# $Id: Makefile.lang_ct_param.app 553128 2017-12-13 16:54:04Z satskyse $

APP = ct95_lang_ct_param
SRC = lang_ct_param common

CPPFLAGS = -DHAVE_CONFIG_H=1 $(FTDS95_INCLUDE) $(ORIG_CPPFLAGS)
LIB      = $(FTDS95_CTLIB_LIB)
LIBS     = $(FTDS95_CTLIB_LIBS) $(NETWORK_LIBS) $(RT_LIBS) $(C_LIBS)
LINK     = $(C_LINK)

CHECK_CMD  = test-ct95 ct95_lang_ct_param

CHECK_REQUIRES = in-house-resources

WATCHERS = ucko satskyse
