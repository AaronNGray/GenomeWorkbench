/*  $Id: feature_check_panel.cpp 26513 2012-09-28 20:22:30Z wuliangs $
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


#include <ncbi_pch.hpp>


#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>

////@begin includes
////@end includes

#include <gui/widgets/seq/feature_check_panel.hpp>

////@begin XPM images
////@end XPM images

#include <gui/widgets/wx/treectrl_ex.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <util/xregexp/regexp.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CFeatureCheckPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFeatureCheckPanel, wxPanel )


/*!
 * CFeatureCheckPanel event table definition
 */

BEGIN_EVENT_TABLE( CFeatureCheckPanel, wxPanel )

////@begin CFeatureCheckPanel event table entries
////@end CFeatureCheckPanel event table entries

END_EVENT_TABLE()


/*!
 * CFeatureCheckPanel constructors
 */

CFeatureCheckPanel::CFeatureCheckPanel()
{
    Init();
}

CFeatureCheckPanel::CFeatureCheckPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CFeatureCheckPanel creator
 */

bool CFeatureCheckPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFeatureCheckPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFeatureCheckPanel creation
    return true;
}


/*!
 * CFeatureCheckPanel destructor
 */

CFeatureCheckPanel::~CFeatureCheckPanel()
{
////@begin CFeatureCheckPanel destruction
////@end CFeatureCheckPanel destruction
}


/*!
 * Member initialisation
 */

void CFeatureCheckPanel::Init()
{
////@begin CFeatureCheckPanel member initialisation
    m_Tree = NULL;
////@end CFeatureCheckPanel member initialisation
    m_Filter = "";
}


/*!
 * Control creation for CFeatureCheckPanel
 */

void CFeatureCheckPanel::CreateControls()
{
////@begin CFeatureCheckPanel content construction
    CFeatureCheckPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Tree = new CwxTreeCtrlEx( itemPanel1, ID_TREECTRL1, wxDefaultPosition, wxSize(250, 250), wxTR_HAS_BUTTONS |wxTR_SINGLE );
    itemBoxSizer2->Add(m_Tree, 1, wxGROW|wxALL, 5);

////@end CFeatureCheckPanel content construction
    x_InitTree();
}


void CFeatureCheckPanel::x_InitTree(void)
{
    wxTreeItemId root_id = m_Tree->AddRoot(wxT("All Feature Types"));

    wxTreeItemId prt_id = m_Tree->AppendItem(root_id, wxT("All Protein"));
    wxTreeItemId rna_id = m_Tree->AppendItem(root_id, wxT("All Rna"));
    wxTreeItemId imp_id = m_Tree->AppendItem(root_id, wxT("All Import"));

    CRegexp * rxp = NULL;
    if (!m_Filter.empty()) {
        string  pattern = CRegexp::WildcardToRegexp(m_Filter);
        rxp = new CRegexp(pattern, CRegexp::fCompile_ignore_case);
    }   

    const CFeatList* feat_list = CSeqFeatData::GetFeatList();
    ITERATE(CFeatList, ft_it, *feat_list) {
        const CFeatListItem& item = *ft_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();

        // if filter set, skip mismatches
        if (rxp && !rxp->IsMatch(desc)) {           
            continue;                   
        }

        // indent subordinate types
        if (item.GetSubtype() != CSeqFeatData::eSubtype_any) {
            switch (feat_type){
            case CSeqFeatData::e_Prot:
                {{
                    m_Tree->AppendItem(prt_id, ToWxString(desc), -1, -1, new wxFeatItemData(item));
                    break;
                }}
            case CSeqFeatData::e_Rna:
                {{
                    m_Tree->AppendItem(rna_id, ToWxString(desc), -1, -1, new wxFeatItemData(item));
                    break;
                }}
            case CSeqFeatData::e_Imp:
                {{
                    m_Tree->AppendItem(imp_id, ToWxString(desc),  -1, -1, new wxFeatItemData(item));
                    break;
                }}
            default:
                {{
                    m_Tree->AppendItem(root_id, ToWxString(desc),  -1, -1, new wxFeatItemData(item));
                    break;
                }}
            }
        }
        else {
            //m_Tree->AppendItem(root_id, desc.c_str());
        }
    }
    m_Tree->SetItemText(prt_id,
                        m_Tree->GetItemText(prt_id) +
                        wxT(" (") +
                        ToWxString(NStr::NumericToString(m_Tree->GetChildrenCount(prt_id))) +
                        wxT(" items)"));

    m_Tree->SetItemText(rna_id,
                        m_Tree->GetItemText(rna_id) +
                        wxT(" (") +
                        ToWxString(NStr::NumericToString(m_Tree->GetChildrenCount(rna_id))) +
                        wxT(" items)"));

    m_Tree->SetItemText(imp_id,
                        m_Tree->GetItemText(imp_id) +
                        wxT(" (") +
                        ToWxString(NStr::NumericToString(m_Tree->GetChildrenCount(imp_id))) +
                        wxT(" items)"));

    m_Tree->Expand(root_id);
    m_Tree->CheckAll(false);

}

void CFeatureCheckPanel::x_GetSelections(const wxTreeItemId &id, TFeatTypeItemSet & set)
{
    if (m_Tree->GetChildrenCount(id, false)==0 && m_Tree->GetCheck(id)) {
        set.insert(*(dynamic_cast<wxFeatItemData*>(m_Tree->GetItemData(id))));
    }
    else {
        wxTreeItemIdValue cookie;
        for (wxTreeItemId cid = m_Tree->GetFirstChild(id, cookie); cid.IsOk(); cid=m_Tree->GetNextChild(id, cookie)){
            x_GetSelections(cid, set);
        }           
    }
}

void CFeatureCheckPanel::x_SetSelections(const wxTreeItemId &id, TFeatTypeItemSet & dset)
{
    if (m_Tree->GetChildrenCount(id, false)==0) {
        const wxTreeItemData* p0 = m_Tree->GetItemData(id);
        const wxFeatItemData* p1 = dynamic_cast<const wxFeatItemData*>(p0);
        if (!p1) return;
        const TFeatTypeItem & fi = *dynamic_cast<wxFeatItemData*>(m_Tree->GetItemData(id));
        bool bCheck = false;
        ITERATE(TFeatTypeItemSet, item, dset) {
            if ((item->GetType() == fi.GetType()) &&
                (item->GetSubtype() == fi.GetSubtype())){
                bCheck = true;
                break;
            }
        }       
        m_Tree->SetCheck(id, bCheck);       
    }
    else {
        wxTreeItemIdValue cookie;
        for (wxTreeItemId cid = m_Tree->GetFirstChild(id, cookie); cid.IsOk(); cid=m_Tree->GetNextChild(id, cookie)){
            x_SetSelections(cid, dset);
        }           
    }
}


void CFeatureCheckPanel::GetSelected(TFeatTypeItemSet& feat_types)
{
    feat_types.clear();
    x_GetSelections(m_Tree->GetRootItem(), feat_types);
}

void CFeatureCheckPanel::SetSelected(TFeatTypeItemSet& feat_types)
{
    x_SetSelections(m_Tree->GetRootItem(), feat_types);
}



/*!
 * Should we show tooltips?
 */

bool CFeatureCheckPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFeatureCheckPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeatureCheckPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeatureCheckPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFeatureCheckPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeatureCheckPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeatureCheckPanel icon retrieval
}


void CFeatureCheckPanel::x_AccessState(const wxTreeItemId &id, TStateVector & check, TStateVector & expand, bool bRead)
{
    string tid = ToStdString(m_Tree->GetItemText(id));
    if (m_Tree->GetChildrenCount(id, false)==0) {       
        if (!bRead) {
            if (m_Tree->GetCheck(id)) {
                check.push_back(tid);           
            }
        }
        else {
            if (find(check.begin(), check.end(), tid)!=check.end()) {
                m_Tree->SetCheck(id);       
            }
        }
    }
    else {
        if (!bRead) {
            if (m_Tree->IsExpanded(id)) {
                expand.push_back(tid);
            }
        }
        else {
            if (find(expand.begin(), expand.end(), tid)!=expand.end()) {
                m_Tree->Expand(id);
            }
        }
        wxTreeItemIdValue cookie;
        for (wxTreeItemId cid = m_Tree->GetFirstChild(id, cookie); cid.IsOk(); cid=m_Tree->GetNextChild(id, cookie)){
            x_AccessState(cid, check, expand, bRead);
        }           
    }
}

void CFeatureCheckPanel::AccessState(TStateVector & check, TStateVector & expand, bool bRead)
{
    if (!bRead) {
        check.clear();
        expand.clear();
    } else {
        m_Check     = check;
        m_Expand    = expand;       
    }
    x_AccessState(m_Tree->GetRootItem(), check, expand, bRead);
}

void CFeatureCheckPanel::Filter(string str)
{   
    m_Filter = str;
    x_AccessState(m_Tree->GetRootItem(), m_Check, m_Expand, false);
    m_Tree->Freeze();
    m_Tree->DeleteAllItems();   
    x_InitTree();   
    x_AccessState(m_Tree->GetRootItem(), m_Check, m_Expand, true);
    m_Tree->Thaw();
}

END_NCBI_SCOPE
