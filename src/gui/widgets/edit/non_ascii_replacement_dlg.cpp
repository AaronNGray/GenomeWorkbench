/*  $Id: non_ascii_replacement_dlg.cpp 40562 2018-03-09 14:59:21Z filippov $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Igor Filippov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/doi_lookup.hpp>
#include <gui/widgets/edit/non_ascii_replacement_dlg.hpp>


BEGIN_NCBI_SCOPE




static wxString GetContext(const wxString &str, int pos)
{
    if (str.Length() <= 20)
        return str;
    pos -= 10;
    if (pos < 0)
        pos = 0;
    if (pos + 20 <= str.Length())
        return str.Mid(pos,20);
    return str.Mid(str.Length() - 20);
}

static void TestWindowForNonAsciiText(wxWindow *win, map<TUnicodeSymbol, string> &nonAsciiChars,  map<TUnicodeSymbol, set<wxString> > &nonAsciiContext)
{
    if (!win)
        return;
    wxTextCtrl *text_ctrl = dynamic_cast<wxTextCtrl *>(win);
    if (text_ctrl)
    {
        wxString str = text_ctrl->GetValue();
        size_t pos = 0;
        for (wxString::const_iterator i = str.begin(); i != str.end(); ++i)
        {
            wxUniChar u = *i;
            if (!u.IsAscii())
            {
				TUnicodeSymbol v = u.GetValue();
                nonAsciiChars[v] = CDoiLookup::GetSpecialCharacterReplacement(v);
                nonAsciiContext[v].insert(GetContext(str,pos));
            }
            pos++;
        }
    }
    else
    {
        for (wxWindowList::iterator node = win->GetChildren().begin(); node != win->GetChildren().end(); ++node) 
        {
            wxWindow *w = *node;
            TestWindowForNonAsciiText(w, nonAsciiChars, nonAsciiContext);
        }
    }
}

static void FixWindowForNonAsciiText(wxWindow *win, map<TUnicodeSymbol, string> &nonAsciiChars)
{
    if (!win)
        return;
    wxTextCtrl *text_ctrl = dynamic_cast<wxTextCtrl *>(win);
    if (text_ctrl)
    {
        wxString str = text_ctrl->GetValue();
        wxString new_str;
        bool modified = false;
        for (wxString::const_iterator i = str.begin(); i != str.end(); ++i)
        {
            wxUniChar u = *i;
            if (!u.IsAscii())
            {
				TUnicodeSymbol v = u.GetValue();
                string replacement = nonAsciiChars[v];
                new_str << replacement;
                modified = true;
            }
            else
            {
                new_str << u;
            }
        }
        if (modified)
            text_ctrl->SetValue(new_str);
    }
    else
    {
        for (wxWindowList::iterator node = win->GetChildren().begin(); node != win->GetChildren().end(); ++node) 
        {
            wxWindow *w = *node;
            FixWindowForNonAsciiText(w, nonAsciiChars);
        }
    }
}

void TestForNonAsciiText(wxWindow *win)
{
    map<TUnicodeSymbol, string> nonAsciiChars;
    map<TUnicodeSymbol, set<wxString> > nonAsciiContext;
    
    TestWindowForNonAsciiText(win, nonAsciiChars, nonAsciiContext);
    if (!nonAsciiChars.empty())
    {	                      
        CNonAsciiCharacterReplacement dlg(NULL, nonAsciiChars, nonAsciiContext);
        dlg.ShowModal();
        nonAsciiChars = dlg.GetReplacementMap();
        FixWindowForNonAsciiText(win, nonAsciiChars);
    }	
}


/*
 * CNonAsciiCharacterReplacement type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CNonAsciiCharacterReplacement, wxDialog )


/*
 * CNonAsciiCharacterReplacement event table definition
 */

BEGIN_EVENT_TABLE( CNonAsciiCharacterReplacement, wxDialog )

////@begin CNonAsciiCharacterReplacement event table entries
////@end CNonAsciiCharacterReplacement event table entries

END_EVENT_TABLE()


/*
 * CNonAsciiCharacterReplacement constructors
 */

CNonAsciiCharacterReplacement::CNonAsciiCharacterReplacement()
{
    Init();
}

CNonAsciiCharacterReplacement::CNonAsciiCharacterReplacement( wxWindow* parent, const map<TUnicodeSymbol, string> &nonAsciiChars, const map<TUnicodeSymbol, set<wxString> > &nonAsciiContext,
                                                              wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_NonAsciiChars(nonAsciiChars), m_NonAsciiContext(nonAsciiContext)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CNonAsciiCharacterReplacement creator
 */

bool CNonAsciiCharacterReplacement::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CNonAsciiCharacterReplacement creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CNonAsciiCharacterReplacement creation
    return true;
}


/*
 * CNonAsciiCharacterReplacement destructor
 */

CNonAsciiCharacterReplacement::~CNonAsciiCharacterReplacement()
{
////@begin CNonAsciiCharacterReplacement destruction
////@end CNonAsciiCharacterReplacement destruction
}


/*
 * Member initialisation
 */

void CNonAsciiCharacterReplacement::Init()
{
////@begin CNonAsciiCharacterReplacement member initialisation
    m_ScrolledWindow = NULL;
////@end CNonAsciiCharacterReplacement member initialisation
}


/*
 * Control creation for CNonAsciiCharacterReplacement
 */

void CNonAsciiCharacterReplacement::CreateControls()
{    
////@begin CNonAsciiCharacterReplacement content construction
    // Generated by DialogBlocks, 20/01/2016 17:14:51 (unregistered)

    CNonAsciiCharacterReplacement* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("You may not include special characters in the text.If you do not choose replacement characters, these special characters will be replaced with '#'."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->Wrap(270);
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, 0, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Character"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Replacement"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Contexts"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemDialog1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxSize(300, 100), wxSUNKEN_BORDER|wxVSCROLL );
    itemBoxSizer2->Add(m_ScrolledWindow, 0, wxGROW|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(0, 1, 0, 0);
    wxBoxSizer *scrollSizer = new wxBoxSizer(wxVERTICAL);
    m_ScrolledWindow->SetSizer(scrollSizer);

    for (map<TUnicodeSymbol, string>::const_iterator i = m_NonAsciiChars.begin(); i != m_NonAsciiChars.end(); ++i)
    {
        CNonAsciiReplacementPanel *panel = new CNonAsciiReplacementPanel(m_ScrolledWindow, wxUniChar(i->first), i->second, m_NonAsciiContext[i->first]);
        scrollSizer->Add(panel, 0, wxALIGN_LEFT, 0);
    }

    m_ScrolledWindow->FitInside();

    wxButton* itemButton10 = new wxButton( itemDialog1, wxID_OK, _("Replace"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CNonAsciiCharacterReplacement content construction
}


map<TUnicodeSymbol, string> CNonAsciiCharacterReplacement::GetReplacementMap()
{
    TransferDataFromWindow();
    return m_NonAsciiChars;
}

bool CNonAsciiCharacterReplacement::TransferDataFromWindow()
{
    if (m_ScrolledWindow)
    {
        m_NonAsciiChars.clear();
        for (wxWindowList::iterator node = m_ScrolledWindow->GetChildren().begin(); node != m_ScrolledWindow->GetChildren().end(); ++node) 
        {
            wxWindow *w = *node;
            CNonAsciiReplacementPanel *panel = dynamic_cast<CNonAsciiReplacementPanel*>(w);
            if (panel)
            {
                pair<TUnicodeSymbol, string> p = panel->GetReplacement();
                m_NonAsciiChars[p.first] = p.second;
            }
        }
    }
    return wxDialog::TransferDataFromWindow();
}

/*
 * Should we show tooltips?
 */

bool CNonAsciiCharacterReplacement::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CNonAsciiCharacterReplacement::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNonAsciiCharacterReplacement bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNonAsciiCharacterReplacement bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CNonAsciiCharacterReplacement::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNonAsciiCharacterReplacement icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNonAsciiCharacterReplacement icon retrieval
}



/*
 * CNonAsciiReplacementPanel type definition
 */

IMPLEMENT_CLASS( CNonAsciiReplacementPanel, wxPanel )


/*
 * CNonAsciiReplacementPanel event table definition
 */

BEGIN_EVENT_TABLE( CNonAsciiReplacementPanel, wxPanel )

////@begin CNonAsciiReplacementPanel event table entries
////@end CNonAsciiReplacementPanel event table entries

END_EVENT_TABLE()


/*
 * CNonAsciiReplacementPanel constructors
 */

CNonAsciiReplacementPanel::CNonAsciiReplacementPanel()
{
    Init();
}

CNonAsciiReplacementPanel::CNonAsciiReplacementPanel( wxWindow* parent,  wxUniChar orig, const string &replacement, const set<wxString> &contexts, 
                                                      wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Orig(orig), m_ReplacementStr(wxString(replacement)), m_Contexts(contexts)
{
    Init();
    Create( parent, id, pos, size, style );
}


/*
 * CNonAsciiReplacementPanel creator
 */

bool CNonAsciiReplacementPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CNonAsciiReplacementPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CNonAsciiReplacementPanel creation
    return true;
}


/*
 * CNonAsciiReplacementPanel destructor
 */

CNonAsciiReplacementPanel::~CNonAsciiReplacementPanel()
{
////@begin CNonAsciiReplacementPanel destruction
////@end CNonAsciiReplacementPanel destruction
}


/*
 * Member initialisation
 */

void CNonAsciiReplacementPanel::Init()
{
////@begin CNonAsciiReplacementPanel member initialisation
    m_Char = NULL;
    m_Replacement = NULL;
////@end CNonAsciiReplacementPanel member initialisation
}


/*
 * Control creation for CNonAsciiReplacementPanel
 */

void CNonAsciiReplacementPanel::CreateControls()
{    
////@begin CNonAsciiReplacementPanel content construction
    // Generated by DialogBlocks, 20/01/2016 17:26:08 (unregistered)

    CNonAsciiReplacementPanel* itemPanel2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel2->SetSizer(itemBoxSizer3);

    m_Char = new wxStaticText( itemPanel2, wxID_STATIC, m_Orig, wxDefaultPosition, wxSize(30,-1), 0 );
    itemBoxSizer3->Add(m_Char, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT|wxTOP, 5);

    m_Replacement = new wxTextCtrl( itemPanel2, wxID_ANY, m_ReplacementStr, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Replacement, 0, wxALIGN_TOP|wxRIGHT|wxTOP, 5);

    wxArrayString array_context;
    for (set<wxString>::const_iterator i = m_Contexts.begin(); i != m_Contexts.end(); ++i)
        array_context.Add(*i);
    wxChoice* itemChoice6 = new wxChoice( itemPanel2, wxID_ANY, wxDefaultPosition, wxSize(120,-1), array_context, 0 );
    itemBoxSizer3->Add(itemChoice6, 0, wxALIGN_TOP|wxRIGHT|wxTOP, 5);
    itemChoice6->SetSelection(0);
////@end CNonAsciiReplacementPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CNonAsciiReplacementPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CNonAsciiReplacementPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNonAsciiReplacementPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNonAsciiReplacementPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CNonAsciiReplacementPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNonAsciiReplacementPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNonAsciiReplacementPanel icon retrieval
}

pair<TUnicodeSymbol, string> CNonAsciiReplacementPanel::GetReplacement()
{
	TUnicodeSymbol orig = m_Char->GetLabel().Last().GetValue();
    string replacement = m_Replacement->GetValue().ToStdString();
    pair<TUnicodeSymbol, string> result(orig, replacement);
    return result;
}

END_NCBI_SCOPE
