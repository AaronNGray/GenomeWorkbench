/*  $Id: assembly_tracking_panel.cpp 41550 2018-08-20 15:22:02Z katargir $
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

#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_hist.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <gui/widgets/edit/assembly_tracking_panel.hpp>
#include <gui/widgets/edit/assembly_tracking_list_panel.hpp>
#include <gui/widgets/edit/single_assembly_tracking_panel.hpp>

#include <gui/widgets/edit/user_object_util.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filedlg.h>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/msgdlg.h> 

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CAssemblyTrackingPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAssemblyTrackingPanel, wxPanel )


/*!
 * CAssemblyTrackingPanel event table definition
 */

BEGIN_EVENT_TABLE( CAssemblyTrackingPanel, wxPanel )

////@begin CAssemblyTrackingPanel event table entries
    EVT_BUTTON( ID_EXPORT_LIST_BTN, CAssemblyTrackingPanel::OnExportListBtnClick )

    EVT_BUTTON( ID_EXPORT_INTERVALS_BTN, CAssemblyTrackingPanel::OnExportIntervalsBtnClick )

    EVT_BUTTON( ID_IMPORT_INTERVALS_BTN, CAssemblyTrackingPanel::OnImportIntervalsBtnClick )

    EVT_BUTTON( ID_POPULATE_BTN, CAssemblyTrackingPanel::OnPopulateBtnClick )

////@end CAssemblyTrackingPanel event table entries

END_EVENT_TABLE()


/*!
 * CAssemblyTrackingPanel constructors
 */

CAssemblyTrackingPanel::CAssemblyTrackingPanel()
{
    Init();
}

CAssemblyTrackingPanel::CAssemblyTrackingPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAssemblyTrackingPanel creator
 */

bool CAssemblyTrackingPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAssemblyTrackingPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAssemblyTrackingPanel creation
    return true;
}


/*!
 * CAssemblyTrackingPanel destructor
 */

CAssemblyTrackingPanel::~CAssemblyTrackingPanel()
{
////@begin CAssemblyTrackingPanel destruction
////@end CAssemblyTrackingPanel destruction
}


/*!
 * Member initialisation
 */

void CAssemblyTrackingPanel::Init()
{
////@begin CAssemblyTrackingPanel member initialisation
    m_AccessionsPanel = NULL;
    m_PopulateBtn = NULL;
////@end CAssemblyTrackingPanel member initialisation
    m_User.Reset(NULL);
    m_Seq.Reset(NULL);
}


/*!
 * Control creation for CAssemblyTrackingPanel
 */

void CAssemblyTrackingPanel::CreateControls()
{    
////@begin CAssemblyTrackingPanel content construction
    CAssemblyTrackingPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_AccessionsPanel = new CAssemblyTrackingListPanel( itemPanel1, ID_WINDOW3, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_AccessionsPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemPanel1, ID_EXPORT_LIST_BTN, _("Export TPA Accession List"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemPanel1, ID_EXPORT_INTERVALS_BTN, _("Export Interval Table"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemPanel1, ID_IMPORT_INTERVALS_BTN, _("Import Interval Table"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_PopulateBtn = new wxButton( itemPanel1, ID_POPULATE_BTN, _("Populate Intervals from Assembly Alignment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(m_PopulateBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer2->Add(380, 1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CAssemblyTrackingPanel content construction
    m_PopulateBtn->Show(false);
}


/*!
 * Should we show tooltips?
 */

bool CAssemblyTrackingPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAssemblyTrackingPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAssemblyTrackingPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAssemblyTrackingPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAssemblyTrackingPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAssemblyTrackingPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAssemblyTrackingPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_LIST_BTN
 */

void CAssemblyTrackingPanel::OnExportListBtnClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
    CRef<CSeq_table> table = SeqTableFromAssemblyFields(*(GetUser_object()));
    if (!table || !table->IsSetColumns() || !table->GetColumns()[0]->IsSetData() || table->GetColumns()[0]->GetData().GetSize() == 0) {
        wxMessageBox(wxT("No accessions listed"), wxT("Error"),
                        wxOK | wxICON_ERROR, this);
        return; 
    }
    wxFileDialog table_save_file(this, wxT("Select a file"), wxEmptyString, wxEmptyString, wxT("*.*"),
                                 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (table_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = table_save_file.GetPath();
        wxString name = table_save_file.GetFilename();

        name.Trim(false);
        name.Trim(true);

        if (name.IsEmpty())
        {
            wxMessageBox(wxT("Please, select file name"), wxT("Error"),
                         wxOK | wxICON_ERROR, this);
            return; 
        }

        if (!SaveTableRowAsListFile(ToStdString(path), table, 0)) {
            wxMessageBox(wxT("Cannot open file ")+ path, wxT("Error"),
                         wxOK | wxICON_ERROR, this);
        }
    }

}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_INTERVALS_BTN
 */

void CAssemblyTrackingPanel::OnExportIntervalsBtnClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
    CRef<CSeq_table> table = SeqTableFromAssemblyFields(*(GetUser_object()));
    if (!table) {
        wxMessageBox(wxT("No accessions listed"), wxT("Error"),
                        wxOK | wxICON_ERROR, this);
        return; 
    }
    wxFileDialog table_save_file(this, wxT("Select a file"), wxEmptyString, wxEmptyString, wxT("*.*"),
                                 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (table_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = table_save_file.GetPath();
        wxString name = table_save_file.GetFilename();

        name.Trim(false);
        name.Trim(true);

        if (name.IsEmpty())
        {
            wxMessageBox(wxT("Please, select file name"), wxT("Error"),
                         wxOK | wxICON_ERROR, this);
            return; 
        }
        if (!SaveTableFile(ToStdString(path), table, false)) {
            wxMessageBox(wxT("Cannot open file ")+ path, wxT("Error"),
                         wxOK | wxICON_ERROR, this);
        }
    }
 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT_INTERVALS_BTN
 */

void CAssemblyTrackingPanel::OnImportIntervalsBtnClick( wxCommandEvent& event )
{
    wxFileDialog table_file(this, wxT("Select a file"), wxEmptyString, wxEmptyString, wxT("*.*"),                              
                               wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if (table_file.ShowModal() == wxID_OK) 
    {
        wxString path = table_file.GetPath();
        wxString name = table_file.GetFilename();
        CRef<CSeq_table> table = ReadTabDelimitedTable(path, false);
        if (table) {
            CRef<CUser_object> new_obj(new CUser_object());
            new_obj->SetType().SetStr("TpaAssembly");
            AddAssemblyUserFieldsFromSeqTable(new_obj, table);
            SetUser_object(new_obj);
        } else {
            wxMessageBox(wxT("Unable to read table"), wxT("Error"),
                        wxOK | wxICON_ERROR, this);
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_POPULATE_BTN
 */

void CAssemblyTrackingPanel::OnPopulateBtnClick( wxCommandEvent& event )
{
    if (!m_Seq || !m_Seq->IsSetInst() || !m_Seq->GetInst().IsSetHist()) {
        return;
    }
    if (!m_User) {
        m_User.Reset(new CUser_object());
    }
    if (m_User->IsSetData()) {
        m_User->ResetData();
    }

    ITERATE(CSeq_hist::TAssembly, ait, m_Seq->GetInst().GetHist().GetAssembly()) {
        if ((*ait)->IsSetSegs() && (*ait)->GetSegs().IsDenseg() 
            && (*ait)->GetSegs().GetDenseg().IsSetDim() && (*ait)->GetSegs().GetDenseg().GetDim() > 1) {
            // find start and stop for second sequence
            try {
                CRef<CSeq_interval> interval0 = (*ait)->GetSegs().GetDenseg().CreateRowSeq_interval(0);
                CRef<CSeq_interval> interval1 = (*ait)->GetSegs().GetDenseg().CreateRowSeq_interval(1);
                if (interval0 && interval1) {
                    string accession = "";
                    interval1->GetId().GetLabel(&accession,CSeq_id::eContent);
                    TSeqPos from =  interval0->GetFrom();
                    TSeqPos to = interval0->GetTo();
                    if (interval1->IsSetStrand() && interval1->GetStrand() == eNa_strand_minus)
                        swap(from, to);
                    CRef<CUser_field> new_field = CSingleAssemblyTrackingPanel::s_CreateTPAAssemblyField(accession, from, to);
                    m_User->SetData().push_back(new_field);
                }
            }  catch(CException &e) {} catch (exception &e) {}
        }
    }
    m_AccessionsPanel->SetUser_object(m_User);
}


void CAssemblyTrackingPanel::SetUser_object(CRef<CUser_object> user)
{
    if (user) {
        m_User.Reset(new CUser_object());
        m_User->Assign(*user);
    } else {
        m_User.Reset(NULL);
    }

    m_AccessionsPanel->SetUser_object(user);
}


CRef<CUser_object> CAssemblyTrackingPanel::GetUser_object()
{
    TransferDataFromWindow();
    m_AccessionsPanel->TransferDataFromWindow();
    CRef<CUser_object> user(new CUser_object());
    
    if (m_User) {
        user->Assign(*m_User);
    }

    user->SetType().SetStr("TpaAssembly");

    // remove previous fields
    m_AccessionsPanel->PopulateFields(*user);
    return user;
}


void CAssemblyTrackingPanel::SetBioseq(CConstRef<CBioseq> seq)
{
    if (seq) {
        m_Seq.Reset(seq);
    } else {
        m_Seq.Reset(NULL);
    }
    if (m_Seq && m_Seq->IsSetInst() && m_Seq->GetInst().IsSetHist()) {
        m_PopulateBtn->Show(true);
    } else {
        m_PopulateBtn->Show(false);
    }
}


void CAssemblyTrackingPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    m_User.Reset(new CUser_object());
    m_User->Assign(desc.GetUser());

    m_AccessionsPanel->SetUser_object(m_User);
    TransferDataToWindow();
    m_AccessionsPanel->TransferDataToWindow();
}


void CAssemblyTrackingPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    CRef<CUser_object> user = GetUser_object();
    desc.SetUser(*user);
}


END_NCBI_SCOPE

