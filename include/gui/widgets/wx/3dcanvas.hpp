#ifndef GUI_WIDGETS_GL___3D_CANVAS__HPP
#define GUI_WIDGETS_GL___3D_CANVAS__HPP

#include <wx/wx.h>

#include <gui/opengl.h>

#if defined(__WXOSX_COCOA__) && defined(USE_METAL)

#include <gui/widgets/wx/mtlcanvas.hpp>

BEGIN_NCBI_SCOPE
typedef CMTLCanvas C3DCanvas;
END_NCBI_SCOPE

#else

#include <gui/widgets/wx/glcanvas.hpp>

BEGIN_NCBI_SCOPE
typedef CGLCanvas C3DCanvas;
END_NCBI_SCOPE

#endif

#endif  // GUI_WIDGETS_GL___3D_CANVAS__HPP

