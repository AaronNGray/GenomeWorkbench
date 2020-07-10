/*  $Id: cds_exception_panel.cpp 38073 2017-03-23 16:34:10Z filippov $
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
 * Authors:  Roman Katargin
 */


// Generated by DialogBlocks (unregistered), 22/12/2011 15:10:59

#include <ncbi_pch.hpp>
////@begin includes
////@end includes

#include "cds_exception_panel.hpp"
#include "single_cds_exception_panel.hpp"


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CCDSExceptionPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCDSExceptionPanel, CQualListItemPanel )


/*!
 * CCDSExceptionPanel event table definition
 */

BEGIN_EVENT_TABLE( CCDSExceptionPanel, CQualListItemPanel )

////@begin CCDSExceptionPanel event table entries
////@end CCDSExceptionPanel event table entries
    EVT_HYPERLINK(wxID_ANY, CCDSExceptionPanel::OnDelete)

END_EVENT_TABLE()


/*!
 * CCDSExceptionPanel constructors
 */

CCDSExceptionPanel::CCDSExceptionPanel()
{
    Init();
}

CCDSExceptionPanel::CCDSExceptionPanel( wxWindow* parent, CSeq_feat& feat, CScope& scope,
                                        wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_Scope(&scope), m_EditedFeat(new CSeq_feat())

{
    _ASSERT(feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion);
    Init();
    m_EditedFeat->Assign (feat);
    m_AllowUpdates = true;
    Create(parent, id, pos, size, style);
}


/*!
 * CCDSExceptionPanel creator
 */

bool CCDSExceptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCDSExceptionPanel creation
    CQualListItemPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCDSExceptionPanel creation
    return true;
}


/*!
 * CCDSExceptionPanel destructor
 */

CCDSExceptionPanel::~CCDSExceptionPanel()
{
////@begin CCDSExceptionPanel destruction
////@end CCDSExceptionPanel destruction
}


/*!
 * Member initialisation
 */

void CCDSExceptionPanel::Init()
{
////@begin CCDSExceptionPanel member initialisation
    m_LabelSizer = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CCDSExceptionPanel member initialisation
}


/*!
 * Control creation for CCDSExceptionPanel
 */

void CCDSExceptionPanel::CreateControls()
{    
////@begin CCDSExceptionPanel content construction
    // Generated by DialogBlocks, 27/12/2011 11:29:33 (unregistered)

    CCDSExceptionPanel* itemCQualListItemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCQualListItemPanel1->SetSizer(itemBoxSizer2);

    m_LabelSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_LabelSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCQualListItemPanel1, wxID_STATIC, _("AA"), wxDefaultPosition, wxSize(40, -1), 0 );
    m_LabelSizer->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCQualListItemPanel1, wxID_STATIC, _("Amino Acid"), wxDefaultPosition, wxSize(100, -1), 0 );
    m_LabelSizer->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCQualListItemPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(30, -1), 0 );
    m_LabelSizer->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ScrolledWindow = new wxScrolledWindow( itemCQualListItemPanel1, ID_SCROLLEDWINDOW4, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_ScrolledWindow, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->FitInside();

////@end CCDSExceptionPanel content construction
    m_ScrollRate = 0;
    m_TotalHeight = 0;
    m_TotalWidth = 0;
    m_ScrollRate = 0;
    m_MinHeight = 0;
    m_MinWidth = 0;
}


void CCDSExceptionPanel::x_AddRow(CRef<CCode_break> code_break)
{
    CSingleCDSExceptionPanel* row = new CSingleCDSExceptionPanel(m_ScrolledWindow, *code_break, *m_EditedFeat, *m_Scope);
    m_Sizer->Add(row, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    row->TransferDataToWindow();

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( m_ScrolledWindow, wxID_ANY, 
              wxT("Delete"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_Sizer->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    x_AdjustForControl (row, itemHyperlinkCtrl);
}


static bool s_IsEmpty (CRef<CCode_break> code_break)
{
    if (!code_break) {
        return true;
    } else if (code_break->IsSetAa() && code_break->GetAa().Which() != CCode_break::C_Aa::e_not_set) {
        return false;
    } else if (code_break->IsSetLoc() && code_break->GetLoc().Which() != CSeq_loc::e_not_set) {
        return false;
    } else {
        return true;
    }
}


bool CCDSExceptionPanel::TransferDataToWindow()
{
    m_AllowUpdates = false;
    Freeze();
    bool need_last_blank = true;

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator ctrl_it = itemList.begin();
    size_t ctrl_pos = 0;
    CCdregion& cds = m_EditedFeat->SetData().SetCdregion();
    if (cds.IsSetCode_break()) {
        for (CCdregion::TCode_break::iterator x = cds.SetCode_break().begin();
             x != cds.SetCode_break().end();
             x++) {
              CSingleCDSExceptionPanel* transl_except = NULL;
              while (transl_except == NULL && ctrl_it != itemList.end()) {
                    transl_except = dynamic_cast<CSingleCDSExceptionPanel*> ((**ctrl_it).GetWindow());
                    ctrl_it++;
                    ctrl_pos++;
              }

              if (transl_except == NULL) {
                x_AddRow (*x);
                ctrl_pos += 2;
              } else {
                transl_except->SetCodeBreak(*x);
                transl_except->TransferDataToWindow();
                ctrl_it++;
                ctrl_pos++;
              }
            
            if (s_IsEmpty(*x)) {
                need_last_blank = false;
            } else {
                need_last_blank = true;
            }
        }        
    }
    if (need_last_blank) {
        CRef<CCode_break> last_code_break (new CCode_break());
        CSingleCDSExceptionPanel* transl_except = NULL;
        while (transl_except == NULL && ctrl_it != itemList.end()) {
            transl_except = dynamic_cast<CSingleCDSExceptionPanel*> ((**ctrl_it).GetWindow());
            ctrl_it++;
            ctrl_pos++;
        }

        if (transl_except == NULL) {
            x_AddRow (last_code_break);
            ctrl_pos += 2;
        } else {
            transl_except->SetCodeBreak(last_code_break);
            ctrl_it++;
            ctrl_pos++;
        }
    }

    // remove extra blanks (or blank out?) 
    while (itemList.size() > ctrl_pos) {
        m_Sizer->GetItem(ctrl_pos)->DeleteWindows();
        m_Sizer->Remove(ctrl_pos);
    }

    int win_height, win_width;

    m_ScrolledWindow->SetVirtualSize(m_TotalWidth, m_TotalHeight);
    m_ScrolledWindow->GetSize(&win_width, &win_height);
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
    m_ScrolledWindow->GetSize(&win_width, &win_height);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->GetSize(&win_width, &win_height);
    Layout();

    int display_rows = 12;

//    m_ScrolledWindow->SetSize(m_TotalWidth + 20, display_rows * m_ScrollRate);
    m_ScrolledWindow->SetMinSize(wxSize(m_TotalWidth + 20, display_rows * m_ScrollRate));
    Layout();
    Thaw();
    if (!wxPanel::TransferDataToWindow())
        return false;
    m_AllowUpdates = true;
    return true;
}


bool CCDSExceptionPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }

    CCdregion& edit_cds = m_EditedFeat->SetData().SetCdregion();
    edit_cds.ResetCode_break();
    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    while (node != m_Sizer->GetChildren().end()) {
        CSingleCDSExceptionPanel* row = dynamic_cast< CSingleCDSExceptionPanel* >((*node)->GetWindow());
        if (row) {
            row->TransferDataFromWindow();
            CRef<CCode_break> code_break = row->GetCodeBreak();
            edit_cds.SetCode_break().push_back(code_break);
        }
        ++node;
    }
    return true;
}


void CCDSExceptionPanel::SetExceptions(objects::CCdregion& cds)
{
    // set exceptions
    cds.ResetCode_break();
    CCdregion& edit_cds = m_EditedFeat->SetData().SetCdregion();
    if (edit_cds.IsSetCode_break()) {
        for (CCdregion::TCode_break::iterator x = edit_cds.SetCode_break().begin();
             x != edit_cds.SetCode_break().end();
             x++) {
             if (((*x)->IsSetAa() && (*x)->GetAa().Which() != CCode_break::C_Aa::e_not_set) && ((*x)->IsSetLoc() && (*x)->GetLoc().Which() != CSeq_loc::e_not_set)) {
                CRef<CCode_break> code_break(new CCode_break());
                code_break->Assign(**x);
                cds.SetCode_break().push_back(code_break);
            }
        }        
    }

}


void CCDSExceptionPanel::OnChildChange (wxWindow* wnd)
{
    if (!m_AllowUpdates) {
        return;
    }
    // is the row being changed the last one?
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    unsigned int num_items = itemList.size();
    if (num_items > 1 && wnd == (itemList[num_items - 2])->GetWindow()) {
        TransferDataFromWindow();
        TransferDataToWindow();
    }
}


/*!
 * Should we show tooltips?
 */

bool CCDSExceptionPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCDSExceptionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCDSExceptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCDSExceptionPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCDSExceptionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCDSExceptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCDSExceptionPanel icon retrieval
}

int CCDSExceptionPanel::x_FindRow(wxWindow* wnd, wxSizerItemList& itemList)
{
    if (NULL == wnd)
        return -1;

    // start with -1, first qual found will bump it to 0
    int row_num = -1;
    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) {
        wxWindow* child = (**it).GetWindow();
        if (child && child == wnd)
            return row_num;
        
        CSingleCDSExceptionPanel* qual = dynamic_cast<CSingleCDSExceptionPanel*> (child);
        if (qual) {
            row_num++;
        }
    }

    return -1;
}


void CCDSExceptionPanel::OnDelete (wxHyperlinkEvent& event)
{
    // find the control, remove it and the item before it, reset the scrolling
    wxWindow* wnd = (wxWindow*)event.GetEventObject();
    if (wnd == NULL) {
        return;
    }

    CCdregion& edit_cds = m_EditedFeat->SetData().SetCdregion();
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    int row = x_FindRow (wnd, itemList);
    if (row > -1) {
        TransferDataFromWindow();
        if (edit_cds.IsSetCode_break()) {
            int j = 0;
            CCdregion::TCode_break::iterator x = edit_cds.SetCode_break().begin();
            while (j != row && x != edit_cds.SetCode_break().end()) {
                    x++;
                    j++;
            }
            if (x != edit_cds.SetCode_break().end()) {
                edit_cds.SetCode_break().erase(x);
            }
            TransferDataToWindow();
        }
    }
}

END_NCBI_SCOPE
