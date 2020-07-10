# $Id: Makefile.gbench_feedback_agent.app 41919 2018-11-13 23:35:47Z rudnev $

APP = gbench_feedback_agent

SRC = gbench_feedback_agent  

LIB  = gui_objects w_gl w_wx w_feedback w_data gui_opengl snputil gui_objutils \
       gencoll_client genome_collection variation gui_utils \
       gbproj submit xobjutil entrezgene valerr eutils_client \
       ncbi_xcache_netcache xconnserv xthrserv xconnect xutil xncbi \
	   xmlwrapp xconnect \
       $(PCRE_LIB) $(COMPRESS_LIBS) $(SOBJMGR_LIBS) 


LIBS = $(WXWIDGETS_GL_LIBS) $(WXWIDGETS_LIBS) $(GLEW_LIBS) $(PCRE_LIBS) \
        $(CMPRS_LIBS) $(NETWORK_LIBS) $(DL_LIBS) $(LIBXSLT_LIBS) $(LIBXML_LIBS) $(ORIG_LIBS)

CPPFLAGS = $(ORIG_CPPFLAGS) $(GLEW_INCLUDE) $(WXWIDGETS_INCLUDE)

REQUIRES = gbench wxWidgets OpenGL WinMain


