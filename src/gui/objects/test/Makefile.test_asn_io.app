# $Id: Makefile.test_asn_io.app 27988 2013-05-06 20:29:47Z rafanovi $

APP = test_asn_io
PROJ_TAG = test

SRC = test_asn_io

LIB  = $(SEQ_LIBS) pub medline biblio general xser xutil xncbi

REQUIRES = OpenGL wxWidgets objects WinMain
