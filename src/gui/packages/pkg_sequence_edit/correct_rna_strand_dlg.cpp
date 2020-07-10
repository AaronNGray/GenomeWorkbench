/*  $Id: correct_rna_strand_dlg.cpp 40191 2018-01-04 21:03:28Z asztalos $
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

#include <corelib/ncbifile.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <util/line_reader.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/correct_rna_strand_dlg.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/stattext.h>
#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/button.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CCorrectRNAStrandDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCorrectRNAStrandDlg, CBulkCmdDlg )


/*!
 * CCorrectRNAStrandDlg event table definition
 */

BEGIN_EVENT_TABLE( CCorrectRNAStrandDlg, CBulkCmdDlg )

////@begin CCorrectRNAStrandDlg event table entries
    EVT_BUTTON( ID_REFRESH_STRAND_BTN, CCorrectRNAStrandDlg::OnRefreshStrandBtnClick )

    EVT_BUTTON( ID_AUTOCORRECT_STRAND_BTN, CCorrectRNAStrandDlg::OnAutocorrectStrandBtnClick )

    EVT_BUTTON( ID_CANCEL_RNA_STRAND_BTN, CCorrectRNAStrandDlg::OnCancelRnaStrandBtnClick )

////@end CCorrectRNAStrandDlg event table entries

END_EVENT_TABLE()


/*!
 * CCorrectRNAStrandDlg constructors
 */

CCorrectRNAStrandDlg::CCorrectRNAStrandDlg()
{
    Init();
}

CCorrectRNAStrandDlg::CCorrectRNAStrandDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


/*!
 * CCorrectRNAStrandDlg creator
 */

bool CCorrectRNAStrandDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCorrectRNAStrandDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCorrectRNAStrandDlg creation
    return true;
}


/*!
 * CCorrectRNAStrandDlg destructor
 */

CCorrectRNAStrandDlg::~CCorrectRNAStrandDlg()
{
////@begin CCorrectRNAStrandDlg destruction
////@end CCorrectRNAStrandDlg destruction
}


/*!
 * Member initialisation
 */

void CCorrectRNAStrandDlg::Init()
{
////@begin CCorrectRNAStrandDlg member initialisation
    m_SequenceListWindow = NULL;
    m_SequenceListSizer = NULL;
    m_ReverseFeaturesBtn = NULL;
    m_UseSmart = NULL;
////@end CCorrectRNAStrandDlg member initialisation
    m_NumRows = 0;
    m_TotalHeight = 0;
    m_ScrollRate = 0;
}


/*!
 * Control creation for CCorrectRNAStrandDlg
 */

void CCorrectRNAStrandDlg::CreateControls()
{    
////@begin CCorrectRNAStrandDlg content construction
    CCorrectRNAStrandDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_SequenceListWindow = new wxScrolledWindow( itemCBulkCmdDlg1, ID_RNA_STRAND_SCROLLEDWINDOW, wxDefaultPosition, wxSize(100, 100), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_SequenceListWindow, 0, wxGROW|wxALL, 5);
    m_SequenceListWindow->SetScrollbars(1, 1, 0, 0);
    m_SequenceListSizer = new wxBoxSizer(wxVERTICAL);
    m_SequenceListWindow->SetSizer(m_SequenceListSizer);

    m_SequenceListWindow->FitInside();

    wxButton* itemButton5 = new wxButton( itemCBulkCmdDlg1, ID_REFRESH_STRAND_BTN, _("Refresh Strand Results"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ReverseFeaturesBtn = new wxCheckBox( itemCBulkCmdDlg1, ID_REVERSE_FEATURES_BTN, _("Also reverse features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ReverseFeaturesBtn->SetValue(false);
    itemBoxSizer2->Add(m_ReverseFeaturesBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_UseSmart = new wxCheckBox( itemCBulkCmdDlg1, ID_USE_SMART_FOR_STRAND_BTN, _("Use SMART for strand information"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseSmart->SetValue(true);
    m_UseSmart->Enable(false);
    itemBoxSizer2->Add(m_UseSmart, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemCBulkCmdDlg1, ID_AUTOCORRECT_STRAND_BTN, _("Autocorrect Minus Strands"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemCBulkCmdDlg1, ID_CANCEL_RNA_STRAND_BTN, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CCorrectRNAStrandDlg content construction
    x_PopulateAccessionList();
}


/*!
 * Should we show tooltips?
 */

bool CCorrectRNAStrandDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCorrectRNAStrandDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCorrectRNAStrandDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCorrectRNAStrandDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCorrectRNAStrandDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCorrectRNAStrandDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCorrectRNAStrandDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REFRESH_STRAND_BTN
 */

void CCorrectRNAStrandDlg::OnRefreshStrandBtnClick( wxCommandEvent& event )
{
    x_PopulateAccessionList();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CANCEL_RNA_STRAND_BTN
 */

void CCorrectRNAStrandDlg::OnCancelRnaStrandBtnClick( wxCommandEvent& event )
{
    Destroy();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_AUTOCORRECT_STRAND_BTN
 */

void CCorrectRNAStrandDlg::OnAutocorrectStrandBtnClick( wxCommandEvent& event )
{
    GetTopLevelSeqEntryAndProcessor();
    CRef<CCmdComposite> cmd = GetCommand();
    if (cmd) {
        ExecuteCmd(cmd);
        Destroy();
    } else {
        string error = GetErrorMessage();
        if (!NStr::IsBlank(error)) {
            wxMessageBox(ToWxString(error), wxT("Error"),
                            wxOK | wxICON_ERROR, NULL);
        }
    }
}


void CCorrectRNAStrandDlg::x_ReverseFeatures(CBioseq_Handle bsh, CRef<CCmdComposite> cmd)
{
    CFeat_CI fit(bsh);
    while (fit) {
        CConstRef<CSeq_feat> f = fit->GetSeq_feat();
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        CSeq_loc* rc = sequence::SeqLocRevCmpl(new_feat->GetLocation(), &(m_TopSeqEntry.GetScope()));
        new_feat->SetLocation().Assign(*rc);
        CSeq_feat_Handle fh = m_TopSeqEntry.GetScope().GetSeq_featHandle(*f);
        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
        ++fit;
    }
}


CRef<CCmdComposite> CCorrectRNAStrandDlg::GetCommand()
{
    m_ErrorMessage = "";
    CRef<CCmdComposite> cmd(new CCmdComposite("Autocorrect RNA sequences"));
    bool any_change = false;
    if (m_SeqList.empty()) {
        m_ErrorMessage = "No sequences selected.";
        return CRef<CCmdComposite>(NULL);
    } 
    for (size_t i = 0; i < m_PickList.size(); i++) {
        if (m_PickList[i]->GetValue()) {
            // process this one
            CBioseq_Handle bsh = m_SeqMap[m_AccessionList[i]]; 
            CRef<CSeq_inst> inst(new CSeq_inst());
            inst->Assign(bsh.GetInst());
            ReverseComplement(*inst, &(bsh.GetScope()));
            CRef<CCmdChangeBioseqInst> flip(new CCmdChangeBioseqInst(bsh, *inst));
            cmd->AddCommand(*flip);
            if (m_ReverseFeaturesBtn->GetValue()) {
                x_ReverseFeatures(bsh, cmd);
            }
            any_change = true;
        }
    }
    if (!any_change) {
        cmd.Reset(NULL);
        m_ErrorMessage = "No sequences selected.";
    }
    return cmd;
}


string CCorrectRNAStrandDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}


bool s_HasGenBankId(CBioseq_Handle bsh)
{
    bool rval = false;
    if (!bsh) {
        return false;
    }
    ITERATE(CBioseq::TId, it, bsh.GetCompleteBioseq()->GetId()) {
        if ((*it)->IsGenbank()) {
            rval = true;
            break;
        }
    }
    return rval;
}

static bool CompByStrand(const pair<string,int>& first, const pair<string,int>& second)
{
    return (first.second > second.second);
}


void CCorrectRNAStrandDlg::x_PopulateAccessionList()
{
    Freeze();
    // remove existing subitems
    while (m_SequenceListSizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_SequenceListSizer->GetItem(pos)->DeleteWindows();
        m_SequenceListSizer->Remove(pos);
    }

    m_SequenceListSizer->Clear();
    m_TotalHeight = 0;
    m_NumRows = 0;
    m_ScrollRate = 0;
    m_SeqList.clear();
    m_StatusList.clear();
    m_PickList.clear();
    m_AccessionList.clear();
    m_SeqMap.clear();

    GetTopLevelSeqEntryAndProcessor();
    bool use_smart = m_UseSmart->GetValue();
    CBioseq_CI bi(m_TopSeqEntry, CSeq_inst::eMol_na); 
    while (bi) {
        if (!use_smart || s_HasGenBankId(*bi)) {
            // only collect sequences with GenBank ID for SMART
            m_SeqList.push_back(*bi);
        }
        ++bi;
    }

    // TODO: Get statuses
    for (size_t i = 0; i < m_SeqList.size(); i++) {
        m_StatusList.push_back(RNAstrand_UNEXPECTED);
    }
    if (use_smart) {
        x_GetStatusSMART();
    } else {
        x_GetStatusLocal();
    }

    vector<pair<string,int> > values;
    size_t pos = 0;
    ITERATE(vector<CBioseq_Handle>, it, m_SeqList) {
        string label;
        ITERATE(CBioseq::TId, it2, (*it).GetCompleteBioseq()->GetId()) {
            if ((*it2)->IsGenbank() && (*it2)->GetGenbank().IsSetAccession()) {
                label = (*it2)->GetGenbank().GetAccession();
                break;
            }
        }
        if (label.empty())
            (*it).GetCompleteBioseq()->GetLabel(&label, CBioseq::eContent); 

        values.push_back(pair<string,int>(label, m_StatusList[pos]));
        m_SeqMap[label] = *it;
        pos++;
    }

    stable_sort(values.begin(),values.end(),CompByStrand);
    for (unsigned int i=0; i<values.size(); i++)
    {
        x_AddRow(values[i].first, x_StringFromStrand(ERNAstrand_return_val(values[i].second)), values[i].second == RNAstrand_MINUS);
        m_AccessionList.push_back(values[i].first);
    }

    m_SequenceListWindow->SetVirtualSize(-1, m_TotalHeight);
    m_SequenceListWindow->SetScrollRate(0, m_ScrollRate);
    m_SequenceListWindow->FitInside();
    Layout();
    Thaw();
}


void CCorrectRNAStrandDlg::x_AddRow(const string& text, const string& status, bool selected)
{
    wxSizer *row_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_SequenceListSizer->Add(row_sizer, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    
    wxStaticText* acc = new wxStaticText( m_SequenceListWindow, wxID_STATIC, ToWxString(text), wxDefaultPosition, wxSize(100, -1));
    row_sizer->Add(acc);
    wxStaticText* stat = new wxStaticText( m_SequenceListWindow, wxID_STATIC, ToWxString(status), wxDefaultPosition, wxSize(100, -1));
    row_sizer->Add(stat);
    wxCheckBox* check = new wxCheckBox( m_SequenceListWindow, wxID_ANY, wxEmptyString);
    check->SetValue(selected);
    row_sizer->Add(check);
    m_PickList.push_back(check);

    int row_width;
    int row_height;
    check->GetSize(&row_width, &row_height);

    m_TotalHeight += row_height;
    m_ScrollRate = row_height;
    m_NumRows++;
}


static const string RNAstrand_strings[] = 
{ "Plus", "Minus", "Mixed", "No Hits", "Unexpected", "Parse Error", "In Progress" };

static const size_t k_NumRNAstrand_strings = sizeof (RNAstrand_strings) / sizeof (string);

CCorrectRNAStrandDlg::ERNAstrand_return_val CCorrectRNAStrandDlg::x_StrandFromString(const string& status)
{
    ERNAstrand_return_val rval = RNAstrand_UNEXPECTED;
    for (size_t i = 0; i < k_NumRNAstrand_strings; i++) {
        if (NStr::Equal (status, RNAstrand_strings [i])
            || (i == RNAstrand_MIXED
                && NStr::StartsWith (status, RNAstrand_strings [i]))) {
            rval = (ERNAstrand_return_val) i;
            break;
        }
    }
    return rval;
}


string CCorrectRNAStrandDlg::x_StringFromStrand(ERNAstrand_return_val rval)
{
    return RNAstrand_strings[rval];
}


void CCorrectRNAStrandDlg::x_ProcessRNAStrandLine(const CTempString& line, size_t pos, size_t max)
{
    SIZE_TYPE tab_pos = line.find('\t');
    if (tab_pos != string::npos) {
        string accession = line.substr(0, tab_pos);
        string status = line.substr(tab_pos + 1);
        // need to find position to match label
        bool found = false;
        for (size_t i = pos; i < max && !found; i++) {
            CBioseq_Handle bsh = m_SeqList[i];
            ITERATE(CBioseq::TId, it, bsh.GetCompleteBioseq()->GetId()) {
                if ((*it)->IsGenbank()) {
                    if (NStr::EqualNocase((*it)->GetGenbank().GetAccession(), accession)) {
                        m_StatusList[i] = x_StrandFromString(status);
                        found = true;
                    }
                    break;
                }
            }
        }
    }
}


void CCorrectRNAStrandDlg::x_GetStatusSMART()
{
    if (m_SeqList.empty()) {
        return;
    }

#ifdef NCBI_OS_MSWIN
    wxString strandcmd = wxT("\\\\snowman\\win-coremake\\App\\Ncbi\\smart\\bin\\tmsgetstrand.bat");
#else
    wxString strandcmd = wxT("/net/snowman/vol/export2/win-coremake/App/Ncbi/smart/bin/linux/tmsgetstrand.sh");
#endif

    if (!wxFileName::FileExists(strandcmd)) {
        wxMessageBox(_("Path to RNA Strand executable does not exist!"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return;
    } 

    size_t pos = 0;
    while (pos < m_SeqList.size()) {
        size_t max = pos + 25;
        if (max > m_SeqList.size()) {
            max = m_SeqList.size();
        }

        wxArrayString   output; 
        {{

            ///
            /// compose the RNA Strand command line
            ///

            wxStringOutputStream strstr;
            wxTextOutputStream os(strstr);

            os  << strandcmd 
                << wxT(" -a ");

#ifndef NCBI_OS_MSWIN
            os << "\"";
#endif
            for (size_t i = pos; i < max; i++) {
                CBioseq_Handle bsh = m_SeqList[i];
                ITERATE(CBioseq::TId, it, bsh.GetCompleteBioseq()->GetId()) {
                    if ((*it)->IsGenbank() && (*it)->GetGenbank().IsSetAccession()) {
                        os << (*it)->GetGenbank().GetAccession();
                        if (i < max - 1) {
                            os << ",";
                        }
                        break;
                    }
                }
            }

#ifndef NCBI_OS_MSWIN
            os << "\"";
#endif
          


            wxString cmdline = strstr.GetString();
            LOG_POST(Info << "Launching RNA Strand:");
            LOG_POST(Info << cmdline);

            long exit_code = ::wxExecute(cmdline, output, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE);
            if (exit_code != 0)
            {
                wxMessageBox(_("Failed to launch RNA Strand executable."), wxT("Error"), wxOK | wxICON_ERROR, NULL);
                return;
            }

        
        }}

        if (output.IsEmpty()) {
             wxMessageBox(_("Unable to read RNA Strand results."), wxT("Error"), wxOK | wxICON_ERROR, NULL);
             return;
        } else {
                for (size_t i=0; i<output.GetCount(); i++)
                    x_ProcessRNAStrandLine(output[i].ToStdString(), pos, max);
        }

      
        // move marker
        pos = max;
    }

}


void CCorrectRNAStrandDlg::x_GetStatusLocal()
{

}


END_NCBI_SCOPE
