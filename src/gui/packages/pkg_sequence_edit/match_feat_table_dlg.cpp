/*  $Id: match_feat_table_dlg.cpp 43448 2019-07-01 16:48:24Z filippov $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <gui/packages/pkg_sequence_edit/match_feat_table_dlg.hpp>

#include <wx/imaglist.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/wupdlock.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
/*!
 * CMatchFeatureTableDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMatchFeatureTableDlg, wxDialog )


/*!
 * CMatchFeatureTableDlg event table definition
 */

BEGIN_EVENT_TABLE( CMatchFeatureTableDlg, wxDialog )

////@begin CMatchFeatureTableDlg event table entries
    EVT_BUTTON( ID_FEATTBLID_BTN, CMatchFeatureTableDlg::OnMapSelectedBtnClick )

    EVT_BUTTON( ID_AUTOMATCH_TABLE_ID, CMatchFeatureTableDlg::OnAutomatchTableIdClick )

////@end CMatchFeatureTableDlg event table entries

END_EVENT_TABLE()


/*!
 * CMatchFeatureTableDlg constructors
 */

CMatchFeatureTableDlg::CMatchFeatureTableDlg()
{
    Init();
}

CMatchFeatureTableDlg::CMatchFeatureTableDlg( wxWindow* parent, const objects::CSeq_entry_Handle& seh,
        const TUniqueFeatTblIDs& ftblids,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_Seh(seh), m_FtblIDs(ftblids)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CMatchFeatureTableDlg creator
 */

bool CMatchFeatureTableDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMatchFeatureTableDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMatchFeatureTableDlg creation
    return true;
}


/*!
 * CMatchFeatureTableDlg destructor
 */

CMatchFeatureTableDlg::~CMatchFeatureTableDlg()
{
////@begin CMatchFeatureTableDlg destruction
////@end CMatchFeatureTableDlg destruction
}


/*!
 * Member initialisation
 */

void CMatchFeatureTableDlg::Init()
{
////@begin CMatchFeatureTableDlg member initialisation
    m_FeatTableIDList = NULL;
    m_SequenceIDList = NULL;
    m_Map = NULL;
    m_AutoMatchType = NULL;
    m_MappedIDs = NULL;
////@end CMatchFeatureTableDlg member initialisation
}

/*!
 * Control creation for CMatchFeatureTableDlg
 */


void CMatchFeatureTableDlg::CreateControls()
{    
////@begin CMatchFeatureTableDlg content construction
    CMatchFeatureTableDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Feature Table IDs not found in record"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_FeatTableIDList = new wxListView(itemDialog1, ID_FEATTBLID_LSTCTRLA, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING);
    itemBoxSizer5->Add(m_FeatTableIDList, 1, wxGROW | wxALL, 5);
    m_FeatTableIDList->AppendColumn(wxT("Feature Table IDs"));
    m_FeatTableIDList->SetColumnWidth(0, 200);
    m_FeatTableIDList->SetMinSize(wxSize(200, -1));

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_SequenceIDList = new wxListView(itemDialog1, ID_FEATTBLID_LSTCTRLB, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING);
    itemBoxSizer8->Add(m_SequenceIDList, 1, wxGROW | wxALL, 5);
    m_SequenceIDList->AppendColumn(wxT("Sequence IDs"));
    m_SequenceIDList->SetColumnWidth(0, 400);
    m_SequenceIDList->SetMinSize(wxSize(400, -1));

    m_Map = new wxButton( itemDialog1, ID_FEATTBLID_BTN, _("Map Selected"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_Map, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, ID_AUTOMATCH_TABLE_ID, _("AutoMatch where table ID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_AutoMatchTypeStrings;
    m_AutoMatchTypeStrings.Add(_("Matches"));
    m_AutoMatchTypeStrings.Add(_("Is contained in"));
    m_AutoMatchTypeStrings.Add(_("Is start of"));
    m_AutoMatchTypeStrings.Add(_("Is end of"));
    m_AutoMatchType = new wxChoice( itemDialog1, ID_AUTO_MATCH_TYPE, wxDefaultPosition, wxDefaultSize, m_AutoMatchTypeStrings, 0 );
    itemBoxSizer12->Add(m_AutoMatchType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("record ID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemDialog1, wxID_STATIC, _("Selected matches"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);

    wxArrayString m_MappedIDsStrings;
    m_MappedIDs = new wxCheckListBox( itemDialog1, ID_FEATTBLID_CHKLSTBX, wxDefaultPosition, wxSize(-1, 109), m_MappedIDsStrings, wxLB_SINGLE | wxLB_SORT );
    itemBoxSizer2->Add(m_MappedIDs, 1, wxEXPAND| wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxButton* itemButton19 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CMatchFeatureTableDlg content construction
    m_AutoMatchType->SetSelection(0);
}


string s_GetDisplayLabel(CBioseq_Handle bh)
{
    string label;
    ITERATE(CBioseq::TId, id_it, bh.GetBioseqCore()->GetId()) {
        if ((*id_it)->IsGeneral() && (*id_it)->GetGeneral().IsSetDb() &&
            ((*id_it)->GetGeneral().GetDb() == "TMSMART" || (*id_it)->GetGeneral().GetDb() == "BankIt" || (*id_it)->GetGeneral().GetDb() == "NCBIFILE"))
            continue;
        string id_label = (*id_it)->AsFastaString();
        if (!label.empty()) {
            label += "|";
        }
        label += id_label;
    }
    return label;
}


bool CMatchFeatureTableDlg::TransferDataToWindow()
{
    m_FtblID_map.clear();
    ITERATE (TUniqueFeatTblIDs, it, m_FtblIDs) {
        CConstRef<CSeq_id> id = (*it).GetSeqId();
        string label;
        id->GetLabel(&label);
        m_FeatTableIDList->InsertItem(m_FeatTableIDList->GetItemCount(), ToWxString(label));
        m_FtblID_map.insert(TStrToIDMap::value_type (label, *it));
    }

    m_SequenceIDList->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_SeqID_map.clear();
    for (CBioseq_CI b_iter(m_Seh, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        string label = s_GetDisplayLabel(*b_iter);
        CSeq_id_Handle idh = sequence::GetId(*b_iter, sequence::eGetId_Seq_id_BestRank);
        m_SequenceIDList->InsertItem(m_SequenceIDList->GetItemCount(), ToWxString(label));
        m_SeqID_map.insert(TStrToIDMap::value_type(label, idh));
    }
    if (!m_SeqID_map.empty())
        m_SequenceIDList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    return wxDialog::TransferDataToWindow();
}

static const char* sToSign = " -> ";

void CMatchFeatureTableDlg::GetFtableIDToSeqIDMap(TFeatSeqIDMap& ftbl_seqid_map)
{
    ftbl_seqid_map.clear();
    wxArrayInt checked_items;
    m_MappedIDs->GetCheckedItems(checked_items);


    for (unsigned int n = 0; n < checked_items.size(); ++n) {
        string text = ToStdString(m_MappedIDs->GetString(checked_items[n]));
        SIZE_TYPE pos = NStr::FindNoCase(text, sToSign);
        string ftbl_label = text.substr(0, pos);
        string seqid_label = text.substr(pos + CTempString(sToSign).length(), NPOS);
        
        TStrToIDMap::const_iterator it1 = m_FtblID_map.find(ftbl_label);
        TStrToIDMap::const_iterator it2 = m_SeqID_map.find(seqid_label);
        if (it1 != m_FtblID_map.end() && it2 != m_SeqID_map.end()) {
            ftbl_seqid_map.insert(TFeatSeqIDMap::value_type(it1->second, it2->second));
        }
    }
}
/*!
 * Should we show tooltips?
 */

bool CMatchFeatureTableDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMatchFeatureTableDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMatchFeatureTableDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMatchFeatureTableDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMatchFeatureTableDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMatchFeatureTableDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMatchFeatureTableDlg icon retrieval
}


void CMatchFeatureTableDlg::x_AddMatch(long item_ftbl, long item_seqid)
{
    int index = m_MappedIDs->Append(m_FeatTableIDList->GetItemText(item_ftbl) + ToWxString(sToSign) + m_SequenceIDList->GetItemText(item_seqid));
    m_MappedIDs->Check(index, true);

    wxWindowUpdateLocker noUpdates(m_FeatTableIDList);
    m_FeatTableIDList->DeleteItem(item_ftbl);
    if (m_FeatTableIDList->GetItemCount() >= item_ftbl + 1)
        m_FeatTableIDList->RefreshItems(item_ftbl, m_FeatTableIDList->GetItemCount() - 1);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FEATTBLID_BTN
 */

void CMatchFeatureTableDlg::OnMapSelectedBtnClick( wxCommandEvent& event )
{
    long item_ftbl = m_FeatTableIDList->GetFirstSelected();
    long item_seqid = m_SequenceIDList->GetFirstSelected();

    if (item_ftbl == -1 || item_seqid == -1) {
        return;
    }
    
    x_AddMatch(item_ftbl, item_seqid);

    event.Skip();
}


bool CMatchFeatureTableDlg::x_TokenMatch(const string& ft_id, const string& rec_id)
{
    if (NStr::IsBlank(rec_id) || NStr::IsBlank(ft_id)) {
        return false;
    }
    int match_type = m_AutoMatchType->GetSelection();
    switch (match_type) {
        case 0:
            // matches
            if (NStr::Equal(ft_id, rec_id)) {
                return true;
            }
            if (NStr::Find(ft_id, ".") == string::npos) {
                size_t ver = NStr::Find(rec_id, ".");
                if (ver != string::npos && NStr::Equal(ft_id, rec_id.substr(0, ver))) {
                    return true;
                }
            }
            if (NStr::StartsWith(ft_id, "gnl|")) {
                size_t pos = NStr::Find(ft_id, ":");
                if (pos != NPOS && NStr::Equal(ft_id.substr(pos + 1, NPOS), rec_id)) {
                    return true;
                }
            }
            break;
        case 1:
            // is contained in
            if (NStr::Find(rec_id, ft_id, NStr::eNocase) != string::npos) {
                return true;
            }
            break;
        case 2:
            // is start of
            if (NStr::StartsWith(rec_id, ft_id, NStr::eNocase)) {
                return true;
            }
            break;
        case 3:
            // is end of
            if (NStr::EndsWith(rec_id, ft_id, NStr::eNocase)) {
                return true;
            }
        default:
            break;
    }
    return false;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_AUTOMATCH_TABLE_ID
 */

void CMatchFeatureTableDlg::OnAutomatchTableIdClick(wxCommandEvent& event)
{
    wxWindowUpdateLocker noUpdates(m_FeatTableIDList);
    wxArrayString matched_ids;
    
    if ( m_AutoMatchType->GetSelection() == 0)
    {
	LinearAutomatch(matched_ids);
    }
    else
    {
	QuadraticAutomatch(matched_ids);
    }
   
    m_FeatTableIDList->Refresh();
    m_MappedIDs->Append(matched_ids);
    for (size_t i = 0; i < matched_ids.size(); ++i) {
        m_MappedIDs->Check(i);
    }
}

void CMatchFeatureTableDlg::QuadraticAutomatch(wxArrayString &matched_ids)
{
    long item_ftbl = -1;
    
    for (;;) {
        item_ftbl = m_FeatTableIDList->GetNextItem(item_ftbl, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
        if (item_ftbl == -1)  break;
        string ftcheck = ToStdString(m_FeatTableIDList->GetItemText(item_ftbl));
        if (NStr::StartsWith(ftcheck, "lcl|")) {
            ftcheck = ftcheck.substr(4);
        }
        long item_seqid = -1;
        bool found_match = false;
        while (!found_match) {
            item_seqid = m_SequenceIDList->GetNextItem(item_seqid, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
            if (item_seqid == -1) break;
            string sicheck = ToStdString(m_SequenceIDList->GetItemText(item_seqid));
            vector<CTempString> tokens;
            NStr::Split(sicheck, "|", tokens);
            ITERATE(vector<CTempString>, t, tokens) {
                if (x_TokenMatch(ftcheck, *t)) {
                    matched_ids.Add(m_FeatTableIDList->GetItemText(item_ftbl) + ToWxString(sToSign) + m_SequenceIDList->GetItemText(item_seqid));
                    m_FeatTableIDList->DeleteItem(item_ftbl);
                    found_match = true;
                    item_ftbl = -1;
                    break;
                }
            }
        }
    }
}

void CMatchFeatureTableDlg::LinearAutomatch(wxArrayString &matched_ids)
{
    long item_ftbl = -1;
    map<string, long> label_to_item;
    for (;;) {
        item_ftbl = m_FeatTableIDList->GetNextItem(item_ftbl, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
        if (item_ftbl == -1)  break;
        string ftcheck = ToStdString(m_FeatTableIDList->GetItemText(item_ftbl));
        if (NStr::StartsWith(ftcheck, "lcl|")) {
            ftcheck = ftcheck.substr(4);
        }
	if (ftcheck.empty())
	    continue;
	label_to_item[ftcheck] = item_ftbl;
	
	if (NStr::StartsWith(ftcheck, "gnl|")) {
	    size_t pos = NStr::Find(ftcheck, ":");
	    if (pos != NPOS) {
		string gnl = ftcheck.substr(pos + 1, NPOS);
		if (!gnl.empty())
		    label_to_item[gnl] = item_ftbl;
	    }
	}
    }

    long item_seqid = -1;
    set<long> to_delete;
    for (;;) {
	item_seqid = m_SequenceIDList->GetNextItem(item_seqid, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
	if (item_seqid == -1) break;
	string sicheck = ToStdString(m_SequenceIDList->GetItemText(item_seqid));
	vector<CTempString> tokens;
	NStr::Split(sicheck, "|", tokens, NStr::fSplit_Tokenize);
	ITERATE(vector<CTempString>, t, tokens) {
	    auto p = label_to_item.find(*t);
	    if (p == label_to_item.end()) {
		size_t ver = NStr::Find(*t, ".");
		if (ver != string::npos) {
		    p = label_to_item.find(t->substr(0, ver));
		}
	    }
	    if (p != label_to_item.end()) {
		matched_ids.Add(m_FeatTableIDList->GetItemText(p->second) + ToWxString(sToSign) + m_SequenceIDList->GetItemText(item_seqid));
		to_delete.insert(p->second);
		break;
	    }
	}
    }    
    for (auto p = to_delete.rbegin(); p != to_delete.rend(); ++p) {
	m_FeatTableIDList->DeleteItem(*p);
    }
}

END_NCBI_SCOPE

