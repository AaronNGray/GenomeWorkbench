/*  $Id: subprep_panel.cpp 43676 2019-08-14 14:28:05Z asztalos $
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
 * Authors:  Colleen Bollin, based on a file by Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <objects/seqset/Seq_entry.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/submit/Submit_block.hpp>
#include <objects/submit/Contact_info.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/biblio/Affil.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/biblio/Cit_gen.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/packages/pkg_sequence_edit/srceditdialog.hpp>
#include <gui/packages/pkg_sequence_edit/seqtechdlg.hpp>
#include <gui/packages/pkg_sequence_edit/submitblockdlg.hpp>
#include <gui/packages/pkg_sequence_edit/import_feat_table.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_molinfo_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/vectortrimdlg.hpp>
#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/reorder_sequences_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/seqid_fix_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_feature_add_dlg_std.hpp>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/radiobox.h>
#include <wx/msgdlg.h>
#include <wx/choice.h>
#include <wx/filedlg.h>

#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/utils/app_popup.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/table_data_base.hpp>
#include "subprep_event.hpp"
#include <gui/widgets/grid_widget/grid_event.hpp>

// includes for validation and cleanup
#include <objtools/validator/validator.hpp>
#include <objmgr/object_manager.hpp>
#include <objects/valerr/ValidError.hpp>
#include <objects/valerr/ValidErrItem.hpp>
#include <objtools/cleanup/cleanup.hpp>

#include "cmd_place_protein_feats.hpp"



////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE


/*!
 * CSubPrep_panel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubPrep_panel, wxScrolledWindow )


/*!
 * CSubPrep_panel event table definition
 */

BEGIN_EVENT_TABLE( CSubPrep_panel, wxScrolledWindow )

////@begin CSubPrep_panel event table entries
    EVT_NOTEBOOK_PAGE_CHANGED( ID_NOTEBOOK, CSubPrep_panel::OnNotebookPageChanged )

    EVT_BUTTON( ID_BUTTON17, CSubPrep_panel::OnButton17Click )

    EVT_BUTTON( ID_ADD_SEQUENCE_BTN, CSubPrep_panel::OnAddSequenceBtnClick )

    EVT_BUTTON( ID_REORDER_SEQ, CSubPrep_panel::OnReorderSeqClick )

    EVT_BUTTON( ID_BUTTON16, CSubPrep_panel::OnRemoveSequencesClick )

    EVT_BUTTON( ID_BUTTON13, CSubPrep_panel::OnClickVectorTrim )

    EVT_BUTTON( ID_PREV_BTN, CSubPrep_panel::OnPrevBtnClick )

    EVT_BUTTON( ID_NEXT_BTN, CSubPrep_panel::OnNextBtnClick )

    EVT_BUTTON( ID_FLATFILE, CSubPrep_panel::OnFlatfileClick )

    EVT_BUTTON( ID_LOOKUP_TAXONOMY, CSubPrep_panel::OnLookupTaxonomyClick )

    EVT_BUTTON( ID_VALIDATE, CSubPrep_panel::OnValidateClick )

    EVT_BUTTON( ID_SAVE_BTN, CSubPrep_panel::OnSaveBtnClick )

    EVT_BUTTON( ID_START_NEW, CSubPrep_panel::OnStartNewClick )

////@end CSubPrep_panel event table entries
   
    EVT_CHOICE ( ID_SET_CHOICE, CSubPrep_panel::OnChangeSet )

END_EVENT_TABLE()


/*!
 * CSubPrep_panel constructors
 */

 CSubPrep_panel::CSubPrep_panel() 
   : m_IsSingle(false), m_CmdProcessor()
{
    Init();
}

CSubPrep_panel::CSubPrep_panel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
   : m_IsSingle(false), m_CmdProcessor(), m_SubmissionOk(false), m_DataIsSaved(false), m_SubmissionErrors("")
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSubPrep_panel creator
 */

bool CSubPrep_panel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubPrep_panel creation
    wxScrolledWindow::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubPrep_panel creation

    wxClientDC dc(this);
    SetScrollRate(dc.GetCharWidth(),dc.GetCharHeight());

    return true;
}


/*!
 * CSubPrep_panel destructor
 */

CSubPrep_panel::~CSubPrep_panel()
{
////@begin CSubPrep_panel destruction
////@end CSubPrep_panel destruction
}


/*!
 * Member initialisation
 */
static CSubPrep_panel::TWizardKeyword s_keywords[] = {
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_uncultured_samples, "uncultured"),
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_rrna_its_igs, "cultured"),
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_tsa, "TSA"),
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_microsatellite, "Microsatellite"),
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_igs, "intergenic"),
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_d_loop, "D-loop"),
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_viruses, "virus"),
  CSubPrep_panel::TWizardKeyword(CSourceRequirements::eWizardType_standard, "standard")
};
static int num_keywords = sizeof (s_keywords) / sizeof (CSubPrep_panel::TWizardKeyword);


void CSubPrep_panel::Init()
{
////@begin CSubPrep_panel member initialisation
    m_Status = NULL;
    m_Book = NULL;
    m_SequenceCount = NULL;
    m_ReorderSequencesBtn = NULL;
    m_RemoveSequencesBtn = NULL;
    m_VectorTrimBtn = NULL;
    m_PrevBtn = NULL;
    m_NextBtn = NULL;
    m_StarExplanation = NULL;
    m_FlatfileButton = NULL;
    m_TaxonomyButton = NULL;
    m_ValidateButton = NULL;
    m_StartNewBtn = NULL;
////@end CSubPrep_panel member initialisation

    m_WizardNames = GetWizardNameList();

    m_Keywords.clear();
    for (int i = 0; i < num_keywords; i++) {
        m_Keywords.push_back(s_keywords[i]);
    }
    m_SaveFileDir = wxT("");
    m_SaveFileName = wxT("");
    m_MainPanel = NULL;
}


/*!
 * Control creation for CSubPrep_panel
 */

void CSubPrep_panel::CreateControls()
{    
////@begin CSubPrep_panel content construction
    CSubPrep_panel* itemScrolledWindow1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemScrolledWindow1->SetSizer(itemBoxSizer2);

    m_Status = new wxStaticText( itemScrolledWindow1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(550, -1), wxALIGN_RIGHT|wxST_NO_AUTORESIZE );
    m_Status->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
    itemBoxSizer2->Add(m_Status, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    m_Book = new wxNotebook( itemScrolledWindow1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    itemBoxSizer2->Add(m_Book, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_SequenceCount = new wxStaticText( itemScrolledWindow1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    itemBoxSizer2->Add(m_SequenceCount, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemScrolledWindow1, ID_BUTTON17, _("Open Existing Submission"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemScrolledWindow1, ID_ADD_SEQUENCE_BTN, _("Add Sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ReorderSequencesBtn = new wxButton( itemScrolledWindow1, ID_REORDER_SEQ, _("Reorder Sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ReorderSequencesBtn->Enable(false);
    itemBoxSizer7->Add(m_ReorderSequencesBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RemoveSequencesBtn = new wxButton( itemScrolledWindow1, ID_BUTTON16, _("Remove Sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RemoveSequencesBtn->Enable(false);
    itemBoxSizer7->Add(m_RemoveSequencesBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_VectorTrimBtn = new wxButton( itemScrolledWindow1, ID_BUTTON13, _("Vector Trim Tool"), wxDefaultPosition, wxDefaultSize, 0 );
    m_VectorTrimBtn->Enable(false);
    itemBoxSizer7->Add(m_VectorTrimBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_PrevBtn = new wxButton( itemScrolledWindow1, ID_PREV_BTN, _("Prev"), wxDefaultPosition, wxDefaultSize, 0 );
    m_PrevBtn->Enable(false);
    itemBoxSizer13->Add(m_PrevBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NextBtn = new wxButton( itemScrolledWindow1, ID_NEXT_BTN, _("Next"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NextBtn->Enable(false);
    itemBoxSizer13->Add(m_NextBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StarExplanation = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_StarExplanation, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemScrolledWindow1, wxID_STATIC, _("*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText17->SetForegroundColour(wxColour(255, 0, 0));
    m_StarExplanation->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemScrolledWindow1, wxID_STATIC, _("Indicates required information has not been provided"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StarExplanation->Add(itemStaticText18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer19, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_FlatfileButton = new wxButton( itemScrolledWindow1, ID_FLATFILE, _("Show Flatfile"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer19->Add(m_FlatfileButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TaxonomyButton = new wxButton( itemScrolledWindow1, ID_LOOKUP_TAXONOMY, _("Lookup Taxonomy"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer19->Add(m_TaxonomyButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ValidateButton = new wxButton( itemScrolledWindow1, ID_VALIDATE, _("Validate"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer19->Add(m_ValidateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton23 = new wxButton( itemScrolledWindow1, ID_SAVE_BTN, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer19->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer24, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_StartNewBtn = new wxButton( itemScrolledWindow1, ID_START_NEW, _("Start New Submission"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(m_StartNewBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSubPrep_panel content construction

    m_IntroPanel = new CSubPrepIntroPanel(m_Book, m_Workbench);
    m_Book->AddPage(m_IntroPanel, wxT("Submission Overview"));
    if (m_TopSeqEntry 
        && (m_TopSeqEntry.IsSeq() 
            || (m_TopSeqEntry.IsSet() 
              && !m_TopSeqEntry.GetSet().IsEmptySeq_set()))) {
        m_MainPanel = new CSubmissionPrepMainPanel(m_Book);
        m_Book->AddPage(m_MainPanel, wxT("Submission Details"));
    }
}


bool CSubPrep_panel::Layout()
{
    wxSize size = m_Book->GetSize();
    size.IncBy(1);
    m_Book->SetSize(size);
    return wxScrolledWindow::Layout();
}


static const string kAssemblyDescription = "Assembly Description";

void CSubPrep_panel::x_CreateWizardExtras()
{
    if (m_MainPanel) {
        m_MainPanel->ResetFieldsInUse();
    }

    m_DBLinkFields.clear();
    m_DBLinkUrls.clear();
    m_DBLinkRequired.clear();

    CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl ();
    CSourceRequirements::EWizardSrcType src_type = x_GetWizardSrcTypeFromCtrls ();

    if (m_TopSeqEntry 
        && (m_TopSeqEntry.IsSeq() 
            || (m_TopSeqEntry.IsSet() 
              && !m_TopSeqEntry.GetSet().IsEmptySeq_set()))) {
        x_CreateMoleculeOptions(wizard_type, src_type);
        x_CreateGenomeOptions (wizard_type, src_type);
        if (m_MainPanel) {
            m_MainPanel->CreateSetChoice(wizard_type, m_TopSeqEntry);
        }

        // TODO: wizard-specific options
        switch (wizard_type) {
            case CSourceRequirements::eWizardType_uncultured_samples:
                x_CreatePrimerOptions ();
                break;     
            case CSourceRequirements::eWizardType_tsa:
                x_CreateCommentOptions (kAssemblyDescription);
                x_CreateDBLinkOptions ();
                break;
            case CSourceRequirements::eWizardType_viruses:
                if (m_MainPanel) {
                    m_MainPanel->CreateVirusMoleculeOptions(m_TopSeqEntry, m_CmdProcessor);
                }
                break;
            default:
                break;
        }

        x_CreateChimeraOptions();

    }

    if (m_MainPanel) {
        m_MainPanel->ClearUnusedFields();
    }

    // update
    Layout();
    FitInside();
    Refresh();
}


CRef<CCmdComposite> CSubPrep_panel::x_ClearWizardSpecificData(CSourceRequirements::EWizardType wizard_type)
{
    CRef<CCmdComposite> cmd(NULL);
    if (!m_TopSeqEntry) {
        return cmd;
    } else if (m_TopSeqEntry.IsSet() 
               && m_TopSeqEntry.GetSet().IsEmptySeq_set()) {
        return cmd;
    }
    vector<CTableFieldCommandConverter*> converter_list;

    switch (wizard_type) {
        case CSourceRequirements::eWizardType_uncultured_samples:
            converter_list.push_back(new CPrimerTableCommandConverter());
            break;
        case CSourceRequirements::eWizardType_tsa:
            converter_list.push_back(new CCommentDescriptorTableCommandConverter(kAssemblyDescription));
            converter_list.push_back(new CDBLinkTableCommandConverter (m_DBLinkFields));
            break;
        default:
            break;
    }

    bool any_data = false;
    for (size_t i = 0; i < converter_list.size(); i++) {
        CRef<objects::CSeq_table> table = converter_list[i]->GetValuesTableFromSeqEntry(m_TopSeqEntry);
        ITERATE (objects::CSeq_table::TColumns, cit, table->GetColumns()) {
            if ((*cit)->IsSetData() && !(*cit)->GetData().IsId() && AreAnyColumnValuesPresent(*cit)) {
                any_data = true;
                break;
            }
        }
    }

    if (any_data) {
        cmd = new CCmdComposite("Clear Wizard-Specific Data");
        for (size_t i = 0; i < converter_list.size(); i++) {
            CRef<CCmdComposite> subcmd = converter_list[i]->ClearAllValues(m_TopSeqEntry);
            if (subcmd) {
                cmd->AddCommand(*subcmd);
            }
        }
    }

    for (size_t i = 0; i < converter_list.size(); i++) {
        delete converter_list[i];
    }
    return cmd;
}



#if 0
bool CSubPrep_panel::x_NeedTopology(CSourceRequirements::EWizardType wizard_type, CMolInfoTableCommandConverter* converter)
{
    bool needed = true;
    if (wizard_type == CSourceRequirements::eWizardType_microsatellite || wizard_type == CSourceR) {
        CRef<objects::CSeq_table> values = converter->GetValuesTableFromSeqEntry(m_TopSeqEntry);
        CRef<objects::CSeqTable_column> col = FindSeqTableColumnByName (values, kTopology);
        if (AreAllColumnValuesTheSame (col, "linear")) {
            string first_val = "linear";
            if (col->IsSetData() && col->GetData().GetString().size() > 0) {
                first_val = col->GetData().GetString().front();
                if (NStr::IsBlank(first_val)) {
                    first_val = "linear";
                }
            }
            if (NStr::Equal(first_val, "linear")) {
                needed = false;
            }
        }
    }
    return needed;
}
#endif

void CSubPrep_panel::x_CreateMoleculeOptions(CSourceRequirements::EWizardType wizard_type,
                                             CSourceRequirements::EWizardSrcType src_type)
{
    if (m_MainPanel) {
        CMolInfoTableCommandConverter* mol_converter = new CMolInfoTableCommandConverter (wizard_type);
        CMolInfoTableCommandConverter* top_converter = new CMolInfoTableCommandConverter (wizard_type);
        bool must_choose = !NStr::Equal("Yes", GetWizardFieldFromSeqEntry(m_TopSeqEntry, string(kMoleculeType) + " Confirmed"));
        if (wizard_type == CSourceRequirements::eWizardType_d_loop) {
            must_choose = false;
        }
        m_MainPanel->AddField (m_TopSeqEntry, m_CmdProcessor, m_Workbench, kMoleculeType, kMoleculeType, mol_converter, must_choose, true, "");
        m_MainPanel->AddField (m_TopSeqEntry, m_CmdProcessor, m_Workbench, kTopology, kTopology, top_converter, false, false, "linear");
    }

}


void CSubPrep_panel::x_CreatePrimerOptions ()
{
    CPrimerTableCommandConverter* converter = new CPrimerTableCommandConverter ();
    if (m_MainPanel) {
        m_MainPanel->AddField (m_TopSeqEntry, m_CmdProcessor, m_Workbench, "Primer Type", "Primer Type", converter,
                               false, true);
    }
}



void CSubPrep_panel::x_CreateCommentOptions (const string& comment_label)
{
    if (m_MainPanel) {
        CCommentDescriptorTableCommandConverter* converter = new CCommentDescriptorTableCommandConverter(comment_label);
        m_MainPanel->AddField(m_TopSeqEntry, m_CmdProcessor, m_Workbench, comment_label, comment_label,
                              converter, false, false, "");
    }    
}


const string kChimeraLabel = "Program and Version";
const string kChimeraField = "Chimera Screening";

void CSubPrep_panel::x_CreateChimeraOptions ()
{
    if (m_MainPanel) {
        m_MainPanel->ShowChimera( m_TopSeqEntry, m_CmdProcessor, m_Workbench, true);
    }
}


void CSubPrep_panel::x_CreateDBLinkOptions ()
{
    CRef<objects::CSeq_table> values_table = BuildDBLinkValuesTableFromSeqEntry(m_TopSeqEntry);

    m_DBLinkFields.push_back ("BioProject");
    m_DBLinkUrls.push_back("https://dsubmit.ncbi.nlm.nih.gov/subs/SUB002235/submitter");
    m_DBLinkRequired.push_back(true);
    m_DBLinkFields.push_back ("Sequence Read Archive");
    m_DBLinkUrls.push_back("https://trace.ncbi.nlm.nih.gov/Traces/sra/sra.cgi?view=announcement");
    m_DBLinkRequired.push_back(false);
    m_DBLinkFields.push_back ("BioSample");
    m_DBLinkUrls.push_back("https://dsubmit.ncbi.nlm.nih.gov/subs/SUB002236/submitter");
    m_DBLinkRequired.push_back(false);

    CDBLinkTableCommandConverter* converter = new CDBLinkTableCommandConverter (m_DBLinkFields);

    int pos = 0;
    for (vector<string>::iterator it = m_DBLinkFields.begin(); it != m_DBLinkFields.end(); it++, pos++) {        
        if (m_MainPanel) {
            m_MainPanel->AddField(m_TopSeqEntry, m_CmdProcessor, m_Workbench, *it, *it, converter, 
                                  false, m_DBLinkRequired[pos], "", m_DBLinkUrls[pos], false);
        }

    }
    Layout();
}


const string kGenome = "genome";

void CSubPrep_panel::x_CreateGenomeOptions(CSourceRequirements::EWizardType wizard_type,
                                             CSourceRequirements::EWizardSrcType src_type)
{
    if (m_MainPanel) {
        // add to main panel
        CGenomeTableCommandConverter* converter = new CGenomeTableCommandConverter (wizard_type, src_type);
        m_MainPanel->AddField (m_TopSeqEntry, m_CmdProcessor, m_Workbench, kLocation, kGenome, converter, false, false, "genomic");
    }

}


bool CSubPrep_panel::x_IsSequencingTechnologyRequired (objects::CSeq_entry_Handle entry, CSourceRequirements::EWizardType wizard_type)
{
    if (wizard_type == CSourceRequirements::eWizardType_tsa) {
        return true;
    }
    if (entry.IsSet()) {
        CConstRef<objects::CBioseq_set> set = entry.GetSet().GetCompleteBioseq_set();
        if (set && set->IsSetSeq_set() && set->GetSeq_set().size() >= 500) {
            return true;
        }
    }
    
    if (m_SeqSubmit && m_SeqSubmit->IsSetSub() 
        && m_SeqSubmit->GetSub().IsSetCit() 
        && m_SeqSubmit->GetSub().GetCit().IsSetDescr()) {
        string title = m_SeqSubmit->GetSub().GetCit().GetDescr();
        if (NStr::Find(title, "454") != string::npos
            || NStr::Find(title, "Complete Genomics") != string::npos
            || NStr::Find(title, "Helicos") != string::npos
            || NStr::Find(title, "Illumina") != string::npos
            || NStr::Find(title, "IonTorrent") != string::npos
            || NStr::Find(title, "PacBio") != string::npos
            || NStr::Find(title, "Pacific Biosciences") != string::npos
            || NStr::Find(title, "SOLiD") != string::npos
            || NStr::Find(title, "pyrosequencing") != string::npos
            || NStr::Find(title, "HiSeq") != string::npos
            || NStr::Find(title, "transcriptome") != string::npos 
            || NStr::Find(title, "solexa") != string::npos
            || NStr::Find(title, "deep sequencing") != string::npos
            || NStr::Find(title, "deep-sequencing") != string::npos
            || NStr::Find(title, "transcriptom") != string::npos
            || NStr::Find(title, "next-gen") != string::npos
            || NStr::Find(title, "RNA-Seq") != string::npos
            || NStr::Find(title, "RNASeq") != string::npos
            || NStr::Find(title, "RNA Seq") != string::npos
            || NStr::Find(title, "high-throughput") != string::npos
            || NStr::Find(title, "high throughput") != string::npos
            || NStr::Find(title, "metagenom") != string::npos
            || NStr::Find(title, "assembl") != string::npos
            || NStr::Find(title, "raw seq") != string::npos
            || NStr::Find(title, "next gen") != string::npos) {
            return true;
        }
    }
    return false;
}


bool CSubPrep_panel::IsSequencingTechnologyRequired (CSourceRequirements::EWizardType wizard_type)
{
    return x_IsSequencingTechnologyRequired (m_TopSeqEntry, wizard_type);
}


CRef<objects::CSeq_table> CSubPrep_panel::GetSeqTechTable ()
{
    // todo - if wizard is WGS, use other structured comment
    CStructuredCommentCommandConverter converter("##Assembly-Data-START##", GetAssemblyDataFields());
    CRef<objects::CSeq_table> table = converter.GetValuesTableFromSeqEntry(m_TopSeqEntry);
    return table;
}


CSourceRequirements::EWizardType CSubPrep_panel::GuessWizardTypefromSeqEntry(objects::CSeq_entry_Handle entry)
{
    CSourceRequirements::EWizardType wizard = CSourceRequirements::eWizardType_standard;
    bool found = false;

    objects::CBioseq_CI b_iter(entry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter && !found; ++b_iter ) {
        objects::CSeqdesc_CI it (*b_iter, objects::CSeqdesc::e_Source);
        while (it && !found) {
            if (it->GetSource().IsSetSubtype()) {
                ITERATE (objects::CBioSource::TSubtype, sit, it->GetSource().GetSubtype()) {
                    if ((*sit)->IsSetSubtype() 
                        && (*sit)->GetSubtype() == objects::CSubSource::eSubtype_other
                        && (*sit)->IsSetName()) {
                        string note = (*sit)->GetName();
                        size_t pos = NStr::FindNoCase (note, " wizard");
                        while (pos != string::npos && !found) {
                            for (int i = 0; i < num_keywords; i++) {
                                size_t k_len = s_keywords[i].second.length();
                                if (pos > k_len && NStr::EqualNocase(note.substr(pos - k_len, k_len), s_keywords[i].second)) {
                                    wizard = s_keywords[i].first;
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                note = note.substr(pos + 7);
                                pos = NStr::FindNoCase (note, " wizard");
                            }
                        }
                    }
                }
            }
            ++it;
        }
    }
    return wizard;
}

const string kWizardType = "WizardType";
const string kSrcType = "SrcType";

string CSubPrep_panel::GetWizardFieldFromSeqEntry (objects::CSeq_entry_Handle entry, string field_name)
{
    objects::CSeqdesc_CI it (entry, objects::CSeqdesc::e_User);

    while (it) {
        if (IsWizardObject(it->GetUser())) {
            string wizard_str = GetFieldFromWizardObject(it->GetUser(), field_name);
            return wizard_str;
        }
        ++it;
    }
    return "";
}


CRef<CCmdComposite> CSubPrep_panel::SetWizardFieldInSeqEntry(objects::CSeq_entry_Handle entry, string field_name, string value)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Wizard Object Edit") );
    bool found = false;

    for (objects::CSeqdesc_CI desc_ci( entry, objects::CSeqdesc::e_User);
         desc_ci;
         ++desc_ci) {
        const objects::CUser_object& u = desc_ci->GetUser();
        if (IsWizardObject (u)) {
            CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
            CRef<objects::CUser_object> new_user (new objects::CUser_object());
            new_user->Assign (u);
            new_desc->SetUser(*new_user);
            objects::CUser_field& new_field = new_user->SetField(field_name);
            new_field.SetData().SetStr(value);
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_desc));
            cmd->AddCommand (*ecmd);
            found = true;
        }            
    }
    if (!found) {
        CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
        CRef<objects::CUser_object> new_user = MakeWizardObject();
        new_desc->SetUser(*new_user);
        objects::CUser_field& new_field = new_user->SetField(field_name);
        new_field.SetData().SetStr(value);
        cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(entry, *new_desc)) );
    }   
    return cmd;
}



CSourceRequirements::EWizardType CSubPrep_panel::GetWizardTypefromSeqEntry(objects::CSeq_entry_Handle entry)
{
    objects::CSeqdesc_CI it (entry, objects::CSeqdesc::e_User);

    while (it) {
        if (IsWizardObject(it->GetUser())) {
            string wizard_str = GetFieldFromWizardObject(it->GetUser(), kWizardType);
            return GetWizardTypeFromName(wizard_str);
        }
        ++it;
    }
    return GuessWizardTypefromSeqEntry(entry);
}


string CSubPrep_panel::GetWizardTypeField()
{
    string value = "";
    objects::CSeqdesc_CI it (m_TopSeqEntry, objects::CSeqdesc::e_User);

    while (it) {
        if (IsWizardObject(it->GetUser())) {
            value = GetFieldFromWizardObject(it->GetUser(), kWizardType);
            break;
        }
        ++it;
    }
    return value;
}


string CSubPrep_panel::GetWizardSrcTypeField()
{
    string value = "";
    objects::CSeqdesc_CI it (m_TopSeqEntry, objects::CSeqdesc::e_User);

    while (it) {
        if (IsWizardObject(it->GetUser())) {
            value = GetFieldFromWizardObject(it->GetUser(), kSrcType);
            break;
        }
        ++it;
    }
    return value;
}


CSourceRequirements::EWizardSrcType CSubPrep_panel::x_GetWizardSrcTypefromSeqEntry(objects::CSeq_entry_Handle entry)
{
    if (!m_MainPanel) {
        return CSourceRequirements::eWizardSrcType_any;
    }
    string value = "";
    objects::CSeqdesc_CI it (entry, objects::CSeqdesc::e_User);

    while (it) {
        if (IsWizardObject(it->GetUser())) {
            value = GetFieldFromWizardObject(it->GetUser(), kSrcType);
            break;
        }
        ++it;
    }
    return m_MainPanel->GetWizardSrcTypeFromName(value);
}


CSourceRequirements::EWizardType CSubPrep_panel::x_GetWizardTypeFromCtrl ()
{
    if (m_IntroPanel) {
        return m_IntroPanel->GetWizardTypeFromCtrl();
    } else {
        return CSourceRequirements::eWizardType_standard;
    }
}


string CSubPrep_panel::GetWizardTypeName (unsigned int wizard_type)
{
    switch (wizard_type) {
        case CSourceRequirements::eWizardType_standard:
            return "Standard";
            break;
        case CSourceRequirements::eWizardType_viruses:
            return "Viruses";
            break;
        case CSourceRequirements::eWizardType_uncultured_samples:
            return "Uncultured Samples";
            break;
        case CSourceRequirements::eWizardType_rrna_its_igs:
            return "rRNA-ITS-IGS sequences";
            break;
        case CSourceRequirements::eWizardType_tsa:
            return "TSA";
            break;
        case CSourceRequirements::eWizardType_igs:
            return "Intergenic Spacer (IGS) sequences";
            break;
        case CSourceRequirements::eWizardType_microsatellite:
            return "Microsatellite sequences";
            break;
        case CSourceRequirements::eWizardType_d_loop:
            return "D-loops and control regions";
            break;
        default:
            return "Unknown";
            break;
    }
}


void CSubPrep_panel::x_SetSourceTypeOptionsForWizardType (CSourceRequirements::EWizardType wizard_type)
{
    if (m_MainPanel) {
        m_MainPanel->SetSourceTypeOptions(wizard_type);
    }
}


void CSubPrep_panel::x_SetSourceType (CSourceRequirements::EWizardSrcType src_type)
{
    if (m_MainPanel) {
        m_MainPanel->SetSourceType(src_type);
    }
}


CSourceRequirements::EWizardSrcType CSubPrep_panel::x_GetWizardSrcTypeFromCtrls ()
{
    if (m_MainPanel) {
        CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl();
        return m_MainPanel->GetWizardSrcTypeFromCtrls(wizard_type);
    } else {
        return CSourceRequirements::eWizardSrcType_any;
    }
}


CRef<objects::CSeq_table> CSubPrep_panel::x_BuildValuesTable()
{
    return GetSeqTableFromSeqEntry (m_TopSeqEntry);
}


string CSubPrep_panel::x_CheckSourceQuals (CSourceRequirements::EWizardType wizard_type)
{
    string qual_report = "";
    CRef<objects::CSeq_table> values_table = x_BuildValuesTable();
    CSourceRequirements *requirements = GetSrcRequirements (wizard_type, x_GetWizardSrcTypeFromCtrls(), values_table);
    CRef<objects::CSeqTable_column> problems = requirements->CheckSourceQuals(values_table);
    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName (values_table, kSequenceIdColLabel);
    delete requirements;
    if (m_MainPanel) {
        m_MainPanel->ShowSourceLabelStar(false);
    }

    for (int row = 0; row < values_table->GetNum_rows(); row++) {
        if (!NStr::IsBlank(problems->GetData().GetString()[row])) {
            string id_label;
            if (id_col) {
                id_col->GetData().GetId()[row]->GetLabel(&id_label, objects::CSeq_id::eContent);
            } else {
                id_label = NStr::NumericToString(row + 1);
            }
            if (!NStr::IsBlank(qual_report)) {
                qual_report += "; ";
            }
            qual_report += id_label + ":" + problems->GetData().GetString()[row];
            m_SubmissionOk = false;
            if (m_MainPanel) {
                m_MainPanel->ShowSourceLabelStar(true);
            }
        }
    }
    
    if (NStr::IsBlank(qual_report)) {
        qual_report = "No Source Qualifier Problems Found";
    }
    return qual_report;
}


void CSubPrep_panel::x_RefreshSubmitterInfoStatus()
{
    if (m_IntroPanel) {
        m_IntroPanel->RefreshSubmitterInfoStatus();
    }
}


void CSubPrep_panel::x_UpdateFeatureStatus(CSourceRequirements::EWizardType wizard_type)
{
    bool has_problem = false;
    bool features_ok = false;
    string status = CheckFeatureAnnotation(m_TopSeqEntry, features_ok);
    if (!features_ok && wizard_type != CSourceRequirements::eWizardType_tsa) {
        has_problem = true;
    }
    if (wizard_type == CSourceRequirements::eWizardType_microsatellite) {
        CFeat_CI fi(m_TopSeqEntry, CSeqFeatData::eSubtype_repeat_region);
        if (fi) {
            CFeatureTableCommandConverter* converter = x_GetMicrosatelliteTableConverter();
            if (converter) {
                CRef<CSeq_table> values = converter->GetValuesTableFromSeqEntry(m_TopSeqEntry);
                CRef<CSeqTable_column> problems = converter->MakeProblemsColumn(values);
                if (problems) {
                    vector<string> problem_strings;
                    ITERATE(CSeqTable_column::TData::TString, it, problems->GetData().GetString()) {
                        if (!NStr::IsBlank(*it)) {
                            problem_strings.push_back(*it);
                        }
                    }
                    problem_strings = SortUniqueStrings(problem_strings);
                    ITERATE(vector<string>, it, problem_strings) {
                        status += *it;
                        status += "\n";
                        has_problem = true;
                    }
                }
                delete converter;
            }
        }
    }

    m_MainPanel->SetFeaturesLabel(status);
    m_MainPanel->ShowFeaturesStar(has_problem);
    if (has_problem) {
        SetSubmissionNotReady(status);
    }
}


void CSubPrep_panel::UpdateForSeqEntryChange()
{
    CSourceRequirements::EWizardType wizard_type = CSourceRequirements::eWizardType_standard;
    CSourceRequirements::EWizardSrcType src_type = CSourceRequirements::eWizardSrcType_any;
    m_SubmissionOk = false;

    if (m_TopSeqEntry) {
        m_SubmissionOk = true;
        m_SubmissionErrors  = "";
        wizard_type = GetWizardTypefromSeqEntry(m_TopSeqEntry);
        src_type = x_GetWizardSrcTypefromSeqEntry(m_TopSeqEntry);
        if (m_IntroPanel) {
            m_IntroPanel->SetWizardTypeCtrl();
        }
        if (!m_MainPanel && m_TopSeqEntry 
            && (m_TopSeqEntry.IsSeq() 
                || (m_TopSeqEntry.IsSet() 
                    && !m_TopSeqEntry.GetSet().IsEmptySeq_set()))) {
            m_MainPanel = new CSubmissionPrepMainPanel(m_Book);
            m_Book->AddPage(m_MainPanel, wxT("Submission Details"));
        }

        x_SetSourceTypeOptionsForWizardType (wizard_type);
        x_SetSourceType (src_type);
        if (m_MainPanel) {
            m_MainPanel->UpdateSeqTechCtrls(wizard_type);
        }

        objects::CBioseq_set::EClass set_class = objects::CBioseq_set::eClass_not_set;
        if (m_TopSeqEntry.IsSet()) {
            if (m_TopSeqEntry.GetSet().IsSetClass()) {
                set_class = m_TopSeqEntry.GetSet().GetClass();
            }
        } else {
            m_IsSingle = true;
        }
        if (m_MainPanel) {
            x_UpdateFeatureStatus(wizard_type);
        }
    } else {
        if (m_IntroPanel) {
            m_IntroPanel->SetWizardTypeCtrl();
        }
    }

    if (m_MainPanel) {
        m_MainPanel->SetSourceLabel(x_CheckSourceQuals(wizard_type));
    }
    x_RefreshSubmitterInfoStatus();
    x_CreateWizardExtras();
    x_SetSubmissionStatus();
    x_EnableSeqButtons();
    x_SetNextPrevBtns();
}


/*!
 * Transfer data to the window
 */

bool CSubPrep_panel::TransferDataToWindow()
{
    m_TopSeqEntry.Reset();
    m_SeqSubmit.Reset();
    m_SubmissionOk = false;
    if (m_InputObjects) {
        NON_CONST_ITERATE (TConstScopedObjects, it, *m_InputObjects) {
            const CObject* ptr = it->object.GetPointer();

            /// CSeq_entry
            const objects::CSeq_entry* seqEntry = dynamic_cast<const objects::CSeq_entry*>(ptr);
            objects::CSeq_entry_Handle seh;
            if (seqEntry) {
                seh = it->scope->GetSeq_entryHandle (*seqEntry);
            } else {
                const objects::CSeq_submit* seqsubmit = dynamic_cast<const objects::CSeq_submit*>(ptr);
                if (seqsubmit) {
                    m_SeqSubmit = const_cast<objects::CSeq_submit*>(seqsubmit);
                    if (seqsubmit->IsEntrys()) {
                        seh = it->scope->GetSeq_entryHandle(*(seqsubmit->GetData().GetEntrys().front()));
                    }
                } else {
                    const objects::CSeq_id* seqid = dynamic_cast<const objects::CSeq_id*>(ptr);
                    if (seqid) {
                        objects::CBioseq_Handle bsh = it->scope->GetBioseqHandle(*seqid);if (bsh) {
                            seh = bsh.GetSeq_entry_Handle();
                        }
                    }
                }
            } 
            if (seh) {
                m_TopSeqEntry = seh;
                UpdateForSeqEntryChange();
            }
        }
    }
    // only show the New Submission button if this was launched from the edit perspective and not a specific object
    if (!m_TopSeqEntry.IsSet() || !m_TopSeqEntry.GetSet().IsEmptySeq_set()) {
        m_StartNewBtn->Show(false);
    }
    return wxPanel::TransferDataToWindow();
}


/*!
 * Transfer data from the window
 */

static const char *kTableTag = "Table";

void CSubPrep_panel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);
        CRegistryWriteView table_view = gui_reg.GetWriteView(reg_path);
    }
}


void CSubPrep_panel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path;
        CRegistryReadView table_view;

        reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);
        table_view = gui_reg.GetReadView(reg_path);
    }
}


/*!
 * Should we show tooltips?
 */

bool CSubPrep_panel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSubPrep_panel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubPrep_panel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubPrep_panel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSubPrep_panel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubPrep_panel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubPrep_panel icon retrieval
}


void CSubPrep_panel::x_SetNextPrevBtns()
{
    size_t num_pages = m_Book->GetPageCount();
    if (num_pages < 2) {
        m_NextBtn->Enable(false);
        m_PrevBtn->Enable(false);
    } else {
        int selection = m_Book->GetSelection();
        if (selection < num_pages - 1 && m_IntroPanel != NULL && m_IntroPanel->IsComplete()) {
            m_NextBtn->Enable(true);
        } else {
            m_NextBtn->Enable(false);
        }
        if (selection > 0) {
            m_PrevBtn->Enable(true);
        } else {
            m_PrevBtn->Enable(false);
        }
    }                    
}


void CSubPrep_panel::x_EnableSeqButtons()
{
    bool any_seq = false;
    bool multi_seq = false;
    if (m_TopSeqEntry) {
      CBioseq_CI bi(m_TopSeqEntry, CSeq_inst::eMol_na);
        if (bi) {
            any_seq = true;
            ++bi;
            if (bi) {
                multi_seq = true;
            }
        }

    }
    m_ReorderSequencesBtn->Enable(multi_seq);
    m_RemoveSequencesBtn->Enable(multi_seq);
    m_VectorTrimBtn->Enable(any_seq);
    m_ValidateButton->Enable(any_seq);
    m_FlatfileButton->Enable(any_seq);
    m_TaxonomyButton->Enable(any_seq);
}


void CSubPrep_panel::x_UpdateSequenceCountLabel()
{
    size_t count = 0;
    if (m_TopSeqEntry) {
        CBioseq_CI bi(m_TopSeqEntry, CSeq_inst::eMol_na);
        while (bi) {
            count++;
            ++bi;
        }
    }
    if (count == 0) {
        m_SequenceCount->SetLabel(wxEmptyString);
    } else {
        string message = "Submission contains " + NStr::NumericToString(count) + " sequences.";
        m_SequenceCount->SetLabel(ToWxString(message));
    }
}


void CSubPrep_panel::OnDataChange()
{
    m_SubmissionOk = true;
    m_SubmissionErrors = "";
    CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl ();

    if (!m_MainPanel) {
        if (m_TopSeqEntry 
            && (m_TopSeqEntry.IsSeq() 
                || (m_TopSeqEntry.IsSet() 
                  && !m_TopSeqEntry.GetSet().IsEmptySeq_set()))) {
            m_MainPanel = new CSubmissionPrepMainPanel(m_Book);
            m_Book->AddPage(m_MainPanel, wxT("Submission Details"));
            x_SetSourceTypeOptionsForWizardType(wizard_type);
        }
    }

    if (m_MainPanel) {
        m_MainPanel->UpdateSeqTechCtrls(wizard_type);
        m_MainPanel->SetSourceLabel(x_CheckSourceQuals(wizard_type));
        m_MainPanel->SetSourceType(x_GetWizardSrcTypefromSeqEntry(m_TopSeqEntry));
        x_UpdateFeatureStatus(wizard_type);
    } else {
        m_SubmissionOk = false;
    }

    x_EnableSeqButtons();
    x_RefreshSubmitterInfoStatus();
    x_CreateWizardExtras();
    x_SetSubmissionStatus();
    x_SetNextPrevBtns();
    x_UpdateSequenceCountLabel();
    m_DataIsSaved = false;
}


void CSubPrep_panel::SetSubmissionNotReady(string error) 
{ 
    m_SubmissionOk = false; 
    if (!NStr::IsBlank(error)) {
        NStr::ReplaceInPlace(error, "\n", " ");
        NStr::TruncateSpacesInPlace(error);
        if (!NStr::IsBlank(m_SubmissionErrors)) {
            string last_char = m_SubmissionErrors.substr(m_SubmissionErrors.length() - 1);
            if (!ispunct(last_char.c_str()[0])) {
                m_SubmissionErrors += ";";
            } 
            m_SubmissionErrors += " ";  
        }
        m_SubmissionErrors += error;
    }
}


void CSubPrep_panel::x_SetSubmissionStatus()
{
    if (m_SubmissionOk) {
        m_Status->SetLabel("Submission is ready!");
        m_Status->SetForegroundColour(*wxBLACK);
    } else {
        m_Status->SetLabel(wxT("Submission is incomplete!"));
        m_Status->SetForegroundColour(*wxRED);
    }
}


objects::CSeqdesc_CI CSubPrep_panel::GetCitSubPub()
{
    objects::CSeqdesc_CI it(m_TopSeqEntry, objects::CSeqdesc::e_Pub);
    while (it) {
        if (it->GetPub().IsSetPub()
            && it->GetPub().GetPub().Get().size() > 0
            && it->GetPub().GetPub().Get().front()->IsGen()) {
            const objects::CCit_gen& orig_gen = it->GetPub().GetPub().Get().front()->GetGen();
            if (orig_gen.IsSetCit() && NStr::Equal(orig_gen.GetCit(), "Unpublished")) {
                break;
            }
        }
        ++it;
    }
    return it;
}


CRef<objects::CSubmit_block> CSubPrep_panel::GetSubmitBlock()
{
    CRef<objects::CSubmit_block> orig_block(new objects::CSubmit_block());
    if (m_SeqSubmit) {
        orig_block->Assign(m_SeqSubmit->GetSub());
    }
    return orig_block;
}


void CSubPrep_panel::x_ConvertToSeqSubmit()
{
    CRef<objects::CSeq_submit> seq_submit (new objects::CSeq_submit());
    m_SeqSubmit = seq_submit;
    /// CSeq_entry
    CRef<objects::CSeq_entry> entry(const_cast<objects::CSeq_entry*>(m_TopSeqEntry.GetCompleteSeq_entry().GetPointer()));

    seq_submit->SetData().SetEntrys().push_back(entry);
    CSubPrepEvent evt(CSubPrepEvent::eWrapSeq_submit, *seq_submit);
    Send(&evt, ePool_Parent);
}


void CSubPrep_panel::UpdateSubmitBlock(CRef<objects::CSubmit_block> new_block, CRef<objects::CCit_gen> new_gen, string alt_email)
{
    CRef<objects::CSubmit_block> orig_block(new objects::CSubmit_block());
    if (m_SeqSubmit) {
        orig_block->Assign(m_SeqSubmit->GetSub());
    }
    objects::CSeqdesc_CI orig_citsub = GetCitSubPub();
    CRef<objects::CCit_gen> gen(new objects::CCit_gen());
    if (orig_citsub) {
        gen->Assign(orig_citsub->GetPub().GetPub().Get().front()->GetGen());
    }

    if (new_block) {
        // if this was not previously a Seq-submit, convert it
        if (!m_SeqSubmit) {
            x_ConvertToSeqSubmit();
        }
        m_SeqSubmit->SetSub(*new_block);
    }
    if (new_gen) {
        CRef<objects::CSeqdesc> new_desc(new objects::CSeqdesc());
        CRef<objects::CPub> new_pub(new objects::CPub());
        new_pub->SetGen(*new_gen);
        new_desc->SetPub().SetPub().Set().push_back(new_pub);
        CRef<CCmdComposite> cmd(new CCmdComposite("Edit Submitter Info"));

        if (orig_citsub) {
            // edit existing descriptor
            CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(orig_citsub.GetSeq_entry_Handle(), *orig_citsub, *new_desc));
            cmd->AddCommand(*ecmd);
        } else {
            // create new descriptor
            CRef<CCmdCreateDesc> ecmd(new CCmdCreateDesc(m_TopSeqEntry, *new_desc));
            cmd->AddCommand(*ecmd);
        }

        // generate create-date descriptor
        if (new_gen->IsSetDate()) {
            CRef<objects::CSeqdesc> new_create_desc(new objects::CSeqdesc());
            new_create_desc->SetCreate_date().Assign(new_gen->GetDate());
            objects::CSeqdesc_CI it(m_TopSeqEntry, objects::CSeqdesc::e_Create_date);
            if (it) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(it.GetSeq_entry_Handle(), *it, *new_create_desc));
                cmd->AddCommand(*ecmd);
            } else {
                // create new descriptor
                CRef<CCmdCreateDesc> ecmd(new CCmdCreateDesc(m_TopSeqEntry, *new_create_desc));
                cmd->AddCommand(*ecmd);
            }
        }

        m_CmdProcessor->Execute(cmd);
    }
    SetAltEmailAddress(alt_email);
    SetWizardFieldInSeqEntryNoUndo(m_TopSeqEntry, kReleaseDateConfirmed, "Yes");
}


void CSubPrep_panel::LaunchSeqTechEditor()
{
    CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl();
    CSeqTechDlg *dlg = new  CSeqTechDlg(NULL, m_TopSeqEntry, wizard_type);
    HandleBulkCmdDlg(dlg);
}


void CSubPrep_panel::ClearSeqTechValues()
{
    // todo - if wizard is WGS, use other structured comment
    CStructuredCommentCommandConverter converter("##Assembly-Data-START##", GetAssemblyDataFields());
    CRef<objects::CSeq_table> table = converter.GetValuesTableFromSeqEntry(m_TopSeqEntry);
    CRef<CCmdComposite> cmd = converter.ClearAllValues(m_TopSeqEntry);
    m_CmdProcessor->Execute(cmd);
}


bool CSubPrep_panel::OkToBulkEditSeqTech()
{
    CStructuredCommentCommandConverter converter("##Assembly-Data-START##", GetAssemblyDataFields());

    bool ok_to_bulk_edit = true;
    CRef<objects::CSeq_table> table = converter.GetValuesTableFromSeqEntry(m_TopSeqEntry);

    if (table) {
        for (size_t i = 1; i < table->GetColumns().size() && ok_to_bulk_edit; i++) {
            string title = table->GetColumns()[i]->GetHeader().GetTitle();
            if (!AreAllColumnValuesTheSame(table->SetColumns()[i], "")) {
                ok_to_bulk_edit = false;
            }
        }
    }
    return ok_to_bulk_edit;
}


void CSubPrep_panel::LaunchSourceEditor()
{
    CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl();
    SrcEditDialog dlg(NULL, m_TopSeqEntry, m_Workbench, m_SeqSubmit); // wizard_type, x_GetWizardSrcTypeFromCtrls());
    if (dlg.ShowModal() == wxID_OK) {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        m_CmdProcessor->Execute(cmd);
        dlg.Close();        
    }
}


bool CSubPrep_panel::x_NeedUniqueMicrosatelliteNames()
{
    vector<string> qual_names;
    qual_names.push_back("taxname");
    qual_names.push_back("clone");

    CRef<objects::CSeq_table> src_table = GetSeqTableForSrcQualFromSeqEntry (m_TopSeqEntry, qual_names);
    if (!src_table || src_table->GetNum_rows() == 0) {
        return true;
    }
    CRef<objects::CSeqTable_column> tax_col = FindSeqTableColumnByName (src_table, "taxname");
    CRef<objects::CSeqTable_column> clone_col = FindSeqTableColumnByName (src_table, "clone");
    vector<string> vals;
    for (int i = 0; i < src_table->GetNum_rows(); i++) {
        string row_val = "";
        if (tax_col && tax_col->GetData().GetSize() > i) {
            row_val += tax_col->GetData().GetString()[i];
        }
        if (clone_col && clone_col->GetData().GetSize() > i) {
            row_val += clone_col->GetData().GetString()[i];
        }
        if (NStr::IsBlank(row_val)) {
            // blank row
            return true;
        } else {
            vals.push_back(row_val);
        }
    }
    
    vector<string> sorted_vals = FindNonUniqueStrings(vals);

    if (sorted_vals.size() > 0) {
        return true;
    } else {
        return false;
    }
}


vector<size_t> CSubPrep_panel::x_GetSequenceLengths(CRef<objects::CSeq_table> table)
{
    vector<size_t> lens;

    CRef<objects::CSeqTable_column> col = FindSeqTableColumnByName (table, kSequenceIdColLabel);
    ITERATE(objects::CSeqTable_column::TData::TId, it, col->GetData().GetId()) {
        objects::CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(**it);
        lens.push_back(bsh.GetBioseqLength());
    }
    return lens;
}


CFeatureTableCommandConverter* CSubPrep_panel::x_GetMicrosatelliteTableConverter()
{
    CRef<objects::CSeq_feat> feat(new objects::CSeq_feat());
    feat->SetData().SetImp().SetKey("repeat_region");
    feat->SetQual().push_back(CRef<objects::CGb_qual>(new objects::CGb_qual("satellite", "microsatellite")));
    feat->SetQual().push_back(CRef<objects::CGb_qual>(new objects::CGb_qual("rpt_type", "tandem")));
    TFeatureSeqTableColumnList reqs;
    reqs.push_back(CRef<CFeatureSeqTableColumnBase>( new CSatelliteTypeNameColumn("microsatellite", x_NeedUniqueMicrosatelliteNames())));
    TFeatureSeqTableColumnList opts;
    opts.push_back(CRef<CFeatureSeqTableColumnBase>( new CRptUnitSeqColumn()));
    CFeatureTableCommandConverter* tmp_converter = new CFeatureTableCommandConverter(*feat, reqs, opts);
    CRef<objects::CSeq_table> table = tmp_converter->GetValuesTableFromSeqEntry(m_TopSeqEntry);
    vector<size_t> lens = x_GetSequenceLengths(table);
    delete tmp_converter;
    opts.push_back(CRef<CFeatureSeqTableColumnBase>( new CRptUnitRangeColumn(lens)));
    CFeatureTableCommandConverter* converter = new CFeatureTableCommandConverter(*feat, reqs, opts);
    return converter;
}


void CSubPrep_panel::LaunchFeatureAdder()
{
    CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl();
    if (wizard_type == CSourceRequirements::eWizardType_microsatellite) {
        CFeatureTableCommandConverter* converter = x_GetMicrosatelliteTableConverter();
        CTblEditDlg *dlg = new CTblEditDlg(this, m_TopSeqEntry, converter, NULL, SYMBOL_CTBLEDITDLG_IDNAME, wxT("Microsatellites"));
        HandleBulkCmdDlg(dlg);
    } else {
        CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, m_TopSeqEntry, m_Workbench);
        HandleBulkCmdDlg(dlg);
    }
}


void CSubPrep_panel::x_LaunchVectorTrimEditor()
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CVectorTrimDlg* dlg = new CVectorTrimDlg(main_window, m_TopSeqEntry, m_CmdProcessor);
    dlg->Show(true);       
}


CSourceRequirements::EWizardType CSubPrep_panel::GetWizardType()
{
    return GetWizardTypefromSeqEntry (m_TopSeqEntry);
}


CSourceRequirements::EWizardType CSubPrep_panel::SetWizardType(CSourceRequirements::EWizardType wizard_type)
{
    CSourceRequirements::EWizardType prev_wizard_type = GetWizardTypefromSeqEntry (m_TopSeqEntry);

    CRef<CCmdComposite> cmd = x_ClearWizardSpecificData(prev_wizard_type);
    if (cmd) {
        string message = "Do you want to clear " + GetWizardTypeName(prev_wizard_type) + "-specific data?";
        wxMessageDialog dlg(this, ToWxString (message), wxT("Clear Data"), wxYES_NO|wxCANCEL|wxYES_DEFAULT);
        int answer = dlg.ShowModal();
        if (answer == wxID_CANCEL) {
            return prev_wizard_type;
        } else if (answer == wxID_NO) {
            cmd.Reset(NULL);
        }
    }

    if (m_TopSeqEntry) {
        string val = GetWizardFieldFromSeqEntry(m_TopSeqEntry, string(kMoleculeType) + " Confirmed");
        if (!NStr::EqualNocase(val, "Yes")) {
            if (wizard_type == CSourceRequirements::eWizardType_d_loop) {
                CMolInfoTableCommandConverter *converter = new CMolInfoTableCommandConverter(wizard_type);
                CRef<objects::CSeq_table> table = converter->GetValuesTableFromSeqEntry(m_TopSeqEntry);
                CRef<objects::CSeqTable_column> col = FindSeqTableColumnByName (table, kMoleculeType);
                bool all_same = AreAllColumnValuesTheSame(col, "");
                bool any_missing = AreAnyColumnValuesMissing (col);
                if (all_same && any_missing) {
                    SetColumnValue(col, kDefaultMoleculeType);
                    if (cmd) {
                        cmd->AddCommand(*(converter->GetCommandFromValuesTable(table, m_TopSeqEntry, false)));
                    } else {
                        cmd = converter->GetCommandFromValuesTable(table, m_TopSeqEntry, false);
                    }
                }
                delete converter;
            } else if (prev_wizard_type == CSourceRequirements::eWizardType_d_loop) {
                CMolInfoTableCommandConverter *converter = new CMolInfoTableCommandConverter(wizard_type);
                CRef<objects::CSeq_table> table = converter->GetValuesTableFromSeqEntry(m_TopSeqEntry);
                CRef<objects::CSeqTable_column> col = FindSeqTableColumnByName (table, kMoleculeType);
                bool all_same = AreAllColumnValuesTheSame(col, "");
                bool any_missing = AreAnyColumnValuesMissing (col);
                if (all_same && !any_missing && NStr::EqualNocase(col->GetData().GetString()[0], kDefaultMoleculeType)) {
                    SetColumnValue(col, "");
                    if (cmd) {
                        cmd->AddCommand(*(converter->GetCommandFromValuesTable(table, m_TopSeqEntry, false)));
                    } else {
                        cmd = converter->GetCommandFromValuesTable(table, m_TopSeqEntry, false);
                    }
                }
                delete converter;
            }
        }
    }

    x_SetSourceTypeOptionsForWizardType (wizard_type);

    if (m_MainPanel) {
        m_MainPanel->SetSourceLabel(x_CheckSourceQuals(wizard_type));

        switch (wizard_type) {
            case CSourceRequirements::eWizardType_standard:
            case CSourceRequirements::eWizardType_d_loop:
            case CSourceRequirements::eWizardType_viruses:
            case CSourceRequirements::eWizardType_uncultured_samples:
            case CSourceRequirements::eWizardType_rrna_its_igs:
            case CSourceRequirements::eWizardType_igs:
            case CSourceRequirements::eWizardType_microsatellite:
                m_MainPanel->ShowAddFeatureBtn(true);
                break;
            default:
                m_MainPanel->ShowAddFeatureBtn(false);
                break;
        }
    }

    if (cmd) {
        cmd->AddCommand(*SetWizardFieldInSeqEntry(m_TopSeqEntry, kWizardType, GetWizardTypeName(wizard_type)));
    } else {
        cmd = SetWizardFieldInSeqEntry(m_TopSeqEntry, kWizardType, GetWizardTypeName(wizard_type));
    }
    // if only one set type allowed, set it here
    if (wizard_type == CSourceRequirements::eWizardType_microsatellite
        || wizard_type == CSourceRequirements::eWizardType_tsa) {
        if (m_TopSeqEntry.IsSet() 
            && (!m_TopSeqEntry.GetSet().IsSetClass() 
            || m_TopSeqEntry.GetSet().GetClass() == objects::CBioseq_set::eClass_not_set)) {
            objects::CBioseq_set_Handle beh = m_TopSeqEntry.GetSet();
            // create command
            CRef<objects::CBioseq_set> new_set(new objects::CBioseq_set());
            new_set->Assign(*(beh.GetCompleteBioseq_set()));
            new_set->SetClass(objects::CBioseq_set::eClass_genbank);
            CCmdChangeBioseqSet *set_cmd = new CCmdChangeBioseqSet(beh, *new_set);
            cmd->AddCommand(*set_cmd);
        }
    }
    m_CmdProcessor->Execute(cmd);
    return wizard_type;
}


void CSubPrep_panel::SetSourceType(string src_type)
{
    CRef<CCmdComposite> cmd = SetWizardFieldInSeqEntry (m_TopSeqEntry, kSrcType, src_type);
    m_CmdProcessor->Execute(cmd);
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_SET_CHOICE
 */

void CSubPrep_panel::OnChangeSet( wxCommandEvent& event )
{
    if (m_TopSeqEntry && m_TopSeqEntry.IsSet()) {
        objects::CBioseq_set::TClass old_class = objects::CBioseq_set::eClass_not_set;
        if (m_TopSeqEntry.GetSet().IsSetClass()) {
            old_class = m_TopSeqEntry.GetSet().GetClass();
        }
        objects::CBioseq_set::TClass new_class = objects::CBioseq_set::eClass_not_set;
        string new_val = ToStdString(event.GetString());
        if (NStr::Find (new_val, "Population") != string::npos) {
            new_class = objects::CBioseq_set::eClass_pop_set;
        } else if (NStr::Find (new_val, "Phylogenetic") != string::npos) {
            new_class = objects::CBioseq_set::eClass_phy_set;
        } else if (NStr::Find (new_val, "Mutation") != string::npos) {
            new_class = objects::CBioseq_set::eClass_mut_set;
        } else if (NStr::Find (new_val, "Environmental set") != string::npos) {
            new_class = objects::CBioseq_set::eClass_eco_set;
        } else if (NStr::Find (new_val, "Batch") != string::npos) {
            new_class = objects::CBioseq_set::eClass_genbank;
        } 

        if (new_class != old_class && new_class != objects::CBioseq_set::eClass_not_set) {
            objects::CBioseq_set_Handle beh = m_TopSeqEntry.GetSet();
            // create command
            CRef<objects::CBioseq_set> new_set(new objects::CBioseq_set());
            new_set->Assign(*(beh.GetCompleteBioseq_set()));
            new_set->SetClass(new_class);
            CCmdChangeBioseqSet *cmd = new CCmdChangeBioseqSet(beh, *new_set);

            // execute command
            m_CmdProcessor->Execute(cmd);
        }
    }
}


static bool s_MatchesFieldName (string field_name, string label)
{
    NStr::ReplaceInPlace(label, " ", "");
    NStr::ReplaceInPlace(label, "\n", "");
    NStr::ReplaceInPlace(field_name, " ", "");
    NStr::ReplaceInPlace(field_name, "\n", "");
    return NStr::EqualNocase(field_name, label);
}


CTableCommandConverter* CSubPrep_panel::GetConverterForFieldName (string field_name)
{
    CTableCommandConverter* converter = NULL;
    if (NStr::FindNoCase(field_name, "primer") != string::npos) {
        converter = new CPrimerTableCommandConverter();
    } else if (s_MatchesFieldName(field_name, kChimeraLabel)) {
        CSourceRequirements::EWizardSrcType src_type = x_GetWizardSrcTypeFromCtrls();
        converter = new CChimeraCommentDescriptorTableCommandConverter(kChimeraLabel, src_type);
    } else if (s_MatchesFieldName(field_name, kAssemblyDescription)) {
        converter = new CCommentDescriptorTableCommandConverter(kAssemblyDescription);
    } else if (s_MatchesFieldName(field_name, kMoleculeType) || s_MatchesFieldName(field_name, kTopology)) {
        CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl ();
        converter = new CMolInfoTableCommandConverter(wizard_type);
    } else if (s_MatchesFieldName(field_name, kLocation) || s_MatchesFieldName(field_name, kGenome)) {
        CSourceRequirements::EWizardType wizard_type = x_GetWizardTypeFromCtrl ();
        CSourceRequirements::EWizardSrcType src_type = x_GetWizardSrcTypeFromCtrls();
        converter = new CGenomeTableCommandConverter (wizard_type, src_type);
    }
    if (!converter) {
        for (vector<string>::iterator it = m_DBLinkFields.begin(); it != m_DBLinkFields.end(); it++) {        
            if (NStr::EqualNocase(*it, field_name))  {
                vector<string> fields;
                fields.push_back(field_name);
                converter = new CDBLinkTableCommandConverter(fields);
                break;
            }
        }
    }

    return converter;
}


void CSubPrep_panel::HandleBulkCmdDlg (CBulkCmdDlg* dlg, string label)
{
    dlg->Show(true);
/*    bool done = false;
    while (!done) {
        if (dlg.ShowModal() == wxID_OK) {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            if (cmd) {
                if (!NStr::IsBlank(label)) {
                    cmd->AddCommand(*SetWizardFieldInSeqEntry(m_TopSeqEntry, label + " Confirmed", "Yes"));
                }
                m_CmdProcessor->Execute(cmd);
                dlg.Close();        
                done = true;
            } else {
                wxString error = ToWxString(dlg.GetErrorMessage());
                wxMessageBox(error, wxT("Error"),
                         wxOK | wxICON_ERROR, this);
            }
        } else {
            done = true;
        }
    }
*/
}


void CSubPrep_panel::RemoveAllFeatures()
{
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > product_to_cds;
    GetProductToCDSMap(m_TopSeqEntry.GetScope(), product_to_cds);
    CRef<CCmdComposite> delete_features_cmd(new CCmdComposite("Remove All Features"));
    size_t count = 0;
    // only delete features on nucleotide sequences, protein features are automatically removed with the coding region
    CBioseq_CI bi(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
    while (bi) {
        CFeat_CI it(*bi);
        while (it) {
	    CRef<CCmdComposite> cmd = GetDeleteFeatureCommand(it->GetSeq_feat_Handle(), true, product_to_cds);
             delete_features_cmd->AddCommand(*cmd);
             count++;
             ++it;
        }
        ++bi;
    }
    if (count > 0) {
        if (wxMessageBox(ToWxString("Are you sure you want to remove " + NStr::NumericToString(count) + " features?"), wxT("Are you sure?"),
                         wxOK | wxCANCEL, this) == wxOK) {
            m_CmdProcessor->Execute(delete_features_cmd);
        }
    } else {
        wxMessageBox(_("No features found!"), wxT("Error"),
                 wxOK | wxICON_ERROR, this);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON13
 */

void CSubPrep_panel::OnClickVectorTrim( wxCommandEvent& event )
{
    x_LaunchVectorTrimEditor();
}


void CSubPrep_panel::ImportFeatureTableFile ()
{
    CImportFeatTable::ShowTableInfo(); // show informative message
    CImportFeatTable worker(m_TopSeqEntry);
    CIRef<IEditCommand> cmd = worker.ImportFeaturesFromFile();
    if (!cmd)
        return;

    m_CmdProcessor->Execute(cmd);
}


void CSubPrep_panel::x_CheckValidation()
{
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    validator::CValidator validator(*objmgr);
    unsigned int options = validator::CValidator::eVal_validate_id_set
                          | validator::CValidator::eVal_use_entrez; 
    CConstRef<CValidError> eval = validator.Validate(m_TopSeqEntry, options);
    if (eval->TotalSize() > 0) {
        wxMessageBox(wxT("File has been saved, but validation errors were found. Please try to correct these before submitting."), wxT("Error"),
                         wxOK | wxICON_ERROR, this);
    }

}


const string kNotReadyForSubmission = "Warning: This submission is not ready to be submitted to GenBank.  Do not submit this file until all required information is provided.";
const string kSubmissionStatus = "Submission Status";
const string kSubmissionOk = "OK";

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SAVE_BTN
 */

void CSubPrep_panel::OnSaveBtnClick( wxCommandEvent& event )
{
    // clean before saving
    CCleanup cleanup;
    CScope& scope = m_TopSeqEntry.GetScope();
    cleanup.SetScope(&scope);
        
        // perform BasicCleanup
        try {
              CConstRef<CCleanupChange> changes = cleanup.BasicCleanup (const_cast<CSeq_entry& >(*(m_TopSeqEntry.GetCompleteSeq_entry())));
    } catch (CException& e) {
                LOG_POST(Error << "error in cleanup: " << e.GetMsg());
      }

    if (!m_SubmissionOk) {
        if (wxMessageBox(ToWxString(kNotReadyForSubmission + " Do you want to save an intermediate file to complete later?"), wxT("Warning"),
                         wxOK | wxCANCEL | wxICON_WARNING, this) == wxCANCEL) {
            return;
        }
        SetWizardFieldInSeqEntryNoUndo(m_TopSeqEntry, kSubmissionStatus, m_SubmissionErrors);
    } else {
        SetWizardFieldInSeqEntryNoUndo(m_TopSeqEntry, kSubmissionStatus, kSubmissionOk);
    }
        
    wxString extensions = GetAsnSqnExtensions();
    wxFileDialog asn_save_file(this, wxT("Select a file"), m_SaveFileDir, m_SaveFileName,
                               //CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               wxT("ASN.1 files (")+extensions + wxT(")|") + extensions + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (asn_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = asn_save_file.GetPath();
        wxString name = asn_save_file.GetFilename();

        name.Trim(false);
        name.Trim(true);

        if (name.IsEmpty())
        {
            wxMessageBox(wxT("Please, select file name"), wxT("Error"),
                         wxOK | wxICON_ERROR, this);
            return; 
        }

        if (name.Find(wxUniChar('.')) == wxNOT_FOUND)
        {
            path += wxT(".sqn");
            name += wxT(".sqn");
        }

        ios::openmode mode = ios::out;
        CNcbiOfstream os(path.fn_str(), mode); 
        if (!os)
        {
            wxMessageBox(wxT("Cannot open file ")+name, wxT("Error"),
                         wxOK | wxICON_ERROR, this);
            return; 
        }
        os << MSerial_AsnText;
        
        const CSerialObject* so = NULL;
        CConstRef<CSeq_entry> seq_entry;
        if (m_SeqSubmit) so = m_SeqSubmit.GetPointer();
        else
        {
            if (m_TopSeqEntry)
                seq_entry = m_TopSeqEntry.GetSeq_entryCore();
            if (seq_entry)
                so = seq_entry.GetPointer();
        }
        
        if (so) 
            os << *so;       
        m_SaveFileDir = asn_save_file.GetDirectory();
        m_SaveFileName = name;
        m_DataIsSaved = true;
        if (m_SubmissionOk) {
            x_CheckValidation();
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON16
 */

void CSubPrep_panel::OnRemoveSequencesClick( wxCommandEvent& event )
{   
    CRemoveSequencesDlg dlg(this, m_TopSeqEntry);

    if (dlg.ShowModal() == wxID_OK) {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) {
            m_CmdProcessor->Execute(cmd);
        }
    }

}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ADD_SEQUENCE_BTN
 */

void CSubPrep_panel::OnAddSequenceBtnClick( wxCommandEvent& event )
{
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    format_ids.push_back("file_loader_fasta");
    format_ids.push_back("file_loader_asn");
    format_ids.push_back("file_loader_text_align");
    fileManager->LoadFormats(format_ids);

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.OpenTable");
    dlg.SetManagers(loadManagers);

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        if (!object_loader) {
            wxMessageBox(wxT("Failed to get object loader"), wxT("Error"),
                         wxOK | wxICON_ERROR);
        } else {
            IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
            if (execute_unit) {
                if (!execute_unit->PreExecute())
                    return;

                if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")))
                    return; // Canceled

                if (!execute_unit->PostExecute())
                    return;
            }
            CRef<CCmdComposite> cmd (new CCmdComposite("Import File"));
            const IObjectLoader::TObjects& objects = object_loader->GetObjects();
            bool has_nuc = false;
            bool has_prot = false;
            bool success = true;
            CRef<objects::CSeq_entry> entry_to_add(new objects::CSeq_entry());
            entry_to_add->SetSet().SetClass(objects::CBioseq_set::eClass_genbank);
            CRef<objects::CSubmit_block> block_to_add(NULL);
            ITERATE(IObjectLoader::TObjects, obj_it, objects) {
                const CObject& ptr = obj_it->GetObject();
                const objects::CSeq_entry* entry = dynamic_cast<const objects::CSeq_entry*>(&ptr);
                if (entry) {
                    CRef<objects::CSeq_entry> add(new objects::CSeq_entry());
                    add->Assign(*entry);
                    entry_to_add->SetSet().SetSeq_set().push_back(add);
                } else {
                    const objects::CSeq_submit* submit = dynamic_cast<const objects::CSeq_submit*>(&ptr);
                    if (submit) {
                        ITERATE (objects::CSeq_submit::TData::TEntrys, sit, submit->GetData().GetEntrys()) {
                            CRef<objects::CSeq_entry> add(new objects::CSeq_entry());
                            add->Assign(**sit);
                            entry_to_add->SetSet().SetSeq_set().push_back(add);
                        }
                        if (submit->IsSetSub() && !m_SeqSubmit && !block_to_add) {
                            if (!block_to_add) {
                                block_to_add.Reset(new objects::CSubmit_block());
                                block_to_add->Assign(submit->GetSub());
                            }
                        }
                    } else {
                        const objects::CSubmit_block* block = dynamic_cast<const objects::CSubmit_block*>(&ptr);
                        if (block) {
                            if (!block_to_add) { 
                                block_to_add.Reset(new objects::CSubmit_block());
                                block_to_add->Assign(*block);
                            }
                        } else {
                            const objects::CSeqdesc* desc = dynamic_cast<const objects::CSeqdesc*>(&ptr);
                            if (desc) {
                                CRef<objects::CSeqdesc> new_desc(new objects::CSeqdesc());
                                new_desc->Assign(*desc);
                                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(m_TopSeqEntry, *new_desc));
                                cmd->AddCommand(*cmdAddDesc);
                            }
                        }
                    }
                }
                if (!success) {
                    break;
                }
            }
            if (objects.size() == 0) {
                success = false;
                wxMessageBox(wxT("Unable to read from file"), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
            } else {
                if (entry_to_add->GetSet().IsSetSeq_set() && entry_to_add->GetSet().GetSeq_set().size() > 0) {
                    CRef<objects::CSeq_table> new_ids = GetIdsFromSeqEntry(*entry_to_add);
                    CRef<objects::CSeq_table> old_ids = GetIdsFromSeqEntry(*(m_TopSeqEntry.GetCompleteSeq_entry()));
                    CRef<objects::CSeqTable_column> problems = GetSeqIdProblems(new_ids, old_ids, 100);
                    if (problems) {
                        if (wxMessageBox(ToWxString(SummarizeIdProblems(problems) + " Do you want to fix the new sequence IDs?"), wxT("Error"),
                                         wxOK | wxCANCEL | wxICON_ERROR, this) == wxCANCEL) {
                            success = false;
                        } else {
                            CSeqIdFixDlg dlg(NULL, entry_to_add, m_TopSeqEntry.GetCompleteSeq_entry());
                            bool done = false;
                            while (!done) {
                                if (dlg.ShowModal() == wxID_OK) {
                                    CRef<objects::CSeq_table> repl_ids = dlg.GetReplacementTable();
                                    if (repl_ids) {
                                        ApplyReplacementIds (*entry_to_add, repl_ids);
                                        dlg.Close();        
                                        done = true;
                                    }
                                } else {
                                    done = true;
                                    success = false;
                                }
                            }
                        }
                    }
                    if (success) {
                        bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
                        success = AddSeqEntryToSeqEntry(entry_to_add, m_TopSeqEntry, cmd, has_nuc, has_prot, create_general_only);
                    }
                }
            }
            if (success) {
                if (block_to_add) {
                    if (!m_SeqSubmit) {
                        x_ConvertToSeqSubmit();
                    }
                    m_SeqSubmit->SetSub().Assign(*block_to_add);
                }
                m_CmdProcessor->Execute(cmd);
                UpdateForSeqEntryChange();
            }
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REORDER_SEQ
 */

void CSubPrep_panel::OnReorderSeqClick( wxCommandEvent& event )
{
    CReorderSequencesDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProcessor->Execute(cmd);                       
        dlg.Close();        
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FLATFILE
 */

void CSubPrep_panel::OnFlatfileClick( wxCommandEvent& event )
{
    static const string sViewType("Text View");

    TConstScopedObjects objects;
    if (m_SeqSubmit) {
        objects.push_back(SConstScopedObject(m_SeqSubmit, &m_TopSeqEntry.GetScope()));
    } else {
        objects.push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteObject(), &m_TopSeqEntry.GetScope()));
    }

    if (!objects.front().object)
        return;

    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    IProjectView* projectView = prjSrv->FindView(*objects.front().object, sViewType);
    if (projectView != 0) {
        prjSrv->ActivateProjectView(projectView);
    } else {
        CRef<CUser_object> params(new CUser_object());
        CRef<CObject_id> type(new CObject_id());
        type->SetStr("TextViewParams");
        params->SetType(*type);

        CUser_object::TData& data = params->SetData();

        {{
            CRef<CUser_field> param(new CUser_field());
            CRef<CObject_id> label(new CObject_id());
            label->SetStr("TextViewType");
            param->SetLabel(*label);
            param->SetData().SetStr("Flat File");
            data.push_back(param);
        }}

        {{
            CRef<CUser_field> param(new CUser_field());
            CRef<CObject_id> label(new CObject_id());
            label->SetStr("ExpandAll");
            param->SetLabel(*label);
            param->SetData().SetBool(true);
            data.push_back(param);
        }}

        {{
            CRef<CUser_field> param(new CUser_field());
            CRef<CObject_id> label(new CObject_id());
            label->SetStr("TrackSelection");
            param->SetLabel(*label);
            param->SetData().SetBool(true);
            data.push_back(param);
        }}

        prjSrv->AddProjectView(sViewType, objects, params);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PREV_BTN
 */

void CSubPrep_panel::OnPrevBtnClick( wxCommandEvent& event )
{
    m_Book->AdvanceSelection(false);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_NEXT_BTN
 */

void CSubPrep_panel::OnNextBtnClick( wxCommandEvent& event )
{
    m_Book->AdvanceSelection(true);
}


/*!
 * wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED event handler for ID_NOTEBOOK
 */

void CSubPrep_panel::OnNotebookPageChanged( wxNotebookEvent& event )
{
    x_SetNextPrevBtns(); 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_VALIDATE
 */

void CSubPrep_panel::OnValidateClick( wxCommandEvent& event )
{
    static const string sViewType("Validate Table View");

    CRef<CObjectFor<TConstScopedObjects> > objects(new CObjectFor<TConstScopedObjects>);
    if (m_SeqSubmit) {
        objects->GetData().push_back(SConstScopedObject(m_SeqSubmit, &m_TopSeqEntry.GetScope()));
    } else {
        objects->GetData().push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteObject(), &m_TopSeqEntry.GetScope()));
    }

    if (!objects->GetData().front().object)
        return;

    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    IProjectView* projectView = prjSrv->FindView(*(objects->GetData().front().object), sViewType);
    if (projectView != 0) {
        prjSrv->ActivateProjectView(projectView);
        //!! send this message only when it's known that validated data has been really changed,
        //!! otherwise unneeded validations may take place
        if(true) {
            // send a message to the validate view that is it should re-read it's data and refresh itself
            CGridWidgetEvent evt(CGridWidgetEvent::eRefreshData);
            Send(&evt, ePool_Default);
        }
    } else {
        prjSrv->AddProjectView(sViewType, objects->GetData(), 0);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_START_NEW
 */

void CSubPrep_panel::OnStartNewClick( wxCommandEvent& event )
{
    if (m_TopSeqEntry.IsSet() && m_TopSeqEntry.GetSet().IsEmptySeq_set() && !m_TopSeqEntry.GetSet().IsSetDescr()) {
        wxMessageDialog dlg(this, wxT("No information to clear.  You may start entering data now."), wxT("Clear Data"), wxOK);
        dlg.ShowModal();
        return;
    }

    if (!m_DataIsSaved) {        
        wxMessageDialog dlg(this, wxT("You have not saved the most recent changes to this submission.  Are you sure you want to abandon this and start a new submission?"), wxT("Clear Data"), wxOK|wxCANCEL);
        int answer = dlg.ShowModal();
        if (answer == wxID_CANCEL) {
            return;
        }
    }
    m_Book->RemovePage(1);
    m_MainPanel = NULL;
    m_SeqSubmit.Reset();
    objects::CSeq_entry_EditHandle th = m_TopSeqEntry.GetEditHandle();
    th.SetSet().Reset();
    while (!m_TopSeqEntry.GetSet().IsEmptySeq_set()) {
        CRef<objects::CSeq_entry> e = m_TopSeqEntry.GetSet().GetCompleteBioseq_set()->GetSeq_set().front();
        objects::CSeq_entry_EditHandle eh = m_TopSeqEntry.GetScope().GetSeq_entryEditHandle(*e);
        eh.Remove();
    }
    UpdateForSeqEntryChange();
    
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_LOOKUP_TAXONOMY
 */

void CSubPrep_panel::OnLookupTaxonomyClick( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd = TaxonomyLookupCommand(m_TopSeqEntry);
    m_CmdProcessor->Execute(cmd);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON17
 */

void CSubPrep_panel::OnButton17Click( wxCommandEvent& event )
{
    if (m_TopSeqEntry.IsSet() && m_TopSeqEntry.GetSet().IsEmptySeq_set() && !m_TopSeqEntry.GetSet().IsSetDescr()) {
        // nothing to clear
    } else {
        if (!m_DataIsSaved) {        
            wxMessageDialog dlg(this, wxT("You have not saved the most recent changes to this submission.  Are you sure you want to abandon this and start a new submission?"), wxT("Clear Data"), wxOK|wxCANCEL);
            int answer = dlg.ShowModal();
            if (answer == wxID_CANCEL) {
                return;
            }
        }
        if (m_MainPanel != NULL) {
            m_Book->RemovePage(1);
            m_MainPanel = NULL;
        }
        m_SeqSubmit.Reset();
        objects::CSeq_entry_EditHandle th = m_TopSeqEntry.GetEditHandle();
        th.SetSet().Reset();
        while (!m_TopSeqEntry.GetSet().IsEmptySeq_set()) {
            CRef<objects::CSeq_entry> e = m_TopSeqEntry.GetSet().GetCompleteBioseq_set()->GetSeq_set().front();
            objects::CSeq_entry_EditHandle eh = m_TopSeqEntry.GetScope().GetSeq_entryEditHandle(*e);
            eh.Remove();
        }
        UpdateForSeqEntryChange();
    }
    OnAddSequenceBtnClick(event);
}


END_NCBI_SCOPE
