/*  $Id: apply_rna_its_dlg.cpp 42666 2019-04-01 14:27:25Z filippov $
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
#include <objmgr/scope.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objtools/alnmgr/aln_container.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/packages/pkg_sequence_edit/apply_rna_its_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_propagate_feat.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <util/static_map.hpp>

#include <wx/textctrl.h>
#include <wx/arrstr.h>
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <wx/valgen.h>
#include <wx/button.h>
#include <wx/event.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

typedef SStaticPair<const char*, CApplyRNAITSDlg::ERNA_ITSType> TITSLabelPair;
static const TITSLabelPair s_Rna_ITSStrings[] = { 
        {"",                            CApplyRNAITSDlg::eRNA_ITS_unknown},
        {"18S rRNA",                    CApplyRNAITSDlg::eRNA_ITS_18S},
        {"18S small subunit rRNA",      CApplyRNAITSDlg::eRNA_ITS_18S_small},
        {"25S large subunit rRNA",      CApplyRNAITSDlg::eRNA_ITS_25S_large},
        {"25S rRNA",                    CApplyRNAITSDlg::eRNA_ITS_25S},
        {"26S large subunit rRNA",      CApplyRNAITSDlg::eRNA_ITS_26S_large},
        {"26S rRNA",                    CApplyRNAITSDlg::eRNA_ITS_26S},
        {"28S large subunit rRNA",      CApplyRNAITSDlg::eRNA_ITS_28S_large},
        {"28S rRNA",                    CApplyRNAITSDlg::eRNA_ITS_28S},
        {"5.8S rRNA",                   CApplyRNAITSDlg::eRNA_ITS_58S},
        {"ITS1",                        CApplyRNAITSDlg::eRNA_ITS_Its1},
        {"ITS2",                        CApplyRNAITSDlg::eRNA_ITS_Its2},
        {"large subunit rRNA",          CApplyRNAITSDlg::eRNA_ITS_large},
        {"small subunit rRNA",          CApplyRNAITSDlg::eRNA_ITS_small},
};

typedef CStaticArrayMap<string, CApplyRNAITSDlg::ERNA_ITSType> TITSMap;
DEFINE_STATIC_ARRAY_MAP( TITSMap, sm_ITSMap, s_Rna_ITSStrings);

const string& CApplyRNAITSDlg::GetRNA_ITSName( ERNA_ITSType field_type )
{
        TITSMap::const_iterator iter = sm_ITSMap.begin();
        for (;  iter != sm_ITSMap.end();  ++iter){
            if (iter->second == field_type){
                return iter->first;
            }
        }
        return kEmptyStr;
}

CApplyRNAITSDlg::ERNA_ITSType CApplyRNAITSDlg::GetRNA_ITSTypeFromName ( const string& label )
{
    TITSMap::const_iterator iter = sm_ITSMap.find(label);
    if (iter != sm_ITSMap.end()){
        return iter->second;
    }
    return CApplyRNAITSDlg::eRNA_ITS_unknown;
}

typedef SStaticPair<const char*, CApplyRNAITSDlg::ERNA_IGSType> TIGSLabelPair;
static const TIGSLabelPair s_Rna_IGSStrings[] = {
    {"",                        CApplyRNAITSDlg::eRNA_IGS_unknown},
    {"16S rRNA",                CApplyRNAITSDlg::eRNA_IGS_16S},
    {"16S-23S rRNA IGS",        CApplyRNAITSDlg::eRNA_IGS_16S_23S_Igs},
    {"23S rRNA",                CApplyRNAITSDlg::eRNA_IGS_23S},
};

typedef CStaticArrayMap<string, CApplyRNAITSDlg::ERNA_IGSType> TIGSMap;
DEFINE_STATIC_ARRAY_MAP( TIGSMap, sm_IGSMap, s_Rna_IGSStrings);

const string& CApplyRNAITSDlg::GetRNA_IGSName( ERNA_IGSType field_type )
{
        TIGSMap::const_iterator iter = sm_IGSMap.begin();
        for (;  iter != sm_IGSMap.end();  ++iter){
            if (iter->second == field_type){
                return iter->first;
            }
        }
        return kEmptyStr;   
}

CApplyRNAITSDlg::ERNA_IGSType CApplyRNAITSDlg::GetRNA_IGSTypeFromName ( const string& label )
{
    TIGSMap::const_iterator iter = sm_IGSMap.find(label);
    if (iter != sm_IGSMap.end()){
        return iter->second;
    }
    return 	CApplyRNAITSDlg::eRNA_IGS_unknown;
}

IMPLEMENT_DYNAMIC_CLASS( CApplyRNAITSDlg, CBulkCmdDlg )

BEGIN_EVENT_TABLE( CApplyRNAITSDlg, CBulkCmdDlg )
    EVT_CHOICE (ID_CHOICE_FIRST, CApplyRNAITSDlg::OnFirstChoiceSelected )
    EVT_CHOICE (ID_CHOICE_SECOND, CApplyRNAITSDlg::OnAnyOtherChoiceSelected )
    EVT_CHOICE (ID_CHOICE_THIRD, CApplyRNAITSDlg::OnAnyOtherChoiceSelected )
    EVT_CHOICE (ID_CHOICE_FOURTH, CApplyRNAITSDlg::OnAnyOtherChoiceSelected )
    EVT_CHOICE (ID_CHOICE_FIFTH, CApplyRNAITSDlg::OnAnyOtherChoiceSelected )
    EVT_RADIOBOX (ID_RADIOBOX_ITS_IGS, CApplyRNAITSDlg::OnITSRadioBoxSelected )
    EVT_BUTTON( ID_APPLY, CApplyRNAITSDlg::OnGoClick )
    EVT_BUTTON( ID_CLOSEBTN, CApplyRNAITSDlg::OnCancelClick )
    EVT_CLOSE ( CApplyRNAITSDlg::OnCloseWindow )
END_EVENT_TABLE()

CApplyRNAITSDlg::CApplyRNAITSDlg()
{
    Init();
}

CApplyRNAITSDlg::CApplyRNAITSDlg( wxWindow* parent, IWorkbench* wb, 
                    wxWindowID id, const wxString& caption, const wxPoint& pos, 
                    const wxSize& size, long style )
: CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}

bool CApplyRNAITSDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CApplyRNAITSDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CApplyRNAITSDlg creation
    return true;
}

CApplyRNAITSDlg::~CApplyRNAITSDlg()
{
}

void CApplyRNAITSDlg::Init()
{
    SetRegistryPath("CApplyRNAITSDlg");
    m_5partial = NULL;
    m_3partial = NULL;
    m_AlignCoords = NULL;
    m_ITSRadiobox = NULL;
    m_LeaveUp = NULL;
    m_Choice = 0;
    m_Alnmap = 0;
    m_PropSeqids.clear();
    LoadSettings();
}

void CApplyRNAITSDlg::CreateControls()
{    
////@begin CApplyRNAITSDlg content construction
    CApplyRNAITSDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);

    m_5partial = new wxCheckBox( itemCBulkCmdDlg1, ID_CHECKBOX7, _("5' Partial"), wxDefaultPosition, wxDefaultSize, 0 );
    m_5partial->SetValue(true);
    itemBoxSizer3->Add(m_5partial, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_3partial = new wxCheckBox( itemCBulkCmdDlg1, ID_CHECKBOX8, _("3' Partial"), wxDefaultPosition, wxDefaultSize, 0 );
    m_3partial->SetValue(true);
    itemBoxSizer3->Add(m_3partial, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    // if there are alignments, allow the user to specify alignment coordinates
    GetTopLevelSeqEntryAndProcessor(); 
    if (x_IsThereAlignment()){
        m_AlignCoords = new wxCheckBox( itemCBulkCmdDlg1, ID_CHECKBOX9, _("Use alignment coordinates"), wxDefaultPosition, wxDefaultSize, 0 );
        m_AlignCoords->SetValue(false);
        itemBoxSizer7->Add(m_AlignCoords, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
        x_SetupAlnMgr();
    }

    wxArrayString m_ITSRadioboxStrings;
    m_ITSRadioboxStrings.Add(_("&ITS"));
    m_ITSRadioboxStrings.Add(_("&IGS"));
    m_ITSRadiobox = new wxRadioBox( itemCBulkCmdDlg1, ID_RADIOBOX_ITS_IGS, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ITSRadioboxStrings, 1, wxRA_SPECIFY_ROWS );
    m_ITSRadiobox->SetSelection(0);
    itemBoxSizer7->Add(m_ITSRadiobox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    wxArrayString itemChoiceStrings;
    if (!m_Choice){         // ITS is selected
        x_Set_ITSArray (itemChoiceStrings);
    }else{                  // IGS is selected
        x_Set_IGSArray (itemChoiceStrings);
    }

    wxFlexGridSizer* itemFlexGridSizer10 = new wxFlexGridSizer(5, 3, -1, -1);
    itemBoxSizer2->Add(itemFlexGridSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    struct SRNAITSChoice first_choice;
    first_choice.rnaType = new wxChoice( itemCBulkCmdDlg1, ID_CHOICE_FIRST, wxDefaultPosition, wxDefaultSize, itemChoiceStrings, 0 );
    itemFlexGridSizer10->Add(first_choice.rnaType, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    first_choice.from = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_FFROM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(first_choice.from, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    first_choice.to = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_FTO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(first_choice.to, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    struct SRNAITSChoice second_choice;
    second_choice.rnaType = new wxChoice( itemCBulkCmdDlg1, ID_CHOICE_SECOND, wxDefaultPosition, wxDefaultSize, itemChoiceStrings, 0 );
    itemFlexGridSizer10->Add(second_choice.rnaType, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    second_choice.from = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_SFROM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(second_choice.from, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    second_choice.to = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_STO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(second_choice.to, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    struct SRNAITSChoice third_choice;
    third_choice.rnaType = new wxChoice( itemCBulkCmdDlg1, ID_CHOICE_THIRD, wxDefaultPosition, wxDefaultSize, itemChoiceStrings, 0 );
    itemFlexGridSizer10->Add(third_choice.rnaType, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    third_choice.from = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_TFROM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(third_choice.from, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    third_choice.to = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_TTO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(third_choice.to, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    struct SRNAITSChoice fourth_choice;
    fourth_choice.rnaType = new wxChoice( itemCBulkCmdDlg1, ID_CHOICE_FOURTH, wxDefaultPosition, wxDefaultSize, itemChoiceStrings, 0 );
    itemFlexGridSizer10->Add(fourth_choice.rnaType, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    fourth_choice.from = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_FOFROM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(fourth_choice.from, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    fourth_choice.to = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_FOTO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(fourth_choice.to, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    struct SRNAITSChoice fifth_choice;
    fifth_choice.rnaType = new wxChoice( itemCBulkCmdDlg1, ID_CHOICE_FIFTH, wxDefaultPosition, wxDefaultSize, itemChoiceStrings, 0 );
    itemFlexGridSizer10->Add(fifth_choice.rnaType, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    fifth_choice.from = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_FIFROM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(fifth_choice.from, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    fifth_choice.to = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL_FITO, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(fifth_choice.to, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer28, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_OK = new wxButton( itemCBulkCmdDlg1, ID_APPLY, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer28->Add(m_OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Cancel = new wxButton( itemCBulkCmdDlg1, ID_CLOSEBTN, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer28->Add(m_Cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LeaveUp = new wxCheckBox( itemCBulkCmdDlg1, ID_LEAVE_DIALOG_UP, _("Leave Dialog Up"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveUp->SetValue(false);
    itemBoxSizer28->Add(m_LeaveUp, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CApplyRNAITSDlg content construction

    // connect events and objects
    first_choice.from->Connect(ID_TEXTCTRL_FFROM, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlFromKillFocus), NULL, this);
    first_choice.to->Connect(ID_TEXTCTRL_FTO, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlToKillFocus), NULL, this);
    m_ChoiceList.push_back(first_choice);
    
    second_choice.from->Connect(ID_TEXTCTRL_SFROM, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlFromKillFocus), NULL, this);
    second_choice.to->Connect(ID_TEXTCTRL_STO, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlToKillFocus), NULL, this);
    m_ChoiceList.push_back(second_choice);
    
    third_choice.from->Connect(ID_TEXTCTRL_TFROM, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlFromKillFocus), NULL, this);
    third_choice.to->Connect(ID_TEXTCTRL_TTO, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlToKillFocus), NULL, this);
    m_ChoiceList.push_back(third_choice);
    
    fourth_choice.from->Connect(ID_TEXTCTRL_FOFROM, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlFromKillFocus), NULL, this);
    fourth_choice.to->Connect(ID_TEXTCTRL_FOTO, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlToKillFocus), NULL, this);
    m_ChoiceList.push_back(fourth_choice);
    
    fifth_choice.from->Connect(ID_TEXTCTRL_FIFROM, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlFromKillFocus), NULL, this);
    fifth_choice.to->Connect(ID_TEXTCTRL_FITO, wxEVT_KILL_FOCUS, wxFocusEventHandler(CApplyRNAITSDlg::OnCtrlToKillFocus), NULL, this);
    m_ChoiceList.push_back(fifth_choice);
    
    for (size_t n= 0; n < m_ChoiceList.size(); n++) {
        m_ChoiceList[n].isEmpty = CApplyRNAITSDlg::eChoice_empty;
    }

    m_ITSRadiobox->SetValidator( wxGenericValidator(&m_Choice));

    wxWindow::InitDialog(); // uncomment this row, if the frame is changed to dialog
}

bool CApplyRNAITSDlg::x_IsThereAlignment( void )
{
    if (m_TopSeqEntry) {
        CAlign_CI align_it(m_TopSeqEntry);
        if (align_it) {
            const CSeq_align& align = align_it.GetOriginalSeq_align();
            if (align.IsSetSegs() && align.GetSegs().IsDenseg()) {
                return true;
            }
        }
    }
    return false;
}

void CApplyRNAITSDlg::x_SetupAlnMgr()
{
    m_Alnmap.Reset();
    if (m_TopSeqEntry) {
        CAlign_CI align_iter(m_TopSeqEntry);
        /* the seqentry should have at most one alignment*/
        if (align_iter) {
            const CSeq_align& align = align_iter.GetOriginalSeq_align();
            if (align.IsSetSegs() && align.GetSegs().IsDenseg()) {
                m_Alnmap.Reset(new CAlnMap(align.GetSegs().GetDenseg()));
            }
        }
    }
}

void CApplyRNAITSDlg::x_Set_ITSArray ( wxArrayString& Its_tags )
{
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_unknown)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_18S)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_small)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_18S_small)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_Its1)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_58S)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_Its2)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_28S)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_26S)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_25S)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_large)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_28S_large)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_26S_large)) );
    Its_tags.Add( ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_25S_large)) );
}

void CApplyRNAITSDlg::x_Set_IGSArray ( wxArrayString& Igs_tags )
{
    TIGSMap::const_iterator iter = sm_IGSMap.begin();
    for (;  iter != sm_IGSMap.end();  ++iter){
        Igs_tags.Add( ToWxString(iter->first) );
    }
}

CApplyRNAITSDlg::TVecConstSeqFeat CApplyRNAITSDlg::x_UpdatePartials(TVecSeqFeat& featlist)
{
    TVecConstSeqFeat cfeatlist;

    // set the first feature to partial
    TVecSeqFeat::iterator feat_it = featlist.begin();
    if (m_5partial->GetValue()) { 
        (*feat_it)->SetLocation().SetPartialStart(true, eExtreme_Biological);
        (*feat_it)->SetPartial(true);
    }
    // set the last feature to partial
    if (m_3partial->GetValue()) { 
        feat_it = featlist.end() - 1;
        (*feat_it)->SetLocation().SetPartialStop(true, eExtreme_Biological);
        (*feat_it)->SetPartial(true);
    }

    ITERATE (TVecSeqFeat, iter, featlist) {
        cfeatlist.push_back(CConstRef<CSeq_feat>(&(**iter)));
    }
    return cfeatlist;
}

static const char* kApplyChoice = "ApplyRNA_ITS";

void CApplyRNAITSDlg::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CApplyRNAITSDlg::SaveSettings() const
{
    if (! m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        view.Set(kApplyChoice, m_Choice);
    }
}

void CApplyRNAITSDlg::LoadSettings()
{
    if (! m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        m_Choice = view.GetInt(kApplyChoice, m_Choice); 
    }
}

void CApplyRNAITSDlg::OnGoClick ( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd = GetCommand();
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
        if (!m_LeaveUp->GetValue()) {
             Close();
            }
    } else {
        string error = GetErrorMessage();
        if (!NStr::IsBlank(error)) {
            wxMessageBox(ToWxString(error), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        }
    }
}

void CApplyRNAITSDlg::OnCancelClick ( wxCommandEvent& event )
{
    x_SaveAndDestroyFrame();
}

void CApplyRNAITSDlg::OnCloseWindow ( wxCloseEvent& event )
{
    x_SaveAndDestroyFrame();
}

void CApplyRNAITSDlg::x_SaveAndDestroyFrame()
{
    wxWindow::TransferDataFromWindow(); // uncomment this row, if the frame is changed to dialog
    SaveSettings();
    Destroy();
}

/*!
 * Should we show tooltips?
 */

bool CApplyRNAITSDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CApplyRNAITSDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CApplyRNAITSDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CApplyRNAITSDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CApplyRNAITSDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CApplyRNAITSDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CApplyRNAITSDlg icon retrieval
}

void CApplyRNAITSDlg::OnFirstChoiceSelected( wxCommandEvent& event )
{
    string value = ToStdString(m_ChoiceList[0].rnaType->GetStringSelection());
    
    if (! m_ITSRadiobox->GetSelection()){    // ITS is selected
        CApplyRNAITSDlg::ERNA_ITSType selected = GetRNA_ITSTypeFromName(value);
        if (selected == CApplyRNAITSDlg::eRNA_ITS_unknown){
            m_ChoiceList[0].from->ChangeValue(wxEmptyString);
            m_ChoiceList[0].to->ChangeValue(wxEmptyString);
            m_ChoiceList[0].isEmpty = CApplyRNAITSDlg::eChoice_empty;
        }else{
            m_ChoiceList[0].from->ChangeValue(_("1"));
            m_ChoiceList[0].from->SetModified(false);
            m_ChoiceList[0].isEmpty = CApplyRNAITSDlg::eChoice_not_empty;
        }
        for (size_t n=1; n<m_ChoiceList.size(); n++){
            selected = GetNextRNA_ITSChoice(selected);
            m_ChoiceList[n].rnaType->SetStringSelection(ToWxString(GetRNA_ITSName(selected)));
            
            if (selected == CApplyRNAITSDlg::eRNA_ITS_unknown){
                m_ChoiceList[n].from->ChangeValue(wxEmptyString);
                m_ChoiceList[n].to->ChangeValue(wxEmptyString);
                m_ChoiceList[n].isEmpty = CApplyRNAITSDlg::eChoice_empty;
            }else{
                m_ChoiceList[n].isEmpty = CApplyRNAITSDlg::eChoice_not_empty;
            }
        } 
        
    } else { // IGS is selected
        CApplyRNAITSDlg::ERNA_IGSType selected = GetRNA_IGSTypeFromName(value);
        if (selected == CApplyRNAITSDlg::eRNA_IGS_unknown){
            m_ChoiceList[0].from->ChangeValue(wxEmptyString);
            m_ChoiceList[0].to->ChangeValue(wxEmptyString);
            m_ChoiceList[0].isEmpty = CApplyRNAITSDlg::eChoice_empty;
        }else{
            m_ChoiceList[0].from->ChangeValue(_("1"));
            m_ChoiceList[0].isEmpty = CApplyRNAITSDlg::eChoice_not_empty;
        }
        for (size_t n = 1; n < m_ChoiceList.size(); ++n){
            selected = GetNextRNA_IGSChoice(selected);
            m_ChoiceList[n].rnaType->SetStringSelection(ToWxString(GetRNA_IGSName(selected)));
            
            if (selected == CApplyRNAITSDlg::eRNA_IGS_unknown){
                m_ChoiceList[n].from->ChangeValue(wxEmptyString);
                m_ChoiceList[n].to->ChangeValue(wxEmptyString);
                m_ChoiceList[n].isEmpty = CApplyRNAITSDlg::eChoice_empty;
            }else{
                m_ChoiceList[n].isEmpty = CApplyRNAITSDlg::eChoice_not_empty;
            }
        }
    } // the first choice is the 'driving field': if that gets empty, everything will become empty
}

void CApplyRNAITSDlg::OnAnyOtherChoiceSelected ( wxCommandEvent& event )
{
    unsigned int event_id = 1;
    
    switch (event.GetId()) {
        case ID_CHOICE_SECOND:
            break;
        case ID_CHOICE_THIRD:
            event_id++;
            break;
        case ID_CHOICE_FOURTH:
            event_id = event_id + 2;
            break;
        case ID_CHOICE_FIFTH:
            event_id = event_id + 3;
            break;
    }
    string value = ToStdString(m_ChoiceList[event_id].rnaType->GetStringSelection());

    if (! m_ITSRadiobox->GetSelection()){    // ITS is selected
        CApplyRNAITSDlg::ERNA_ITSType selected = GetRNA_ITSTypeFromName(value);
        if (selected == CApplyRNAITSDlg::eRNA_ITS_unknown) {
            for (SIZE_TYPE n = event_id; n < m_ChoiceList.size(); ++n) {
                m_ChoiceList[n].rnaType->SetStringSelection(ToWxString(GetRNA_ITSName(CApplyRNAITSDlg::eRNA_ITS_unknown)));
                m_ChoiceList[n].from->ChangeValue(wxEmptyString);
                m_ChoiceList[n].to->ChangeValue(wxEmptyString);
                m_ChoiceList[n].isEmpty = CApplyRNAITSDlg::eChoice_empty;
            }
        } else {
            m_ChoiceList[event_id].isEmpty = CApplyRNAITSDlg::eChoice_not_empty;
        }
    } else { // IGS is selected
        CApplyRNAITSDlg::ERNA_IGSType selected = GetRNA_IGSTypeFromName(value);
        if (selected == CApplyRNAITSDlg::eRNA_IGS_unknown){
            for (SIZE_TYPE n = event_id; n < m_ChoiceList.size(); ++n) {
                m_ChoiceList[n].rnaType->SetStringSelection(ToWxString(GetRNA_IGSName(CApplyRNAITSDlg::eRNA_IGS_unknown)));
                m_ChoiceList[n].from->ChangeValue(wxEmptyString);
                m_ChoiceList[n].to->ChangeValue(wxEmptyString);
                m_ChoiceList[n].isEmpty = CApplyRNAITSDlg::eChoice_empty;
            }
        } else {
            m_ChoiceList[event_id].isEmpty = CApplyRNAITSDlg::eChoice_not_empty;
        }
    }	
}


/* if the 'FROM' field was set to n in row i, the 'TO' field becomes n-1 in row i-1 */
void CApplyRNAITSDlg::OnCtrlFromKillFocus( wxFocusEvent& event )
{
    event.Skip();
    unsigned int row_sel = 0;      // the updated row
    while (row_sel < m_ChoiceList.size() && !m_ChoiceList[row_sel].from->IsModified())
        row_sel++;
    
    if (row_sel == m_ChoiceList.size()) return;
    if (m_ChoiceList[row_sel].isEmpty == CApplyRNAITSDlg::eChoice_empty){
        m_ChoiceList[row_sel].from->ChangeValue(wxEmptyString);
    } else if (m_ChoiceList[row_sel].from->GetLineLength(0) > 0){
        string from_value = ToStdString(m_ChoiceList[row_sel].from->GetValue());
        if (from_value.find_first_not_of("0123456789") != NPOS){
            m_ChoiceList[row_sel].from->ChangeValue(_("1"));
        } else {
            try{
                int from = NStr::StringToInt(from_value);      // selected "FROM" position in 'row_sel' row
                if (from > 1 && row_sel >= 1){
                    m_ChoiceList[row_sel - 1].to->ChangeValue(wxString(NStr::NumericToString(from-1)));
                }
            
                if (from <= 0){
                    from = 1;
                    m_ChoiceList[row_sel].from->ChangeValue(wxString(NStr::NumericToString(from)));
                }
            } catch (const CException& e) { 
                LOG_POST(Error << "CApplyRNAITSDlg::OnCtrlFromKillFocus: " << e.GetMsg());
                m_ChoiceList[row_sel].from->SetModified(false);
                return; 
            }
        } 
    }
    m_ChoiceList[row_sel].from->SetModified(false);
}

/* if the 'TO' field was set to n in row i, the 'FROM' field becomes n+1 in row i+1 */
void CApplyRNAITSDlg::OnCtrlToKillFocus( wxFocusEvent& event )
{
    event.Skip();
    unsigned int row_sel = 0;         // the updated row
        
    while (row_sel < m_ChoiceList.size() && !m_ChoiceList[row_sel].to->IsModified())
        row_sel++;
    
    if (row_sel == m_ChoiceList.size()) return;
    if (m_ChoiceList[row_sel].isEmpty == CApplyRNAITSDlg::eChoice_empty){ 
        m_ChoiceList[row_sel].to->ChangeValue(wxEmptyString);
    } else if (m_ChoiceList[row_sel].to->GetLineLength(0) > 0 && m_ChoiceList[row_sel].from->GetLineLength(0) > 0) {
        string from_value = ToStdString(m_ChoiceList[row_sel].from->GetValue());	
        string to_value   = ToStdString(m_ChoiceList[row_sel].to->GetValue());
        if ((from_value.find_first_not_of("0123456789") != NPOS) || (to_value.find_first_not_of("0123456789") != NPOS)){
            m_ChoiceList[row_sel].to->ChangeValue(_("1"));
        } else {
            try {
                int to   = NStr::StringToInt(to_value);     // "TO" position in 'row_sel' row
                int from = NStr::StringToInt(from_value);   // "FROM" position in 'row_sel' row
                if (to <= from){
                    to = from + 1;
                    m_ChoiceList[row_sel].to->ChangeValue(wxString(NStr::NumericToString(to)));
                }
                if (row_sel < m_ChoiceList.size() - 1 && 
                    m_ChoiceList[row_sel +1].isEmpty != CApplyRNAITSDlg::eChoice_empty){
                    m_ChoiceList[row_sel + 1].from->ChangeValue(wxString(NStr::NumericToString(to + 1)));
                }
            }
            catch (const CException& e) {
                LOG_POST(Error << "CApplyRNAITSDlg::OnCtrlToKillFocus: " << e.GetMsg());
                m_ChoiceList[row_sel].to->SetModified(false);
                return;
            }
        }
    }	 
    m_ChoiceList[row_sel].to->SetModified(false);
}

void CApplyRNAITSDlg::OnITSRadioBoxSelected( wxCommandEvent& event )
{
    m_Choice = m_ITSRadiobox->GetSelection(); 
    
    wxArrayString labels;
    if (!m_Choice){          // ITS selected
        x_Set_ITSArray (labels);
    }else{                   // IGS selected
        x_Set_IGSArray (labels);
    }
    
    for (size_t n=0; n < m_ChoiceList.size(); n++){
        m_ChoiceList[n].rnaType->Clear();
        m_ChoiceList[n].rnaType->Append(labels);
        m_ChoiceList[n].from->Clear();
        m_ChoiceList[n].to->Clear();
    }
}


CApplyRNAITSDlg::ERNA_ITSType CApplyRNAITSDlg::GetNextRNA_ITSChoice( CApplyRNAITSDlg::ERNA_ITSType field_type )
{
    CApplyRNAITSDlg::ERNA_ITSType next_choice = CApplyRNAITSDlg::eRNA_ITS_unknown;
    switch(field_type){
        case CApplyRNAITSDlg::eRNA_ITS_18S:
            next_choice = CApplyRNAITSDlg::eRNA_ITS_Its1;
            break;
        case CApplyRNAITSDlg::eRNA_ITS_small:
            next_choice = CApplyRNAITSDlg::eRNA_ITS_Its1;
            break;
        case CApplyRNAITSDlg::eRNA_ITS_18S_small:
            next_choice = CApplyRNAITSDlg::eRNA_ITS_Its1;
            break;
        case CApplyRNAITSDlg::eRNA_ITS_Its1:
            next_choice = CApplyRNAITSDlg::eRNA_ITS_58S;
            break;
        case CApplyRNAITSDlg::eRNA_ITS_58S:
            next_choice = CApplyRNAITSDlg::eRNA_ITS_Its2;
            break;
        case CApplyRNAITSDlg::eRNA_ITS_Its2:{
            next_choice = CApplyRNAITSDlg::eRNA_ITS_28S;
            CApplyRNAITSDlg::ERNA_ITSType first_selected = 
                GetRNA_ITSTypeFromName( ToStdString(m_ChoiceList[0].rnaType->GetStringSelection()) );
            if ( first_selected == CApplyRNAITSDlg::eRNA_ITS_small){
                next_choice = CApplyRNAITSDlg::eRNA_ITS_large;
            } else if (first_selected == CApplyRNAITSDlg::eRNA_ITS_18S_small){
                next_choice = CApplyRNAITSDlg::eRNA_ITS_28S_large;
            }
            break;
        }
        default:
            next_choice = CApplyRNAITSDlg::eRNA_ITS_unknown;
            break;
    }
    
    return next_choice;
}

CApplyRNAITSDlg::ERNA_IGSType CApplyRNAITSDlg::GetNextRNA_IGSChoice( CApplyRNAITSDlg::ERNA_IGSType field_type )
{
    CApplyRNAITSDlg::ERNA_IGSType next_choice = CApplyRNAITSDlg::eRNA_IGS_unknown;
    switch (field_type){
        case CApplyRNAITSDlg::eRNA_IGS_16S:
            next_choice = CApplyRNAITSDlg::eRNA_IGS_16S_23S_Igs;
            break;
        case CApplyRNAITSDlg::eRNA_IGS_16S_23S_Igs:
            next_choice = CApplyRNAITSDlg::eRNA_IGS_23S;
            break;
        default:
            next_choice = CApplyRNAITSDlg::eRNA_IGS_unknown;
            break;
    }
    
    return next_choice;
}

string CApplyRNAITSDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}

CRef<CCmdComposite> CApplyRNAITSDlg::GetCommand()
{
    CRef<CCmdComposite> empty(NULL);
    GetTopLevelSeqEntryAndProcessor(); 

    bool align_present = false;
    if (x_IsThereAlignment()){
        align_present = true;
        x_SetupAlnMgr();
    } 
    m_ErrorMessage = kEmptyStr;
    m_PropSeqids.clear();
    if (!m_TopSeqEntry) {
        m_ErrorMessage = "Unknown error";
        return empty;
    }
    
    TConstScopedObjects objects = GetSelectedObjects();
    if (objects.empty()) {
        m_ErrorMessage = "No sequences were selected!";
        return empty;
    }
    // check whether all selected objects belong to the same scope:
    CRef<CScope> sel_scope(NULL);
    ITERATE(TConstScopedObjects, it, objects) {
        if (!sel_scope) {
            sel_scope = it->scope;
        } else if (sel_scope != it->scope) {
            m_ErrorMessage = "Selected objects are from different projets.\nPlease move all objects to the same project and try again.";
            return empty;
        }
    }

    CRef<CCmdComposite> composite(new CCmdComposite("Apply rRNA and ITS"));
    bool modified = false;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    if (objects.size() == 1) {
        const CObject* ptr = objects.begin()->object.GetPointer();

        const CSeq_entry* seqentry = dynamic_cast<const CSeq_entry* >(ptr);
        const CSeq_submit* seqsubmit = dynamic_cast<const CSeq_submit* >(ptr);
        if (seqentry || (seqsubmit && seqsubmit->IsEntrys())) {
            Apply_RNAITS_ToTSE(composite, modified);
        }
    } else if (FocusedOnNucleotides(objects, m_TopSeqEntry)) {
        const CObject* ptr = objects.back().object.GetPointer();

        const CSeq_entry* seqentry = dynamic_cast<const CSeq_entry* >(ptr);
        const CSeq_submit* seqsubmit = dynamic_cast<const CSeq_submit* >(ptr);
        if (seqentry || (seqsubmit && seqsubmit->IsEntrys())) {
            Apply_RNAITS_ToTSE(composite, modified);
        }
    } else {
        NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
            const CObject* ptr = it->object.GetPointer();

            const CSeq_loc* seqloc = dynamic_cast<const CSeq_loc*>(ptr);
            const CBioseq* bioseq = dynamic_cast<const CBioseq* >(ptr);
            const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set* >(ptr);

            if (seqloc && seqloc->IsWhole()) {
                CRef<CSeq_id> seqid(new CSeq_id());
                seqid->Assign(seqloc->GetWhole());
                CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*seqid);
                if (bsh.IsNa()) { // skip the amino acid sequences
                    if ( ! align_present || (align_present && ! x_WasPropagatedTo(bsh)) ) {
                        Apply_RNAITS_ToBioseq(bsh, composite, modified, create_general_only);
                    }
                } else if (bsh.IsAa()){
                    m_ErrorMessage = "Selected sequence is a protein.";
                    break;
                }
            } else if (bioseq) {
                if (bioseq->IsNa()) { 
                    CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*bioseq, CScope::eMissing_Null);
                    if ( ! align_present || (align_present && ! x_WasPropagatedTo(bsh)) ) {
                        Apply_RNAITS_ToBioseq(bsh, composite, modified, create_general_only);
                    }
                } else if (bioseq->IsAa()){ 
                    m_ErrorMessage = "Selected sequence is a protein.";
                    break;
                }
            } else if (bioseq_set) { 
                CBioseq_set_Handle bssh = m_TopSeqEntry.GetScope().GetBioseq_setHandle(*bioseq_set, CScope::eMissing_Null);
                CSeq_entry_Handle seh = bssh.GetParentEntry();
                Apply_RNAITS_ToBioseqSet(seh, composite, modified);
            } 
        }
    }
    
    if (modified) {
        return composite;
    }
    return empty;
}

/* create features (featlist) on the bioseq */
void CApplyRNAITSDlg::AddFeatures (const CBioseq_Handle& bsh, TVecConstSeqFeat& featlist, CRef<CCmdComposite> composite, bool& modified)
{
    if ( ! featlist.empty()) {
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        TVecConstSeqFeat::iterator feat_it = featlist.begin();
        for ( ; feat_it != featlist.end(); ++feat_it) {
            composite->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, (*feat_it).GetObject())));
            modified = true;
        }
    }
}

CApplyRNAITSDlg::TVecConstSeqFeat CApplyRNAITSDlg::ObtainRNAITSList(const CBioseq_Handle& bsh)
{
    if (m_AlignCoords && m_AlignCoords->GetValue()) {    // alignment coordinates have been entered   
        return GetRNAITSList_AlignCoordFromDlg(bsh);
    } else {                                             // sequence coordinates have been entered
        return GetRNAITSList_SeqCoordFromDlg(bsh);
    }
}


void CApplyRNAITSDlg::Apply_RNAITS_ToBioseq( const CBioseq_Handle& bsh, CRef<CCmdComposite> composite, bool& modified, bool create_general_only )
{
    if ( ! bsh ) return;
    SIZE_TYPE align_nr= x_BioseqInAlignments(bsh);
    if (align_nr > 1) {
        m_ErrorMessage = "Cannot proceed. This bioseq belongs to more than one alignment.";
        return;
    } else if (align_nr == 1) {  // bioseq belongs to exactly one alignment
        int answer = wxMessageBox (ToWxString("Propagate to other sequences?"), ToWxString("Question"),
                                   wxYES_NO | wxCANCEL | wxICON_QUESTION);
        switch (answer) {
            case wxYES: {
                /* apply the features to this sequence and propagate them to other sequences as well, 
                   but to those only, that are in alignment with this particular sequence */
                TVecConstSeqFeat featlist = ObtainRNAITSList(bsh);
                AddFeatures(bsh, featlist, composite, modified);
                Propagate_RNAITSList(bsh, featlist, composite, modified, create_general_only); 
                break;
            }
            case wxNO: {
                TVecConstSeqFeat featlist = ObtainRNAITSList(bsh);
                AddFeatures(bsh, featlist, composite, modified);
                break;
            }
            case wxCANCEL:
                break;
        }
    } else { // the sequence does not belong to any alignment
        if (m_AlignCoords && m_AlignCoords->GetValue()) {
            m_ErrorMessage = "This sequence is not in the alignment!";
            return;
        } else {
            /* enter when viewing one sequence, without alignment */
            TVecConstSeqFeat featlist = GetRNAITSList_SeqCoordFromDlg(bsh);
            AddFeatures(bsh, featlist, composite, modified);
        }
    } 
}

void CApplyRNAITSDlg::Apply_RNAITS_ToBioseqSet( CSeq_entry_Handle entry, CRef<CCmdComposite> composite, bool& modified )
{
    /* apply the feature list to all nt sequences within the set */
    if ( !entry || entry.IsSeq() ) return;
    CBioseq_CI b_iter(entry, CSeq_inst::eMol_na);
    for ( ; b_iter; ++b_iter) {
            TVecConstSeqFeat featlist = ObtainRNAITSList(*b_iter);
            AddFeatures(*b_iter, featlist, composite, modified);
        }
}

void CApplyRNAITSDlg::Apply_RNAITS_ToTSE( CRef<CCmdComposite> composite, bool& modified )
{
    if ( ! m_TopSeqEntry) return;
    int answer = wxMessageBox (ToWxString("You are not viewing a single Bioseq. Apply to all Bioseqs?"), 
                            ToWxString("Question"), wxYES_NO | wxCANCEL | wxICON_QUESTION);
    switch (answer) {
        case wxYES: {
            CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na);
            for ( ; b_iter; ++b_iter) {
                TVecConstSeqFeat featlist = ObtainRNAITSList(*b_iter);
                AddFeatures(*b_iter, featlist, composite, modified);
            }
            break;
        }
        case wxNO: {
            /* apply only to the first nt sequence in TSE and do not give the option of propagation */
            CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na);
            if (b_iter) {
                TVecConstSeqFeat featlist = ObtainRNAITSList(*b_iter);
                AddFeatures(*b_iter, featlist, composite, modified);
            }
            break;
        }
        case wxCANCEL:
            break;
    }
}

void CApplyRNAITSDlg::Propagate_RNAITSList( const CBioseq_Handle& bsh, TVecConstSeqFeat& featlist, CRef<CCmdComposite> composite, bool& modified, bool create_general_only )
{
    if ( ! featlist.empty()) {
        CConstRef<CSeq_id> seq_id = bsh.GetSeqId();
        vector<CConstRef<CSeq_align> > alignments = x_GetAlignments(bsh);
        if (alignments.size() == 1) {
            // find the target sequence IDs:
            const CSeq_align& align = *(alignments[0]);
            TVecConstSeqId targetIDs = x_SeqIdsAlignedWith(bsh, align);
                 
            CMessageListener_Basic listener;
            ITERATE (TVecConstSeqId, target_id, targetIDs) {
                const CSeq_id& tgseq_id = (*target_id).GetObject();
                CBioseq_Handle target_bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(tgseq_id);
                PropagateToTarget(composite, featlist, bsh, target_bsh, align,
                                  false, false, true, true, create_general_only, &listener);
                modified = true;
            }
            if (listener.Count() > 0) {
                string msg = "";
                for (size_t i = 0; i < listener.Count(); i++) {
                    msg += listener.GetMessage(i).GetText() + "\n";
                }
                wxMessageBox(ToWxString(msg), wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
            }

        }
    }
}

vector<CConstRef<CSeq_align> > CApplyRNAITSDlg::x_GetAlignments( const CBioseq_Handle& bsh )
{
    vector<CConstRef<CSeq_align> > alignments;

    if (m_TopSeqEntry) {
        CAlign_CI align_it(m_TopSeqEntry);
        for ( ; align_it; ++align_it) {
            const CSeq_align& align = align_it.GetOriginalSeq_align();
            
            /* proceed only if the alignment is of dense-seg type*/
            if (align.IsSetSegs() && align.GetSegs().IsDenseg() && align.GetSegs().GetDenseg().IsSetIds()) {
                CDense_seg_Base::TIds alignIDs = align.GetSegs().GetDenseg().GetIds();
                ITERATE (CDense_seg_Base::TIds, align_id, alignIDs) {
                    CBioseq::TId bseqIDs = bsh.GetCompleteBioseq()->GetId();
                    ITERATE (CBioseq::TId, bseq_id, bseqIDs) {
                        if ((*align_id)->Match(**bseq_id)) {
                            alignments.push_back(CConstRef<CSeq_align>(&align));
                        }
                    }
                }
            }
        }
    }

    return alignments;
}

SIZE_TYPE CApplyRNAITSDlg::x_BioseqInAlignments ( const CBioseq_Handle& bsh )
{
    vector<CConstRef<CSeq_align> > alignments = x_GetAlignments(bsh);
    return alignments.size();
}

CApplyRNAITSDlg::TVecConstSeqId CApplyRNAITSDlg::x_SeqIdsAlignedWith( const CBioseq_Handle& bsh, const CSeq_align& align)
{
    TVecConstSeqId seqids;

    if (m_TopSeqEntry) {
        CDense_seg::TIds alignIDs = align.GetSegs().GetDenseg().GetIds();
        ITERATE (CDense_seg::TIds, align_id, alignIDs) {
            CBioseq::TId bseqIDs = bsh.GetCompleteBioseq()->GetId();
            bool found = false;
            for (CBioseq::TId::const_iterator bseq_id = bseqIDs.begin(); bseq_id != bseqIDs.end() && !found; ++bseq_id) {
                if ((*align_id)->Match(**bseq_id)) {
                    found = true;
                }
            }
            if ( !found) {
                seqids.push_back(CConstRef<CSeq_id>(&(**align_id)));
                m_PropSeqids.push_back(CConstRef<CSeq_id>(&(**align_id)));
            }
        }
    }
    return seqids;
}

bool CApplyRNAITSDlg::x_WasPropagatedTo(const CBioseq_Handle& bsh)
{
    CBioseq::TId bseqIDs = bsh.GetCompleteBioseq()->GetId();
    ITERATE (CBioseq::TId, bseq_id, bseqIDs) {
        ITERATE (TVecConstSeqId, iter, m_PropSeqids) {
            const CSeq_id& prop_id = (*iter).GetObject();
            if ((*bseq_id)->Match(prop_id)) {
                return true;
            }
        }
    }
    return false;
}

IAlnExplorer::TNumrow CApplyRNAITSDlg::x_GetRow(const CBioseq& bioseq)
{ 
    if (m_Alnmap) {
        CBioseq::TId ids = bioseq.GetId();
        ITERATE (CBioseq::TId, id, ids) {
            for (IAlnExplorer::TNumrow row = 0; row < m_Alnmap->GetNumRows(); ++row) {
                if ((*id)->Match(m_Alnmap->GetSeqId(row))) {
                    return row;
                }
            }
        }
    } 
    
    IAlnExplorer::TNumrow row = -1;
    return row;
}

CApplyRNAITSDlg::TVecConstSeqFeat CApplyRNAITSDlg::GetRNAITSList_AlignCoordFromDlg( const CBioseq_Handle& bsh )
{
    TVecConstSeqFeat const_featlist;
    TVecSeqFeat featlist;
    if (m_Alnmap) { // alignment coordinates have been entered

        int min = 0;
        int max = bsh.GetBioseqLength() - 1;
        for (unsigned int row =0; row < m_ChoiceList.size(); ++row) {
            if (m_ChoiceList[row].isEmpty == CApplyRNAITSDlg::eChoice_empty) {
                const_featlist = x_UpdatePartials(featlist);
                return const_featlist;
            } else {
                int aln_from = 0, seq_from = min;
                int aln_to = 0, seq_to = max;

                string from_value = ToStdString(m_ChoiceList[row].from->GetValue());
                if (NStr::IsBlank(from_value)) {
                    break;
                } else {
                    const CBioseq& bioseq = *(bsh.GetCompleteBioseq());
                    IAlnExplorer::TNumrow seqrow = x_GetRow(bioseq);
                    if (seqrow == -1) {
                        m_ErrorMessage = "This sequence is not in the alignment!";
                        const_featlist = x_UpdatePartials(featlist);
                        return const_featlist;
                    } 
                    try {
                        aln_from = NStr::StringToInt(from_value); 
                        aln_from = aln_from - 1;
                        seq_from = m_Alnmap->GetSeqPosFromAlnPos(seqrow, aln_from, IAlnExplorer::eForward);
                        if (seq_from < min) { seq_from = min; }
                        
                        // stop if the chain extends past the length of this Bioseq
                        if (seq_from > max) { 
                            const_featlist = x_UpdatePartials(featlist);
                            return const_featlist; 
                        }
                    } catch(const CException& e) {
                        LOG_POST(Error << "CApplyRNAITSDlg::GetRNAITSList_AlignCoordFromDlg: " << e.GetMsg());
                        const_featlist = x_UpdatePartials(featlist);
                        return const_featlist; 
                    }
                

                    string to_value = ToStdString(m_ChoiceList[row].to->GetValue());
                    if (!NStr::IsBlank(to_value)) {
                        try {
                            aln_to = NStr::StringToInt(to_value);
                            aln_to = aln_to - 1;
                            seq_to = m_Alnmap->GetSeqPosFromAlnPos(seqrow, aln_to, IAlnExplorer::eBackwards);
                            if (seq_to > max) { seq_to = max; }
                        } catch(const CException& e) {
                            LOG_POST(Error << "CApplyRNAITSDlg::GetRNAITSList_AlignCoordFromDl: " << e.GetMsg());
                            const_featlist = x_UpdatePartials(featlist);
                            return const_featlist; 
                        }
                    }

                    if (seq_from == seq_to) {
                        continue;
                    }

                    string rna_name = ToStdString(m_ChoiceList[row].rnaType->GetStringSelection());
                    NStr::ReplaceInPlace(rna_name, "rRNA", "ribosomal RNA");
                    NStr::ReplaceInPlace(rna_name, "ITS", "internal transcribed spacer ");
                    NStr::ReplaceInPlace(rna_name, "IGS", "intergenic spacer");
                
                    if (NStr::FindNoCase(rna_name, "spacer") != NPOS) {
                        CRef<CSeq_feat> new_feat = AddMiscRNA(bsh, rna_name, seq_from, seq_to, false, false);
                        featlist.push_back(new_feat);
                    } else {
                        CRef<CSeq_feat> new_feat = AddRibosomalRNA(bsh, rna_name, seq_from, seq_to, false, false);
                        featlist.push_back(new_feat);
                    }
                }
            }
        }
    }
    
    const_featlist = x_UpdatePartials(featlist);
    return const_featlist;
}



CApplyRNAITSDlg::TVecConstSeqFeat CApplyRNAITSDlg::GetRNAITSList_SeqCoordFromDlg( const CBioseq_Handle& bsh )
{ 
    TVecConstSeqFeat const_featlist;
    TVecSeqFeat featlist;
    
    int min = 0;
    int max = bsh.GetBioseqLength() - 1;
    for (unsigned int row = 0; row < m_ChoiceList.size(); ++row) {
        if (m_ChoiceList[row].isEmpty == CApplyRNAITSDlg::eChoice_empty) {
            const_featlist = x_UpdatePartials(featlist);
            return const_featlist;
        } else {
            int from = min;
            int to = max;
            
            string from_value = ToStdString(m_ChoiceList[row].from->GetValue());
            if (NStr::IsBlank(from_value)) {
                break;
            } else {
                try {
                    from = NStr::StringToInt(from_value);
                    from = from - 1;
                    if (from < min ) { from = min; }
                    
                    // stop if the chain extends past the length of this Bioseq
                    if (from >= max) { 
                        const_featlist = x_UpdatePartials(featlist);
                        return const_featlist; 
                    }
                }
                catch (const CException& e) {
                    LOG_POST(Error << "CApplyRNAITSDlg::GetRNAITSList_SeqCoordFromDlg: " << e.GetMsg());
                    const_featlist = x_UpdatePartials(featlist);
                    return const_featlist;
                }
                string to_value = ToStdString(m_ChoiceList[row].to->GetValue());
                if (!NStr::IsBlank(to_value)) {
                    try {
                        to = NStr::StringToInt(to_value);
                        to = to - 1;
                        if (to > max) { to = max; } 
                    }
                    catch (const CException& e) {
                        LOG_POST(Error << "CApplyRNAITSDlg::GetRNAITSList_SeqCoordFromDlg: " << e.GetMsg());
                        const_featlist = x_UpdatePartials(featlist);
                        return const_featlist;
                    }
                }

                if (from == to) {
                    continue;
                }
                string rna_name = ToStdString(m_ChoiceList[row].rnaType->GetStringSelection());
                NStr::ReplaceInPlace(rna_name, "rRNA", "ribosomal RNA");
                NStr::ReplaceInPlace(rna_name, "ITS", "internal transcribed spacer ");
                NStr::ReplaceInPlace(rna_name, "IGS", "intergenic spacer");
                
                if (NStr::FindNoCase(rna_name, "spacer") != NPOS) {
                    CRef<CSeq_feat> new_feat = AddMiscRNA(bsh, rna_name, from, to, false, false);
                    featlist.push_back(new_feat);
                } else {
                    CRef<CSeq_feat> new_feat = AddRibosomalRNA(bsh, rna_name, from, to, false, false);
                    featlist.push_back(new_feat);
                }
            }
        }
    } 
    const_featlist = x_UpdatePartials(featlist);
    return const_featlist;
}

END_NCBI_SCOPE
