/*  $Id: identifiers_panel.cpp 41991 2018-11-28 18:39:32Z asztalos $
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


#include <ncbi_pch.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/general/Object_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objmgr/util/feature.hpp>

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/toplevel.h>
#include <gui/widgets/wx/hyperlink.hpp>

#include "identifiers_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CIdentifiersPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CIdentifiersPanel, wxPanel )


/*!
 * CIdentifiersPanel event table definition
 */

BEGIN_EVENT_TABLE( CIdentifiersPanel, wxPanel )
    EVT_HYPERLINK(wxID_ANY, CIdentifiersPanel::OnDelete)
END_EVENT_TABLE()


/*!
 * CIdentifiersPanel constructors
 */

CIdentifiersPanel::CIdentifiersPanel()
{
    Init();
}

CIdentifiersPanel::CIdentifiersPanel( wxWindow* parent, CSeq_feat& feat, CRef<CScope> scope, 
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_EditedFeat = new CSeq_feat();
    m_EditedFeat->Assign(feat);
    m_Scope = scope;
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CIdentifiersPanel creator
 */

bool CIdentifiersPanel::Create( wxWindow* parent, wxWindowID id, 
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CIdentifiersPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CIdentifiersPanel creation
    return true;
}


/*!
 * CIdentifiersPanel destructor
 */

CIdentifiersPanel::~CIdentifiersPanel()
{
}


/*!
 * Member initialisation
 */

void CIdentifiersPanel::Init()
{
    m_FeatureIdCtrl = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
}


/*!
 * Control creation for CIdentifiersPanel
 */

void CIdentifiersPanel::CreateControls()
{    
////@begin CIdentifiersPanel content construction
    CIdentifiersPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Feature ID for this feature"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FeatureIdCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL25, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_FeatureIdCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("ID Xref to associated features (* indicates non-reciprocal link)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(500, 100), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_ScrolledWindow, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxBoxSizer(wxVERTICAL);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->FitInside();

////@end CIdentifiersPanel content construction
}

void CIdentifiersPanel::AddXref(const string &value)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_Sizer->Add(sizer, 0, wxALIGN_LEFT, 0);

    wxTextCtrl *xref = new wxTextCtrl( m_ScrolledWindow, wxID_ANY, wxString(value), wxDefaultPosition, wxDefaultSize, 0 );
    sizer->Add(xref, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT, 5);
    xref->Bind(wxEVT_TEXT, &CIdentifiersPanel::OnNewText, this);

    wxHyperlinkCtrl* del = new CHyperlink( m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT(""), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    sizer->Add(del, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    wxStaticText* id = new wxStaticText(m_ScrolledWindow, wxID_STATIC,  wxT("identifier"), wxDefaultPosition, wxDefaultSize, 0 );
    sizer->Add(id, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CIdentifiersPanel::GetIdXrefs()
{
    m_Sizer->Clear(true);
    if (m_EditedFeat->IsSetXref()) 
    {
        ITERATE (CSeq_feat::TXref, xit, m_EditedFeat->GetXref()) 
        { 
            if ((*xit)->IsSetId() && (*xit)->GetId().IsLocal()) 
            {
                if ((*xit)->GetId().GetLocal().IsStr()) 
                {
                    string xref = (*xit)->GetId().GetLocal().GetStr();
                    AddXref(xref);
                } 
                else if ((*xit)->GetId().GetLocal().IsId()) 
                {
                    string xref = NStr::NumericToString((*xit)->GetId().GetLocal().GetId());
                    AddXref(xref);
                }
            }
        }
    }
    AddXref(kEmptyStr);
    x_UpdateXrefDescriptions();
    m_ScrolledWindow->FitInside();
}


bool CIdentifiersPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }

    if (m_EditedFeat->IsSetId() && m_EditedFeat->GetId().IsLocal()) {
        if (m_EditedFeat->GetId().GetLocal().IsStr()) {
            m_FeatureIdCtrl->SetValue(ToWxString(m_EditedFeat->GetId().GetLocal().GetStr()));
        } else if (m_EditedFeat->GetId().GetLocal().IsId()) {
            m_FeatureIdCtrl->SetValue(ToWxString(NStr::NumericToString(m_EditedFeat->GetId().GetLocal().GetId())));
        } else {
            m_FeatureIdCtrl->SetValue(wxEmptyString);
        }          
    } else {
        m_FeatureIdCtrl->SetValue(wxEmptyString);
    }

    GetIdXrefs();
    return true;
}


static void s_SetFeatId (CFeat_id& feat_id, string val)
{
    int nval = -1;
    try {
	    nval = NStr::StringToInt(val);  
    } catch (const CStringException&) {}
    
    if (nval < 0) {
        feat_id.SetLocal().SetStr(val);
    } else {
        feat_id.SetLocal().SetId(nval);
    }
}

void CIdentifiersPanel::SetIdXrefs(CSeq_feat& seq_feat)
{
    vector<CConstRef<CFeat_id>> present_featids;
    if (seq_feat.IsSetXref()) 
    {
        EDIT_EACH_SEQFEATXREF_ON_SEQFEAT(xit, seq_feat) 
        { 
            if ((*xit)->IsSetId() && (*xit)->GetId().IsLocal()) 
            {
                const CFeat_id& featid = (*xit)->GetId();
                if ((*xit)->IsSetData() && (*xit)->GetData().IsGene()) {
                    present_featids.push_back(ConstRef(&featid));
                }
                if (featid.GetLocal().IsStr())  {
                    ERASE_SEQFEATXREF_ON_SEQFEAT(xit, seq_feat);
                } 
                else if (featid.GetLocal().IsId()) {
                    ERASE_SEQFEATXREF_ON_SEQFEAT(xit, seq_feat);
                }
            }
        }
        if (seq_feat.GetXref().empty())
            seq_feat.ResetXref();
    }

    wxWindowList &children = m_ScrolledWindow->GetChildren();
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
    {
        wxTextCtrl* win = dynamic_cast<wxTextCtrl*>(*child);
        if (win)
        {
            string value = win->GetValue().ToStdString();
            if (!NStr::IsBlank(value))
            {
                CRef<CSeqFeatXref> x(new CSeqFeatXref());
                s_SetFeatId (x->SetId(), value);
                bool found = false;
                for (auto&& it : present_featids) {
                    if (x->GetId().Equals(it.GetObject())) {
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    seq_feat.SetXref().push_back(x);
                }
            }
        }
    }
}


bool CIdentifiersPanel::x_IsReciprocalXref(const CSeq_feat& one, const CSeq_feat& two)
{
    if (!one.IsSetId() || !two.IsSetXref()) {
        return false;
    }
    for (auto it : two.GetXref()) {
        if (it->IsSetId() && it->GetId().IsLocal() &&
            one.GetId().Equals(it->GetId())) {
            return true;
        }
    }
    return false;
}


void CIdentifiersPanel::x_UpdateXrefDescriptions()
{
    CRef<CSeqFeatXref> x(NULL);
    wxWindowList &children = m_ScrolledWindow->GetChildren();
    wxHyperlinkCtrl* lastdel = NULL;
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
    {
        wxTextCtrl* win = dynamic_cast<wxTextCtrl*>(*child);
        wxHyperlinkCtrl* del = dynamic_cast<wxHyperlinkCtrl*>(*child);
        wxStaticText* lbl = dynamic_cast<wxStaticText*>(*child);
        if (win) {
            string value = win->GetValue().ToStdString();
            if (NStr::IsBlank(value)) {
                x.Reset(NULL);
            }
            else {
                x.Reset(new CSeqFeatXref());
                s_SetFeatId(x->SetId(), value);
            }
        } else if (del) {
            lastdel = del;
            del->Enable(true);
        } else if (lbl) {
            if (x) {
                vector<CSeq_feat_Handle> handles = m_Scope->GetBioseqHandle(m_EditedFeat->GetLocation()).GetTSE_Handle().GetFeaturesWithId(CSeqFeatData::e_not_set, x->GetId().GetLocal());
                if (handles.size() == 0) {
                    lbl->SetLabel(_("No features with this ID"));
                } else if (handles.size() == 1) {
                    const CSeq_feat& feat = *(handles.front().GetSeq_feat());
                    x_CollectFeatId();
                    if (feat.IsSetId() && m_EditedFeat->IsSetId() &&
                        feat.GetId().Equals(m_EditedFeat->GetId())) {
                        lbl->SetLabel("Feature links to itself!");
                    } else {
                        string content_label;
                        feature::GetLabel(feat, &content_label, feature::fFGL_Both, m_Scope);
                        if (!x_IsReciprocalXref(*m_EditedFeat, feat)) {
                            content_label = "(*)" + content_label;
                        }
                        lbl->SetLabel(ToWxString(content_label));
                    }
                } else {
                    lbl->SetLabel(_("Multiple features with this ID"));
                }
                x.Reset(NULL);
            } else {
                lbl->SetLabel(wxEmptyString);
            }
        }
    }
    if (lastdel) {
        lastdel->Enable(false);
    }
}


bool CIdentifiersPanel::x_CollectFeatId()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }

    string val = ToStdString (m_FeatureIdCtrl->GetValue());
    if (NStr::IsBlank(val)) {
        if (m_EditedFeat->IsSetId() && m_EditedFeat->GetId().IsLocal()) {
            m_EditedFeat->ResetId();
        }
    } else {
        s_SetFeatId (m_EditedFeat->SetId(), val);
    }
    return true;
}


bool CIdentifiersPanel::TransferDataFromWindow()
{
    if (!x_CollectFeatId()) {
      return false;
    }

    SetIdXrefs(*m_EditedFeat);
    return true;
}


void CIdentifiersPanel::PopulateIdentifiers(CSeq_feat& seq_feat)
{
    if (m_EditedFeat->IsSetId()) {
        seq_feat.SetId().Assign(m_EditedFeat->GetId());
    } else {
        seq_feat.ResetId();
    }

    SetIdXrefs(seq_feat);   
}


/*!
 * Should we show tooltips?
 */

bool CIdentifiersPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CIdentifiersPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CIdentifiersPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CIdentifiersPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CIdentifiersPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CIdentifiersPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CIdentifiersPanel icon retrieval
}

void CIdentifiersPanel::OnDelete (wxHyperlinkEvent& event)
{
    wxWindow *win = dynamic_cast<wxWindow*>(event.GetEventObject());
    if (!win)
        return;
    wxWindowList &children = m_ScrolledWindow->GetChildren();
    wxWindow *prev = NULL;
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
    {
        if (*child == win)
        {
            wxSizer *sizer = win->GetContainingSizer();
            win->Destroy();
            prev->Destroy();
            m_Sizer->Remove(sizer);
            m_Sizer->Layout();
            m_ScrolledWindow->FitInside();
            break;
        }
        prev = *child;
    }
}


bool CIdentifiersPanel::x_IsLastXref(wxTextCtrl* win)
{
    wxWindowList::reverse_iterator last_child = m_ScrolledWindow->GetChildren().rbegin();
    while (last_child != m_ScrolledWindow->GetChildren().rend()) {
        wxTextCtrl* childwin = dynamic_cast<wxTextCtrl*>(*last_child);
        if (childwin) {
            if (childwin == win) {
                return true;
            }
            else {
                return false;
            }
        }
        last_child++;
    }
    return false;
}

void CIdentifiersPanel::OnNewText(wxCommandEvent& event)
{
    wxTextCtrl *win = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if (!win) return;
    if (!win->GetValue().IsEmpty() && x_IsLastXref(win)) {
        AddXref(kEmptyStr);
    }
    x_UpdateXrefDescriptions();
    m_ScrolledWindow->FitInside();
}


END_NCBI_SCOPE
