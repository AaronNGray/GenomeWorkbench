# $Id: Makefile.psiblast_iteration_unit_test.app 553578 2017-12-18 23:33:36Z fongah2 $

APP = psiblast_iteration_unit_test
SRC = psiblast_iteration_unit_test 

CPPFLAGS = -DNCBI_MODULE=BLAST $(ORIG_CPPFLAGS) $(BOOST_INCLUDE) -I$(srcdir)/../../api
LIB = test_boost $(BLAST_LIBS) xobjsimple $(OBJMGR_LIBS:ncbi_x%=ncbi_x%$(DLL))
LIBS = $(NETWORK_LIBS) $(CMPRS_LIBS) $(DL_LIBS) $(BLAST_THIRD_PARTY_LIBS) $(ORIG_LIBS)

CHECK_REQUIRES = MT in-house-resources
CHECK_CMD = psiblast_iteration_unit_test
CHECK_COPY = psiblast_iteration_unit_test.ini

WATCHERS = boratyng madden camacho
