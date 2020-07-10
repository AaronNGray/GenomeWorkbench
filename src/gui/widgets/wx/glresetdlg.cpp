
#include <ncbi_pch.hpp>

#include <gui/widgets/wx/glresetdlg.hpp>

#include <gui/widgets/wx/glcanvas.hpp>

#include <util/image/image.hpp>

#include "wx/utils.h"
#include "wx/dc.h"
#include "wx/intl.h"
#include "wx/sizer.h"
#include "wx/cmndata.h"
#include "wx/dcclient.h"
#include "wx/panel.h"

#include <stdlib.h>
#include <string.h>

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------

BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CGLCanvas
/// This is the OpenGL widget for our simple dialog.
///
class CSimpleCanvas :
    public CGLCanvas
{
    DECLARE_EVENT_TABLE();
public:
    CSimpleCanvas(wxWindow* parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0)
    : CGLCanvas(parent, id, pos, size, style) {}

    void OnPaint(wxPaintEvent& event);
};


BEGIN_EVENT_TABLE(CSimpleCanvas, CGLCanvas)
    EVT_PAINT(CSimpleCanvas::OnPaint)
END_EVENT_TABLE()
 

void CSimpleCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    LOG_POST("CSimpleCanvas::OnPaint()");

    // This is a dummy, to avoid an endless succession of paint messages.
    // OnPaint handlers must always create a wxPaintDC.
    wxPaintDC dc(this);

    x_SetupGLContext();
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    SwapBuffers();
}


IMPLEMENT_CLASS(GlResetDlg, wxFrame)

GlResetDlg::GlResetDlg(wxWindow *parent, const wxPoint& pos)
    : wxFrame( parent,
               wxID_ANY,
               wxT("OpenGL Kicker"),
               pos,
               wxSize(20,20),
               wxFRAME_NO_TASKBAR|wxBORDER_SIMPLE )
    , m_Canvas(NULL)
{
    x_InitDialog();
}


GlResetDlg::~GlResetDlg()
{
}

void GlResetDlg::x_InitDialog() 
{
    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );
    wxPanel* p = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    wxBoxSizer *widget_sizer = new wxBoxSizer( wxHORIZONTAL );

    m_Canvas = new CSimpleCanvas(p, wxID_ANY, wxDefaultPosition, wxSize(20,20), 0);
    widget_sizer->Add(m_Canvas, 0, wxALL|wxEXPAND, 1);

    p->SetSizerAndFit(widget_sizer);
    mainsizer->Add(p, 1, wxALL|wxEXPAND, 1);

    SetSizerAndFit(mainsizer);

    InitDialog();
}

void GlResetDlg::SwitchContext() 
{
    if (m_Canvas != NULL) {
        Refresh();
        Update();
    }
}
    


END_NCBI_SCOPE
