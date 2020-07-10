/*  $Id: seqid_fix_dlg.cpp 43676 2019-08-14 14:28:05Z asztalos $
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

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/seqid_fix_dlg.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CSeqIdFixDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSeqIdFixDlg, wxDialog )


/*!
 * CSeqIdFixDlg event table definition
 */

BEGIN_EVENT_TABLE( CSeqIdFixDlg, wxDialog )

////@begin CSeqIdFixDlg event table entries
    EVT_BUTTON( ID_RECHECK_SEQID_PROBLEMS_BTN, CSeqIdFixDlg::OnRecheckSeqidProblemsBtnClick )

////@end CSeqIdFixDlg event table entries

END_EVENT_TABLE()


/*!
 * CSeqIdFixDlg constructors
 */

CSeqIdFixDlg::CSeqIdFixDlg()
{
    Init();
}

CSeqIdFixDlg::CSeqIdFixDlg( wxWindow* parent, CRef<objects::CSeq_entry> entry_to_add, CConstRef<objects::CSeq_entry> curr_entry, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_EntryToAdd(entry_to_add), m_CurrEntry(curr_entry), m_OldIds(NULL)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CSeqIdFixDlg creator
 */

bool CSeqIdFixDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqIdFixDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqIdFixDlg creation
    return true;
}


/*!
 * CSeqIdFixDlg destructor
 */

CSeqIdFixDlg::~CSeqIdFixDlg()
{
////@begin CSeqIdFixDlg destruction
////@end CSeqIdFixDlg destruction
}


/*!
 * Member initialisation
 */

void CSeqIdFixDlg::Init()
{
////@begin CSeqIdFixDlg member initialisation
////@end CSeqIdFixDlg member initialisation
    m_NewGrid = NULL;
    m_OldGrid = NULL;
    m_OldIds = NULL;
}


/*!
 * Control creation for CSeqIdFixDlg
 */

void CSeqIdFixDlg::CreateControls()
{    
////@begin CSeqIdFixDlg content construction
    CSeqIdFixDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_RECHECK_SEQID_PROBLEMS_BTN, _("Recheck Problems"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSeqIdFixDlg content construction

    // no multiple choice values for these tables
    CRef<objects::CSeq_table> choices(NULL);

    if (m_CurrEntry) {
        m_OldIds = GetIdsFromSeqEntry(*m_CurrEntry);
        if (m_OldIds && m_OldIds->IsSetColumns() && m_OldIds->GetNum_rows() > 0) {
            wxStaticText* old_banner = new wxStaticText( this, wxID_STATIC, _("Existing Sequence IDs"), wxDefaultPosition, wxDefaultSize, 0 );
            itemBoxSizer3->Add(old_banner, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

            m_OldGrid = new CSeqTableGridPanel(this, m_OldIds, choices);
            itemBoxSizer3->Add(m_OldGrid, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
            // make all columns read-only
            m_OldGrid->MakeAllColumnsReadOnly();
        }
    }

    CRef<objects::CSeq_table> new_table = GetIdsFromSeqEntry(*m_EntryToAdd);

    CRef<objects::CSeqTable_column> problems = GetSeqIdProblems(new_table, m_OldIds, 100);
    new_table->SetColumns().push_back(problems);

    // add in a column for new sequence IDs
    CRef<objects::CSeqTable_column> new_ids(new objects::CSeqTable_column());
    new_ids->SetHeader().SetTitle(kNewId);
    new_ids->SetData().SetString();
    objects::CSeq_table::TColumns::iterator it = new_table->SetColumns().begin();
    it++;
    new_table->SetColumns().insert(it, new_ids);

    // copy over any IDs that are not duplicates or not too long
    CRef<objects::CSeqTable_column> orig_ids = FindSeqTableColumnByName(new_table, kSequenceIdColLabel);
    for (size_t i = 0; i < orig_ids->GetData().GetId().size(); i++) {
        string id_str = "";
        if (!problems || !problems->IsSetData() 
            || !problems->GetData().IsString() 
            || i >= problems->GetData().GetSize()
            || NStr::IsBlank(problems->GetData().GetString()[i])) { 
              orig_ids->GetData().GetId()[i]->GetLabel(&id_str, objects::CSeq_id::eContent);
        }
        new_ids->SetData().SetString().push_back(id_str);
    }            

    wxStaticText* new_banner = new wxStaticText( this, wxID_STATIC, _("New Sequence IDs"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(new_banner, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_NewGrid = new CSeqTableGridPanel(this, new_table, choices);
    itemBoxSizer3->Add(m_NewGrid, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    // make all columns other than new_id read-only
    m_NewGrid->MakeAllColumnsReadOnly();
    m_NewGrid->MakeColumnReadOnly(kNewId, false);

}


/*!
 * Should we show tooltips?
 */

bool CSeqIdFixDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSeqIdFixDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqIdFixDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqIdFixDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSeqIdFixDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqIdFixDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqIdFixDlg icon retrieval
}


void CSeqIdFixDlg::x_RefreshProblems()
{
    CRef<objects::CSeq_table> values_table = m_NewGrid->GetValuesTable();
    CRef<objects::CSeq_table> old_ids(NULL);
    if (m_OldGrid) {
        old_ids = m_OldGrid->GetValuesTable();
    }

    CRef<objects::CSeqTable_column> prev_problems = FindSeqTableColumnByName(values_table, kProblems);
    CRef<objects::CSeqTable_column> new_problems = GetSeqIdProblems(values_table, old_ids, 100);
    prev_problems->SetData().SetString().clear();
    if (new_problems && new_problems->IsSetData() && new_problems->GetData().IsString() && new_problems->GetData().GetSize() > 0) {
        prev_problems->SetData().SetString().assign(new_problems->GetData().GetString().begin(), new_problems->GetData().GetString().end());
    } else {
        for (int i = 0; i < values_table->GetNum_rows(); i++) {
            prev_problems->SetData().SetString().push_back("");
        }
    }
    m_NewGrid->SetValuesTable(values_table);
    m_NewGrid->MakeAllColumnsReadOnly();
    m_NewGrid->MakeColumnReadOnly(kNewId, false);
}


CRef<objects::CSeq_table> CSeqIdFixDlg::GetReplacementTable()
{
    CRef<objects::CSeq_table> values_table = m_NewGrid->GetValuesTable();
    CRef<objects::CSeq_table> old_ids(NULL);
    if (m_OldGrid) {
        old_ids = m_OldGrid->GetValuesTable();
    }

    CRef<objects::CSeqTable_column> problems = GetSeqIdProblems(values_table, old_ids, 100);
    if (problems) {
        wxMessageBox(ToWxString(SummarizeIdProblems(problems)), wxT("Error"), wxOK, this);
        values_table.Reset(NULL);
        x_RefreshProblems();
    }
    return values_table;
} 


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RECHECK_PROBLEMS_BTN
 */

void CSeqIdFixDlg::OnRecheckSeqidProblemsBtnClick( wxCommandEvent& event )
{
    x_RefreshProblems();
}


END_NCBI_SCOPE



