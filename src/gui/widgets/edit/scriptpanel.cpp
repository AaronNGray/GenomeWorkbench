/*  $Id:  macrofloweditorapp.cpp 38070 06/07/2017 15:12:46 17:48:35Z Igor Filippov$
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */
#include <ncbi_pch.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include <wx/graphics.h>
#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/utils.h> 
#include <wx/app.h>
#include <wx/evtloop.h>
#include <wx/notifmsg.h>
#include <wx/generic/notifmsg.h>
#include <wx/splash.h>
#include <wx/graphics.h>
#include <wx/gdicmn.h> 
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
////@end includes
#include <sstream>
#include <util/line_reader.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/widgets/edit/macrofloweditor.hpp>
#include <gui/widgets/edit/macro_simple.hpp>
#include "scriptpanel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



/*
 * CScriptPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CScriptPanel, wxPanel )


/*
 * CScriptPanel event table definition
 */

BEGIN_EVENT_TABLE( CScriptPanel, wxPanel )

////@begin CScriptPanel event table entries
EVT_CONTEXT_MENU(CScriptPanel::OnRightClick)
////@end CScriptPanel event table entries

END_EVENT_TABLE()


/*
 * CScriptPanel constructors
 */

CScriptPanel::CScriptPanel()
    : m_timer(this)
{
    Init();
}

CScriptPanel::CScriptPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_timer(this)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CScriptPanel creator
 */

bool CScriptPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CScriptPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CScriptPanel creation
    return true;
}


/*
 * CScriptPanel destructor
 */

CScriptPanel::~CScriptPanel()
{
    m_ScriptWindow->RemoveEventHandler(m_handler);
    delete m_handler;
}


/*
 * Member initialisation
 */

void CScriptPanel::Init()
{
////@begin CScriptPanel member initialisation
    m_ScriptWindow = NULL;
    m_drag = false;
    m_once_per_drag = true;
    m_font = wxNullFont;
    m_modified = false;
    m_displaced = INT_MAX;

    wxArtProvider::Push( new CwxSplittingArtProvider() );
    wxFileArtProvider* provider =  new wxFileArtProvider();
    wxArtProvider::Push(provider);
    provider->AddDirectory( CSysPath::ResolvePath( wxT("<res>") ));
    provider->RegisterFileAlias(wxT("menu::dm_tiles"), wxT("tiles.png"));
    wxBitmap bitmap(wxArtProvider::GetBitmap(wxT("menu::dm_tiles"), wxART_TOOLBAR, wxSize(16,16))); 
    wxImage image(bitmap.ConvertToImage());
    m_copy_cursor = wxCursor(image);
    m_scroll_pos = 0;
////@end CScriptPanel member initialisation
}


/*
 * Control creation for CScriptPanel
 */

void CScriptPanel::CreateControls()
{    
////@begin CScriptPanel content construction
    CScriptPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_ScriptWindow = new wxScrolledWindow( itemPanel1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxSize(400, -1), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_ScriptWindow, 1, wxGROW|wxTOP, 10);
    m_ScriptWindow->SetScrollbars(1, 1, 0, 0);
    wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    m_ScriptWindow->SetSizer(sizer1);
    m_ScriptWindow->FitInside();
    m_handler = new CMacroLabelHandler(this);
    m_ScriptWindow->PushEventHandler(m_handler);

    wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer6, 0, wxGROW|wxALL, 0);

    m_Parameters = new CMacroParamsPanel( itemStaticBoxSizer6->GetStaticBox(), ID_SCROLLEDWINDOW1, wxDefaultPosition, wxSize(400, -1), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemStaticBoxSizer6->Add(m_Parameters, 0, wxGROW|wxALL, 5);  

////@end CScriptPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CScriptPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CScriptPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CScriptPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CScriptPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CScriptPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CScriptPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CScriptPanel icon retrieval
}

CMacroLabel* CScriptPanel::AddMacro(CRef<macro::CMacroRep> macro)
{
    CMacroLabel *label = new CMacroLabel(m_ScriptWindow, macro, m_font);
    m_ScriptWindow->GetSizer()->Add(label, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);    
    label->SetIndex(m_ScriptWindow->GetSizer()->GetChildren().size());
    return label;
}

void CScriptPanel::FitPage()
{
    Layout();
    Refresh();
    m_ScriptWindow->FitInside();
}

CMacroLabel* CScriptPanel::InsertMacro(CRef<macro::CMacroRep> macro, size_t index)
{
    CMacroLabel *label = new CMacroLabel(m_ScriptWindow, macro, m_font);
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    sizer->Insert(index, label, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);    
    wxSizerItemList& children = sizer->GetChildren();
    for (size_t i = index; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
        win->SetIndex(i + 1);
    }
    return label;
}

size_t CScriptPanel::GetCount()
{
    size_t count = 0;
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
	count++;
    }
    return count;
}

void CScriptPanel::UnselectMacro()
{
    for (auto &label : m_selected_macros)
    {
        label.second->Unselect();
    }
    m_Parameters->SetMacro(CRef<macro::CMacroRep>(NULL));
    m_selected_macros.clear();
}

void CScriptPanel::SelectMacro(CMacroLabel *label) 
{
    if (!label)
        return;
    size_t index = GetIndex(label);
    if (m_selected_macros.find(index) != m_selected_macros.end())
        return;    
    m_Parameters->SetMacro(label->GetMacro(), label);
    m_selected_macros[index] = label;
    label->Select();
}

void CScriptPanel::SelectMacros(CMacroLabel *label) 
{
    if (!label)
        return;
    size_t index = GetIndex(label);
    if (!m_selected_macros.empty())	
    {
	size_t min_selected = m_selected_macros.begin()->first;
	size_t max_selected = m_selected_macros.rbegin()->first;
	wxSizer *sizer = m_ScriptWindow->GetSizer();
	wxSizerItemList& children = sizer->GetChildren();
	for (size_t i = min(index, min_selected); i < children.size() && i < max(index, max_selected); i++)
	{
	    CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
	    if (!win)
		continue;
	    if (m_selected_macros.find(i) != m_selected_macros.end())
		continue;    
	    m_selected_macros[i] = win;
	    win->Select();
	}
    }

    if (m_selected_macros.find(index) != m_selected_macros.end())
        return;    
    m_Parameters->SetMacro(label->GetMacro(), label);
    m_selected_macros[index] = label;
    label->Select();
}

void CScriptPanel::SelectItem(size_t item)
{
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
        if (i == item)
        {
            SelectMacro(win);

            wxRect rect = win->GetRect();
            int rect_y;
            m_ScriptWindow->CalcUnscrolledPosition(0, rect.y, NULL, &rect_y);
            int client_w, client_h;
            m_ScriptWindow->GetClientSize( &client_w, &client_h );
            int hLine;
            m_ScriptWindow->GetScrollPixelsPerUnit(NULL, &hLine);
            int view_y;
            m_ScriptWindow->GetViewStart(NULL, &view_y);
            view_y *= hLine;
            if (rect_y < view_y)
                m_ScriptWindow->Scroll( -1, rect_y / hLine );
            if (rect_y + rect.height + 5 > view_y + client_h)
                m_ScriptWindow->Scroll( -1, (rect_y + rect.height - client_h + hLine) / hLine );
            break;
        }
    }
}

void CScriptPanel::SaveScrollPos()
{
   
    m_ScriptWindow->GetViewStart(NULL, &m_scroll_pos);   
}

void CScriptPanel::LoadScrollPos()
{
    m_ScriptWindow->Scroll(-1, m_scroll_pos);
}

size_t CScriptPanel::GetIndex(CMacroLabel* label)
{
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (win == label)
            return i;
    }
    return wxNOT_FOUND;
}

void CScriptPanel::OnMouseUp()
{
    if (m_selected_macros.empty())
        return;   
    DropMacro();
}

void CScriptPanel::OnMouseDown()
{
    if (m_selected_macros.empty())
        return;
    if (!m_drag)
    {
        UnselectMacro();
        return;
    }
}

vector<pair<CRef<macro::CMacroRep>, bool> > CScriptPanel::GetScript()
{
    vector<pair<CRef<macro::CMacroRep>, bool> > script;
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    script.reserve(children.size());
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
        script.push_back(make_pair(win->GetMacro(), !win->IsSkipped()));
    }
    return script;
}

void CScriptPanel::UpdateCounter(size_t i, size_t counter)
{
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
    if (!win)
        return;
    win->UpdateCounter(counter);
}

void CScriptPanel::ResetCounters()
{
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
        win->UpdateCounter(0);
    }
}

void CScriptPanel::DropMacro()
{
    if (m_selected_macros.empty())
        return;
    if (!m_drag)
        return;

    wxPoint p = m_ScriptWindow->ScreenToClient(wxGetMousePosition());

    m_ScriptWindow->SetCursor(wxNullCursor);
    wxSetCursor(wxNullCursor);

    if (m_ScriptWindow->HasCapture())
	m_ScriptWindow->ReleaseMouse();
    m_displaced = INT_MAX;
    bool lock = false;
    m_drag = false;
    m_once_per_drag = true;
    if (m_timer.IsRunning())
        m_timer.Stop();

    CMacroFlowEditor* frame =  dynamic_cast<CMacroFlowEditor*>(GetParent()->GetParent()->GetParent());
    if (frame)
    {
	lock = frame->GetLockDrag();
    }

    int client_w, client_h;
    m_ScriptWindow->GetClientSize( &client_w, &client_h );
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    bool rearranged = false;
    if (p.x > 0 && p.x < client_w && p.y >= 0 && p.y <= client_h && !lock)    
    {
	for (auto &macro : m_selected_macros)
	{
	    sizer->Detach(macro.second);
	}
	size_t index = GetInsertIndex(p.y, children);   
	if (index < children.size())
	{
	    for (auto &macro : m_selected_macros)
	    {
		sizer->Insert(index, macro.second, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
		index++;
	    }       
	}
	else
	{
	    for (auto &macro : m_selected_macros)
	    {
		sizer->Add(macro.second, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
	    }
	}
	m_modified = true;  
	rearranged = true;
    }



    for (size_t i = 0; i < children.size(); i++)
    {
	wxWindow* wxwin = children[i]->GetWindow();
	wxwin->SetPosition(wxDefaultPosition);
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(wxwin);
        if (!win)
            continue;
        win->SetIndex(i + 1);
    }

    Layout();
    Refresh();
    m_ScriptWindow->FitInside();

    for (auto &macro : m_selected_macros)
    {
	macro.second->ShowWithEffect(wxSHOW_EFFECT_EXPAND);
    }


    UnselectMacro();
    if (!rearranged)
    {	    
	frame->Shake();
	if (lock)
	  ShowSplashWindow(frame);
    }
}

void CScriptPanel::ShowSplashWindow(CMacroFlowEditor* frame)
{
    wxBitmap bitmap(frame->GetLockBitmap());
    wxImage image(280, 80, true);
    wxColour bg(GetBackgroundColour());
    image.SetRGB(wxRect(0, 0, 280, 80), bg.Red(), bg.Green(), bg.Blue()); 
    image.Paste(bitmap.ConvertToImage(), 5, 40 - bitmap.GetHeight() / 2);
    wxBitmap composite(image);
    wxSplashScreen *splash = new wxSplashScreen(composite, wxSPLASH_CENTRE_ON_PARENT | wxSPLASH_TIMEOUT,  2000, frame, wxID_ANY);
    wxWindow *win = splash->GetSplashWindow();
    wxStaticText *text = new wxStaticText( win, wxID_STATIC, wxT("Click the lock icon to enable dragging"), wxPoint(10 + bitmap.GetWidth(), 40 - bitmap.GetHeight() / 2 + 3));
    wxGraphicsContext *gc = wxGraphicsContext::Create();
    gc->SetFont(text->GetFont(), *wxBLACK);
    wxDouble width;
    wxDouble height;
    wxDouble descent;
    wxDouble externalLeading;
    gc->GetTextExtent(text->GetLabel(), &width, &height, &descent, &externalLeading);
    splash->SetSize(ceil(width) + 20 + bitmap.GetWidth(), 80);
    delete gc;
}

size_t CScriptPanel::GetInsertIndex(int y, wxSizerItemList& children)
{
    set<CMacroLabel*> selected;
    for (auto &macro : m_selected_macros)
    {
	selected.insert(macro.second);
    }

    size_t index = children.size();
    int miny = INT_MAX;
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
	if (selected.find(win) != selected.end())
	    continue;
        wxPoint pos = win->GetPosition();
        if (pos.y > y && pos.y < miny)
        {
            index = i;
            miny = pos.y;
        }
    }
    return index;
}

void CScriptPanel::DeleteSelected()
{
    if (m_selected_macros.empty())
        return;
    m_Parameters->SetMacro(CRef<macro::CMacroRep>(NULL));
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    for (auto &macro : m_selected_macros)
    {
        sizer->Detach(macro.second); 
        macro.second->Unselect();
        macro.second->Destroy();
    }
    m_selected_macros.clear();
    wxSizerItemList& children = sizer->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
        win->SetIndex(i + 1);
    }
    Layout();
    Refresh();
    m_ScriptWindow->FitInside();
    m_drag = false;    
    m_once_per_drag = true;
    if (m_timer.IsRunning())
        m_timer.Stop();
}


void CScriptPanel::DragMacro()
{
    if (m_selected_macros.empty())
        return;

    m_CurrentMousePos = wxGetMousePosition();
    if (m_once_per_drag)
    {
	wxPoint d = m_CurrentMousePos - m_Position;
	if (abs(d.x) < 10 && abs(d.y) < 10)
	    return;
    }

    int client_h;
    m_ScriptWindow->GetClientSize( NULL, &client_h );
    int hLine;
    m_ScriptWindow->GetScrollPixelsPerUnit(NULL, &hLine);

    wxPoint p = m_ScriptWindow->ScreenToClient(m_CurrentMousePos);
    int view_y;
    m_ScriptWindow->GetViewStart(NULL, &view_y);
    view_y *= hLine;
    bool moved = false;
    const int d = 20;
    if (  p.y < 5 )
      {
	view_y -= d;
	if (view_y < 0)
	  view_y = 0;
	m_ScriptWindow->Scroll(-1, view_y/hLine);
	moved = true;
      }
    else if ( p.y > client_h - 5)
      {
	view_y += d;
	m_ScriptWindow->Scroll(-1, view_y/hLine);
	moved = true;
      }

    if (moved)
    {
        if (!m_timer.IsRunning())
            m_timer.Start(10);
    }
    else
    {
        if (m_timer.IsRunning())
            m_timer.Stop();
    }

    m_drag = true;    
    SetMousePosition(m_CurrentMousePos);

    ShiftBottomLabels();
}

void CScriptPanel::ShiftBottomLabels()
{

    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    size_t index = GetInsertIndex(m_ScriptWindow->ScreenToClient(m_CurrentMousePos).y, children);
    vector<int> displacement(children.size(), 0);
    if (m_once_per_drag)
    {
      	m_once_per_drag = false;
	int height = 0;
	if (!m_ScriptWindow->HasCapture())
	    m_ScriptWindow->CaptureMouse();

	m_ScriptWindow->SetCursor(m_copy_cursor);
	wxSetCursor(m_copy_cursor);

	for (auto &macro : m_selected_macros)
	{
	    macro.second->HideWithEffect(wxSHOW_EFFECT_EXPAND);
	}    

	for (size_t i = 0; i < children.size(); i++)
	{
	    CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
	    if (!win)
		continue;
	    if (!win->IsShown())
	    {
		height += win->GetSize().GetHeight() + 10;
		continue;
	    }

	    if (height > 0)
	    {
		displacement[i] = -height;
	    }
	}
	
    }

    for (size_t i = index; i < children.size() && i < m_displaced; i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win || !win->IsShown())
            continue;
	displacement[i] += 50;
    }
    for (size_t i = m_displaced; i < children.size() && i < index; i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win || !win->IsShown())
            continue;
	displacement[i] -= 50;
    }

    for (size_t i = 0; i < children.size(); i++)
    {
	int height = displacement[i];
	if (height == 0)
	    continue;
	CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
	if (!win)
	    continue;
	if (!win->IsShown())
	{
	    continue;
	}
	
	wxPoint displace(0, height);
	wxPoint current = win->GetPosition();
	win->SetPosition(current + displace);
    }
	
#ifdef __WXMSW__
    for (size_t i = min(index,m_displaced); i < children.size() && i < max(index, m_displaced); i++)
    {
	CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win || !win->IsShown())
            continue;
	win->Refresh();
	win->Update();
    }
#endif
    m_displaced = index;
}

void CScriptPanel::ToggleSkip(CMacroLabel* label)
{
    if (label)
    {
        label->ToggleSkip();
    }
}

void CScriptPanel::ToggleSkip()
{
    for (auto &macro : m_selected_macros)
    {
        ToggleSkip(macro.second);
    }
}

void CScriptPanel::IncreaseFont()
{
    wxBusyCursor wait;
    wxYield();
    m_ScriptWindow->Freeze();
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();  
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
        win->IncreaseFont();    
        if (i == 0)
            m_font = win->GetFont();
    }
    m_ScriptWindow->FitInside();   
    m_ScriptWindow->Thaw();
}

void CScriptPanel::DecreaseFont()
{
    wxBusyCursor wait;
    wxYield();
    m_ScriptWindow->Freeze();
    wxSizer *sizer = m_ScriptWindow->GetSizer();
    wxSizerItemList& children = sizer->GetChildren();
    for (size_t i = 0; i < children.size(); i++)
    {
        CMacroLabel *win = dynamic_cast<CMacroLabel*>(children[i]->GetWindow());
        if (!win)
            continue;
        win->DecreaseFont();
        if (i == 0)
            m_font = win->GetFont();
    }
    m_ScriptWindow->FitInside();
    m_ScriptWindow->Thaw();
}

void CScriptPanel::UpdateParamsPanel() 
{
    m_Parameters->UpdateMacro();
}

void CScriptPanel::CommitChanges()
{   
    m_Parameters->CommitChanges();
}

void CScriptPanel::OnRightClick(wxContextMenuEvent &evt)
 {
     if (m_drag)
	 return;
     wxMenu itemMenu8;
     itemMenu8.Append(ID_MACROFLOW_CUT, _("Cut\tCtrl+X"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_COPY, _("Copy\tCtrl+C"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_PASTE, _("Paste\tCtrl+V"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_DELETE, _("Delete\tDel"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_DUPLICATE, _("Duplicate\tCtrl+D"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_APPEND, _("Add To Library\tAlt+A"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_ADD, _("Add New Macro"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_SKIP_STEP, _("Enable/Disable Step\tF3"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_ZOOM_IN, _("Increase Font\tAlt+I"), wxEmptyString, wxITEM_NORMAL);
     itemMenu8.Append(ID_MACROFLOW_ZOOM_OUT, _("Decrease Font\tAlt+D"), wxEmptyString, wxITEM_NORMAL);
     PopupMenu(&itemMenu8);
 }

BEGIN_EVENT_TABLE( CMacroLabelHandler, wxEvtHandler )
 EVT_LEFT_UP(CMacroLabelHandler::OnMouseUp)
 EVT_LEFT_DOWN(CMacroLabelHandler::OnMouseDown)
 EVT_MOTION(CMacroLabelHandler::OnMouseDrag)
 EVT_LEFT_DCLICK(CMacroLabelHandler::OnMouseDoubleClick)
END_EVENT_TABLE()


void CMacroLabelHandler::OnMouseUp(wxMouseEvent& event)
{
    if (m_win)
    {	
        m_win->OnMouseUp();
    }
    else if (m_panel)
    {
        m_panel->OnMouseUp();
    }
    event.Skip();
}


void CMacroLabelHandler::OnMouseDown(wxMouseEvent& event)
{
    if (m_win)
    {
	if ( event.GetModifiers() != wxMOD_SHIFT)
	{
	    if ( !m_win->IsSelected())
	    {
		if (event.GetModifiers() != wxMOD_CONTROL)
		{
		    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(m_win->GetGrandParent());
		    if (panel)
			panel->UnselectMacro();
		}
		m_win->OnMouseDown();
	    }
	}
	else
	{
	    m_win->OnMouseAndShiftDown();
	}
    }
    else if (m_panel)
    {
        m_panel->OnMouseDown();
    }

    event.Skip();
}


void CMacroLabelHandler::OnMouseDrag(wxMouseEvent& event)
{
    if (event.Dragging())
    {
        if (m_win)
            m_win->OnMouseDrag();
        else if (m_panel)
            m_panel->DragMacro();
    }
    event.Skip();
}


void CMacroLabelHandler::OnMouseDoubleClick(wxMouseEvent& event)
{
    if (m_win)
    {
        CScriptPanel *panel = dynamic_cast<CScriptPanel*>(m_win->GetGrandParent());
        if (panel)
            panel->SaveScrollPos();
	    m_win->CommitChanges();
	    CallAfter(&CMacroLabelHandler::RunSimpleEditor);
    }
    event.Skip();
}

void CMacroLabelHandler::RunSimpleEditor()
{
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(m_win->GetGrandParent());        
    NMacroStats::ReportUsage(wxT("ScriptPanel"), "view macro step");
    CRef<macro::CMacroRep> macro = m_win->GetMacro();
    CMacroSimple dlg(NULL, macro->GetSource());
    if (dlg.ShowModal() != wxID_OK)
    {       
        panel->LoadScrollPos();
        return;
    }
    m_win->UpdateMacro(dlg.GetMacro());
    panel->LoadScrollPos();
}

IMPLEMENT_CLASS( CMacroLabel, TMacroLabelParent )

BEGIN_EVENT_TABLE( CMacroLabel, TMacroLabelParent )
END_EVENT_TABLE()

#define LABEL_WIDTH 500
#define MAX_TITLE_LENGTH 200
CMacroLabel::CMacroLabel()
{
    Init();
}

CMacroLabel::CMacroLabel( wxWindow* parent, CRef<macro::CMacroRep> macro, const wxFont& font, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
    :  m_font(font)
{
    if (macro)
    {
	m_macro.Reset(m_MEngine.Parse(macro->GetSource()));
    }

    Init();
    Create( parent, id, pos, size, style );   
    RoundedShape();
}

bool CMacroLabel::Create( wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
{
    TMacroLabelParent::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();   
    return true;
}

void CMacroLabel::RoundedShape()
{
    wxGraphicsPath  path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();
    wxSize sz = GetSize();
    path.AddRoundedRectangle(0, 0, sz.GetWidth(), sz.GetHeight(), 15);
    SetShape(path);
    Refresh();
}
/*
bool CMacroLabel::Layout()
{
    bool r = TMacroLabelParent::Layout();

    if (m_bitmap)
    {
        int x = (m_panel->GetSize().GetWidth() -  m_static_bitmap.GetWidth()) / 2;
        int y = (m_panel->GetSize().GetHeight() -  m_static_bitmap.GetHeight()) / 2;
        m_bitmap->SetPosition(wxPoint(x,y));
        m_bitmap->Show(m_skipped);
        m_bitmap->Refresh();
    }
    return r;
}
*/
CMacroLabel::~CMacroLabel()
{
    m_panel->RemoveEventHandler(m_handler1);
    m_index->RemoveEventHandler(m_handler2);
    m_text->RemoveEventHandler(m_handler3);
    m_bitmap->RemoveEventHandler(m_handler4);
    m_counter->RemoveEventHandler(m_handler5);

    delete m_handler1;
    delete m_handler2;
    delete m_handler3;
    delete m_handler4;
    delete m_handler5;
}

void CMacroLabel::Init()
{
    m_static_bitmap = wxArtProvider::GetBitmap(wxT("menu::delete")); // wxART_CLOSE, wxART_OTHER
    m_skipped = false;
    m_bitmap = NULL;
    m_text = NULL;
    m_selected = false;
}

wxColour CMacroLabel::GetLabelColour(const string &str)
{
    unsigned long hash = wxStringHash::stringHash(str.c_str());
    wxColour old = *wxGREEN; //notebook->GetBackgroundColour();
    wxImage::HSVValue hsv = wxImage::RGBtoHSV(wxImage::RGBValue(old.Red(), old.Green(), old.Blue()));
    hsv.hue = double(hash % 360) / 360;
    hsv.saturation = 0.2;
    wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
    wxColour upd(rgb.red, rgb.green, rgb.blue);
    return upd;
}

void CMacroLabel::CreateControls()
{    
    CMacroLabel* itemFrame1 = this;
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemFrame1->SetSizer(itemBoxSizer2);

    m_panel = new wxPanel( itemFrame1, wxID_ANY, wxDefaultPosition, wxSize(LABEL_WIDTH, -1));
    itemBoxSizer2->Add(m_panel, 1, wxGROW|wxALL, 0);
    
    wxColour upd = GetLabelColour(m_macro->GetForEachString());
    m_panel->SetBackgroundColour(upd);
    
    m_handler1 = new CMacroLabelHandler(this);
    m_panel->PushEventHandler(m_handler1);


    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    m_panel->SetSizer(itemBoxSizer4);    

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxGROW, 0);

    m_bitmap = new wxGenericStaticBitmap(m_panel, wxID_ANY, m_static_bitmap);
    //m_bitmap->SetScaleMode(wxStaticBitmap::Scale_AspectFit);
    itemBoxSizer5->Add(m_bitmap, 0, wxALIGN_LEFT|wxRESERVE_SPACE_EVEN_IF_HIDDEN|wxALL, 0); 
    m_handler4 = new CMacroLabelHandler(this);
    m_bitmap->PushEventHandler(m_handler4);
    m_bitmap->Hide();

    itemBoxSizer5->AddStretchSpacer();

    m_index = new wxStaticText( m_panel, wxID_STATIC, _("   "), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemBoxSizer5->Add(m_index, 0, wxALIGN_LEFT|wxLEFT, 5);
    m_handler2 = new CMacroLabelHandler(this);
    m_index->PushEventHandler(m_handler2);

    wxString title = ConstructTitle();
    m_text = new wxStaticText( m_panel, wxID_STATIC, title, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL );
    itemBoxSizer4->Add(m_text, 1, wxGROW|wxALL, 5);
    m_text->SetFont(m_font);
    m_handler3 = new CMacroLabelHandler(this);
    m_text->PushEventHandler(m_handler3);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer6, 0, wxGROW, 0);

    itemBoxSizer6->AddStretchSpacer();

    m_counter = new wxStaticText( m_panel, wxID_STATIC, _("   "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    itemBoxSizer6->Add(m_counter, 0, wxALIGN_RIGHT|wxRIGHT, 5);
    m_handler5 = new CMacroLabelHandler(this);
    m_counter->PushEventHandler(m_handler5);

    m_text->Wrap(GetTextWidth());
}

wxString CMacroLabel::ConstructTitle()
{
    wxString title = wxString(m_macro->GetTitle());
    if (title.Len() > MAX_TITLE_LENGTH)
    {
        title = title.Left(MAX_TITLE_LENGTH - 3) + "...";
    }
    return title;
}

int CMacroLabel::GetTextWidth()
{
    return LABEL_WIDTH - max(m_static_bitmap.GetWidth(), m_index->GetSize().GetWidth()) - m_counter->GetSize().GetWidth() - 10;
}

void CMacroLabel::SetIndex(size_t index)
{
    wxString label;
    label << index;
    m_index_val = index;
    m_index->SetLabel(label);
}

void CMacroLabel::UpdateCounter(size_t counter)
{
    wxString label;
    if (counter > 0)
        label << counter;
    m_counter->SetLabel(label);
}

bool CMacroLabel::ShowToolTips()
{
    return true;
}

wxBitmap CMacroLabel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CMacroLabel::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

void CMacroLabel::ToggleSkip()
{
    m_skipped = !m_skipped;

    wxColour old = m_panel->GetBackgroundColour();
    wxImage::HSVValue hsv = wxImage::RGBtoHSV(wxImage::RGBValue(old.Red(), old.Green(), old.Blue()));

    if (m_skipped)
        hsv.saturation = 0.1;
    else
        hsv.saturation = 0.2;

    wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
    wxColour upd(rgb.red, rgb.green, rgb.blue);
    m_panel->SetBackgroundColour(upd);
    m_panel->Refresh();
    m_bitmap->Show(m_skipped);
}

void CMacroLabel::Select()
{
    wxColour old = m_panel->GetBackgroundColour();
    wxImage::HSVValue hsv = wxImage::RGBtoHSV(wxImage::RGBValue(old.Red(), old.Green(), old.Blue()));

    hsv.value = 0.7;

    wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
    wxColour upd(rgb.red, rgb.green, rgb.blue);
    m_panel->SetBackgroundColour(upd);
    m_panel->Refresh();
    m_selected = true;
}

void CMacroLabel::Unselect()
{
    wxColour old = m_panel->GetBackgroundColour();
    wxImage::HSVValue hsv = wxImage::RGBtoHSV(wxImage::RGBValue(old.Red(), old.Green(), old.Blue()));
    
    hsv.value = 1.0;

    wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
    wxColour upd(rgb.red, rgb.green, rgb.blue);
    m_panel->SetBackgroundColour(upd);
    m_panel->Refresh();
    m_selected = false;
}

void CMacroLabel::OnMouseDrag()
{
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
    if (!panel)
        return;
    panel->DragMacro();   
}

void CMacroLabel::DragMacro()
{
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
    if (!panel)
        return;
    wxPoint p = panel->GetCurrentMousePosition();
    wxPoint d = p - panel->GetMousePosition();
    wxPoint current = GetPosition();
    SetPosition(current + d);
}

void CMacroLabel::OnMouseUp()
{
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
    if (!panel)
        return;

    panel->DropMacro();
}

void CMacroLabel::OnMouseDown()
{
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
    if (!panel)
        return;
    if (!m_selected)
    {
	panel->SelectMacro(this);
    }
    else
    {
	CommitChanges();
    }
    panel->SetMousePosition(wxGetMousePosition());
}

void CMacroLabel::OnMouseAndShiftDown()
{
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
    if (!panel)
        return;
    panel->SelectMacros(this);
    panel->SetMousePosition(wxGetMousePosition());
}

void CMacroLabel::CommitChanges()
{
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
    if (!panel)
        return;
    panel->CommitChanges();
}

void CMacroLabel::IncreaseFont()
{
    wxSize text_sz = m_text->GetSize();
    wxSize panel_sz = GetSize();
    wxFont font = m_text->GetFont();
    font.MakeLarger();
    m_text->SetFont(font);
    wxString title = ConstructTitle();
    m_text->SetLabel(title);
    m_text->Wrap(GetTextWidth());   
    wxSize new_text_sz = m_text->GetSize();
    int new_height = panel_sz.GetHeight() + new_text_sz.GetHeight() - text_sz.GetHeight() + m_text->GetCharHeight();
    SetMinSize(wxSize(panel_sz.GetWidth(), new_height));
    SetSize(wxSize(panel_sz.GetWidth(), new_height));
    RoundedShape();
}

void CMacroLabel::DecreaseFont()
{
    wxSize text_sz = m_text->GetSize();
    wxSize panel_sz = GetSize();
    wxFont font = m_text->GetFont();
    font.MakeSmaller();
    m_text->SetFont(font);
    wxString title = ConstructTitle();
    m_text->SetLabel(title);
    m_text->Wrap(GetTextWidth());
    wxSize new_text_sz = m_text->GetSize();
    int new_height = panel_sz.GetHeight() + new_text_sz.GetHeight() - text_sz.GetHeight() + m_text->GetCharHeight();
    SetMinSize(wxSize(panel_sz.GetWidth(), new_height));
    SetSize(wxSize(panel_sz.GetWidth(), new_height));
    RoundedShape();
}

wxFont CMacroLabel::GetFont()
{
    return m_text->GetFont();
}

void CMacroLabel::UpdateText()
{
    wxSize text_sz = m_text->GetSize();
    wxSize panel_sz = GetSize();
    wxString title = ConstructTitle();
    m_text->SetLabel(title);
    m_text->Wrap(GetTextWidth());
    wxSize new_text_sz = m_text->GetSize();
    int new_height = panel_sz.GetHeight() + new_text_sz.GetHeight() - text_sz.GetHeight() + m_text->GetCharHeight();
    SetMinSize(wxSize(panel_sz.GetWidth(), new_height));
    SetSize(wxSize(panel_sz.GetWidth(), new_height));   
    RoundedShape();   
    GetParent()->FitInside();
    CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
    if (panel)
	panel->SetModified(true);
}

void CMacroLabel::UpdateMacro(CRef<macro::CMacroRep> macro)
{
    if (!macro)
	return;
    m_macro.Reset(m_MEngine.Parse(macro->GetSource()));
    UpdateText();
    if (m_selected)
    {
	CScriptPanel *panel = dynamic_cast<CScriptPanel*>(GetGrandParent());
	if (panel)
	    panel->UpdateParamsPanel();
    }
	
}

IMPLEMENT_DYNAMIC_CLASS( CMacroParamsPanel, wxPanel )

BEGIN_EVENT_TABLE( CMacroParamsPanel, wxPanel )
END_EVENT_TABLE()

CMacroParamsPanel::CMacroParamsPanel()
{
    Init();
}

CMacroParamsPanel::CMacroParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CMacroParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}

CMacroParamsPanel::~CMacroParamsPanel()
{
}

void CMacroParamsPanel::Init()
{
    m_label = NULL;
}

void CMacroParamsPanel::CreateControls()
{ 
    wxBoxSizer* bs_vertical = new wxBoxSizer(wxVERTICAL);
    SetSizer(bs_vertical);

    m_PropGridCtrl = new wxPropertyGrid ( this, wxID_ANY, wxDefaultPosition, wxSize(380, -1), wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE );
    bs_vertical->Add(m_PropGridCtrl, 1, wxEXPAND | wxALL, 5);
    m_PropGridCtrl->Connect( wxEVT_PG_CHANGED, wxPropertyGridEventHandler( CMacroParamsPanel::OnPropertyChanged ), NULL, this );
}

bool CMacroParamsPanel::ShowToolTips()
{
    return true;
}

wxBitmap CMacroParamsPanel::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon CMacroParamsPanel::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

void CMacroParamsPanel::UpdateMacro()
{
    if (!m_label)
	return;
    SetMacro(m_label->GetMacro(), m_label);
}

void CMacroParamsPanel::CommitChanges()
{
    m_PropGridCtrl->CommitChangesFromEditor();
}

void CMacroParamsPanel::SetMacro(CRef<macro::CMacroRep> macro, CMacroLabel *label)
{    
    CommitChanges();
    m_macro = macro;
    m_label = label;
    m_PropGridCtrl->Clear();
    m_props.clear();
    if (!m_macro)
    {
        Refresh();
        return;
    }
    int blk; // var block number in macro
    macro::IMacroVar* var = m_macro->GetFirstVar(blk);
    wxPGProperty* property;
    while (var)  
    {
        property = NULL;
        if (var->IsGUIResolvable()) 
        {
            string value = var->GetGUIResolvedValue();
            macro::CMacroVarAsk* pAsk = dynamic_cast<macro::CMacroVarAsk*>(var);
            if (pAsk) 
            {
                property = new wxStringProperty ( pAsk->GetName(), wxPG_LABEL, value );
            } 
            else 
            {
                macro::CMacroVarChoice* pChoice = dynamic_cast<macro::CMacroVarChoice*>(var);
                if (pChoice) 
                {
                    int sel = 0;
                    int i = 0;
                    wxArrayString tchoices;
                    const string* stored_value = pChoice->GetFirstChoice();
                    while (stored_value) 
                    {
                        tchoices.Add(wxString(*stored_value));
                        if (*stored_value == value)
                            sel = i;
                        i++;
                        stored_value = pChoice->GetNextChoice();
                    }
                    property = new wxEnumProperty ( wxString(pChoice->GetName()), wxPG_LABEL, tchoices,  wxArrayInt(), sel );
                }
            }
            if (property)
            {
                m_PropGridCtrl->Append(property);
                m_props.push_back(make_pair(var, property));
            }
        }
        var = m_macro->GetNextVar(blk);
    }   
    Refresh();
}

void CMacroParamsPanel::OnPropertyChanged( wxPropertyGridEvent& event )
{
    if (!m_macro || !m_label)
	return;
    map<string, string> old_to_new;
    map<string, string> var_to_new;
    map<string, string> var_to_old;
    for (auto &p : m_props)
    {
        macro::IMacroVar* var = p.first;
        wxPGProperty* property = p.second;
        if (var && property)
        {
	    string old = var->GetGUIResolvedValue();
	    string value = property->GetValueAsString().ToStdString();
            var->SetGUIResolvedValue(value);
	    old_to_new[old] = value;
	    var_to_new[var->GetName()] = value;
	    var_to_old[var->GetName()] = old;
        }
    }
    string title = m_macro->GetTitle();
    string old_title = title;
    for (auto p : old_to_new)
    {
	NStr::ReplaceInPlace(title, "'" + p.first + "'", "'" + p.second + "'");
    }
    if (title != old_title)
    {
	m_macro->SetTitle(title);
    }

    string new_source;
    string source = m_macro->GetSource();
    bool header = true;
    bool is_var = false;
    CNcbiIstrstream istr(source.c_str());
    CStreamLineReader line_reader(istr);
    do {
        string str = *++line_reader;
        NStr::TruncateSpacesInPlace(str);
	if (header)
	{
	    NStr::ReplaceInPlace(str, old_title, title);
	    header = false;
	    new_source += str + "\n";
	    continue;
	}
        if (str.empty())
            continue;   
	if (str == "VAR")
	{
	    is_var = true;
	    new_source += str + "\n";
	    continue;
	}
	if (NStr::StartsWith(str, "FOR EACH", NStr::eNocase))
	{
	    is_var = false;
	    new_source += str + "\n";
	    continue;
	}
	if (!is_var)
	{
	    new_source += str + "\n";
	    continue;
	}
        list<string> row_values;
        NStr::Split(str, "=", row_values, NStr::fSplit_MergeDelimiters);
        if (row_values.size() == 2) 
	{
            string var = row_values.front();
            string value = row_values.back();
	    NStr::TruncateSpacesInPlace(var);
	    NStr::TruncateSpacesInPlace(value);
            if (var.empty())
                continue;
	    if (var_to_new.find(var) != var_to_new.end())
		NStr::ReplaceInPlace(value,  "%" + var_to_old[var] + "%", "%" + var_to_new[var] + "%");
	    str = var + " = " + value;
        }
	new_source += str + "\n";
    } while (!line_reader.AtEOF());
    if (source != new_source)
    {
	m_macro->SetSource(new_source);
	m_label->UpdateText();
    }
}

END_NCBI_SCOPE
// TODO
// Add buttons to toolbar


