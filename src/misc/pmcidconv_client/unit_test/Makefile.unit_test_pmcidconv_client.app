#
# Makefile:  Makefile.unit_test_pmcidconv_client.app
#
#

###  BASIC PROJECT SETTINGS
APP = unit_test_pmcidconv_client
SRC = unit_test_pmcidconv_client
# OBJ =

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = pmcidconv_client \
      $(OBJREAD_LIBS) $(XFORMAT_LIBS) xobjutil tables \
      macro xregexp xmlwrapp test_boost $(OBJMGR_LIBS) $(PCRE_LIB)

LIBS = $(PCRE_LIBS) \
       $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(ORIG_LIBS) $(LIBXML_LIBS) $(LIBXSLT_LIBS)

REQUIRES = Boost.Test.Included

# Comment out if you do not want it to run automatically as part of
# "make check".
CHECK_CMD =
# If your test application uses config file, then uncomment this line -- and,
# remember to rename 'unit_test_discrepancy_report.ini' to '<your_app_name>.ini'.
#CHECK_COPY = unit_test_discrepancy_report.ini

###  EXAMPLES OF OTHER SETTINGS THAT MIGHT BE OF INTEREST
# PRE_LIBS = $(NCBI_C_LIBPATH) .....
# CFLAGS   = $(FAST_CFLAGS)
# CXXFLAGS = $(FAST_CXXFLAGS)
# LDFLAGS  = $(FAST_LDFLAGS)

WATCHERS = bollin
