/*  $Id: alignment_assistant.cpp 42430 2019-02-22 13:19:16Z filippov $
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
 * Authors:  Igor Filippov
 */
  

#include <ncbi_pch.hpp>

////@begin includes
////@end includes
#include <sstream>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <objtools/validator/validator.hpp>
#include <objtools/validator/validatorp.hpp>
#include <objtools/validator/utilities.hpp>
#include <objtools/validator/validerror_format.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp> 
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_seq_graph.hpp>
#include <gui/objutils/cmd_change_graph.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/widgets/edit/utilities.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/widgets/edit/edit_object_feature_propagate.hpp>
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/textfile.h>

#include "location_panel.hpp"
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/widgets/edit/alignment_assistant.hpp>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CAlignmentAssistant, wxFrame )


/*!
 * CAlignmentAssistant event table definition
 */



BEGIN_EVENT_TABLE( CAlignmentAssistant, wxFrame )

EVT_MENU( wxID_CLOSE, CAlignmentAssistant::OnClose)
EVT_BUTTON( ID_CLOSE_ALIGNMENT_ASSISTANT_BUTTON, CAlignmentAssistant::OnClose)
EVT_BUTTON( ID_GOTO_ALIGNMENT_ASSISTANT_BUTTON, CAlignmentAssistant::OnGoTo)
EVT_BUTTON( ID_GOTO_SEQ_ALIGNMENT_ASSISTANT_BUTTON, CAlignmentAssistant::OnGoToSeq)

EVT_MENU(  ID_EXPORT_INTER_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnExportInter )
EVT_MENU(  ID_EXPORT_CONT_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnExportCont ) 
EVT_MENU( ID_EDIT_RM_SEQ_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnRemoveSeqFromAlign )
EVT_MENU( ID_EDIT_REV_STRANDS_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnReverseStrand )
EVT_MENU(  ID_EDIT_PROPAGATE_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnPropagateFeatures )
EVT_MENU(  ID_EDIT_VALIDATE_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnValidate )  
EVT_MENU( ID_SHOW_FEATURES_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnFeatures)
EVT_MENU( ID_SHOW_SUBSTITUTE_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnSubstitute)
EVT_MENU( ID_ADD_ALIGN_CDS_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnAddAlignCDS)
EVT_MENU( ID_ADD_ALIGN_RNA_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnAddAlignRNA)
EVT_MENU( ID_ADD_ALIGN_OTHER_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::OnAddAlignOther)
EVT_UPDATE_UI_RANGE(ID_ADD_ALIGN_CDS_ALIGNMENT_ASSISTANT_MENU, ID_ADD_ALIGN_RNA_ALIGNMENT_ASSISTANT_MENU, CAlignmentAssistant::IsNucleotide)

EVT_MENU (eCmdCreateGene, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateLTR, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateProt, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateSecondaryStructure, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateRegion, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatePubFeatureLabeled, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateStemLoop, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateSTS, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateCodingRegion, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateRepeatRegion, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateVariation, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateOperon, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMobileElement, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateCentromere, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateTelomere, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatePreRNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMRNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateExon, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateIntron, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreate3UTR, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreate5UTR, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatetRNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreaterRNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateRNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatencRNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatetmRNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateCommentDescriptor, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateBiosourceFeat, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatePolyASite, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatePrimerBind, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateProteinBind, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateRepOrigin, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateOriT, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateCRegion, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateDLoop, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateDSegment, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateiDNA, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateJSegment, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMiscBinding, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMiscDifference, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMiscFeature, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMiscRecomb, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMiscStructure, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateModifiedBase, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateNRegion, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreatePrimTranscript, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateRegulatory, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateSRegion, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateUnsure, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateVRegion, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateVSegment, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateProprotein, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateMatPeptide, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateSigPeptide, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateTransitPeptide, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateBond, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateSite, CAlignmentAssistant::CreateFeature)
EVT_MENU (eCmdCreateAssemblyGap, CAlignmentAssistant::CreateFeature)

EVT_UPDATE_UI_RANGE(eCmdCreateBiosourceFeat, eCmdCreatePubFeatureLabeled, CAlignmentAssistant::IsSelectionAndClean )

END_EVENT_TABLE()


static
WX_DEFINE_MENU(kAnnotateMenu)
        WX_SUBMENU("Genes and Named Regions")
            WX_MENU_ITEM(eCmdCreateGene)
            WX_MENU_ITEM_INT(eCmdCreateProt)
            WX_MENU_ITEM(eCmdCreateRepeatRegion)
            WX_MENU_ITEM(eCmdCreateStemLoop)
            WX_MENU_ITEM_INT(eCmdCreateSTS)
            WX_MENU_ITEM_INT(eCmdCreateRegion)
            WX_MENU_ITEM_INT(eCmdCreateSecondaryStructure)
            WX_MENU_ITEM(eCmdCreateProprotein)
            WX_MENU_ITEM(eCmdCreateMatPeptide)
            WX_MENU_ITEM(eCmdCreateSigPeptide)
            WX_MENU_ITEM(eCmdCreateTransitPeptide)
            WX_MENU_ITEM(eCmdCreateOperon)
            WX_MENU_ITEM(eCmdCreateMobileElement)
            WX_MENU_ITEM(eCmdCreateCentromere)
            WX_MENU_ITEM(eCmdCreateTelomere)
            WX_MENU_ITEM(eCmdCreateRegulatory)
        WX_END_SUBMENU()
        WX_SUBMENU("Coding Regions and Transcripts")
              WX_MENU_ITEM(eCmdCreateCodingRegion)
              WX_MENU_ITEM_INT(eCmdCreatePreRNA)
              WX_MENU_ITEM(eCmdCreateMRNA)
              WX_MENU_ITEM(eCmdCreateExon)
              WX_MENU_ITEM(eCmdCreateIntron)
              WX_MENU_ITEM(eCmdCreate3UTR)
              WX_MENU_ITEM(eCmdCreate5UTR)
        WX_END_SUBMENU()
        WX_SUBMENU("Structural RNAs")
              WX_MENU_ITEM(eCmdCreatetRNA)
              WX_MENU_ITEM(eCmdCreaterRNA)
              WX_MENU_ITEM(eCmdCreateRNA)
              WX_MENU_ITEM(eCmdCreatencRNA)
              WX_MENU_ITEM(eCmdCreatetmRNA)
        WX_END_SUBMENU()
        WX_SUBMENU("Bibliographic and Comments")
              WX_MENU_ITEM_INT(eCmdCreateCommentDescriptor)
              WX_MENU_ITEM(eCmdCreateBiosourceFeat)
        WX_END_SUBMENU()
        WX_SUBMENU("Sites and Bonds")
              WX_MENU_ITEM(eCmdCreatePolyASite)
              WX_MENU_ITEM(eCmdCreatePrimerBind)
              WX_MENU_ITEM(eCmdCreateProteinBind)
              WX_MENU_ITEM(eCmdCreateRepOrigin)
              WX_MENU_ITEM_INT(eCmdCreateBond)
              WX_MENU_ITEM_INT(eCmdCreateSite)
              WX_MENU_ITEM(eCmdCreateOriT)
        WX_END_SUBMENU()
        WX_SUBMENU("Remaining Features")
              WX_MENU_ITEM_INT(eCmdCreateCRegion)
              WX_MENU_ITEM(eCmdCreateDLoop)
              WX_MENU_ITEM_INT(eCmdCreateDSegment)
              WX_MENU_ITEM(eCmdCreateiDNA)
              WX_MENU_ITEM_INT(eCmdCreateJSegment)
              WX_MENU_ITEM(eCmdCreateMiscBinding)
              WX_MENU_ITEM(eCmdCreateMiscDifference)
              WX_MENU_ITEM(eCmdCreateMiscFeature)
              WX_MENU_ITEM(eCmdCreateMiscRecomb)
              WX_MENU_ITEM(eCmdCreateMiscStructure)
              WX_MENU_ITEM_INT(eCmdCreateModifiedBase)
              WX_MENU_ITEM_INT(eCmdCreateNRegion)
              WX_MENU_ITEM_INT(eCmdCreatePrimTranscript)
              WX_MENU_ITEM_INT(eCmdCreateSRegion)
              WX_MENU_ITEM(eCmdCreateUnsure)
              WX_MENU_ITEM_INT(eCmdCreateVRegion)
              WX_MENU_ITEM_INT(eCmdCreateVSegment)
              WX_MENU_ITEM(eCmdCreateVariation)
        WX_END_SUBMENU()
WX_END_MENU()


/*!
 * CAlignmentAssistant constructors
 */

CAlignmentAssistant::CAlignmentAssistant()
{
    Init();
}


CAlignmentAssistant::CAlignmentAssistant( wxWindow* parent, CSeq_entry_Handle seh, ICommandProccessor* processor, CConstRef<CSeq_align> align,
               wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_CmdProcessor(processor)
{
    Init();
    GetAlignment(align);
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
    ReportUnresolvedBioseqs();
}


bool CAlignmentAssistant::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignmentAssistant creation
//    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);

////@end CAlignmentAssistant creation

    return true;
}


/*!
 * CAlignmentAssistant destructor
 */

CAlignmentAssistant::~CAlignmentAssistant()
{
////@begin CAlignmentAssistant destruction
////@end CAlignmentAssistant destruction
}


/*!
 * Member initialisation
 */

void CAlignmentAssistant::Init()
{
    m_Panel = NULL;
    m_GoTo_TextCtrl = NULL;
    m_GoToSeq_TextCtrl = NULL;
}


/*!
 * Control creation for CAlignmentAssistant
 */

void CAlignmentAssistant::CreateControls()
{    
    wxMenuBar *menubar = new wxMenuBar();  

    wxMenu *file_menu = new wxMenu();
    menubar->Append(file_menu, wxT("&File"));

    wxMenu *export_submenu = new wxMenu();
    file_menu->AppendSubMenu(export_submenu, _("Export"));

    wxMenuItem *interleave_item = new wxMenuItem(export_submenu, ID_EXPORT_INTER_ALIGNMENT_ASSISTANT_MENU, "Interleave", wxEmptyString, wxITEM_NORMAL);
    export_submenu->Append(interleave_item);
    wxMenuItem *cont_item = new wxMenuItem(export_submenu, ID_EXPORT_CONT_ALIGNMENT_ASSISTANT_MENU, "Contiguous", wxEmptyString, wxITEM_NORMAL);
    export_submenu->Append(cont_item);
//    wxMenuItem *text_item = new wxMenuItem(export_submenu, ID_EXPORT_TEXT_ALIGNMENT_ASSISTANT_MENU, "Text Representation", wxEmptyString, wxITEM_NORMAL);
//    export_submenu->Append(text_item);

    wxMenuItem *close_item = new wxMenuItem(file_menu, wxID_CLOSE, _("Close"), _("Close"), wxITEM_NORMAL);
    file_menu->Append(close_item);

    wxMenu *edit_menu = new wxMenu();
    menubar->Append(edit_menu, wxT("&Edit"));

    wxMenuItem *remove_seq_item = new wxMenuItem(edit_menu, ID_EDIT_RM_SEQ_ALIGNMENT_ASSISTANT_MENU, _("Remove Sequence from Alignment"), wxEmptyString, wxITEM_NORMAL);
    edit_menu->Append(remove_seq_item);
       
    wxMenuItem *rev_strands_item = new wxMenuItem(edit_menu, ID_EDIT_REV_STRANDS_ALIGNMENT_ASSISTANT_MENU, _("Reverse Sequence Strands"), wxEmptyString, wxITEM_NORMAL);
    edit_menu->Append(rev_strands_item);

    wxMenuItem *validate_item = new wxMenuItem(edit_menu, ID_EDIT_VALIDATE_ALIGNMENT_ASSISTANT_MENU, _("Validate Alignment"), wxEmptyString, wxITEM_NORMAL);
    edit_menu->Append(validate_item);

    wxMenuItem *propagate_item = new wxMenuItem(edit_menu, ID_EDIT_PROPAGATE_ALIGNMENT_ASSISTANT_MENU, _("Propagate Features"), wxEmptyString, wxITEM_NORMAL);
    edit_menu->Append(propagate_item);

    wxMenu *view_menu = new wxMenu();
    menubar->Append(view_menu, wxT("&View"));

    m_target_submenu = new wxMenu();
    view_menu->AppendSubMenu(m_target_submenu, _("Target"));

    for (size_t i = 0; i < m_Labels.size(); i++)
    {
        wxMenuItem *target_item = new wxMenuItem(m_target_submenu, wxID_ANY, wxString(m_Labels[i]), wxEmptyString, wxITEM_NORMAL);
        m_target_submenu->Append(target_item);
    }
    m_target_submenu->Bind(wxEVT_MENU, &CAlignmentAssistant::OnTarget, this);

    wxMenuItem *substitute_item = new wxMenuItem(view_menu, ID_SHOW_SUBSTITUTE_ALIGNMENT_ASSISTANT_MENU, _("Show all bases\tAlt+s"), _("Show all bases"), wxITEM_CHECK);
    view_menu->Append(substitute_item);

    wxMenuItem *features_item = new wxMenuItem(view_menu, ID_SHOW_FEATURES_ALIGNMENT_ASSISTANT_MENU, _("Features\tAlt+f"), _("Show features"), wxITEM_CHECK);
    view_menu->Append(features_item);

    wxMenu *feature_menu = new wxMenu();
    menubar->Append(feature_menu, wxT("Features"));

    wxMenu *annotate_seq_submenu = CUICommandRegistry::GetInstance().CreateMenu(kAnnotateMenu);
    feature_menu->AppendSubMenu(annotate_seq_submenu,_("Apply To Target Sequence"));

    wxMenu *annotate_alignment_submenu = new wxMenu();
    feature_menu->AppendSubMenu(annotate_alignment_submenu,_("Apply To Alignment"));

    wxMenuItem *align_cds_item = new wxMenuItem(annotate_alignment_submenu, ID_ADD_ALIGN_CDS_ALIGNMENT_ASSISTANT_MENU, _("CDS"), _("CDS"), wxITEM_NORMAL);
    annotate_alignment_submenu->Append(align_cds_item);

    wxMenuItem *align_rna_item = new wxMenuItem(annotate_alignment_submenu, ID_ADD_ALIGN_RNA_ALIGNMENT_ASSISTANT_MENU, _("RNA"), _("RNA"), wxITEM_NORMAL);
    annotate_alignment_submenu->Append(align_rna_item);

    wxMenuItem *align_other_item = new wxMenuItem(annotate_alignment_submenu, ID_ADD_ALIGN_OTHER_ALIGNMENT_ASSISTANT_MENU, _("Other Feature"), _("Other Feature"), wxITEM_NORMAL);
    annotate_alignment_submenu->Append(align_other_item);

    SetMenuBar(menubar);

    wxPanel *Dialog1 = new wxPanel(this, wxID_ANY);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    Dialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT, 0);

    wxButton* itemButton6 = new wxButton( Dialog1, ID_GOTO_ALIGNMENT_ASSISTANT_BUTTON, wxT("Go to alignment position:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GoTo_TextCtrl = new wxTextCtrl( Dialog1, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_GoTo_TextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( Dialog1, ID_GOTO_SEQ_ALIGNMENT_ASSISTANT_BUTTON, wxT("Go to sequence position:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    m_GoToSeq_TextCtrl = new wxTextCtrl( Dialog1, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_GoToSeq_TextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT, 0);

    m_Range = new wxStaticText( Dialog1, wxID_STATIC, wxT(""), wxDefaultPosition, wxSize(Dialog1->ConvertDialogToPixels(wxSize(40, -1)).x, -1), wxNO_BORDER );
    itemBoxSizer4->Add(m_Range, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    wxBoxSizer *BoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(BoxSizer, 1, wxGROW, 0);    
    m_Panel = new CPaintAlignment(Dialog1, m_Alignment, m_FeatRanges, m_FeatTypes, m_FeatStrand);
    BoxSizer->Add(m_Panel, 1, wxGROW|wxALL, 5);
  
    m_Panel->SetFocus();

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL, 0);

    wxButton* itemButton13 = new wxButton( Dialog1, ID_CLOSE_ALIGNMENT_ASSISTANT_BUTTON, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStatusBar *status = CreateStatusBar();
}



void CAlignmentAssistant::GetAlignment(CConstRef<CSeq_align> align)
{
    m_Alignment.Reset();
    if (m_TopSeqEntry)
    {
        for (CAlign_CI align_ci(m_TopSeqEntry); align_ci; ++align_ci)
            if (!align || align->Equals(align_ci.GetOriginalSeq_align()) || align->Equals(*align_ci))
            {
                m_Alignment = align_ci.GetSeq_align_Handle();
                break;
            }
    }
    
    if (!m_Alignment)
    {
        NCBI_THROW( CException, eUnknown, "No Seq-align found" );
    }    
    
    GetFeatures();
}

/*!
 * Should we show tooltips?
 */

bool CAlignmentAssistant::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAlignmentAssistant::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CAlignmentAssistant::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}


void CAlignmentAssistant::OnClose( wxCommandEvent& event )
{ 
    Close();
}


void CAlignmentAssistant::ReportPos(int pos, const string &label)
{
    wxStatusBar *status = GetStatusBar();
    status->SetStatusText(_("current position: ") + wxString(label) + _(" ") + ToWxString(NStr::IntToString(pos)));
}

void CAlignmentAssistant::ReportRange(int pos1, int pos2, const string &label)
{
    if (pos1 > 0 && pos2 > 0)
    {
        m_Range->SetLabel(wxString(label) + _(" ") + ToWxString(NStr::IntToString(pos1)+"-"+NStr::IntToString(pos2)));
    }
    else
    {
        m_Range->SetLabel(wxEmptyString);
    }
}


void CAlignmentAssistant::OnGoTo( wxCommandEvent& event )
{ 
    wxString val = m_GoTo_TextCtrl->GetValue();
    long pos;
    if(val.ToLong(&pos)) 
    {
        m_Panel->SetPos(pos);
        m_Panel->SetFocus();
    }   
}

void CAlignmentAssistant::OnGoToSeq( wxCommandEvent& event )
{ 
    wxString val = m_GoToSeq_TextCtrl->GetValue();
    long pos;
    if(val.ToLong(&pos)) 
    {
        m_Panel->SetPosSeq(pos);
        m_Panel->SetFocus();
    }   
}

void CAlignmentAssistant::ReportUnresolvedBioseqs()
{
    if ( m_Alignment &&   m_Alignment.GetSegs().Which() == CSeq_align::C_Segs::e_Denseg )
    {
        const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
        if ( denseg.IsSetDim() && denseg.IsSetNumseg() &&
             denseg.IsSetIds() && denseg.IsSetStarts() &&
             denseg.IsSetLens()) 
        {
            vector<string> labels;
            CDense_seg::TDim num_rows = denseg.GetDim();
            CDense_seg::TNumseg num_segs = denseg.GetNumseg();
            for (CDense_seg::TDim row = 0; row < num_rows; ++row)
            {
                const CSeq_id& id = denseg.GetSeq_id(row);
                CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(id);
                if (!bsh)
                {
                    string label;
                    id.GetLabel(&label, CSeq_id::eContent);
                    labels.push_back(label);
                }
            }
            if (!labels.empty())
            {
                string msg;
                msg  = "Cannot resolve the following bioseq";
                msg += (labels.size() > 1) ? "s: \n" : ": \n";
                msg += NStr::Join(labels, "\n");
                CGenericReportDlg* report = new CGenericReportDlg(this);
                report->SetTitle(wxT("Unresolved bioseqs"));
                report->SetText(ToWxString(msg));
                report->Show(true);
                report->SetFocus();
            }
        }        
    }
}

void CAlignmentAssistant::GetFeatures()  
{
    m_FeatRanges.clear();
    m_FeatTypes.clear();
    m_FeatStrand.clear();
    m_Labels.clear();

    if ( m_Alignment &&   m_Alignment.GetSegs().Which() == CSeq_align::C_Segs::e_Denseg )
    {
        const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
        if ( denseg.IsSetDim() && denseg.IsSetNumseg() &&
             denseg.IsSetIds() && denseg.IsSetStarts() &&
             denseg.IsSetLens()) 
        {
            CDense_seg::TDim num_rows = denseg.GetDim();
            CDense_seg::TNumseg num_segs = denseg.GetNumseg();
            for (CDense_seg::TDim row = 0; row < num_rows; ++row)
            {
                const CSeq_id& id = denseg.GetSeq_id(row);
                CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(id);
                if (!bsh)
                    continue;
                m_Labels.push_back(CPaintAlignment::GetSeqTitle(bsh));

                m_is_nucleotide = bsh.IsNucleotide();
                int length = bsh.GetBioseqLength();
                CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(0,0);
                TSeqPos seq_start = loc->GetStart(objects::eExtreme_Positional);
                vector<vector<pair<TSeqPos,TSeqPos> > >  feat_ranges;
                vector<pair<string,CSeqFeatData::ESubtype> >  feat_types;
                vector<CBioseq_Handle::EVectorStrand>  feat_strand;
                CFeat_CI feat_it(bsh);
                while (feat_it) 
                {
                    if (feat_it->GetOriginalFeature().IsSetData())
                    {
                        const CSeq_loc& feat_loc = feat_it->GetLocation();
                        CBioseq_Handle::EVectorStrand strand = CBioseq_Handle::eStrand_Plus;
                        if (feat_loc.GetStrand() == eNa_strand_minus)
                            strand = CBioseq_Handle::eStrand_Minus;
                        vector<pair<TSeqPos,TSeqPos> > vec;
                        for(CSeq_loc_CI loc_it(feat_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)     
                        {
                            CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                            TSeqPos feat_start = feat_range.GetFrom() - seq_start;
                            TSeqPos feat_stop = feat_range.GetTo() - seq_start;                           
                            vec.push_back(pair<TSeqPos,TSeqPos>(feat_start,feat_stop));
                        }                        
                        if (!vec.empty())
                        {
                            CSeqFeatData::ESubtype subtype = feat_it->GetOriginalFeature().GetData().GetSubtype();
                            string label;
                            feature::GetLabel(feat_it->GetOriginalFeature(), &label, feature::fFGL_Content);
                            feat_types.push_back(pair<string,CSeqFeatData::ESubtype>(label,subtype));
                            feat_ranges.push_back(vec);
                            feat_strand.push_back(strand);               
                        }
                    }
                    ++feat_it;
                }
                m_FeatTypes.push_back(feat_types);
                m_FeatRanges.push_back(feat_ranges);
                m_FeatStrand.push_back(feat_strand);           
            }
        }
    }
}

bool CAlignmentAssistant::x_ExecuteCommand(IEditCommand* command)
{
    if (m_TopSeqEntry && m_CmdProcessor && command)
    {
        m_CmdProcessor->Execute(command);
        return true;
    }
    return false;
}


void CAlignmentAssistant::OnFeatures( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableFeatures(true);
    }
    else
    {
        m_Panel->EnableFeatures(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CAlignmentAssistant::OnSubstitute( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableSubstitute(false);
    }
    else
    {
        m_Panel->EnableSubstitute(true);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}



void CAlignmentAssistant::IsSelectionAndClean(wxUpdateUIEvent &event)
{
    CSeqFeatData::ESubtype subtype = CBioseqEditor::GetFeatTypeFromCmdID(event.GetId());
    bool allow = true;
    CSeqFeatData::EFeatureLocationAllowed allowed_type = CSeqFeatData::AllowedFeatureLocation(subtype);
    if ( (m_is_nucleotide && allowed_type == CSeqFeatData::eFeatureLocationAllowed_ProtOnly) ||
         (!m_is_nucleotide && allowed_type == CSeqFeatData::eFeatureLocationAllowed_NucOnly) )
        allow = false;
    event.Enable(m_Panel->IsSelection() && allow && m_TopSeqEntry && m_Panel->GetVisibleRow() >= 0);
}

void CAlignmentAssistant::IsNucleotide(wxUpdateUIEvent &event)
{
    event.Enable(m_is_nucleotide && m_TopSeqEntry);
}


void CAlignmentAssistant::CreateFeature ( wxCommandEvent& event) 
{
    if (!m_TopSeqEntry)
        return;
    int row = m_Panel->GetVisibleRow();
    if (row < 0)
        return;
    pair<int,int> sel = m_Panel->GetSelection(row);
    row = m_Panel->GetRow();

    if (sel.first >=0 && sel.second >= 0 && sel.second != sel.first)
    {
        CRef<CSeq_loc> interval(new CSeq_loc);
        if (sel.first > sel.second)
        {
            interval->SetInt().SetStrand(eNa_strand_minus);
            swap(sel.first, sel.second);
        }
        interval->SetInt().SetFrom(sel.first);
        interval->SetInt().SetTo(sel.second);
       
        CRef<CSeq_id> id(new CSeq_id);

        const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
        const CSeq_id& row_id = denseg.GetSeq_id(row);
        CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(row_id);
        CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(0,0);
        id->Assign(*loc->GetId());
        interval->SetInt().SetId(*id);
        CSeqFeatData::ESubtype subtype = CBioseqEditor::GetFeatTypeFromCmdID(event.GetId());
        CRef<CSeq_feat> feat = CBioseqEditor::MakeDefaultFeature(subtype);
        feat->SetLocation().Assign(*interval);

        CIRef<IEditObject> editor(new CEditObjectSeq_feat(*feat, bsh.GetSeq_entry_Handle(), bsh.GetScope(), true));
        CEditObjViewDlgModal edit_dlg(NULL, true);
        wxWindow* editorWindow = editor->CreateWindow(&edit_dlg);
        edit_dlg.SetEditorWindow(editorWindow);
        edit_dlg.SetEditor(editor);
        if (edit_dlg.ShowModal() == wxID_OK) 
        {
            CIRef<IEditCommand> cmd(editor->GetEditCommand());
            if (cmd && x_ExecuteCommand(cmd)) {
                GetFeatures();
                m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
                m_Panel->Refresh();
                m_Panel->SetFocus();
            }
        } else
            m_Panel->SetFocus();
    }
}

void CAlignmentAssistant::DeleteSelection()
{
    if (!m_TopSeqEntry || !m_Panel || !m_Panel->IsSelection())
        return;
    pair<int, int> align_sel = m_Panel->GetAlignSelection();
    int from_align = align_sel.first;
    int to_align = align_sel.second;
    if (from_align == 0 && to_align == m_Panel->GetTotalLength() - 1)
        return;
    if (from_align != 0 && to_align != m_Panel->GetTotalLength() - 1)
    {
        if (wxYES != wxMessageBox(_("You are not trimming the ends, the selection is internal. Are you sure?"), wxT("Confirm"), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION))
            return;
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Delete Selection"));

    const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
    CDense_seg::TDim num_rows = denseg.GetDim();
    int visible_row = 0;
    vector<int> offset(num_rows, 0);
    for (CDense_seg::TDim row = 0; row < num_rows; ++row)
    {
        const CSeq_id& row_id = denseg.GetSeq_id(row);
        CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(row_id);
        if (!bsh)
            continue;
        pair<int,int> sel = m_Panel->GetSelection(visible_row);         
        visible_row++;
        if (sel.first < 0 || sel.second < 0)
            continue;
        int from = sel.first;
        int to = sel.second;
        if (from > to)
            swap(from, to);
        offset[row] = to - from + 1;
        AdjustBioseq(bsh, from, to, cmd);
        AdjustFeatureLocations(bsh, from, to, cmd);
        TrimQualityScores(bsh, from, to, cmd); 
    }
   

    CRef<CSeq_align> align = AdjustAlign(from_align, to_align, offset, cmd);

     if (cmd && x_ExecuteCommand(cmd)) 
     {
         GetAlignment(align);
         m_Panel->SetAlign(m_Alignment);
         m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
         m_Panel->Refresh();
     }
     m_Panel->SetFocus();
}


void CAlignmentAssistant::AdjustBioseq(CBioseq_Handle bsh, int from, int to, CRef<CCmdComposite> cmd)
{
    CRef<CObjectManager> object_manager = CObjectManager::GetInstance();
    CRef<CScope> scope_copy(new CScope(*object_manager));
    CRef<CSeq_entry> copy(new CSeq_entry);
    copy->Assign(*bsh.GetSeq_entry_Handle().GetCompleteSeq_entry());
    CSeq_entry_Handle edited_seh = scope_copy->AddTopLevelSeqEntry(*copy);
    CBioseq_Handle edited_bsh = edited_seh.GetSeq();
    SSeqMapSelector selector;
    selector.SetFlags(CSeqMap::fFindData); 
    CBioseq_EditHandle besh(edited_bsh);
    CSeqMap_I seqmap_i(besh, selector);
    int start = 0;
    while (seqmap_i)
    {
        TSeqPos len = seqmap_i.GetLength();
        if (start >= from && start + len - 1 <= to)
        {
            seqmap_i = seqmap_i.Remove(); 
        }
        else if (from > start && from < start+len && to >= start+len-1)
        {
            string seq_in;
            if (besh.IsNa())
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacna);
            else
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacaa);
            string seq_out = seq_in.substr(0, from - start);
            if (besh.IsNa())
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacna, CSeq_data::e_Iupacna);
            else
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacaa, CSeq_data::e_Iupacaa);            
            ++seqmap_i;
        }
        else if (from > start && from < start+len && to < start+len-1)
        {
            string seq_in;
            if (besh.IsNa())
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacna);
            else
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacaa);
            string seq_out = seq_in.substr(0, from - start) + seq_in.substr(to - start + 1);
            if (besh.IsNa())
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacna, CSeq_data::e_Iupacna);
            else
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacaa, CSeq_data::e_Iupacaa);            
            ++seqmap_i;
        }
        else if (from <= start && to >= start && to < start + len - 1)
        {
            string seq_in;
            if (besh.IsNa())
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacna);
            else
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacaa);
            string seq_out = seq_in.substr(to - start + 1);
            if (besh.IsNa())
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacna, CSeq_data::e_Iupacna);
            else
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacaa, CSeq_data::e_Iupacaa);            
            ++seqmap_i;
        }
        else
        {
            ++seqmap_i;
        }
        start += len;
    }

    CRef<CCmdChangeBioseqInst> cmd_bioseq(new CCmdChangeBioseqInst(bsh, edited_seh.GetCompleteSeq_entry()->GetSeq().GetInst()));
    cmd->AddCommand(*cmd_bioseq);
}

CRef<CSeq_align> CAlignmentAssistant::AdjustAlign(int from, int to, const vector<int> &offset, CRef<CCmdComposite> cmd)
{
    CRef<CSeq_align> align(new CSeq_align());
    align->Assign(*m_Alignment.GetSeq_align());
    align->ResetSegs();
    const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
    CDense_seg::TDim num_rows = denseg.GetDim();
    CDense_seg::TNumseg num_segs = denseg.GetNumseg();

    CRef<CDense_seg> ds(new CDense_seg);
    ds->SetDim(num_rows);
    

    ITERATE(CDense_seg::TIds, idI, denseg.GetIds()) 
    {
        CSeq_id *si = new CSeq_id;
        si->Assign(**idI);
        ds->SetIds().push_back(CRef<CSeq_id>(si));
    }
    bool already_moved = false;
    TSeqPos pos = 0;    
    for (CDense_seg::TNumseg seg = 0;  seg < num_segs;  ++seg) 
    {
        TSeqPos len = denseg.GetLens()[seg];
        TSeqPos new_len = len;
        if (from > pos && from < pos + len && to >= pos + len - 1)
        {
            new_len = from - pos;
            ds->SetLens().push_back(new_len);
            for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
            {
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + dim];
                if (start != -1) 
                {
                    if (denseg.IsSetStrands() && denseg.GetStrands()[seg * num_rows + dim] == eNa_strand_minus) 
                    {
                        start += len - new_len - offset[dim];                       
                    }
                    else
                    {
                        // no change
                    }
                }
                ds->SetStarts().push_back(start);
            }
            if (!already_moved)
            {
                already_moved = true;
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    for (CDense_seg::TNumseg seg1 = 0;  seg1 < seg;  ++seg1) 
                    {
                        TSignedSeqPos start1 = ds->GetStarts()[seg1 * num_rows + dim];
                        if (start1 != -1 && ds->IsSetStrands() && ds->GetStrands()[seg1 * num_rows + dim] == eNa_strand_minus)
                        {
                            start1 -= offset[dim];
                            ds->SetStarts()[seg1 * num_rows + dim] = start1;
                        }
                    }
                }
            }
            
            if (denseg.IsSetStrands()) 
            {
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    ds->SetStrands().push_back(denseg.GetStrands()[seg * num_rows + dim]);
                }
            }
        }
        else if (from <= pos && to >= pos + len - 1)
        {           
            // remove segment
            if (!already_moved)
            {
                already_moved = true;
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    for (CDense_seg::TNumseg seg1 = 0;  seg1 < seg;  ++seg1) 
                    {
                        TSignedSeqPos start1 = ds->GetStarts()[seg1 * num_rows + dim];
                        if (start1 != -1 && ds->IsSetStrands() && ds->GetStrands()[seg1 * num_rows + dim] == eNa_strand_minus)
                        {
                            start1 -= offset[dim];
                            ds->SetStarts()[seg1 * num_rows + dim] = start1;
                        }
                    }
                }
            }
        }
        else if (from <= pos && to >= pos && to < pos + len - 1)
        {
            new_len = pos + len - 1 - to;
            ds->SetLens().push_back(new_len);
            for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
            {
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + dim];
                if (start != -1) 
                {
                    if (denseg.IsSetStrands() && denseg.GetStrands()[seg * num_rows + dim] == eNa_strand_minus) 
                    {
                        // no change
                    }
                    else
                    {
                        start += len - new_len - offset[dim];             
                    }
                }
                ds->SetStarts().push_back(start);
            }
            if (denseg.IsSetStrands()) 
            {
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    ds->SetStrands().push_back(denseg.GetStrands()[seg * num_rows + dim]);
                }
            }
        }
        else if (from > pos && from < pos + len - 1 && to > pos && to < pos + len - 1)
        {
            TSeqPos new_len1 = from - pos;
            ds->SetLens().push_back(new_len1);
            for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
            {
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + dim];
                if (start != -1)  
                {
                    if (denseg.IsSetStrands() && denseg.GetStrands()[seg * num_rows + dim] == eNa_strand_minus) 
                    {
                        start += len - new_len1 - offset[dim];                       
                    }
                    else
                    {
                        // no change
                    }
                }
                ds->SetStarts().push_back(start);
            }
            if (!already_moved)
            {
                already_moved = true;
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    for (CDense_seg::TNumseg seg1 = 0;  seg1 < seg;  ++seg1) 
                    {
                        TSignedSeqPos start1 = ds->GetStarts()[seg1 * num_rows + dim];
                        if (start1 != -1 && ds->IsSetStrands() && ds->GetStrands()[seg1 * num_rows + dim] == eNa_strand_minus)
                        {
                            start1 -= offset[dim];
                            ds->SetStarts()[seg1 * num_rows + dim] = start1;
                        }
                    }
                }
            }
            
            TSeqPos new_len2 = pos + len - 1 - to;
            ds->SetLens().push_back(new_len2);
            for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
            {
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + dim];
                if (start != -1) 
                {
                    if (denseg.IsSetStrands() && denseg.GetStrands()[seg * num_rows + dim] == eNa_strand_minus) 
                    {
                        // no change
                    }
                    else
                    {
                        start += len - new_len2 - offset[dim];             
                    }        
                }
                ds->SetStarts().push_back(start);
            }
            if (denseg.IsSetStrands()) 
            {
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    ds->SetStrands().push_back(denseg.GetStrands()[seg * num_rows + dim]);
                }
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    ds->SetStrands().push_back(denseg.GetStrands()[seg * num_rows + dim]);
                }
            }
        }
        else if (from < pos && to < pos)
        {
            ds->SetLens().push_back(new_len);
            for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
            {
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + dim];
                if (start != -1) 
                {
                    if (denseg.IsSetStrands() && denseg.GetStrands()[seg * num_rows + dim] == eNa_strand_minus) 
                    {
                        // no change
                    }
                    else
                    {
                        start -= offset[dim];
                    }
                }
                ds->SetStarts().push_back(start);
            }
            if (denseg.IsSetStrands()) 
            {
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    ds->SetStrands().push_back(denseg.GetStrands()[seg * num_rows + dim]);
                }
            }
        }
        else
        {
            ds->SetLens().push_back(new_len);
            for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
            {
                TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + dim];
                ds->SetStarts().push_back(start);
            }
            if (denseg.IsSetStrands()) 
            {
                for (CDense_seg::TDim dim = 0;  dim < num_rows;  ++dim) 
                {
                    ds->SetStrands().push_back(denseg.GetStrands()[seg * num_rows + dim]);
                }
            }
        }        
        
        pos += len;
    }
    ds->SetNumseg(ds->GetLens().size());     
    align->SetSegs().SetDenseg(*ds);

    CIRef<IEditCommand> cmd_align(new CCmdChangeAlign(m_Alignment,*align));
    cmd->AddCommand(*cmd_align);
    return align;
}

void CAlignmentAssistant::AdjustFeatureLocations(CBioseq_Handle bsh, int from, int to, CRef<CCmdComposite> cmd)
{
    CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(0,0);
    SAnnotSelector sel;
    CScope &scope = bsh.GetScope();

    for (CFeat_CI feat_it(scope, *loc, sel); feat_it; ++feat_it) 
    {
        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(feat_it->GetOriginalFeature());
        const CSeq_loc& feat_loc = feat_it->GetLocation();
        bool modified = false;
        bool removed = false;
        CRef<CSeq_loc> loc = AdjustLoc(feat_loc, from, to, modified, removed);

        if (modified)
        {
            if (removed)
            {
                CRef<CCmdDelSeq_feat> cmd_del(new CCmdDelSeq_feat(feat_it->GetSeq_feat_Handle()));
                if (cmd_del)
                {
                    cmd->AddCommand(*cmd_del);
                }
            }
            else
            {
                if (new_feat->IsSetData() && new_feat->GetData().IsCdregion())
                {
                    EDIT_EACH_CODEBREAK_ON_CDREGION(code_break, new_feat->SetData().SetCdregion())
                    {
                        if ((*code_break)->IsSetLoc())
                        {
                            bool cb_modified = false;
                            bool cb_removed = false;
                            CRef<CSeq_loc> cb_loc = AdjustLoc((*code_break)->GetLoc(), from, to,  cb_modified, cb_removed);
                            if (cb_modified)
                            {
                                if (cb_removed)
                                    ERASE_CODEBREAK_ON_CDREGION(code_break, new_feat->SetData().SetCdregion());
                                else
                                    (*code_break)->SetLoc(*cb_loc);
                            }
                        }
                    }
                    if (from <= feat_loc.GetStart(objects::eExtreme_Biological) && to >= feat_loc.GetStart(objects::eExtreme_Biological))
                    {
                        new_feat->SetData().SetCdregion().ResetFrame();
                    }
                    //int diff = loc->GetStart(objects::eExtreme_Biological) - feat_loc.GetStart(objects::eExtreme_Biological);
                    //if (feat_loc.GetStrand() == eNa_strand_minus)
                    //diff = feat_loc.GetStart(objects::eExtreme_Biological) - loc->GetStart(objects::eExtreme_Biological);
                    //AdjustFrame(new_feat->SetData().SetCdregion(), diff); 
                }
                if (new_feat->IsSetData() && new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt() 
                    && new_feat->GetData().GetRna().GetExt().IsTRNA() && new_feat->GetData().GetRna().GetExt().GetTRNA().IsSetAnticodon())
                {
                    bool ac_modified = false;
                    bool ac_removed = false;
                    CRef<CSeq_loc> ac_loc = AdjustLoc(new_feat->GetData().GetRna().GetExt().GetTRNA().GetAnticodon(), from, to, ac_modified, ac_removed);
                    if (ac_modified)
                    {
                        if (ac_removed)
                            new_feat->SetData().SetRna().SetExt().SetTRNA().ResetAnticodon();
                        else
                            new_feat->SetData().SetRna().SetExt().SetTRNA().SetAnticodon(*ac_loc);
                    }
                }
                new_feat->SetLocation(*loc);      
                CRef< CCmdChangeSeq_feat > cmd_change_feat(new CCmdChangeSeq_feat(feat_it->GetSeq_feat_Handle(),*new_feat)); 
                if (cmd_change_feat) 
                {
                    cmd->AddCommand(*cmd_change_feat);  
                }
            }
        }
        
    }
}

CRef<CSeq_loc> CAlignmentAssistant::AdjustLoc(const CSeq_loc& feat_loc, int from, int to, bool &modified, bool &removed)
{
    CRef<CSeq_loc> new_loc(new CSeq_loc);
    new_loc->Assign(feat_loc);
    CSeq_loc_I loc_it(*new_loc);
    while(loc_it)      
    {
        CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
        TSeqPos feat_start = feat_range.GetFrom();
        TSeqPos feat_stop = feat_range.GetTo();
        
        if (feat_start < from && feat_stop >= from && feat_stop <= to)
        {
            loc_it.SetTo(from - 1);
        }
        else if (feat_start < from && feat_stop > to)
        {
            loc_it.SetTo(feat_stop - (to - from + 1));
        }
        else if (feat_start >= from && feat_start <= to && feat_stop > to)
        {
            loc_it.SetFrom(to + 1 - (to - from + 1));
            loc_it.SetTo(feat_stop - (to - from + 1));
        }
        else if (feat_start >= from && feat_start <= to &&  feat_stop >= from && feat_stop <= to)
        {
            loc_it.Delete();
            continue;
        }
        else if (feat_start > to)
        {
            loc_it.SetFrom(feat_start - (to - from + 1));
            loc_it.SetTo(feat_stop - (to - from + 1));
        }
       
        ++loc_it;
    }
    CRef<CSeq_loc> loc = loc_it.MakeSeq_loc();
    modified = loc_it.HasChanges();
    removed = (loc_it.GetSize() == 0 || loc->GetStart(objects::eExtreme_Positional) == loc->GetStop(objects::eExtreme_Positional));
    loc_it.Rewind();
    return loc;
}
/*
void CAlignmentAssistant::AdjustFrame(CCdregion &feat, int diff)
{
    if (diff <= 0)
        return;
    int orig_frame = 0;
    if (feat.IsSetFrame()) 
    {
        if (feat.GetFrame() == CCdregion::eFrame_two) 
        {
            orig_frame = 1;
        } else if (feat.GetFrame() == CCdregion::eFrame_three) 
        {
            orig_frame = 2;
        }
    }
    int new_offset = (orig_frame - diff) % 3;
    if (orig_frame < diff && new_offset != 0)
    {
        new_offset = 3 - ((diff - orig_frame) % 3);
    }
            
    CCdregion::EFrame new_frame = CCdregion::eFrame_not_set;
    switch (new_offset) {
    case 0:
        new_frame = CCdregion::eFrame_one;
        break;
    case 1:
        new_frame = CCdregion::eFrame_two;
        break;
    case 2:
        new_frame = CCdregion::eFrame_three;
            break;
    default:
        new_frame = CCdregion::eFrame_not_set;
        break;
    }
    feat.SetFrame(new_frame);
}
*/

void CAlignmentAssistant::TrimQualityScores(CBioseq_Handle bsh, int from, int to, CRef<CCmdComposite> command)
{
    CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(0,0);

    SAnnotSelector graph_sel(CSeq_annot::C_Data::e_Graph);
    CGraph_CI graph_ci(bsh, graph_sel);
    for (; graph_ci; ++graph_ci)
    {

        const CMappedGraph& graph = *graph_ci;

        CRef<CSeq_graph> new_graph(new CSeq_graph());
        new_graph->Assign(graph.GetOriginalGraph());
        
        if (new_graph->IsSetLoc())
        {
            bool g_modified = false;
            bool g_removed = false;
            CRef<CSeq_loc> g_loc = AdjustLoc(new_graph->GetLoc(), from, to, g_modified, g_removed);
            if (g_modified)
            {
                if (g_removed)
                {
                    new_graph->ResetLoc();
                    CIRef<IEditCommand> delGraph(new CCmdDelSeq_graph( graph.GetSeq_graph_Handle() )); 
                    command->AddCommand(*delGraph);
                }
                else
                {
                    CSeq_graph::TGraph& dst_data = new_graph->SetGraph();
                    const CSeq_graph::TGraph& src_data = graph.GetOriginalGraph().GetGraph();
                    switch ( src_data.Which() ) 
                    {
                    case CSeq_graph::TGraph::e_Byte:
                    {
                        dst_data.SetByte().ResetValues();
                    }   
                    break;
                    case CSeq_graph::TGraph::e_Int:
                    {
                        dst_data.SetInt().ResetValues();
                    }   
                    break;
                    case CSeq_graph::TGraph::e_Real:
                    {
                        dst_data.SetReal().ResetValues();                        
                    }   
                    break;
                    default: break;
                    }

                    int graph_pos = 0;
                    int step = 1;
                    if (new_graph->IsSetComp())
                        step = new_graph->GetComp();
                    for (CSeq_loc_CI loc_it(new_graph->GetLoc(), CSeq_loc_CI:: eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)      
                    {
                        CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                        TSeqPos feat_start = feat_range.GetFrom();
                        TSeqPos feat_stop = feat_range.GetTo();                       
                        for (TSeqPos pos = feat_start; pos <= feat_stop; pos += step, graph_pos++)
                        {
                            if (pos >= from && pos <= to)
                                continue;
                            switch ( src_data.Which() ) 
                            {
                            case CSeq_graph::TGraph::e_Byte:
                            {
                                if (graph_pos < src_data.GetByte().GetValues().size())
                                    dst_data.SetByte().SetValues().push_back(src_data.GetByte().GetValues()[graph_pos]);
                            }   
                            break;
                            case CSeq_graph::TGraph::e_Int:
                            {
                                if (graph_pos < src_data.GetInt().GetValues().size())
                                    dst_data.SetInt().SetValues().push_back(src_data.GetInt().GetValues()[graph_pos]);
                            }   
                            break;
                            case CSeq_graph::TGraph::e_Real:
                            {
                                if (graph_pos < src_data.GetReal().GetValues().size())
                                    dst_data.SetReal().SetValues().push_back(src_data.GetReal().GetValues()[graph_pos]);
                        
                            }   
                            break;
                            default: break;
                            }
                        }
                    }
                    
                    new_graph->SetLoc(*g_loc);
                    switch ( src_data.Which() ) 
                    {
                    case CSeq_graph::TGraph::e_Byte:
                    {
                        new_graph->SetNumval(dst_data.SetByte().SetValues().size());
                    }   
                    break;
                    case CSeq_graph::TGraph::e_Int:
                    {
                        new_graph->SetNumval(dst_data.SetInt().SetValues().size());
                    }   
                    break;
                    case CSeq_graph::TGraph::e_Real:
                    {
                        new_graph->SetNumval(dst_data.SetReal().SetValues().size());
                        
                    }   
                    break;
                    default: break;
                    }
                    CIRef<IEditCommand> chgGraph( new CCmdChangeGraph( graph.GetSeq_graph_Handle(), *new_graph ));
                    command->AddCommand(*chgGraph);
                }
            }   
        }
    }
}

void CAlignmentAssistant::OnAddAlignCDS(wxCommandEvent& /*event*/)
{
    if (!m_TopSeqEntry)
        return;

    pair<int,int> sel = m_Panel->GetAlignSelection();
 
    CRef<CSeq_loc> interval(new CSeq_loc);
    if (sel.first > sel.second)
    {
        interval->SetInt().SetStrand(eNa_strand_minus);
        swap(sel.first, sel.second);
    }
    interval->SetInt().SetFrom(sel.first);
    interval->SetInt().SetTo(sel.second);

    string str(m_Labels[0]);
    for (size_t i = 1; i < m_Labels.size(); i++)
        str += ", " + m_Labels[i]; 
    CRef<CSeq_id> id(new CSeq_id);
    id->SetLocal().SetStr(str);

    interval->SetInt().SetId(*id);
    
    CAlignCDSAddPanel edit_dlg(this, interval, m_TopSeqEntry.GetScope());

    if (edit_dlg.ShowModal() == wxID_OK) 
    {
        CIRef<IEditCommand> cmd(edit_dlg.GetCommand());
        if (cmd && x_ExecuteCommand(cmd)) 
        {
            GetFeatures();
            m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
            m_Panel->Refresh();
        }
    } 
    
    m_Panel->SetFocus();
}

void CAlignmentAssistant::OnAddAlignRNA(wxCommandEvent& /*event*/)
{
    if (!m_TopSeqEntry)
        return;

    pair<int,int> sel = m_Panel->GetAlignSelection();
 
    CRef<CSeq_loc> interval(new CSeq_loc);
    if (sel.first > sel.second)
    {
        interval->SetInt().SetStrand(eNa_strand_minus);
        swap(sel.first, sel.second);
    }
    interval->SetInt().SetFrom(sel.first);
    interval->SetInt().SetTo(sel.second);

    string str(m_Labels[0]);
    for (size_t i = 1; i < m_Labels.size(); i++)
        str += ", " + m_Labels[i]; 
    CRef<CSeq_id> id(new CSeq_id);
    id->SetLocal().SetStr(str);

    interval->SetInt().SetId(*id);
    
    CAlignRNAAddPanel edit_dlg(this, interval, m_TopSeqEntry.GetScope());

    if (edit_dlg.ShowModal() == wxID_OK) 
    {
        CIRef<IEditCommand> cmd(edit_dlg.GetCommand());
        if (cmd && x_ExecuteCommand(cmd)) 
        {
            GetFeatures();
            m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
            m_Panel->Refresh();
        }
    } 
    
    m_Panel->SetFocus();
}

void CAlignmentAssistant::OnAddAlignOther(wxCommandEvent& /*event*/)
{
    if (!m_TopSeqEntry)
        return;

    pair<int,int> sel = m_Panel->GetAlignSelection();
 
    CRef<CSeq_loc> interval(new CSeq_loc);
    if (sel.first > sel.second)
    {
        interval->SetInt().SetStrand(eNa_strand_minus);
        swap(sel.first, sel.second);
    }
    interval->SetInt().SetFrom(sel.first);
    interval->SetInt().SetTo(sel.second);

    string str(m_Labels[0]);
    for (size_t i = 1; i < m_Labels.size(); i++)
        str += ", " + m_Labels[i]; 
    CRef<CSeq_id> id(new CSeq_id);
    id->SetLocal().SetStr(str);

    interval->SetInt().SetId(*id);
    
    CAlignOtherAddPanel edit_dlg(this, interval, m_TopSeqEntry);

    if (edit_dlg.ShowModal() == wxID_OK) 
    {
        CIRef<IEditCommand> cmd(edit_dlg.GetCommand());
        if (cmd && x_ExecuteCommand(cmd)) 
        {
            GetFeatures();
            m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
            m_Panel->Refresh();
        }
    } 
    
    m_Panel->SetFocus();
}

void CAlignmentAssistant::TranslateLocations(CRef<CSeq_loc> loc, vector<pair<CRef<CSeq_loc>, CBioseq_Handle> > &locations)
{
    const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
    CDense_seg::TDim num_rows = denseg.GetDim();
    for (size_t row = 0; row < num_rows; ++row)
    {
        CRef<CSeq_loc> new_loc(new CSeq_loc);
        new_loc->Assign(*loc);
        const CSeq_id& row_id = denseg.GetSeq_id(row);
        CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(row_id);
        TSeqPos seq_start = bsh.GetRangeSeq_loc(0,0)->GetStart(objects::eExtreme_Positional);
        int length = bsh.GetBioseqLength();
        CRef<CSeq_id> id(new CSeq_id);
        id->Assign(row_id);
        new_loc->SetId(*id);
        ENa_strand row_strand = m_Alignment.GetSeq_align()->GetSeqStrand(row);
        CSeq_loc_I loc_it(*new_loc); 
        while(loc_it)      
        {
            if (loc_it.IsEmpty())
            {
                loc_it.Delete();
                continue;
            }
            CSeq_loc_CI::TRange range = loc_it.GetRange();
            TSeqPos start = range.GetFrom() - seq_start;
            TSeqPos stop = range.GetTo() - seq_start;
            TSeqPos new_start = m_Panel->AlignPosToSeqPos(start, row, true);
            TSeqPos new_stop = m_Panel->AlignPosToSeqPos(stop, row, false);
            if (new_start < 0 || new_stop < 0)
            {
                loc_it.Delete();
                continue;
            }
            if (new_stop < new_start)
            {
                swap(new_start, new_stop);
            }
            ENa_strand strand = loc_it.GetStrand();
            if (IsReverse(row_strand))
            {
                loc_it.SetStrand(Reverse(strand));
                CRef<CInt_fuzz> fuzz_from;
                if (loc_it.GetFuzzFrom())
                {
                    fuzz_from.Reset(new CInt_fuzz);
                    fuzz_from->Assign(*loc_it.GetFuzzFrom());
                    loc_it.ResetFuzzFrom();
                }
                CRef<CInt_fuzz> fuzz_to;
                if (loc_it.GetFuzzTo())
                {
                    fuzz_to.Reset(new CInt_fuzz);
                    fuzz_to->Assign(*loc_it.GetFuzzTo());
                    loc_it.ResetFuzzTo();
                }
                if (fuzz_from)
                {
                    loc_it.SetFuzzTo(*fuzz_from->Negative(0));
                }
                if (fuzz_to)
                {
                    loc_it.SetFuzzFrom(*fuzz_to->Negative(0));
                }
            }
            loc_it.SetFrom(new_start);
            loc_it.SetTo(new_stop);
            ++loc_it;
        }
        CRef<CSeq_loc> add_loc = loc_it.MakeSeq_loc();
        if (add_loc && add_loc->Which() != CSeq_loc::e_not_set && add_loc->Which() != CSeq_loc::e_Null && add_loc->Which() != CSeq_loc::e_Empty)
            locations.push_back(pair<CRef<CSeq_loc>, CBioseq_Handle>(add_loc, bsh));
    }
}

                
void CAlignmentAssistant::OnTarget( wxCommandEvent& event) 
{
    wxMenuBar *menu_bar = GetMenuBar();
    wxMenuItem *menu_item = menu_bar->FindItem( event.GetId() );
    string label = menu_item->GetItemLabelText().ToStdString();
    int row = -1;
    for (size_t i = 0; i < m_Labels.size(); i++)
        if (m_Labels[i] == label)
        {
            row = i;
            break;
        }
    m_Panel->SetTarget(row);
    m_Panel->SetFocus();
}

void CAlignmentAssistant::OnExportInter( wxCommandEvent& event) 
{
    wxFileDialog save_file(this, wxT("Export to file"), wxEmptyString, wxEmptyString,
                           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                           wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (save_file.ShowModal() == wxID_OK)
    {
        wxString path = save_file.GetPath();
        if( !path.IsEmpty())
        {
            ios::openmode mode = ios::out;
            CNcbiOfstream os(path.fn_str(), mode);
            os << m_Panel->GetExportInter();
        }
    }
    m_Panel->SetFocus();
}

void CAlignmentAssistant::OnExportCont( wxCommandEvent& event) 
{
    wxFileDialog save_file(this, wxT("Export to file"), wxEmptyString, wxEmptyString,
                           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                           wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (save_file.ShowModal() == wxID_OK)
    {
        wxString path = save_file.GetPath();
        if( !path.IsEmpty())
        {
            ios::openmode mode = ios::out;
            CNcbiOfstream os(path.fn_str(), mode);
            os << m_Panel->GetExportCont();
        }
    }
    m_Panel->SetFocus();
}


void CAlignmentAssistant::OnValidate( wxCommandEvent& event) 
{
    unsigned int options = validator::CValidator::eVal_val_align;
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    CRef<CValidError> errors(new CValidError(&*m_TopSeqEntry.GetCompleteSeq_entry()));
    validator::CValidErrorFormat::SetSuppressionRules(m_TopSeqEntry, *errors);
    validator::CValidError_imp imp(*objmgr, &(*errors), NULL, options);
    imp.Validate(m_Alignment.GetAnnot());
    string msg;
    for ( CValidError_CI vit(*errors); vit; ++vit) 
    {
        string description =  vit->GetAccnver() + ":"
            + CValidErrItem::ConvertSeverity(vit->GetSeverity()) + ":"
            + vit->GetErrCode() + ":"
            + vit->GetMsg();
        if (!description.empty())
            msg += description + "\n";
    }
    if (msg.empty())
        msg = "Validation test of the alignment succeeded";
    CGenericReportDlg* report = new CGenericReportDlg(NULL);
    report->SetTitle(wxT("Validator Report"));
    report->SetText(ToWxString(msg));
    report->Show(true);
    report->SetFocus();
}

void CAlignmentAssistant::OnPropagateFeatures(wxCommandEvent& /*event*/)
{
    int row = m_Panel->GetTarget();
    const CDense_seg& denseg = m_Alignment.GetSegs().GetDenseg();
    const CSeq_id& id = denseg.GetSeq_id(row);
    CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(id);
    if (!bsh) {
        return;
    }
    
    vector<CConstRef<CSeq_feat>> propagatedFeats;
    for (CFeat_CI ci(bsh); ci; ++ci) {
        propagatedFeats.push_back(ci->GetSeq_feat());
    }

    //Create and trigger the magic object editor that will take care of the 
    // rest 
    CIRef<IEditObject> editor(
        new CEditObjectFeaturePropagate(bsh, propagatedFeats));

    CEditObjViewDlgModal edit_dlg(NULL, true);
    wxWindow* editorWindow = editor->CreateWindow(&edit_dlg);
    edit_dlg.SetEditorWindow(editorWindow);
    edit_dlg.SetEditor(editor);
    if (edit_dlg.ShowModal() == wxID_OK) 
    {
        try
        {
            CIRef<IEditCommand> cmd(editor->GetEditCommand());
            if (cmd && x_ExecuteCommand(cmd)) {
                GetFeatures();
                m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
                m_Panel->Refresh();
                m_Panel->SetFocus();
            }
        } catch(CException &e) {} catch (exception &e) {}
    } else
        m_Panel->SetFocus();
    
}

void CAlignmentAssistant::OnRemoveSeqFromAlign(wxCommandEvent& /*event*/)
 {
     int index = m_Panel->GetTarget();
     if (index < 0)
         return;
     if (wxYES != wxMessageBox(_("You are about to remove 1 sequence from the alignment. Are you sure?"), wxT("Confirm"), wxYES_NO | wxICON_QUESTION))
         return;
     
     CRef<CSeq_align> align(new CSeq_align());
     align->Assign(*m_Alignment.GetSeq_align());
     
     unsigned int dim = align->GetSegs().GetDenseg().GetIds().size();
    
     int row = 0;
     CDense_seg::TIds ids;
     for (CDense_seg::TIds::const_iterator id_iter = align->GetSegs().GetDenseg().GetIds().begin(); id_iter != align->GetSegs().GetDenseg().GetIds().end(); ++id_iter)
     {
         if (row != index)
             ids.push_back(*id_iter);
         row++;
     }
     

     if (align->GetSegs().GetDenseg().IsSetNumseg() && index < dim)
     {
         unsigned int numseg = align->GetSegs().GetDenseg().GetNumseg();
         // remove starts, strands
         CDense_seg::TStarts starts;
         CDense_seg::TStarts::const_iterator start_iter;
         CDense_seg::TStrands strands;
         CDense_seg::TStrands::const_iterator strand_iter;
         
         if (align->GetSegs().GetDenseg().IsSetStarts()) start_iter = align->GetSegs().GetDenseg().GetStarts().begin();
         if (align->GetSegs().GetDenseg().IsSetStrands()) strand_iter = align->GetSegs().GetDenseg().GetStrands().begin();
         
         for (unsigned int seg = 0; seg<numseg; seg++)
             for (unsigned int i=0; i<dim; i++)
                 if (i != index)
                 {
                     if (align->GetSegs().GetDenseg().IsSetStarts() && start_iter != align->GetSegs().GetDenseg().GetStarts().end())
                     {
                         starts.push_back(*start_iter);
                         start_iter++;
                     }
                     if (align->GetSegs().GetDenseg().IsSetStrands() && strand_iter != align->GetSegs().GetDenseg().GetStrands().end())
                     {
                         strands.push_back(*strand_iter);
                         strand_iter++;
                     }
                 }
                 else
                 {
                     if (align->GetSegs().GetDenseg().IsSetStarts() && start_iter != align->GetSegs().GetDenseg().GetStarts().end())
                         start_iter++;
                     
                     if (align->GetSegs().GetDenseg().IsSetStrands() && strand_iter != align->GetSegs().GetDenseg().GetStrands().end())
                         strand_iter++;                        
                 }
         
         if (align->GetSegs().GetDenseg().IsSetStarts())  align->SetSegs().SetDenseg().SetStarts().swap(starts);
         if (align->GetSegs().GetDenseg().IsSetStrands()) align->SetSegs().SetDenseg().SetStrands().swap(strands);
     }
     
     align->SetSegs().SetDenseg().SetDim(ids.size());        
     align->SetDim(ids.size());
     align->SetSegs().SetDenseg().SetIds().swap(ids);
     align->SetSegs().SetDenseg().RemovePureGapSegs(); // sets numsegs correctly        
     
     
     
     if (ids.size() < 2)
     {
         wxMessageBox(_("Cannot remove last pair of bioseqs from alignment"), wxT("Error"), wxOK|wxICON_ERROR);
     }
     else
     {       
         CIRef<IEditCommand> cmd(new CCmdChangeAlign(m_Alignment,*align));
         if (cmd && x_ExecuteCommand(cmd)) 
         {
             int item_id = m_target_submenu->FindItem(wxString(m_Labels[index]));
             m_target_submenu->Destroy(item_id);
             GetAlignment(align);
             m_Panel->SetAlign(m_Alignment);
             m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
             m_Panel->RefreshWithScroll();                         
         }
     }
     m_Panel->SetFocus();
 }

void CAlignmentAssistant::OnReverseStrand(wxCommandEvent& /*event*/)
 {
     int row = m_Panel->GetTarget();
     if (row < 0)
         return;
     if (wxYES != wxMessageBox(_("You are about to reverse 1 sequence in the alignment. Are you sure?"), wxT("Confirm"), wxYES_NO | wxICON_QUESTION))
         return;
     
     CRef<CSeq_align> align(new CSeq_align());
     align->Assign(*m_Alignment.GetSeq_align());
     
     CDense_seg& denseg = align->SetSegs().SetDenseg();
     CDense_seg::TDim num_rows = denseg.GetDim();
     CDense_seg::TNumseg num_segs = denseg.GetNumseg();
     const CSeq_id& id = denseg.GetSeq_id(row);
     CBioseq_Handle bsh = m_Alignment.GetScope().GetBioseqHandle(id);
     if (!bsh)
         return;
     int length = bsh.GetBioseqLength();
     if (!denseg.IsSetStrands())
     {
         vector<ENa_strand> new_strands(num_rows * num_segs, eNa_strand_plus);
         denseg.SetStrands() = new_strands;
     }
     for (CDense_seg::TNumseg seg = 0; seg < num_segs; ++seg) 
     {
         TSignedSeqPos start = denseg.GetStarts()[seg * num_rows + row];
         ENa_strand strand = denseg.GetStrands()[seg * num_rows + row];
         TSignedSeqPos len   = denseg.GetLens()[seg];
         if (start != -1)
             denseg.SetStarts()[seg * num_rows + row] = length - start - len;
         if (strand == eNa_strand_plus)
             denseg.SetStrands()[seg * num_rows + row] = eNa_strand_minus;
         else
             denseg.SetStrands()[seg * num_rows + row] = eNa_strand_plus;
     }
              
     

     CIRef<IEditCommand> cmd(new CCmdChangeAlign(m_Alignment,*align));
     if (cmd && x_ExecuteCommand(cmd)) {
         GetAlignment(align);
         m_Panel->SetAlign(m_Alignment);
         m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand);
         m_Panel->RefreshWithScroll();
     }
 }




/*!
 * CAlignCDSAddPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAlignCDSAddPanel, wxDialog )


/*!
 * CAlignCDSAddPanel event table definition
 */

BEGIN_EVENT_TABLE( CAlignCDSAddPanel, wxDialog )

////@begin CAlignCDSAddPanel event table entries
////@end CAlignCDSAddPanel event table entries

END_EVENT_TABLE()


/*!
 * CAlignCDSAddPanel constructors
 */

CAlignCDSAddPanel::CAlignCDSAddPanel()
{
    Init();
}

CAlignCDSAddPanel::CAlignCDSAddPanel( wxWindow* parent, CRef<CSeq_loc> align_loc, CScope &scope, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Loc(align_loc), m_scope(&scope)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAlignCDSAddPanel creator
 */

bool CAlignCDSAddPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignCDSAddPanel creation
    wxDialog::Create( parent, id, SYMBOL_CALIGNCDSADDPANEL_TITLE, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlignCDSAddPanel creation
    return true;
}


/*!
 * CAlignCDSAddPanel destructor
 */

CAlignCDSAddPanel::~CAlignCDSAddPanel()
{
////@begin CAlignCDSAddPanel destruction
////@end CAlignCDSAddPanel destruction
}


/*!
 * Member initialisation
 */

void CAlignCDSAddPanel::Init()
{
////@begin CAlignCDSAddPanel member initialisation
    m_ProteinName = NULL;
    m_ProteinDescription = NULL;
    m_GeneSymbol = NULL;
    m_GeneDescription = NULL;
    m_Comment = NULL;
    m_Location = NULL;
////@end CAlignCDSAddPanel member initialisation
}


/*!
 * Control creation for CAlignCDSAddPanel
 */

void CAlignCDSAddPanel::CreateControls()
{    
////@begin CAlignCDSAddPanel content construction
    CAlignCDSAddPanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Gene Symbol"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GeneSymbol = new wxTextCtrl( itemCBulkCmdPanel1, ID_ALIGNCDSADD_GENE_NAME, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_GeneSymbol, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Gene Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GeneDescription = new wxTextCtrl( itemCBulkCmdPanel1, ID_ALIGNCDSADD_GENE_DESC, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_GeneDescription, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Protein Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinName = new wxTextCtrl( itemCBulkCmdPanel1, ID_ALIGNCDSADD_PROTEIN_NAME, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_ProteinName, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Protein Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinDescription = new wxTextCtrl( itemCBulkCmdPanel1, ID_ALIGNCDSADD_PROTEIN_DESC, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_ProteinDescription, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);   

    wxStaticText* itemStaticText10 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemCBulkCmdPanel1, ID_ALIGNCDSADD_COMMENT, wxEmptyString, wxDefaultPosition, wxSize(300, 70), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Location = new CLocationPanel(itemCBulkCmdPanel1, *m_Loc, m_scope, true, false, false, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_Location, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 5);
    m_Location->TransferDataToWindow();

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemCBulkCmdPanel1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemCBulkCmdPanel1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
////@end CAlignCDSAddPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CAlignCDSAddPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAlignCDSAddPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAlignCDSAddPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAlignCDSAddPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAlignCDSAddPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlignCDSAddPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlignCDSAddPanel icon retrieval
}


CRef<CCmdComposite> CAlignCDSAddPanel::GetCommand()
{
    m_Location->TransferDataFromWindow();
    CRef<objects::CSeq_loc> loc = m_Location->GetSeq_loc();
    CAlignmentAssistant *parent = dynamic_cast<CAlignmentAssistant*>(GetParent());
    vector<pair<CRef<CSeq_loc>, CBioseq_Handle> > locations;
    parent->TranslateLocations(loc, locations);
    m_create_general_only = false;
    if (!locations.empty())
        m_create_general_only = objects::edit::IsGeneralIdProtPresent(locations.front().second.GetTopLevelEntry());
    CRef<CCmdComposite> cmd (new CCmdComposite("Align Add CDS"));   
    for (size_t i = 0; i < locations.size(); ++i)
    {
        CRef<CSeq_loc> seq_loc = locations[i].first;
        CBioseq_Handle bsh = locations[i].second;
        AddOneCommand(bsh,seq_loc,cmd);
    }
   
    return cmd;
}

void CAlignCDSAddPanel::AddOneCommand(const CBioseq_Handle& bsh, CRef<CSeq_loc> seq_loc, CRef<CCmdComposite> cmd)
{
    if (!seq_loc || !bsh || (seq_loc->IsMix() && !seq_loc->GetMix().IsSet()))
        return;


    string prot_name = ToStdString(m_ProteinName->GetValue());
    string prot_desc = ToStdString(m_ProteinDescription->GetValue());
    string cds_comment = ToStdString(m_Comment->GetValue());
    string gene_symbol = ToStdString(m_GeneSymbol->GetValue());
    string gene_desc = ToStdString(m_GeneDescription->GetValue());
  
    CRef<objects::CSeq_feat> cds(new objects::CSeq_feat());
    cds->SetData().SetCdregion();
    if (!NStr::IsBlank(cds_comment)) {
        cds->SetComment(cds_comment);
    }

    // add appropriate genetic code
    CRef<CGenetic_code> code = edit::GetGeneticCodeForBioseq(bsh);
    if (code) {
        cds->SetData().SetCdregion().SetCode(*code);
    }
    
    cds->SetLocation().Assign(*seq_loc);
    cds->SetPartial(cds->GetLocation().IsPartialStart(objects::eExtreme_Biological) || cds->GetLocation().IsPartialStop(objects::eExtreme_Biological));

    objects::CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CCleanup cleanup;

    if (seq_loc->IsPnt())
    {
        cds->SetData().SetCdregion().SetFrame(CCdregion::eFrame_not_set);
    }
    else
    {
        cds->SetData().SetCdregion().SetFrame(objects::CSeqTranslator::FindBestFrame(*cds, bsh.GetScope()));

        CRef<objects::CSeq_entry> protein = CreateTranslatedProteinSequence (cds, bsh, m_create_general_only);
        CRef<objects::CSeq_feat> prot = AddProteinFeatureToProtein (protein,
                                                                    cds->GetLocation().IsPartialStart(objects::eExtreme_Biological), 
                                                                    cds->GetLocation().IsPartialStop(objects::eExtreme_Biological)); 
    
        if (!NStr::IsBlank(prot_name)) {
            prot->SetData().SetProt().SetName().push_back(prot_name);
        }
        if (!NStr::IsBlank(prot_desc)) {
            prot->SetData().SetProt().SetDesc(prot_desc);
        }
        
        cleanup.BasicCleanup(*prot);
        cmd->AddCommand(*CRef<objects::CCmdAddSeqEntry>(new objects::CCmdAddSeqEntry(protein, seh)));
    }
 
    if (!NStr::IsBlank(gene_symbol) || !NStr::IsBlank(gene_desc)) 
    {
        // create gene feature
        CRef<objects::CSeq_feat> new_gene(new objects::CSeq_feat());
        if (!NStr::IsBlank(gene_symbol))
            new_gene->SetData().SetGene().SetLocus(gene_symbol);
        if (!NStr::IsBlank(gene_desc))
            new_gene->SetData().SetGene().SetDesc(gene_desc);
        new_gene->SetLocation().Assign(cds->GetLocation());
        if (new_gene->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || new_gene->GetLocation().IsPartialStop(objects::eExtreme_Biological)) 
        {
            new_gene->SetPartial(true);
        }
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_gene)));
    }    
    
    cleanup.BasicCleanup(*cds);
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *cds)));
}




/*!
 * CAlignRNAAddPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAlignRNAAddPanel, wxDialog )


/*!
 * CAlignRNAAddPanel event table definition
 */

BEGIN_EVENT_TABLE( CAlignRNAAddPanel, wxDialog )

////@begin CAlignRNAAddPanel event table entries
    EVT_CHOICE( ID_CALIGN_RNA_ADD_PANEL_CHOICE1, CAlignRNAAddPanel::OnSelectRNAType )

    EVT_BUTTON( ID_CALIGN_RNA_ADD_PANEL_BUTTON11, CAlignRNAAddPanel::OnAdd18SToComment )

    EVT_BUTTON( ID_CALIGN_RNA_ADD_PANEL_BUTTON12, CAlignRNAAddPanel::OnAdd16StoComment )

////@end CAlignRNAAddPanel event table entries

END_EVENT_TABLE()


/*!
 * CAlignRNAAddPanel constructors
 */

CAlignRNAAddPanel::CAlignRNAAddPanel()
{
    Init();
}

CAlignRNAAddPanel::CAlignRNAAddPanel( wxWindow* parent, CRef<CSeq_loc> align_loc, CScope &scope, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Loc(align_loc), m_scope(&scope)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAlignRNAAddPanel creator
 */

bool CAlignRNAAddPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignRNAAddPanel creation
    wxDialog::Create( parent, id, SYMBOL_CALIGN_RNA_ADD_PANEL_TITLE, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlignRNAAddPanel creation
    return true;
}


/*!
 * CAlignRNAAddPanel destructor
 */

CAlignRNAAddPanel::~CAlignRNAAddPanel()
{
////@begin CAlignRNAAddPanel destruction
////@end CAlignRNAAddPanel destruction
}


/*!
 * Member initialisation
 */

void CAlignRNAAddPanel::Init()
{
////@begin CAlignRNAAddPanel member initialisation
    m_RNAType = NULL;
    m_ncRNAClass = NULL;
    m_FieldSizer = NULL;
    m_RNANameSizer = NULL;
    m_Comment = NULL;
    m_ButtonsSizer = NULL;
    m_Location = NULL;
////@end CAlignRNAAddPanel member initialisation
    m_Locus = NULL;
    m_GeneDesc = NULL;
    m_tRNAname = NULL;
    m_RNAname = NULL;
    m_rRNAname = NULL;
}


/*!
 * Control creation for CAlignRNAAddPanel
 */

void CAlignRNAAddPanel::CreateControls()
{    
////@begin CAlignRNAAddPanel content construction
    CAlignRNAAddPanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(0, 500, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCBulkCmdPanel1, wxID_ANY, _("RNA Type"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_RNATypeStrings;
    m_RNATypeStrings.Add(_("preRNA"));
    m_RNATypeStrings.Add(_("mRNA"));
    m_RNATypeStrings.Add(_("tRNA"));
    m_RNATypeStrings.Add(_("rRNA"));
    m_RNATypeStrings.Add(_("ncRNA"));
    m_RNATypeStrings.Add(_("tmRNA"));
    m_RNATypeStrings.Add(_("miscRNA"));
    m_RNAType = new wxChoice( itemStaticBoxSizer5->GetStaticBox(), ID_CALIGN_RNA_ADD_PANEL_CHOICE1, wxDefaultPosition, wxDefaultSize, m_RNATypeStrings, 0 );
    m_RNAType->SetStringSelection(_("rRNA"));
    itemStaticBoxSizer5->Add(m_RNAType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemStaticBoxSizer5->GetStaticBox(), wxID_STATIC, _("ncRNA class"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ncRNAClassStrings;
    m_ncRNAClass = new wxComboBox( itemStaticBoxSizer5->GetStaticBox(), ID_CALIGN_RNA_ADD_PANEL_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ncRNAClassStrings, wxCB_DROPDOWN );
    m_ncRNAClass->Enable(false);
    itemBoxSizer7->Add(m_ncRNAClass, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FieldSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer4->Add(m_FieldSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("RNA Name"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FieldSizer->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RNANameSizer = new wxBoxSizer(wxHORIZONTAL);
    m_FieldSizer->Add(m_RNANameSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText13 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FieldSizer->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemCBulkCmdPanel1, ID_CALIGN_RNA_ADD_PANEL_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(300, 70), wxTE_MULTILINE );
    m_FieldSizer->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(m_ButtonsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemCBulkCmdPanel1, ID_CALIGN_RNA_ADD_PANEL_BUTTON11, _("Add '18S-ITS-5.8S-ITS-28S' to comment"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ButtonsSizer->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemCBulkCmdPanel1, ID_CALIGN_RNA_ADD_PANEL_BUTTON12, _("Add '16S-IGS-23S' to comment"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ButtonsSizer->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Location = new CLocationPanel(itemCBulkCmdPanel1, *m_Loc, m_scope, true, false, false, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer4->Add(m_Location, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 5);
    m_Location->TransferDataToWindow();

////@end CAlignRNAAddPanel content construction
    vector<string> class_vals = CRNA_gen::GetncRNAClassList();
    ITERATE(vector<string>, it, class_vals) {
        m_ncRNAClass->AppendString(ToWxString(*it));
    }
    // because we start with rRNA as the default:
    x_AddrRNAName();
    m_ButtonsSizer->ShowItems(false);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemCBulkCmdPanel1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemCBulkCmdPanel1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CAlignRNAAddPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAlignRNAAddPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAlignRNAAddPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAlignRNAAddPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAlignRNAAddPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlignRNAAddPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlignRNAAddPanel icon retrieval
}


void CAlignRNAAddPanel::x_RemoveRNANames ()
{
    // hide feature details
    if (m_RNANameSizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_RNANameSizer->GetItem(pos)->DeleteWindows();
        m_RNANameSizer->Remove(pos);
        m_RNAname = NULL;
        m_tRNAname = NULL;
        m_rRNAname = NULL;
    }
}


bool CAlignRNAAddPanel::x_AddGeneFields ()
{
    if (m_Locus == NULL && m_GeneDesc == NULL) {
        wxStaticText* itemStaticText11 = new wxStaticText( this, wxID_STATIC, _("Gene Symbol"), wxDefaultPosition, wxDefaultSize, 0 );
        m_FieldSizer->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_Locus = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
        m_FieldSizer->Add(m_Locus, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        wxStaticText* itemStaticText13 = new wxStaticText( this, wxID_STATIC, _("Gene Description"), wxDefaultPosition, wxDefaultSize, 0 );
        m_FieldSizer->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_GeneDesc = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
        m_FieldSizer->Add(m_GeneDesc, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
        return true;
    } else {
        return false;
    }

}


bool CAlignRNAAddPanel::x_RemoveGeneFields ()
{
    bool need_layout = false;
    size_t num_items = m_FieldSizer->GetItemCount();
    while (num_items > 4) {
        size_t pos = num_items - 1;
        m_FieldSizer->GetItem(pos)->DeleteWindows();
        m_FieldSizer->Remove(pos);
        need_layout = true;
        num_items = m_FieldSizer->GetItemCount();
    }
    m_Locus = NULL;
    m_GeneDesc = NULL;
    return need_layout;
}


bool CAlignRNAAddPanel::x_AddrRNAName()
{
    bool need_layout = false;
    if (m_rRNAname == NULL) {
        x_RemoveRNANames();
        wxArrayString m_NameStrings;
        m_NameStrings.Add(_("16S ribosomal RNA"));
        m_NameStrings.Add(_("18S ribosomal RNA"));
        m_NameStrings.Add(_("23S ribosomal RNA"));
        m_NameStrings.Add(_("26S ribosomal RNA"));
        m_NameStrings.Add(_("28S ribosomal RNA"));
        m_NameStrings.Add(_("5.8S ribosomal RNA"));
        m_NameStrings.Add(_("large subunit ribosomal RNA"));
        m_NameStrings.Add(_("small subunit ribosomal RNA"));
        m_rRNAname = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_NameStrings, wxCB_DROPDOWN );
        m_RNANameSizer->Add(m_rRNAname, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        need_layout = true;
    }
    return need_layout;
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE1
 */

void CAlignRNAAddPanel::OnSelectRNAType( wxCommandEvent& event )
{
    string rna_type = ToStdString(m_RNAType->GetStringSelection());
    bool need_layout = false;
    if (NStr::Equal(rna_type, "ncRNA")) {
        m_ncRNAClass->Enable(true);
    } else {
        m_ncRNAClass->Enable(false);
    }
    bool buttons_shown = m_ButtonsSizer->IsShown((size_t)0);
    if (NStr::Equal(rna_type, "miscRNA")) {
        if (!buttons_shown) {
            m_ButtonsSizer->ShowItems(true);
            need_layout = true;
        }
    } else {
        if (buttons_shown) {
            m_ButtonsSizer->ShowItems(false);
            need_layout = true;
        }
    }
    // create RNA Name control
    if (NStr::Equal(rna_type, "tRNA")) {
        if (m_tRNAname == NULL) {
            x_RemoveRNANames();
            wxArrayString m_tRNAStrings;
            m_tRNAStrings.Add(_("A Alanine"));
            m_tRNAStrings.Add(_("B Asp or Asn"));
            m_tRNAStrings.Add(_("C Cysteine"));
            m_tRNAStrings.Add(_("D Aspartic Acid"));
            m_tRNAStrings.Add(_("E Glutamic Acid"));
            m_tRNAStrings.Add(_("F Phenylalanine"));
            m_tRNAStrings.Add(_("G Glycine"));
            m_tRNAStrings.Add(_("H Histidine"));
            m_tRNAStrings.Add(_("I Isoleucine"));
            m_tRNAStrings.Add(_("J Leu or Ile"));
            m_tRNAStrings.Add(_("K Lysine"));
            m_tRNAStrings.Add(_("L Leucine"));
            m_tRNAStrings.Add(_("M Methionine"));
            m_tRNAStrings.Add(_("N Asparagine"));
            m_tRNAStrings.Add(_("O Pyrrolysine"));
            m_tRNAStrings.Add(_("P Proline"));
            m_tRNAStrings.Add(_("Q Glutamine"));
            m_tRNAStrings.Add(_("R Arginine"));
            m_tRNAStrings.Add(_("S Serine"));
            m_tRNAStrings.Add(_("T Threonine"));
            m_tRNAStrings.Add(_("U Selenocysteine"));
            m_tRNAStrings.Add(_("V Valine"));
            m_tRNAStrings.Add(_("W Tryptophan"));
            m_tRNAStrings.Add(_("X Undetermined or atypical"));
            m_tRNAStrings.Add(_("Y Tyrosine"));
            m_tRNAStrings.Add(_("Z Glu or Gln"));
            m_tRNAname = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_tRNAStrings, 0 );
            m_RNANameSizer->Add(m_tRNAname, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
            need_layout = true;
        }
        need_layout |= x_AddGeneFields();
    } else if (NStr::Equal(rna_type, "rRNA")) {
        need_layout |= x_RemoveGeneFields();
        need_layout |= x_AddrRNAName();

    } else {
        if (NStr::Equal(rna_type, "ncRNA") || NStr::Equal(rna_type, "mRNA")) {
            need_layout |= x_AddGeneFields();
        } else {
            need_layout |= x_RemoveGeneFields();
        }

        if (m_RNAname == NULL) {
            x_RemoveRNANames();
            m_RNAname = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), 0 );
            m_RNANameSizer->Add(m_RNAname, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
            need_layout = true;
        }
    }
    if (need_layout) {
        Fit();        
        Layout();
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
 */

void CAlignRNAAddPanel::OnAdd18SToComment( wxCommandEvent& event )
{
    string comment = ToStdString(m_Comment->GetValue());
    if (!NStr::IsBlank(comment)) {
        comment += "; ";
    }
    comment += "contains 18S ribosomal RNA, internal transcribed spacer 1, 5.8S ribosomal RNA, internal transcribed spacer 2, and 28S ribosomal RNA";
    m_Comment->SetValue(ToWxString(comment));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
 */

void CAlignRNAAddPanel::OnAdd16StoComment( wxCommandEvent& event )
{
    string comment = ToStdString(m_Comment->GetValue());
    if (!NStr::IsBlank(comment)) {
        comment += "; ";
    }
    comment += "contains 16S ribosomal RNA, 16S-23S ribosomal RNA intergenic spacer, and 23S ribosomal RNA";
    m_Comment->SetValue(ToWxString(comment));
}


static void s_SettRNAProduct(string aa, CRef<CRNA_ref> rna)
{
    rna->SetExt().SetTRNA();
    if (NStr::IsBlank(aa)) {
        return;
    }
    aa = NStr::ToUpper(aa);
    char a = aa.c_str()[0];
    rna->SetExt().SetTRNA().SetAa().SetIupacaa(a);
}


CRNA_ref::TType CAlignRNAAddPanel::x_GetRnaType()
{
    CRNA_ref::TType rna_type = CRNA_ref::eType_miscRNA;
    switch (m_RNAType->GetSelection()) {
        case 0:
            rna_type = CRNA_ref::eType_premsg;
            break;
        case 1:
            rna_type = CRNA_ref::eType_mRNA;
            break;
        case 2:
            rna_type = CRNA_ref::eType_tRNA;
            break;
        case 3:
            rna_type = CRNA_ref::eType_rRNA;
            break;
        case 4:
            rna_type = CRNA_ref::eType_ncRNA;
            break;
        case 5:
            rna_type = CRNA_ref::eType_tmRNA;
            break;
        case 6:
            rna_type = CRNA_ref::eType_miscRNA;
            break;
        case 7:
            rna_type = CRNA_ref::eType_other;
            break;
    }
    return rna_type;
}


string CAlignRNAAddPanel::x_GetRnaName()
{
    string rna_name = "";
    if (m_RNAname != NULL) {
        rna_name = ToStdString(m_RNAname->GetValue());
    } else if (m_rRNAname != NULL) {
        rna_name = ToStdString(m_rRNAname->GetValue());
    }
    rna_name = GetAdjustedRnaName(rna_name);
    return rna_name;
}


CRef<CCmdComposite> CAlignRNAAddPanel::GetCommand()
{   
    CRNA_ref::TType rna_type = x_GetRnaType();
    string rna_name = x_GetRnaName();

    if (NStr::IsBlank(rna_name) && rna_type == CRNA_ref::eType_rRNA) {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }
 
    m_Location->TransferDataFromWindow();
    CRef<objects::CSeq_loc> loc = m_Location->GetSeq_loc();
    CAlignmentAssistant *parent = dynamic_cast<CAlignmentAssistant*>(GetParent());
    vector<pair<CRef<CSeq_loc>, CBioseq_Handle> > locations;
    parent->TranslateLocations(loc, locations);

    CRef<CCmdComposite> cmd (new CCmdComposite("Align Add RNA"));   
    for (size_t i = 0; i < locations.size(); ++i)
    {
        CRef<CSeq_loc> seq_loc = locations[i].first;
        CBioseq_Handle bsh = locations[i].second;
        AddOneCommand(bsh,seq_loc,cmd);
    }

  
    return cmd;
}

void CAlignRNAAddPanel::AddOneCommand(const CBioseq_Handle& bsh, CRef<CSeq_loc> seq_loc, CRef<CCmdComposite> cmd)
{
    if (!seq_loc || !bsh || (seq_loc->IsMix() && !seq_loc->GetMix().IsSet()))
        return;

    CRNA_ref::TType rna_type = x_GetRnaType();
    string rna_name = x_GetRnaName();
    string comment = ToStdString(m_Comment->GetValue());
    string ncrna_class = ToStdString (m_ncRNAClass->GetStringSelection());
    string gene_symbol = "";
    if (m_Locus) {
        gene_symbol = ToStdString(m_Locus->GetValue());
    }
    string gene_desc = "";
    if (m_GeneDesc) {
        gene_desc = ToStdString(m_GeneDesc->GetValue());
    }
    CRef<CRNA_ref> rna_ref(new CRNA_ref());
    rna_ref->SetType(rna_type);
    // set product
    switch (rna_type) {
        case CRNA_ref::eType_ncRNA:
        case CRNA_ref::eType_tmRNA:
        case CRNA_ref::eType_miscRNA:
            rna_ref->SetExt().SetGen();
            if (!NStr::IsBlank(rna_name)) {
                rna_ref->SetExt().SetGen().SetProduct(rna_name);
            }
            if (rna_type == CRNA_ref::eType_ncRNA && !NStr::IsBlank(ncrna_class)) {
                rna_ref->SetExt().SetGen().SetClass(ncrna_class);
            }
            break;
        case CRNA_ref::eType_tRNA:
            if (m_tRNAname != NULL) {
                s_SettRNAProduct(ToStdString(m_tRNAname->GetStringSelection()), rna_ref);
            }
            break;
        default:
            if (!NStr::IsBlank(rna_name)) {
                rna_ref->SetExt().SetName(rna_name);
            }
            break;
    }

    objects::CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CRef<objects::CSeq_feat> rna(new objects::CSeq_feat());
    rna->SetData().SetRna().Assign(*rna_ref);
    
    if (!NStr::IsBlank(comment)) {
        rna->SetComment(comment);
    }

    rna->SetLocation().Assign(*seq_loc);
    if (rna->GetLocation().IsPartialStart(objects::eExtreme_Biological)
        || rna->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
        rna->SetPartial(true);
    }
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *rna)));

    if (!NStr::IsBlank(gene_symbol) || !NStr::IsBlank(gene_desc)) {
        // create gene feature
        CRef<objects::CSeq_feat> new_gene(new objects::CSeq_feat());
        new_gene->SetData().SetGene().SetLocus(gene_symbol);
        new_gene->SetData().SetGene().SetDesc(gene_desc);
        new_gene->SetLocation().Assign(*seq_loc);
        if (new_gene->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || new_gene->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
            new_gene->SetPartial(true);
        }
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_gene)));
    }
}




/*!
 * CAlignOtherAddPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAlignOtherAddPanel, wxDialog )


/*!
 * CAlignOtherAddPanel event table definition
 */

BEGIN_EVENT_TABLE( CAlignOtherAddPanel, wxDialog )

////@begin CAlignOtherAddPanel event table entries
    EVT_LISTBOX( ID_CALIGN_OTHER_ADDPANEL_FEATURE_TYPE_LISTBOX, CAlignOtherAddPanel::OnFeatureTypeListboxSelected )

////@end CAlignOtherAddPanel event table entries

END_EVENT_TABLE()


/*!
 * CAlignOtherAddPanel constructors
 */

CAlignOtherAddPanel::CAlignOtherAddPanel()
{
    Init();
}

CAlignOtherAddPanel::CAlignOtherAddPanel( wxWindow* parent, CRef<CSeq_loc> align_loc, CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Loc(align_loc), m_TopSeqEntry(seh), m_scope(&seh.GetScope())
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAlignOtherAddPanel creator
 */

bool CAlignOtherAddPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignOtherAddPanel creation
    wxDialog::Create( parent, id, SYMBOL_CALIGN_OTHER_ADDPANEL_TITLE, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlignOtherAddPanel creation
    return true;
}


/*!
 * CAlignOtherAddPanel destructor
 */

CAlignOtherAddPanel::~CAlignOtherAddPanel()
{
////@begin CAlignOtherAddPanel destruction
////@end CAlignOtherAddPanel destruction
}


/*!
 * Member initialisation
 */

void CAlignOtherAddPanel::Init()
{
////@begin CAlignOtherAddPanel member initialisation
    m_FeatureType = NULL;
    m_GBQualSizer = NULL;
    m_Locus = NULL;
    m_GeneDesc = NULL;
    m_Comment = NULL;
    m_Location = NULL;
////@end CAlignOtherAddPanel member initialisation
    m_GBQualPanel = NULL;
    m_QualFeat.Reset(new CSeq_feat());
    m_QualFeat->SetData().SetImp().SetKey("misc_feature");
}


/*!
 * Control creation for CAlignOtherAddPanel
 */

void CAlignOtherAddPanel::CreateControls()
{    
////@begin CAlignOtherAddPanel content construction
    CAlignOtherAddPanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Feature Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_FeatureTypeStrings;
    m_FeatureType = new wxListBox( itemCBulkCmdPanel1, ID_CALIGN_OTHER_ADDPANEL_FEATURE_TYPE_LISTBOX, wxDefaultPosition, wxDefaultSize, m_FeatureTypeStrings, wxLB_SINGLE );
    itemBoxSizer4->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_GBQualSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(m_GBQualSizer, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Gene Symbol"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Locus = new wxTextCtrl( itemCBulkCmdPanel1, ID_CALIGN_OTHER_ADDPANEL_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(m_Locus, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Gene Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GeneDesc = new wxTextCtrl( itemCBulkCmdPanel1, ID_CALIGN_OTHER_ADDPANEL_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(m_GeneDesc, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemCBulkCmdPanel1, ID_CALIGN_OTHER_ADDPANEL_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(300, 70), wxTE_MULTILINE );
    itemFlexGridSizer8->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);


    m_Location = new CLocationPanel(itemCBulkCmdPanel1, *m_Loc, m_scope, true, false, false, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_Location, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 5);
    m_Location->TransferDataToWindow();

    itemBoxSizer2->Add(500, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CAlignOtherAddPanel content construction

    x_ListFeatures();

    m_GBQualPanel = new CGBQualPanel (this, *m_QualFeat);
    m_GBQualSizer->Add(m_GBQualPanel, 1, wxGROW|wxALL, 0);
    x_ChangeFeatureType("Gene");

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton13 = new wxButton( itemCBulkCmdPanel1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton( itemCBulkCmdPanel1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}

static bool s_IsRNAFeature(int feat_subtype)
{
    if (feat_subtype == CSeqFeatData::eSubtype_preRNA 
        || feat_subtype == CSeqFeatData::eSubtype_mRNA
        || feat_subtype == CSeqFeatData::eSubtype_tRNA
        || feat_subtype == CSeqFeatData::eSubtype_rRNA
        || feat_subtype == CSeqFeatData::eSubtype_otherRNA
        || feat_subtype == CSeqFeatData::eSubtype_misc_RNA
        || feat_subtype == CSeqFeatData::eSubtype_precursor_RNA
        || feat_subtype == CSeqFeatData::eSubtype_ncRNA
        || feat_subtype == CSeqFeatData::eSubtype_tmRNA) {
        return true;
    }
    return false;
}

void CAlignOtherAddPanel::x_ListFeatures()
{
    vector<string> listed_feat;
    set<string> existing;
    map<pair<int,int>,string> types;
    
    listed_feat.push_back("Gene");
    listed_feat.push_back("misc_feature");
    existing.insert(listed_feat[0]);
    existing.insert(listed_feat[1]);
    
    vector<const CFeatListItem *> featlist = GetSortedFeatList(m_TopSeqEntry);
    ITERATE(vector<const CFeatListItem *>, feat_it, featlist) {
        const CFeatListItem& item = **feat_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();
        CSeqFeatData::ESubtype feat_subtype = (CSeqFeatData::ESubtype)item.GetSubtype();
        if (feat_type == CSeqFeatData::e_Imp
            && !CFeatureTypePanel::s_IsRarelyUsedOrDiscouragedFeatureType(feat_subtype)
            && !CSeqFeatData::IsRegulatory(feat_subtype)) {
            types[pair<int,int>(feat_type,feat_subtype)] = desc;
            if (existing.find(desc) == existing.end()) {
                existing.insert(desc);
                listed_feat.push_back(desc);
            }
        }
    }

    m_FeatureType->Clear();
    for (size_t i = 0; i < listed_feat.size(); ++i) {
        m_FeatureType->Append(ToWxString(listed_feat[i]));
    }

    if (find(listed_feat.begin(), listed_feat.end(), m_DefaultKey) != listed_feat.end()) {
        m_FeatureType->SetStringSelection(ToWxString(m_DefaultKey));
        m_QualFeat->SetData().SetImp().SetKey(m_DefaultKey);
    }

}

/*!
 * Should we show tooltips?
 */

bool CAlignOtherAddPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAlignOtherAddPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAlignOtherAddPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAlignOtherAddPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAlignOtherAddPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlignOtherAddPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlignOtherAddPanel icon retrieval
}

CRef<CCmdComposite> CAlignOtherAddPanel::GetCommand()
{
    string key = ToStdString(m_FeatureType->GetStringSelection());
   
    if (NStr::IsBlank(key)) {
        CRef<CCmdComposite> empty;
        return empty;
    }

    m_Location->TransferDataFromWindow();
    CRef<objects::CSeq_loc> loc = m_Location->GetSeq_loc();
    CAlignmentAssistant *parent = dynamic_cast<CAlignmentAssistant*>(GetParent());
    vector<pair<CRef<CSeq_loc>, CBioseq_Handle> > locations;
    parent->TranslateLocations(loc, locations);
    
    CRef<CCmdComposite> cmd (new CCmdComposite("Align Add Other Feature"));   
    for (size_t i = 0; i < locations.size(); ++i)
    {
        CRef<CSeq_loc> seq_loc = locations[i].first;
        CBioseq_Handle bsh = locations[i].second;
        AddOneCommand(bsh,seq_loc,cmd);
    }

    
    return cmd;
}

void CAlignOtherAddPanel::AddOneCommand(const CBioseq_Handle& bsh, CRef<CSeq_loc> seq_loc, CRef<CCmdComposite> cmd)
{
    if (!seq_loc || !bsh || (seq_loc->IsMix() && !seq_loc->GetMix().IsSet()))
        return;

    string key = ToStdString(m_FeatureType->GetStringSelection());
    string comment = ToStdString(m_Comment->GetValue());
    string gene_symbol = ToStdString(m_Locus->GetValue());
    string gene_desc = ToStdString(m_GeneDesc->GetValue());
    m_GBQualPanel->TransferDataFromWindow();
    m_GBQualPanel->PopulateGBQuals(*m_QualFeat);

    objects::CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();

    if (!NStr::EqualNocase(key, "Gene")) {
        CRef<objects::CSeq_feat> feat(new objects::CSeq_feat());
        feat->SetData().SetImp().SetKey(key);
        if (!NStr::IsBlank(comment)) {
            feat->SetComment(comment);
        }
        feat->SetLocation().Assign(*seq_loc);
        if (feat->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || feat->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
            feat->SetPartial(true);
        }
        x_AddQuals(*feat);
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *feat)));
    }
    if (!NStr::IsBlank(gene_symbol) || !NStr::IsBlank(gene_desc) || NStr::EqualNocase(key, "Gene")) {
        // create gene feature
        CRef<objects::CSeq_feat> new_gene(new objects::CSeq_feat());
        new_gene->SetData().SetGene().SetLocus(gene_symbol);
        if (!NStr::IsBlank(gene_desc)) {
            new_gene->SetData().SetGene().SetDesc(gene_desc);
        }
        if (NStr::EqualNocase(key, "Gene") && !NStr::IsBlank(comment)) {
            new_gene->SetComment(comment);
            }
        new_gene->SetLocation().Assign(*seq_loc);
        if (new_gene->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || new_gene->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
            new_gene->SetPartial(true);
        }
        x_AddGeneQuals(*new_gene);
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_gene)));
    }
}


void CAlignOtherAddPanel::x_ChangeFeatureType(const string& key)
{
    m_GBQualPanel->TransferDataFromWindow();
    m_GBQualPanel->PopulateGBQuals(*m_QualFeat);
    if (NStr::EqualNocase(key, "Gene")) {
        m_QualFeat->SetData().SetGene();
        m_QualFeat->ResetQual();
    } else {
        m_QualFeat->SetData().SetImp().SetKey(key);
    }
    while (m_GBQualSizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_GBQualSizer->GetItem(pos)->DeleteWindows();
        m_GBQualSizer->Remove(pos);
    }
    m_GBQualPanel = new CGBQualPanel (this, *m_QualFeat);
    m_GBQualSizer->Add(m_GBQualPanel, 1, wxGROW|wxALL, 0);
    m_GBQualPanel->TransferDataToWindow();
//    if (NStr::EqualNocase(key, "Gene")) {
//        m_GBQualPanel->Show(false);
//    }
    Layout();
}


/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_CALIGN_OTHER_ADDPANEL_FEATURE_TYPE_LISTBOX
 */

void CAlignOtherAddPanel::OnFeatureTypeListboxSelected( wxCommandEvent& event )
{
    string key = ToStdString(m_FeatureType->GetStringSelection());
    x_ChangeFeatureType(key);
}


void CAlignOtherAddPanel::x_AddQuals(objects::CSeq_feat& feat)
{
    if (m_QualFeat && m_QualFeat->IsSetQual()) {
        ITERATE(CSeq_feat::TQual, it, m_QualFeat->GetQual()) {
            CRef<CGb_qual> q(new CGb_qual());
            q->Assign(**it);
            feat.SetQual().push_back(q);

            if ( NStr::EqualNocase(q->GetQual(), "mobile_element_type") && feat.IsSetData() && feat.GetData().IsImp()
                       && feat.GetData().GetImp().IsSetKey() && feat.GetData().GetImp().GetKey() == "repeat_region" )
                {
                    feat.SetData().SetImp().SetKey("mobile_element");
                }
        }
    }
}

void CAlignOtherAddPanel::x_AddGeneQuals(objects::CSeq_feat& gene)
{
    if (m_QualFeat && m_QualFeat->IsSetQual()) 
    {
        ITERATE(CSeq_feat::TQual, it, m_QualFeat->GetQual()) 
        {
            string val = (*it)->GetVal();
            string qual = (*it)->GetQual();
            if (qual == "locus")
                gene.SetData().SetGene().SetLocus(val);
            if (qual == "locus_tag")
                gene.SetData().SetGene().SetLocus_tag(val);
            if (qual == "allele")
                gene.SetData().SetGene().SetAllele(val);
            if (qual == "map")
                gene.SetData().SetGene().SetMaploc(val);
            if (qual == "gene_synonym")
                gene.SetData().SetGene().SetSyn().push_back(val);
        }
    }
}
END_NCBI_SCOPE
