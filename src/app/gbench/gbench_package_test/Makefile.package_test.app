# $Id: Makefile.package_test.app 20526 2009-12-09 02:25:32Z dicuccio $

APP = package_test

SRC = package_test

LIB  =

LIBS = $(DL_LIBS) $(C_LIBS)

LINK = $(C_LINK)

REQUIRES = gbench unix
