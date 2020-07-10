# $Id: Makefile.demo_font.cgi.app 41919 2018-11-13 23:35:47Z rudnev $

SRC = demo_font

APP = demo_font.cgi

PROJ_TAG = demo

LIB_ = gui_glmesa gui_opengl gui_utils \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   ximage xcompress $(CMPRS_LIB) xcgi xhtml xconnect xutil xncbi 
LIB = $(LIB_:%=%$(STATIC))

LIBS = $(FTGL_LIBS) $(OSMESA_LIBS) $(IMAGE_LIBS) $(CMPRS_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(FTGL_INCLUDE) $(OPENGL_INCLUDE) $(GLEW_INCLUDE) $(ORIG_CPPFLAGS)
