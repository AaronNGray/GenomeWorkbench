/////////////////////////////////////////////////////////////////////////////
// Name:        gl_demo_dlg.cpp
// Purpose:
// Author:      Roman Katargin
// Modified by:
// Created:     07/08/2007 12:04:04
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <ncbi_pch.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>

#include "gl_demo_dlg.hpp"
#include "ruler_demo_panel.hpp"
#include "font_demo_panel.hpp"
#include "gl3d_demo_panel.hpp"
#include "glinfo_demo_panel.hpp"
#include "glpanewidget_demo_panel.hpp"

#include <gui/opengl/ftglfontmanager.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/widgets/wx/3dcanvas.hpp>
#include <gui/widgets/wx/sys_path.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CGLDemoDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGLDemoDlg, wxDialog )


/*!
 * CGLDemoDlg event table definition
 */

BEGIN_EVENT_TABLE( CGLDemoDlg, wxDialog )

////@begin CGLDemoDlg event table entries
    EVT_CLOSE( CGLDemoDlg::OnCloseWindow )

    EVT_BUTTON( wxID_CLOSE, CGLDemoDlg::OnCloseClick )

////@end CGLDemoDlg event table entries

END_EVENT_TABLE()


/*!
 * CGLDemoDlg constructors
 */

CGLDemoDlg::CGLDemoDlg()
{
    Init();
}

CGLDemoDlg::CGLDemoDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CGLDemoDlg creator
 */

bool CGLDemoDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGLDemoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGLDemoDlg creation
    return true;
}


/*!
 * CGLDemoDlg destructor
 */

CGLDemoDlg::~CGLDemoDlg()
{
////@begin CGLDemoDlg destruction
////@end CGLDemoDlg destruction
}


/*!
 * Member initialisation
 */

void CGLDemoDlg::Init()
{
////@begin CGLDemoDlg member initialisation
////@end CGLDemoDlg member initialisation

    wxSize ppi = wxGetDisplayPPI();
    CFtglFontManager::Instance().SetDeviceResolution(ppi.y);
    CFtglFontManager::Instance().SetFontPath(string(CSysPath::GetResourcePath().ToUTF8()));
}


class CMetalWindow : public C3DCanvas
{
public:
    CMetalWindow(wxWindow* parent) : C3DCanvas(parent, wxID_ANY) {}

protected:
    virtual void x_Render();
};

void CMetalWindow::x_Render()
{
    IRender& gl = GetGl();
    wxSize sz = GetClientSize();
    gl.Viewport(0, 0, sz.x, sz.y);
    gl.MatrixMode(GL_PROJECTION);
    gl.LoadIdentity();
    gl.Ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    gl.Begin(GL_LINE_STRIP);
    gl.ColorC(CRgbaColor(1.0f, 0.0f, 0.0f));
    gl.Vertex2f(-0.9f, -0.9f);
    gl.Vertex2f(0.9f, -0.9f);
    gl.Vertex2f(0.0f, 0.9f);
    gl.Vertex2f(-0.9f, -0.9f);
    gl.End();

    gl.Begin(GL_LINE_STRIP);
    gl.ColorC(CRgbaColor(0.0f, 1.0f, 0.0f));
    gl.Vertex2f(0.0f, -0.9f);
    gl.Vertex2f(-0.9f, 0.9f);
    gl.Vertex2f(0.9f, 0.9f);
    gl.Vertex2f(0.0f, -0.9f);
    gl.End();
}


/*!
 * Control creation for CGLDemoDlg
 */

void CGLDemoDlg::CreateControls()
{
////@begin CGLDemoDlg content construction
    CGLDemoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxNotebook* itemNotebook3 = new wxNotebook( itemDialog1, ID_NOTEBOOK1, wxDefaultPosition, wxSize(512, 384), wxBK_DEFAULT|wxCLIP_CHILDREN );

    itemBoxSizer2->Add(itemNotebook3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CGLDemoDlg content construction

    wxNotebook* itemNotebook = (wxNotebook*)FindWindow(ID_NOTEBOOK1);

    wxWindow* itemPane = new CRulerDemoPanel(itemNotebook, wxID_ANY);
    itemNotebook->AddPage(itemPane, wxT("CRuler"));

    itemPane = new CFontDemoPanel(itemNotebook, wxID_ANY);
    itemNotebook->AddPage(itemPane, wxT("OpenGL Fonts"));

    itemPane = new CGl3dDemoPanel(itemNotebook, wxID_ANY);
    itemNotebook->AddPage(itemPane, wxT("Gl3d"));

    itemPane = new CGlInfoDemoPanel(itemNotebook, wxID_ANY);
    itemNotebook->AddPage(itemPane, wxT("GlSysInfo"));

    itemPane = new CGlPaneWidgetDemoPanel(itemNotebook, wxID_ANY);
    itemNotebook->AddPage(itemPane, wxT("GlPaneWidget"));
   
#ifdef __WXOSX_COCOA__
    itemNotebook->AddPage(new CMetalWindow(itemNotebook),  wxT("Metal View"));
#endif
}


/*!
 * Should we show tooltips?
 */

bool CGLDemoDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGLDemoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGLDemoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGLDemoDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGLDemoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGLDemoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGLDemoDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void CGLDemoDlg::OnCloseClick( wxCommandEvent& WXUNUSED(event) )
{
    Close();
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_CGLDEMODLG
 */

void CGLDemoDlg::OnCloseWindow( wxCloseEvent& WXUNUSED(event) )
{
    Destroy();
}


END_NCBI_SCOPE

