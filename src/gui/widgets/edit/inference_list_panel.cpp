/*  $Id: inference_list_panel.cpp 41038 2018-05-11 19:59:47Z filippov $
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
 * Authors:  Colleen Bollin
 */


// Generated by DialogBlocks (unregistered), 13/12/2011 15:28:48

#include <ncbi_pch.hpp>
////@begin includes
#include <wx/dialog.h>
////@end includes

#include "inference_list_panel.hpp"
#include <gui/widgets/edit/inference_panel.hpp>
#include <objects/seqfeat/Seq_feat.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CInferenceListPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CInferenceListPanel, CFormattedQualPanel )


/*!
 * CInferenceListPanel event table definition
 */

BEGIN_EVENT_TABLE( CInferenceListPanel, CFormattedQualPanel )

////@begin CInferenceListPanel event table entries
////@end CInferenceListPanel event table entries

EVT_HYPERLINK(wxID_ANY, CInferenceListPanel::OnDelete)

END_EVENT_TABLE()


/*!
 * CInferenceListPanel constructors
 */

CInferenceListPanel::CInferenceListPanel()
{
    Init();
}

CInferenceListPanel::CInferenceListPanel( wxWindow* parent, CSerialObject& object, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_AllowUpdates = true;
    m_Object = dynamic_cast<CSeq_feat*>(&object);
    const CSeq_feat& seqfeat = dynamic_cast<const CSeq_feat&>(*m_Object);
    m_EditedFeat.Reset((CSerialObject*)CSeq_feat::GetTypeInfo()->Create());
    m_EditedFeat->Assign(seqfeat);
    Create(parent, id, pos, size, style);
}


/*!
 * CInferenceListPanel creator
 */

bool CInferenceListPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CInferenceListPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CInferenceListPanel creation
    return true;
}


/*!
 * CInferenceListPanel destructor
 */

CInferenceListPanel::~CInferenceListPanel()
{
////@begin CInferenceListPanel destruction
////@end CInferenceListPanel destruction
}


/*!
 * Member initialisation
 */

void CInferenceListPanel::Init()
{
////@begin CInferenceListPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CInferenceListPanel member initialisation
}


/*!
 * Control creation for CInferenceListlPanel
 */

void CInferenceListPanel::CreateControls()
{    
////@begin CInferenceListPanel content construction
    // Generated by DialogBlocks, 30/12/2011 12:07:48 (unregistered)

    CInferenceListPanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCFormattedQualPanel1, wxID_STATIC, _("Category"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCFormattedQualPanel1, wxID_STATIC, _("Type"), wxDefaultPosition, wxSize(120, -1), 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemCFormattedQualPanel1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxSize(300, 300), wxSUNKEN_BORDER|wxVSCROLL|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ScrolledWindow, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->FitInside();

////@end CInferenceListPanel content construction
}


void CInferenceListPanel::x_AddInference(string value)
{
    CInferencePanel* panel = new CInferencePanel(m_ScrolledWindow);
    m_Sizer->Add(panel);
    panel->SetValue (value);
    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( m_ScrolledWindow, wxID_ANY, 
							      wxT("Delete"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_Sizer->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    x_AdjustForControl(panel, itemHyperlinkCtrl);
}


bool CInferenceListPanel::x_SetInferences(bool keep_blanks)
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    m_AllowUpdates = false;

    m_TotalHeight = 0;
    m_TotalWidth = 0;
    m_ScrollRate = 0;
    m_MinHeight = 0;
    m_NumRows = 0;
    bool add_last_blank = true;

    Freeze();
    m_Sizer->Clear(true);
    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    if (edited_feat.IsSetQual()) {
        CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
        while (qual != edited_feat.SetQual().end()) {
	    if ((*qual)->IsSetQual() && NStr::EqualNocase((*qual)->GetQual(), "inference")) {
		string value = "";
		if ((*qual)->IsSetVal()) {
		    value = (*qual)->GetVal();
		}
		if (keep_blanks || !NStr::IsBlank(value)) {
		    x_AddInference(value);
		    if (NStr::IsBlank(value)) {
                        add_last_blank = false;
		    } else {
			add_last_blank = true;
		    }
		}
	    }
            qual++;
	}
    }
    // add blank
    if (add_last_blank) {
        x_AddInference("");
    }

    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
    m_ScrolledWindow->SetMinSize(wxSize(m_TotalWidth + 20, 4 * m_MinHeight));
    Thaw();
    m_AllowUpdates = true;
    return true;
}


bool CInferenceListPanel::TransferDataToWindow()
{
    return x_SetInferences();
}



bool CInferenceListPanel::x_GetInferences(bool keep_blanks, int skip_index)
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    edited_feat.ResetQual();

    int pos = 0;
    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        CInferencePanel* singlequal_panel = dynamic_cast<CInferencePanel*> (w);
        if (singlequal_panel) {
            if (pos != skip_index) {
                string value = singlequal_panel->GetValue();
                if (keep_blanks || !NStr::IsBlank(value)) {
                    CRef<CGb_qual> edited_qual (new CGb_qual("inference", value));
                    edited_feat.SetQual().push_back(edited_qual);
                }
            }
            pos++;
        }
    }
    return true;
}


bool CInferenceListPanel::TransferDataFromWindow()
{
    return x_GetInferences();
}


void CInferenceListPanel::PopulateGBQuals(objects::CSeq_feat& seq_feat)
{
    TransferDataFromWindow();
    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    RemoveRepresentedQuals(seq_feat, "inference");

    for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
         qual != edited_feat.SetQual().end();
         qual++) {
        if ((*qual)->IsSetQual() && !NStr::IsBlank((*qual)->GetQual())
	    && NStr::EqualNocase((*qual)->GetQual(), "inference")
            && (*qual)->IsSetVal() && !NStr::IsBlank((*qual)->GetVal())) {
            CRef<CGb_qual> new_qual(new CGb_qual());
            new_qual->Assign(**qual);

            seq_feat.SetQual().push_back(new_qual);
        }
    }
}


/*!
 * Should we show tooltips?
 */

bool CInferenceListPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CInferenceListPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CInferenceListPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CInferenceListPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CInferenceListPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CInferenceListPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CInferenceListPanel icon retrieval
}


void CInferenceListPanel::OnChildChange (wxWindow* wnd)
{
    if (!m_AllowUpdates) {
        return;
    }
    // is the row being changed the last one?
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    unsigned int num_items = itemList.size();
    if (num_items > 1 && wnd == (itemList[num_items - 2])->GetWindow()) {
	x_GetInferences(true);
	x_SetInferences(true);
    }

    m_ScrolledWindow->Layout();
    m_ScrolledWindow->Refresh();
    wxWindow *win = GetParent();
    while (win)
    {
	wxFrame *frame = dynamic_cast<wxFrame*>(win);
	wxDialog *dialog = dynamic_cast<wxDialog*>(win);
	if (frame || dialog)
	{
	    if (win->GetSizer())
	    {
		win->GetSizer()->SetSizeHints(this);
	    }
	    win->Layout();
	    win->Refresh();
	    break;
	}
	win = win->GetParent();
    }
}


int CInferenceListPanel::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return -1;

    // start with -1, first qual found will bump it to 0
    int row_num = -1;
    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) {
        wxWindow* child = (**it).GetWindow();
        if (child && child == wnd)
            return row_num;
        
        CFormattedQualPanel* qual = dynamic_cast<CFormattedQualPanel*> (child);
	if (qual) {
	    row_num++;
	}
    }

    return -1;
}


void CInferenceListPanel::OnDelete (wxHyperlinkEvent& event)
{
    // find the control, remove it and the item before it, reset the scrolling
    wxWindow* wnd = (wxWindow*)event.GetEventObject();
    if (wnd == NULL) {
	return;
    }
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    int row = x_FindRow (wnd, itemList);
    if (row > -1) {
	x_GetInferences(true, row);
        x_SetInferences(true);
    }
    m_ScrolledWindow->Layout();
    m_ScrolledWindow->Refresh();
    wxWindow *win = GetParent();
    while (win)
    {
	wxFrame *frame = dynamic_cast<wxFrame*>(win);
	wxDialog *dialog = dynamic_cast<wxDialog*>(win);
	if (frame || dialog)
	{
	    if (win->GetSizer())
	    {
		win->GetSizer()->SetSizeHints(this);
	    }
	    win->Layout();
	    win->Refresh();
	    break;
	}
	win = win->GetParent();
    }
}



END_NCBI_SCOPE
