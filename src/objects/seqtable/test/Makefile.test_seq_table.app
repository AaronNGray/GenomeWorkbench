# $Id: Makefile.test_seq_table.app 428967 2014-03-10 16:26:03Z vasilche $

APP = test_seq_table
SRC = test_seq_table

LIB = $(SEQ_LIBS) pub medline biblio general xcompress xser xutil xncbi
LIBS = $(CMPRS_LIBS)

WATCHERS = vasilche

CHECK_CMD =
