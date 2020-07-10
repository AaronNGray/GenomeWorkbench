  /*  $Id: edit_sequence.cpp 44613 2020-02-04 19:08:11Z filippov $
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

#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Code_break.hpp>
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
#include <wx/display.h>
#include <wx/hyperlink.h>

#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/widgets/edit/edit_sequence.hpp>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CEditSequence, wxFrame )


/*!
 * CEditSequence event table definition
 */


BEGIN_EVENT_TABLE( CEditSequence, wxFrame )

EVT_BUTTON( ID_CLOSE_EDSEQ_BUTTON, CEditSequence::OnClose)
EVT_BUTTON( ID_CANCEL_EDSEQ_BUTTON, CEditSequence::OnCancel)
EVT_BUTTON( ID_ACCEPT_EDSEQ_BUTTON, CEditSequence::OnAccept)
EVT_BUTTON( ID_GOTO_EDSEQ_BUTTON, CEditSequence::OnGoTo)
EVT_BUTTON( ID_RANGE_EDSEQ_BUTTON, CEditSequence::OnRange)
EVT_BUTTON( ID_RETRANSLATE_EDSEQ_BUTTON, CEditSequence::OnRetranslate)

EVT_MENU( ID_SHOW_TRANSLATE_EDSEQ_MENU, CEditSequence::OnTranslate)
EVT_UPDATE_UI( ID_SHOW_TRANSLATE_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_TRANSLATE1_EDSEQ_MENU, CEditSequence::OnTranslate1)
EVT_UPDATE_UI( ID_SHOW_TRANSLATE1_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_TRANSLATE2_EDSEQ_MENU, CEditSequence::OnTranslate2)
EVT_UPDATE_UI( ID_SHOW_TRANSLATE2_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_TRANSLATE_ALL_EDSEQ_MENU, CEditSequence::OnTranslateAll)
EVT_UPDATE_UI( ID_SHOW_TRANSLATE_ALL_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_REVTRANSLATE_EDSEQ_MENU, CEditSequence::OnRevTranslate)
EVT_UPDATE_UI( ID_SHOW_REVTRANSLATE_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_REVTRANSLATE1_EDSEQ_MENU, CEditSequence::OnRevTranslate1)
EVT_UPDATE_UI( ID_SHOW_REVTRANSLATE1_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_REVTRANSLATE2_EDSEQ_MENU, CEditSequence::OnRevTranslate2)
EVT_UPDATE_UI( ID_SHOW_REVTRANSLATE2_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_REVTRANSLATE_ALL_EDSEQ_MENU, CEditSequence::OnRevTranslateAll)
EVT_UPDATE_UI( ID_SHOW_REVTRANSLATE_ALL_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_TRANSLATE_EVERYTHING_EDSEQ_MENU, CEditSequence::OnTranslateEverything)
EVT_UPDATE_UI( ID_SHOW_TRANSLATE_EVERYTHING_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_TRANSLATE_NONE_EDSEQ_MENU, CEditSequence::OnTranslateNone)
EVT_UPDATE_UI( ID_SHOW_TRANSLATE_NONE_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_ONTHEFLY_EDSEQ_MENU, CEditSequence::OnOnTheFly)
EVT_UPDATE_UI( ID_SHOW_ONTHEFLY_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_MISMATCH_EDSEQ_MENU, CEditSequence::OnMismatch)
EVT_UPDATE_UI( ID_SHOW_MISMATCH_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_REVCOMP_EDSEQ_MENU, CEditSequence::OnComplement)
EVT_UPDATE_UI( ID_SHOW_REVCOMP_EDSEQ_MENU, CEditSequence::IsNucleotide )

EVT_MENU( ID_SHOW_FEATURES_EDSEQ_MENU, CEditSequence::OnFeatures)

EVT_MENU( ID_EDIT_COPY_EDSEQ_MENU, CEditSequence::OnCopy )
EVT_UPDATE_UI( ID_EDIT_COPY_EDSEQ_MENU, CEditSequence::IsSelection )
EVT_MENU( ID_EDIT_CUT_EDSEQ_MENU, CEditSequence::OnCut )
EVT_UPDATE_UI( ID_EDIT_CUT_EDSEQ_MENU, CEditSequence::IsSelection )
EVT_MENU( ID_EDIT_PASTE_EDSEQ_MENU, CEditSequence::OnPaste )
EVT_UPDATE_UI( ID_EDIT_PASTE_EDSEQ_MENU, CEditSequence::IsClipboard )

EVT_MENU( ID_EDIT_FIND_EDSEQ_MENU, CEditSequence::OnFind )

EVT_MENU (eCmdCreateGene, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateLTR, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateProt, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateSecondaryStructure, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateRegion, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatePubFeatureLabeled, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateStemLoop, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateSTS, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateCodingRegion, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateCodingRegion_ext, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateRepeatRegion, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateVariation, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateOperon, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMobileElement, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateCentromere, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateTelomere, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatePreRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateExon, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateIntron, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreate3UTR, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreate5UTR, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatetRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreaterRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatencRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatetmRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatemiscRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatepreRNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateCommentDescriptor, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateBiosourceFeat, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatePolyASite, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatePrimerBind, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateProteinBind, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateRepOrigin, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateOriT, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateCRegion, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateDLoop, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateDSegment, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateiDNA, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateJSegment, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMiscBinding, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMiscDifference, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMiscFeature, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMiscRecomb, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMiscStructure, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateModifiedBase, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateNRegion, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreatePrimTranscript, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateRegulatory, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateRegulatory_promoter, CEditSequence::CreateRegulatory)
EVT_MENU (eCmdCreateRegulatory_enhancer, CEditSequence::CreateRegulatory)
EVT_MENU (eCmdCreateRegulatory_ribosome_binding_site, CEditSequence::CreateRegulatory)
EVT_MENU (eCmdCreateRegulatory_riboswitch, CEditSequence::CreateRegulatory)
EVT_MENU (eCmdCreateRegulatory_terminator, CEditSequence::CreateRegulatory)
EVT_MENU (eCmdCreateSRegion, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateUnsure, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateVRegion, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateVSegment, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateProprotein, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateMatPeptide, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateSigPeptide, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateTransitPeptide, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateBond, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateSite, CEditSequence::CreateFeature)
EVT_MENU (eCmdCreateAssemblyGap, CEditSequence::CreateFeature)

EVT_UPDATE_UI_RANGE(eCmdCreateBiosourceFeat, eCmdCreatePubFeatureLabeled, CEditSequence::IsSelectionAndClean )

EVT_TIMER(READONLY_TIMER_ID, CEditSequence::OnReadOnlyTimer)

END_EVENT_TABLE()


static
WX_DEFINE_MENU(kAnnotateMenu)
        WX_SUBMENU_EXT("gene/mRNA/CDS")
            WX_MENU_ITEM_EXT(eCmdCreateGene)
            WX_MENU_ITEM_EXT(eCmdCreateMRNA)
            WX_MENU_ITEM_EXT(eCmdCreateCodingRegion_ext)
            WX_MENU_ITEM_EXT(eCmdCreateOperon)
            WX_MENU_ITEM_EXT(eCmdCreateIntron)
            WX_MENU_ITEM_EXT(eCmdCreateExon)
            WX_MENU_ITEM_EXT(eCmdCreate5UTR)
            WX_MENU_ITEM_EXT(eCmdCreate3UTR)
        WX_END_SUBMENU()
        WX_SUBMENU_EXT("Structural RNAs")
            WX_MENU_ITEM_EXT(eCmdCreaterRNA)
            WX_MENU_ITEM_EXT(eCmdCreatetRNA)
            WX_MENU_ITEM_EXT(eCmdCreatencRNA)
            WX_MENU_ITEM_EXT(eCmdCreatepreRNA)
            WX_MENU_ITEM_EXT(eCmdCreatetmRNA)
            WX_MENU_ITEM_EXT(eCmdCreatemiscRNA)
        WX_END_SUBMENU()
        WX_SUBMENU_EXT("Regulatory")
            WX_MENU_ITEM_EXT(eCmdCreateRegulatory_promoter)
            WX_MENU_ITEM_EXT(eCmdCreateRegulatory_enhancer)
            WX_MENU_ITEM_EXT(eCmdCreateRegulatory_ribosome_binding_site)
            WX_MENU_ITEM_EXT(eCmdCreateRegulatory_riboswitch)
            WX_MENU_ITEM_EXT(eCmdCreateRegulatory_terminator)
            WX_MENU_ITEM_EXT(eCmdCreateRegulatory)
        WX_END_SUBMENU()
        WX_SUBMENU_EXT("Protein Features")
            WX_MENU_ITEM_EXT(eCmdCreateMatPeptide)
            WX_MENU_ITEM_EXT(eCmdCreateSigPeptide)
            WX_MENU_ITEM_EXT(eCmdCreateProprotein)
            WX_MENU_ITEM_EXT(eCmdCreateTransitPeptide)
        WX_END_SUBMENU()
        WX_SUBMENU_EXT("Other Features")
            WX_MENU_ITEM_EXT(eCmdCreateCentromere)
            WX_MENU_ITEM_EXT(eCmdCreateDLoop)
            WX_MENU_ITEM_EXT(eCmdCreateMiscBinding)
            WX_MENU_ITEM_EXT(eCmdCreateMiscDifference)
            WX_MENU_ITEM_EXT(eCmdCreateMiscFeature)
            WX_MENU_ITEM_EXT(eCmdCreateMiscRecomb)
            WX_MENU_ITEM_EXT(eCmdCreateMobileElement)
            WX_MENU_ITEM_EXT(eCmdCreateOriT)
            WX_MENU_ITEM_EXT(eCmdCreateProteinBind)
            WX_MENU_ITEM_EXT(eCmdCreateRepOrigin)
            WX_MENU_ITEM_EXT(eCmdCreateRepeatRegion)
            WX_MENU_ITEM_EXT(eCmdCreateSite)
            WX_MENU_ITEM_EXT(eCmdCreateTelomere)
            WX_MENU_ITEM_EXT(eCmdCreateVariation)
        WX_END_SUBMENU()

        WX_SUBMENU_INT("Genes and Named Regions")
            WX_MENU_ITEM_INT(eCmdCreateGene)
            WX_MENU_ITEM_INT(eCmdCreateProt)
            WX_MENU_ITEM_INT(eCmdCreateRepeatRegion)
            WX_MENU_ITEM_INT(eCmdCreateStemLoop)
            WX_MENU_ITEM_INT(eCmdCreateSTS)
            WX_MENU_ITEM_INT(eCmdCreateRegion)
            WX_MENU_ITEM_INT(eCmdCreateSecondaryStructure)
            WX_MENU_ITEM_INT(eCmdCreateProprotein)
            WX_MENU_ITEM_INT(eCmdCreateMatPeptide)
            WX_MENU_ITEM_INT(eCmdCreateSigPeptide)
            WX_MENU_ITEM_INT(eCmdCreateTransitPeptide)
            WX_MENU_ITEM_INT(eCmdCreateOperon)
            WX_MENU_ITEM_INT(eCmdCreateMobileElement)
            WX_MENU_ITEM_INT(eCmdCreateCentromere)
            WX_MENU_ITEM_INT(eCmdCreateTelomere)
            WX_MENU_ITEM_INT(eCmdCreateRegulatory)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Coding Regions and Transcripts")
              WX_MENU_ITEM_INT(eCmdCreateCodingRegion)
              WX_MENU_ITEM_INT(eCmdCreatePreRNA)
              WX_MENU_ITEM_INT(eCmdCreateMRNA)
              WX_MENU_ITEM_INT(eCmdCreateExon)
              WX_MENU_ITEM_INT(eCmdCreateIntron)
              WX_MENU_ITEM_INT(eCmdCreate3UTR)
              WX_MENU_ITEM_INT(eCmdCreate5UTR)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Structural RNAs")
              WX_MENU_ITEM_INT(eCmdCreatetRNA)
              WX_MENU_ITEM_INT(eCmdCreaterRNA)
              WX_MENU_ITEM_INT(eCmdCreateRNA)
              WX_MENU_ITEM_INT(eCmdCreatencRNA)
              WX_MENU_ITEM_INT(eCmdCreatetmRNA)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Bibliographic and Comments")
              WX_MENU_ITEM_INT(eCmdCreateCommentDescriptor)
              WX_MENU_ITEM_INT(eCmdCreateBiosourceFeat)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Sites and Bonds")
              WX_MENU_ITEM_INT(eCmdCreatePolyASite)
              WX_MENU_ITEM_INT(eCmdCreatePrimerBind)
              WX_MENU_ITEM_INT(eCmdCreateProteinBind)
              WX_MENU_ITEM_INT(eCmdCreateRepOrigin)
              WX_MENU_ITEM_INT(eCmdCreateBond)
              WX_MENU_ITEM_INT(eCmdCreateSite)
              WX_MENU_ITEM_INT(eCmdCreateOriT)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Remaining Features")
              WX_MENU_ITEM_INT(eCmdCreateCRegion)
              WX_MENU_ITEM_INT(eCmdCreateDLoop)
              WX_MENU_ITEM_INT(eCmdCreateDSegment)
              WX_MENU_ITEM_INT(eCmdCreateiDNA)
              WX_MENU_ITEM_INT(eCmdCreateJSegment)
              WX_MENU_ITEM_INT(eCmdCreateMiscBinding)
              WX_MENU_ITEM_INT(eCmdCreateMiscDifference)
              WX_MENU_ITEM_INT(eCmdCreateMiscFeature)
              WX_MENU_ITEM_INT(eCmdCreateMiscRecomb)
              WX_MENU_ITEM_INT(eCmdCreateMiscStructure)
              WX_MENU_ITEM_INT(eCmdCreateModifiedBase)
              WX_MENU_ITEM_INT(eCmdCreateNRegion)
              WX_MENU_ITEM_INT(eCmdCreatePrimTranscript)
              WX_MENU_ITEM_INT(eCmdCreateSRegion)
              WX_MENU_ITEM_INT(eCmdCreateUnsure)
              WX_MENU_ITEM_INT(eCmdCreateVRegion)
              WX_MENU_ITEM_INT(eCmdCreateVSegment)
              WX_MENU_ITEM_INT(eCmdCreateVariation)
        WX_END_SUBMENU()
WX_END_MENU()


/*!
 * CEditSequence constructors
 */

CEditSequence::CEditSequence()
{
    Init();
}


const char* kExclusiveEditDescr = "The Sequence editor requires exclusive access to a project";

CEditSequence::CEditSequence( wxWindow* parent, const CSeq_loc *loc, CScope &scope, ICommandProccessor* processor, int start,
               wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_loc(loc), m_scope(&scope), m_start(start), m_readonly_timer(this, READONLY_TIMER_ID)
{
    m_UndoManager = dynamic_cast<IUndoManager*>(processor);
    Init();
    Create(parent, id, caption, pos, size, style);
    if (m_is_nucleotide)
    {
        SetRegistryPath("Dialogs.Edit.EditNucleotideSequence");
    }
    LoadSettings();
    if (m_Panel)
        m_Panel->InitPanel();
    NEditingStats::ReportUsage(caption);
}


bool CEditSequence::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditSequence creation
//    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CEditSequence creation
    return true;
}


/*!
 * CEditSequence destructor
 */

CEditSequence::~CEditSequence()
{
////@begin CEditSequence destruction
////@end CEditSequence destruction

    if (m_UndoManager && m_seh && m_NumCmds > 0)
        m_UndoManager->ReleaseExclusiveEdit(this);
    SaveSettings();
}

static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";

void CEditSequence::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CEditSequence::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);
}


void CEditSequence::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        SetSize(wxSize(width,height));

    int pos_x = view.GetInt(kFramePosX, -1);
    int pos_y = view.GetInt(kFramePosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       SetPosition(wxPoint(pos_x,pos_y));
   }
}

/*!
 * Member initialisation
 */

void CEditSequence::Init()
{
    m_modified = false;
    m_Panel = NULL;
    m_Pos = NULL;
    m_GoTo_TextCtrl = NULL;
    m_RangeFrom = NULL;
    m_RangeTo = NULL;
    m_NumCmds = 0;
    m_commit_warning = NULL;
    m_readonly_warning = NULL;
}


/*!
 * Control creation for CEditSequence
 */

void CEditSequence::CreateControls()
{    
    wxMenuBar *menubar = new wxMenuBar();
    wxMenu *menu = new wxMenu();
    menubar->Append(menu, wxT("&Show"));

    wxMenu *translate_menu = new wxMenu();

    m_translate_item = new wxMenuItem(menu, ID_SHOW_TRANSLATE_EDSEQ_MENU, _("+1\tAlt+1"), _("Show translation at offset 0"), wxITEM_CHECK);
    translate_menu->Append(m_translate_item);

    m_translate_item1 = new wxMenuItem(menu, ID_SHOW_TRANSLATE1_EDSEQ_MENU, _("+2\tAlt+2"), _("Show translation at offset 1"), wxITEM_CHECK);
    translate_menu->Append(m_translate_item1);

    m_translate_item2 = new wxMenuItem(menu, ID_SHOW_TRANSLATE2_EDSEQ_MENU, _("+3\tAlt+3"), _("Show translation at offset 2"), wxITEM_CHECK);
    translate_menu->Append(m_translate_item2);

    m_translate_item_all = new wxMenuItem(menu, ID_SHOW_TRANSLATE_ALL_EDSEQ_MENU, _("All +\tAlt++"), _("Show translation at all + offsets"), wxITEM_CHECK);
    translate_menu->Append(m_translate_item_all);

    m_revtranslate_item = new wxMenuItem(menu, ID_SHOW_REVTRANSLATE_EDSEQ_MENU, _("-1\tCtrl+1"), _("Show translation at offset 0"), wxITEM_CHECK);
    translate_menu->Append(m_revtranslate_item);

    m_revtranslate_item1 = new wxMenuItem(menu, ID_SHOW_REVTRANSLATE1_EDSEQ_MENU, _("-2\tCtrl+2"), _("Show translation at offset -1"), wxITEM_CHECK);
    translate_menu->Append(m_revtranslate_item1);

    m_revtranslate_item2 = new wxMenuItem(menu, ID_SHOW_REVTRANSLATE2_EDSEQ_MENU, _("-3\tCtrl+3"), _("Show translation at offset -2"), wxITEM_CHECK);
    translate_menu->Append(m_revtranslate_item2);

    m_revtranslate_item_all = new wxMenuItem(menu, ID_SHOW_REVTRANSLATE_ALL_EDSEQ_MENU, _("All -\tCtrl+-"), _("Show translation at all - offsets"), wxITEM_CHECK);
    translate_menu->Append(m_revtranslate_item_all);

    m_translate_item_everything = new wxMenuItem(menu, ID_SHOW_TRANSLATE_EVERYTHING_EDSEQ_MENU, _("All \tAlt+a"), _("Show translation at all offsets"), wxITEM_CHECK);
    translate_menu->Append(m_translate_item_everything);

    m_translate_item_none = new wxMenuItem(menu, ID_SHOW_TRANSLATE_NONE_EDSEQ_MENU, _("None \tAlt+n"), _("Show no translation"), wxITEM_CHECK);
    translate_menu->Append(m_translate_item_none);

    menu->AppendSubMenu(translate_menu,_("Reading frames"));

    wxMenuItem *revcomp_item = new wxMenuItem(menu, ID_SHOW_REVCOMP_EDSEQ_MENU, _("Complement\tAlt+c"), _("Show complement"), wxITEM_CHECK);
    menu->Append(revcomp_item);

    wxMenuItem *features_item = new wxMenuItem(menu, ID_SHOW_FEATURES_EDSEQ_MENU, _("Features\tAlt+f"), _("Show features"), wxITEM_CHECK);
    menu->Append(features_item);

    wxMenuItem * onthefly_item = new wxMenuItem(menu, ID_SHOW_ONTHEFLY_EDSEQ_MENU, _("On-the-fly\tAlt+t"), _("Show on-the-fly translation"), wxITEM_CHECK);
    menu->Append(onthefly_item);

    wxMenuItem * mismatch_item = new wxMenuItem(menu, ID_SHOW_MISMATCH_EDSEQ_MENU, _("Mismatch\tAlt+m"), _("Show mismatch"), wxITEM_CHECK);
    menu->Append(mismatch_item);


    wxMenu *menu2 = new wxMenu();
    menubar->Append(menu2, wxT("&Edit"));

    wxMenuItem *cut_item = new wxMenuItem(menu2, ID_EDIT_CUT_EDSEQ_MENU, _("Cut\tCtrl+X"), _("Cut to clipboard"), wxITEM_NORMAL);
    menu2->Append(cut_item);
       
    wxMenuItem *copy_item = new wxMenuItem(menu2, ID_EDIT_COPY_EDSEQ_MENU, _("Copy\tCtrl+C"), _("Copy to clipboard"), wxITEM_NORMAL);
    menu2->Append(copy_item);

    wxMenuItem *paste_item = new wxMenuItem(menu2, ID_EDIT_PASTE_EDSEQ_MENU, _("Paste\tCtrl+V"), _("Paste from clipboard"), wxITEM_NORMAL);
    menu2->Append(paste_item);

    wxMenuItem *find_item = new wxMenuItem(menu2, ID_EDIT_FIND_EDSEQ_MENU, _("Find\tCtrl+F"), _("Find pattern"), wxITEM_NORMAL);
    menu2->Append(find_item);

    wxMenu* menu3 = CUICommandRegistry::GetInstance().CreateMenu(kAnnotateMenu);
    if (RunningInsideNCBI())
        menubar->Append(menu3, wxT("Annotate"));
    else
        menubar->Append(menu3, wxT("Features"));

    SetMenuBar(menubar);

    wxPanel *Dialog1 = new wxPanel(this, wxID_ANY);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    Dialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxButton* itemButton6 = new wxButton( Dialog1, ID_GOTO_EDSEQ_BUTTON, wxT("Go to:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GoTo_TextCtrl = new wxTextCtrl( Dialog1, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_GoTo_TextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( Dialog1, wxID_STATIC, wxT("Position:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Pos = new wxStaticText( Dialog1, wxID_STATIC, wxT(""), wxDefaultPosition, wxSize(Dialog1->ConvertDialogToPixels(wxSize(40, -1)).x, -1), wxNO_BORDER );
    itemBoxSizer3->Add(m_Pos, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    wxButton* itemButton7 = new wxButton( Dialog1, ID_RANGE_EDSEQ_BUTTON, wxT("Select:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RangeFrom = new wxTextCtrl( Dialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_RangeFrom, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);
    m_RangeFrom->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

    wxStaticText* rangeDash = new wxStaticText( Dialog1, wxID_STATIC, wxT("-"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(rangeDash, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    m_RangeTo = new wxTextCtrl( Dialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_RangeTo, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);    
    m_RangeTo->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

    GetSequence();
    string title = GetSeqTitle();
    SetTitle(wxString(title));
    string seq;    
    vector<int> seq_len;
    vector<bool> read_only;
    SSeqMapSelector selector;
    selector.SetFlags(CSeqMap::fDefaultFlags); 
    CBioseq_Handle bsh(m_seh.GetSeq());
    m_is_nucleotide = bsh.IsNucleotide();
    CSeqMap_CI seqmap_ci(bsh, selector);
    while (seqmap_ci)
    {
        TSeqPos len = seqmap_ci.GetLength();
        seq_len.push_back(len);
        if (seqmap_ci.IsValid() && seqmap_ci.GetType() == CSeqMap::eSeqData)
            read_only.push_back(false);
        else
            read_only.push_back(true);
        ++seqmap_ci;
    }
    objects::CSeqVector vec(bsh,CBioseq_Handle::eCoding_Iupac);
    vec.GetSeqData(0,bsh.GetBioseqLength(),seq);
    if (seq.empty())
        NCBI_THROW( CException, eUnknown, "No bioseq selected" );
    if (seq_len.empty())
    {
        seq_len.push_back(seq.size());
        read_only.push_back(true);
    }

    string allowed_char_set = "ATUCGRYSWKMBDHVN";
    if (!m_is_nucleotide)
        allowed_char_set = "ABCDEFGHIJKLMNOPQRSTUVWXYZ*";

    wxBoxSizer *BoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(BoxSizer, 1, wxGROW|wxALL, 5);    
    m_Panel = new CPaintSequence(Dialog1, seq, seq_len, m_FeatRanges, m_FeatTypes, m_FeatStrand, m_FeatFrames, m_GeneticCode, m_Feat5Partial, allowed_char_set, m_RealProt, read_only, m_ProtFeatRanges, m_start);
    BoxSizer->Add(m_Panel, 1, wxGROW|wxALL, 5);
    features_item->Check();
    m_Panel->EnableFeatures(true);
    if (m_is_nucleotide)
    {
        mismatch_item->Check();
        m_Panel->EnableMismatch(true);
    }
    m_Panel->SetFocus();

    m_commit_warning = new wxStaticText( Dialog1, wxID_STATIC, wxT("You need to commit the changes to the sequence before annotating or retranslating"), wxDefaultPosition, wxDefaultSize, 0 );
    m_commit_warning->SetForegroundColour(*wxRED);
    itemBoxSizer2->Add(m_commit_warning, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
    m_commit_warning->Hide();

    m_readonly_warning = new wxStaticText( Dialog1, wxID_STATIC, wxT("You need to convert to raw sequence before editing within the gap"), wxDefaultPosition, wxDefaultSize, 0 );
    m_readonly_warning->SetForegroundColour(*wxRED);
    itemBoxSizer2->Add(m_readonly_warning, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
    m_readonly_warning->Hide();

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CommitButton = new wxButton( Dialog1, ID_ACCEPT_EDSEQ_BUTTON, _("Commit"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_CommitButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_CommitButton->Disable();

    m_RetranslateButton = new wxButton( Dialog1, ID_RETRANSLATE_EDSEQ_BUTTON, _("Retranslate"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_RetranslateButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( Dialog1, ID_CLOSE_EDSEQ_BUTTON, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( Dialog1, ID_CANCEL_EDSEQ_BUTTON, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( Dialog1, wxID_HELP, _("Help"), wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual9/#edit-sequence"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    itemBoxSizer12->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
}

string CEditSequence::GetSeqTitle()
{
    string accession;
    string local;
    string label;
    for (CBioseq_Handle::TId::const_iterator it = m_seh.GetSeq().GetId().begin(); it != m_seh.GetSeq().GetId().end(); ++it)
    {
        const CSeq_id &id = *(it->GetSeqId());
        if (id.IsGenbank() && id.GetGenbank().IsSetAccession())
            accession = id.GetGenbank().GetAccession();
        if (id.IsLocal() && id.GetLocal().IsStr())
            local = id.GetLocal().GetStr();
    }
    if (!accession.empty())
        return accession;
    if (!local.empty())
        return local;

    m_seh.GetSeq().GetBioseqCore()->GetLabel(&label, CBioseq::eContent);
    return label;
}

void CEditSequence::EnableCommit(bool enable)
{
    m_CommitButton->Enable(enable);
    m_RetranslateButton->Enable(!enable);
    m_commit_warning->Show(enable);
}

void CEditSequence::ShowReadOnlyWarning()
{
    m_readonly_warning->Show();
    if (!m_readonly_timer.IsRunning())
        m_readonly_timer.StartOnce(2000);
}

void CEditSequence::OnReadOnlyTimer(wxTimerEvent& event)
{
    m_readonly_warning->Hide();
}

void CEditSequence::GetSequence()
{
    CRef<CObjectManager> object_manager = CObjectManager::GetInstance();
    CRef<CScope> scope_copy(new CScope(*object_manager));
  
    if (m_loc && m_loc->IsWhole()) 
    {
        m_seh = m_scope->GetBioseqHandle(*m_loc).GetSeq_entry_Handle();
        if (!m_seh.IsSeq())
        {
            NCBI_THROW( CException, eUnknown, "No bioseq selected" );
        }
        GetFeatures(*m_loc, m_seh.GetScope());
        CRef<CSeq_entry> copy(new CSeq_entry);
        copy->Assign(*m_seh.GetCompleteSeq_entry());
        m_edited_seh = scope_copy->AddTopLevelSeqEntry(*copy);
    }
    else
    {
        NCBI_THROW( CException, eUnknown, "No bioseq selected" );
    }
}

/*!
 * Should we show tooltips?
 */

bool CEditSequence::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditSequence::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CEditSequence::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

void CEditSequence::OnAccept( wxCommandEvent& event )
{
    if (!m_seh)
        return;
    CRef<CCmdComposite> composite(new CCmdComposite("Adjust Features Locations"));
    string seq = m_Panel->GetSeq();
    vector<int> &seq_len = m_Panel->GetSeqLen();
    vector<bool> &read_only = m_Panel->GetReadOnly();
    SSeqMapSelector selector;
    selector.SetFlags(CSeqMap::fDefaultFlags); 
    CBioseq_EditHandle besh(m_edited_seh.GetSeq());
    CSeqMap_I seqmap_i(besh, selector);
    int start = 0;
    int i = 0;
    bool modified = false;
    while (seqmap_i)
      {
          bool removed = false;
          if (!read_only[i])
          {
              string seq_in;
              if (m_is_nucleotide)
                  seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacna);
              else
                  seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacaa);
              string seq_out = seq.substr(start,seq_len[i]);
              if (seq_in != seq_out) 
              {
                  if (!seq_out.empty())
                  {
                      if (m_is_nucleotide)
                          seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacna, CSeq_data::e_Iupacna);
                      else
                          seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacaa, CSeq_data::e_Iupacaa);
                  }
                  else
                  {
                      seqmap_i = seqmap_i.Remove(); 
                      removed = true;
                  }
                  modified = true;
              }             
          }
          if (!removed)
          {
              ++seqmap_i;
          }
          start += seq_len[i];
          ++i;
      }
      vector<int>::iterator seq_len_i = seq_len.begin();
      vector<bool>::iterator read_only_i = read_only.begin();
      while (seq_len_i != seq_len.end() && read_only_i != read_only.end())
      {
          if (*seq_len_i == 0)
          {
              seq_len_i = seq_len.erase(seq_len_i);
              read_only_i = read_only.erase(read_only_i);
              continue;
          }
          ++seq_len_i;
          ++read_only_i;
      }
      if (modified)
      {
          CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(m_seh.GetSeq(), m_edited_seh.GetCompleteSeq_entry()->GetSeq().GetInst()));
          if (cmd) 
              composite->AddCommand(*cmd);
      }
      AdjustFeatureLocations(composite);
      x_ExecuteCommand(composite);
      m_Panel->SetClean(true);
      m_Panel->SetFocus();
}

void CEditSequence::OnClose( wxCommandEvent& event )
{ 
    if (!m_Panel->GetClean())
    {
        int answer = wxMessageBox (ToWxString("Uncommitted changes, proceed with closing?"), ToWxString("Warning"), wxYES_NO | wxICON_QUESTION);
        if (answer == wxYES)
            Close();
    }
    else
        Close();
}

void CEditSequence::OnCancel( wxCommandEvent& event )
{
    if (m_UndoManager && m_seh) {
        bool exclusive = (m_NumCmds > 0);
        while (m_UndoManager->CanUndo() && m_NumCmds > 0) {
            m_UndoManager->Undo(this);
            --m_NumCmds;
        }

        m_NumCmds = 0;
        if (exclusive)
            m_UndoManager->ReleaseExclusiveEdit(this);
    }

    Close();
}

void CEditSequence::ReportPos(int pos)
{
    m_Pos->SetLabel(ToWxString(NStr::IntToString(pos)));
}

void CEditSequence::ReportRange(int pos1, int pos2)
{
    if (pos1 > 0 && pos2 > 0)
    {
        m_RangeFrom->SetValue(ToWxString(NStr::IntToString(pos1)));
        m_RangeTo->SetValue(ToWxString(NStr::IntToString(pos2)));
    }
    else
    {
        m_RangeFrom->SetValue(wxEmptyString);
        m_RangeTo->SetValue(wxEmptyString);
    }
}


void CEditSequence::OnGoTo( wxCommandEvent& event )
{ 
    wxString val = m_GoTo_TextCtrl->GetValue();
    long pos;
    if(val.ToLong(&pos)) 
    {
        m_Panel->SetPos(pos);
        m_Panel->SetFocus();
    }
    else if (!val.IsEmpty())
    {
        m_Panel->Search(val.ToStdString());
        m_Panel->SetFocus();
    }
}

void CEditSequence::OnRange( wxCommandEvent& event )
{ 
    wxString val1 = m_RangeFrom->GetValue();
    wxString val2 = m_RangeTo->GetValue();
    long pos1, pos2;
    int seq_len = m_Panel->GetLength();
    if(val1.ToLong(&pos1) && val2.ToLong(&pos2) && pos1 > 0 && pos2 > 0 && pos2 > pos1 && pos1 <= seq_len && pos2 <= seq_len) 
    {
        m_Panel->SetRange(pos1, pos2);
    }   
    m_Panel->SetFocus();
}

void CEditSequence::OnFind( wxCommandEvent& event )
{
    CFindEditSequence *dlg = new CFindEditSequence(this, m_Panel, m_is_nucleotide);
    dlg->Show(true);      
}

void CEditSequence::OnTranslate( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableTranslation(true);
        m_translate_item_none->Check(false);
        m_Panel->UpdateData();
    }
    else
    {
        m_translate_item_all->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableTranslation(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnTranslate1( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableTranslation1(true);
        m_translate_item_none->Check(false);
        m_Panel->UpdateData();
    }
    else
    {
        m_translate_item_all->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableTranslation1(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnTranslate2( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableTranslation2(true);
        m_translate_item_none->Check(false);
        m_Panel->UpdateData();
    }
    else
    {
        m_translate_item_all->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableTranslation2(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnTranslateAll( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_translate_item->Check(true);
        m_translate_item1->Check(true);
        m_translate_item2->Check(true);
        m_translate_item_none->Check(false);
        m_Panel->EnableTranslation(true);
        m_Panel->EnableTranslation1(true);
        m_Panel->EnableTranslation2(true);
        m_Panel->UpdateData();
    }
    else
    {
        m_translate_item->Check(false);
        m_translate_item1->Check(false);
        m_translate_item2->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableTranslation(false);
        m_Panel->EnableTranslation1(false);
        m_Panel->EnableTranslation2(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnRevTranslate( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableRevTranslation(true);
        m_translate_item_none->Check(false);
        m_Panel->UpdateData();
    }
    else
    {
        m_revtranslate_item_all->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableRevTranslation(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnRevTranslate1( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableRevTranslation1(true);
        m_translate_item_none->Check(false);
        m_Panel->UpdateData();
    }
    else
    {
        m_revtranslate_item_all->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableRevTranslation1(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnRevTranslate2( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableRevTranslation2(true);
        m_translate_item_none->Check(false);
        m_Panel->UpdateData();
    }
    else
    {
        m_revtranslate_item_all->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableRevTranslation2(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnRevTranslateAll( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_revtranslate_item->Check(true);
        m_revtranslate_item1->Check(true);
        m_revtranslate_item2->Check(true);
        m_translate_item_none->Check(false);
        m_Panel->EnableRevTranslation(true);
        m_Panel->EnableRevTranslation1(true);
        m_Panel->EnableRevTranslation2(true);
        m_Panel->UpdateData();
    }
    else
    {
        m_revtranslate_item->Check(false);
        m_revtranslate_item1->Check(false);
        m_revtranslate_item2->Check(false);
        m_translate_item_everything->Check(false);
        m_Panel->EnableRevTranslation(false);
        m_Panel->EnableRevTranslation1(false);
        m_Panel->EnableRevTranslation2(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnTranslateEverything( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_translate_item->Check(true);
        m_translate_item1->Check(true);
        m_translate_item2->Check(true);
        m_Panel->EnableTranslation(true);
        m_Panel->EnableTranslation1(true);
        m_Panel->EnableTranslation2(true);

        m_revtranslate_item->Check(true);
        m_revtranslate_item1->Check(true);
        m_revtranslate_item2->Check(true);
        m_Panel->EnableRevTranslation(true);
        m_Panel->EnableRevTranslation1(true);
        m_Panel->EnableRevTranslation2(true);

        m_translate_item_none->Check(false);
        m_translate_item_all->Check(true);
        m_revtranslate_item_all->Check(true);
        m_Panel->UpdateData();
    }
    else
    {
        m_translate_item->Check(false);
        m_translate_item1->Check(false);
        m_translate_item2->Check(false);
        m_Panel->EnableTranslation(false);
        m_Panel->EnableTranslation1(false);
        m_Panel->EnableTranslation2(false);

        m_revtranslate_item->Check(false);
        m_revtranslate_item1->Check(false);
        m_revtranslate_item2->Check(false);
        m_Panel->EnableRevTranslation(false);
        m_Panel->EnableRevTranslation1(false);
        m_Panel->EnableRevTranslation2(false);

        m_translate_item_all->Check(false);
        m_revtranslate_item_all->Check(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}


void CEditSequence::OnTranslateNone( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {

        m_translate_item->Check(false);
        m_translate_item1->Check(false);
        m_translate_item2->Check(false);
        m_Panel->EnableTranslation(false);
        m_Panel->EnableTranslation1(false);
        m_Panel->EnableTranslation2(false);

        m_revtranslate_item->Check(false);
        m_revtranslate_item1->Check(false);
        m_revtranslate_item2->Check(false);
        m_Panel->EnableRevTranslation(false);
        m_Panel->EnableRevTranslation1(false);
        m_Panel->EnableRevTranslation2(false);
       
        m_translate_item_all->Check(false);
        m_revtranslate_item_all->Check(false);
        m_translate_item_everything->Check(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}


void CEditSequence::OnOnTheFly( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableOnTheFly(true);
        m_Panel->UpdateData();
    }
    else
    {
        m_Panel->EnableOnTheFly(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnComplement( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableComplement(true);
        m_Panel->UpdateData();
    }
    else
    {
        m_Panel->EnableComplement(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::GetFeatures(const CSeq_loc &loc, CScope &scope)
{
    m_FeatRanges.clear();
    m_FeatTypes.clear();
    m_FeatStrand.clear();
    m_RealProt.clear();
    m_FeatFrames.clear();
    m_GeneticCode.clear();
    m_Feat5Partial.clear();
    m_ProtFeatRanges.clear();

    TSeqPos seq_start = loc.GetStart(objects::eExtreme_Positional);
    SAnnotSelector sel;
    CFeat_CI feat_it(scope, loc, sel);
    const CSeq_id* seq_id = loc.GetId();
    
    while (feat_it) 
    {
        if (feat_it->GetOriginalFeature().IsSetData())
        {
            vector<pair<TSeqPos,TSeqPos> > vec;
            const CSeq_loc& feat_loc = feat_it->GetLocation();
            for(CSeq_loc_CI loc_it(feat_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)     
            {
                if (seq_id && !sequence::IsSameBioseq(*seq_id, loc_it.GetSeq_id(), &scope)) {
                    continue;
                }
                CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                TSeqPos feat_start = feat_range.GetFrom() - seq_start;
                TSeqPos feat_stop = feat_range.GetTo() - seq_start;
                vec.push_back(pair<TSeqPos, TSeqPos>(feat_start, feat_stop));
            }
            CBioseq_Handle::EVectorStrand strand = CBioseq_Handle::eStrand_Plus;
            if (feat_loc.GetStrand() == eNa_strand_minus)
                strand = CBioseq_Handle::eStrand_Minus;
            if (!vec.empty())
            {
                CSeqFeatData::ESubtype subtype = feat_it->GetOriginalFeature().GetData().GetSubtype();
                string label;
                string prot_seq;
                if (subtype == CSeqFeatData::eSubtype_cdregion && feat_it->IsSetProduct())
                {
                    CBioseq_Handle prot_bsh = feat_it->GetScope().GetBioseqHandle(feat_it->GetProduct());
                    if (prot_bsh && prot_bsh.IsProtein()) 
                    {
                        prot_bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, strand).GetSeqData(0,prot_bsh.GetBioseqLength(),prot_seq);
                        CFeat_CI prot_feat_it(prot_bsh, SAnnotSelector().IncludeFeatType(CSeqFeatData::e_Prot));
                        if (prot_feat_it)
                            prot_feat_it->GetData().GetProt().GetLabel(&label);
                        GetProteinFeatures(prot_bsh, m_FeatRanges.size(), seq_id, feat_it->GetOriginalFeature(), seq_start);
                    }
                }
                if (strand == objects::CBioseq_Handle::eStrand_Minus && !prot_seq.empty())
                {
                    reverse(prot_seq.begin(), prot_seq.end());
                }
                if (label.empty())
                {
                    feature::GetLabel(feat_it->GetOriginalFeature(), &label, feature::fFGL_Content);
                }
                m_FeatTypes.push_back(pair<string,CSeqFeatData::ESubtype>(label,subtype));
                m_FeatRanges.push_back(vec);
                m_FeatStrand.push_back(strand);
                m_Feat5Partial.push_back(feat_loc.IsPartialStart(objects::eExtreme_Positional));
                m_RealProt.push_back(prot_seq);
                int frame = 0;
                if ( subtype == CSeqFeatData::eSubtype_cdregion && feat_it->GetOriginalFeature().GetData().GetCdregion().IsSetFrame())
                {
                    switch(feat_it->GetOriginalFeature().GetData().GetCdregion().GetFrame())
                    {
                    case CCdregion::eFrame_two: frame = 1; break;
                    case CCdregion::eFrame_three: frame = 2; break;
                    default: break;
                    }
                }
                m_FeatFrames.push_back(frame);
                
                CRef<CGenetic_code> code;
                if (feat_it->GetOriginalFeature().GetData().IsCdregion() && feat_it->GetOriginalFeature().GetData().GetCdregion().IsSetCode())
                {
                    code.Reset(new CGenetic_code);
                    code->Assign(feat_it->GetOriginalFeature().GetData().GetCdregion().GetCode());
                }
                m_GeneticCode.push_back(code);

            }
        }
        ++feat_it;
    }
}

void CEditSequence::GetProteinFeatures(CBioseq_Handle prot_bsh, int i, const CSeq_id* seq_id, const CSeq_feat &old_cds,  TSeqPos seq_start)
{ 
    CScope &scope = prot_bsh.GetScope();
    CSeq_loc_Mapper map_to_nuc(old_cds, CSeq_loc_Mapper::eProductToLocation, &scope);
    map_to_nuc.SetMergeAbutting();

    for (CFeat_CI feat_ci(prot_bsh); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        if (!fh.IsPlainFeat() || fh.GetFeatSubtype() == CSeqFeatData::eSubtype_prot)
            continue;
        
        const CSeq_loc &loc = feat_ci->GetLocation();
        CRef<CSeq_loc> nuc_loc = map_to_nuc.Map(loc);
        vector<pair<TSeqPos,TSeqPos> > vec;
        for(CSeq_loc_CI loc_it(*nuc_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)     
        {
            if (seq_id && !sequence::IsSameBioseq(*seq_id, loc_it.GetSeq_id(), &scope)) {
                continue;
            }
            CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
            TSeqPos feat_start = feat_range.GetFrom() - seq_start;
            TSeqPos feat_stop = feat_range.GetTo() - seq_start;
            vec.push_back(pair<TSeqPos, TSeqPos>(feat_start, feat_stop));
        }
        m_ProtFeatRanges[i].push_back(vec);
    }
}


void CEditSequence::AdjustFeatureLocations(CRef<CCmdComposite> composite)
{
    CRef<CSeq_loc> loc = m_seh.GetSeq().GetRangeSeq_loc(0,0);
    TSeqPos seq_start = loc->GetStart(objects::eExtreme_Positional);
    SAnnotSelector sel;
    CScope &scope = m_seh.GetScope();
    CFeat_CI feat_it(scope, *loc, sel);
    const CSeq_id* seq_id = loc->GetId();
    unsigned int i = 0;
    while (feat_it) 
    {
        if (feat_it->GetOriginalFeature().IsSetData())
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->Assign(feat_it->GetOriginalFeature());
            const CSeq_loc& feat_loc = feat_it->GetLocation();
            CRef<CSeq_loc> new_loc(new CSeq_loc);
            new_loc->Assign(feat_loc);
            unsigned int j = 0;
            CSeq_loc_I loc_it(*new_loc); // , CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional
            while(loc_it)      
            {
                if (loc_it.IsEmpty())
                {
                    ++loc_it;
                    continue;
                }
                if (seq_id && !sequence::IsSameBioseq(*seq_id, loc_it.GetSeq_id(), &scope))
                {
                    ++loc_it;
                    continue;
                }

                CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                TSeqPos feat_start = feat_range.GetFrom() - seq_start;
                TSeqPos feat_stop = feat_range.GetTo() - seq_start;
                TSeqPos new_start = m_Panel->GetFeatureStart(i,j);
                TSeqPos new_stop = m_Panel->GetFeatureStop(i,j);
                if (new_stop == numeric_limits<int>::max() || new_start == numeric_limits<int>::max())
                {
                    loc_it.Delete();
                    ++j;               
                    continue;
                }
                if (feat_start != new_start)
                {
                    loc_it.SetFrom(new_start);
                }
                if (feat_stop != new_stop)
                {
                    loc_it.SetTo(new_stop);
                }
                ++loc_it;
                ++j;
            }
            CRef<CSeq_loc> loc = loc_it.MakeSeq_loc();
            bool modified = loc_it.HasChanges();
            bool removed = loc->IsNull() || loc_it.GetSize() == 0;
           
            if (modified)
            {
                if (removed)
                {
                    CRef<CCmdDelSeq_feat> cmd_del(new CCmdDelSeq_feat(feat_it->GetSeq_feat_Handle()));
                    if (cmd_del)
                    {
                        composite->AddCommand(*cmd_del);
                    }
                }
                else
                {
                    new_feat->SetLocation(*loc);  
                    if (new_feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion)
                        new_feat->SetData().SetCdregion().SetFrame(CSeqTranslator::FindBestFrame(*new_feat, m_edited_seh.GetScope())); 
                    if (m_ProtFeatRanges.find(i) != m_ProtFeatRanges.end())
                    {
                        AdjustProtFeatureLocations(composite, feat_it->GetOriginalFeature(), *new_feat, i, seq_id, seq_start);
                    }
                    CRef< CCmdChangeSeq_feat > cmd(new CCmdChangeSeq_feat(feat_it->GetSeq_feat_Handle(),*new_feat));
                    if (cmd) 
                    {
                        composite->AddCommand(*cmd);
                    }
                }
            }
        }
        ++feat_it;
        i++;
    }
}

void CEditSequence::AdjustProtFeatureLocations(CRef<CCmdComposite> composite, const CSeq_feat &old_cds, const CSeq_feat &cds, int i, const CSeq_id* seq_id, TSeqPos seq_start)
{
    CScope &scope = m_seh.GetScope();
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds.GetProduct());
    CSeq_loc_Mapper map_to_nuc(old_cds, CSeq_loc_Mapper::eProductToLocation, &scope);
    map_to_nuc.SetMergeAbutting();
    CSeq_loc_Mapper map_to_prot(cds, CSeq_loc_Mapper::eLocationToProduct, &scope);
    map_to_prot.SetMergeAbutting();
    unsigned int k = 0;
    for (CFeat_CI feat_ci(prot_bsh); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        if (!fh.IsPlainFeat() || fh.GetFeatSubtype() == CSeqFeatData::eSubtype_prot)
            continue;
        
        CRef<CSeq_loc> nuc_loc = map_to_nuc.Map(feat_ci->GetLocation());
        unsigned int j = 0;
        CSeq_loc_I loc_it(*nuc_loc); // , CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional
        while(loc_it)      
        {
            if (loc_it.IsEmpty())
            {
                ++loc_it;
                continue;
            }
            if (seq_id && !sequence::IsSameBioseq(*seq_id, loc_it.GetSeq_id(), &scope))
            {
                ++loc_it;
                continue;
            }

            CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
            TSeqPos feat_start = feat_range.GetFrom() - seq_start;
            TSeqPos feat_stop = feat_range.GetTo() - seq_start;
            TSeqPos new_start = m_Panel->GetProtFeatureStart(i,k,j);
            TSeqPos new_stop = m_Panel->GetProtFeatureStop(i,k,j);
            if (new_stop == numeric_limits<int>::max() || new_start == numeric_limits<int>::max())
            {
                loc_it.Delete();
                ++j;               
                continue;
            }
            if (feat_start != new_start)
            {
                loc_it.SetFrom(new_start);
            }
            if (feat_stop != new_stop)
            {
                loc_it.SetTo(new_stop);
            }
            ++loc_it;
            ++j;
        }

        CRef<CSeq_loc> loc = loc_it.MakeSeq_loc();
        bool modified = loc_it.HasChanges();
        bool removed = loc->IsNull() || loc_it.GetSize() == 0;

           
        if (modified)
        {
            if (removed)
            {
                CRef<CCmdDelSeq_feat> cmd_del(new CCmdDelSeq_feat(feat_ci->GetSeq_feat_Handle()));
                if (cmd_del)
                {
                    composite->AddCommand(*cmd_del);
                }
            }
            else
            {
                CRef<CSeq_loc> prot_loc = map_to_prot.Map(*loc);
                CRef<CSeq_feat> new_feat(new CSeq_feat);
                new_feat->Assign(feat_ci->GetOriginalFeature());
                new_feat->SetLocation(*prot_loc);  
                CRef< CCmdChangeSeq_feat > cmd(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(),*new_feat));
                if (cmd) 
                {
                    composite->AddCommand(*cmd);
                }
            }
        }
        k++;
    }
}

bool CEditSequence::x_ExecuteCommand(IEditCommand* command)
{
    if (m_seh && m_UndoManager && m_UndoManager->RequestExclusiveEdit(this, kExclusiveEditDescr)) {
        m_NumCmds++;
        m_UndoManager->Execute(command, this);
        return true;
    }
    return false;
}


void CEditSequence::OnFeatures( wxCommandEvent& event )
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

void CEditSequence::OnMismatch( wxCommandEvent& event )
{ 
    if (event.IsChecked())
    {
        m_Panel->EnableMismatch(true);
    }
    else
    {
        m_Panel->EnableMismatch(false);
    }
    m_Panel->Refresh();
    m_Panel->SetFocus();
}

void CEditSequence::OnCopy( wxCommandEvent& event )
{
    m_Panel->OnCopy(event);
    m_Panel->SetFocus();
}

void CEditSequence::OnCut( wxCommandEvent& event )
{
    m_Panel->OnCut(event);
    m_Panel->SetFocus();
}

void CEditSequence::OnPaste( wxCommandEvent& event )
{
    m_Panel->OnPaste(event);
    m_Panel->SetFocus();
}

void CEditSequence::IsClipboard(wxUpdateUIEvent &event)
{
    event.Enable(m_Panel->IsClipboard() && m_seh);
}

void CEditSequence::IsSelection(wxUpdateUIEvent &event)
{
    event.Enable(m_Panel->IsSelection() && m_seh);
}

void CEditSequence::IsNucleotide(wxUpdateUIEvent &event)
{
    event.Enable(m_is_nucleotide && m_seh);
}

void CEditSequence::IsSelectionAndClean(wxUpdateUIEvent &event)
{
    CSeqFeatData::ESubtype subtype = CBioseqEditor::GetFeatTypeFromCmdID(event.GetId());
    bool allow = true;
    CSeqFeatData::EFeatureLocationAllowed allowed_type = CSeqFeatData::AllowedFeatureLocation(subtype);
    if ( (m_is_nucleotide && allowed_type == CSeqFeatData::eFeatureLocationAllowed_ProtOnly) ||
         (!m_is_nucleotide && allowed_type == CSeqFeatData::eFeatureLocationAllowed_NucOnly) )
        allow = false;
    event.Enable(m_Panel->IsSelection() && m_Panel->GetClean() && allow && m_seh);
}


void CEditSequence::CreateFeature ( wxCommandEvent& event)
{
    if (!m_seh)
        return;
    CSeq_entry_Handle seh = m_seh;      
    pair<int,int> sel = m_Panel->GetSelection();
 
    if (sel.first >=0 && sel.second >= 0 && sel.second >= sel.first)
    {
        CRef<CSeq_loc> interval(new CSeq_loc);
        interval->SetInt().SetFrom(sel.first);
        interval->SetInt().SetTo(sel.second);
        CRef<CSeq_id> id(new CSeq_id);
        CRef<CSeq_loc> loc = seh.GetSeq().GetRangeSeq_loc(0,0);//seh.GetSeq().GetInst_Length()-1); 
        id->Assign(*loc->GetId());
        interval->SetInt().SetId(*id);
        CSeqFeatData::ESubtype subtype = CBioseqEditor::GetFeatTypeFromCmdID(event.GetId());
        CRef<CSeq_feat> feat = CBioseqEditor::MakeDefaultFeature(subtype);
        feat->SetLocation().Assign(*interval);

        CIRef<IEditObject> editor(new CEditObjectSeq_feat(*feat, seh, seh.GetScope(), true));
        CEditObjViewDlgModal edit_dlg(NULL, true);
        wxWindow* editorWindow = editor->CreateWindow(&edit_dlg);
        edit_dlg.SetEditorWindow(editorWindow);
        edit_dlg.SetEditor(editor);
        if (edit_dlg.ShowModal() == wxID_OK) 
        {
            CIRef<IEditCommand> cmd(editor->GetEditCommand());
            if (cmd && x_ExecuteCommand(cmd)) {
                GetFeatures(*loc, seh.GetScope());
                m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand, m_FeatFrames, m_GeneticCode, m_Feat5Partial, m_RealProt, m_ProtFeatRanges);
                m_Panel->Refresh();
                m_Panel->SetFocus();
            }
        } else
            m_Panel->SetFocus();
    }
}

void CEditSequence::CreateRegulatory ( wxCommandEvent& evt)
{
    if (!m_seh)
        return;
    CSeq_entry_Handle seh = m_seh;      
    pair<int,int> sel = m_Panel->GetSelection();
    
    if (sel.first >=0 && sel.second >= 0 && sel.second >= sel.first)
    {
        CRef<CSeq_loc> interval(new CSeq_loc);
        interval->SetInt().SetFrom(sel.first);
        interval->SetInt().SetTo(sel.second);
        CRef<CSeq_id> id(new CSeq_id);
        CRef<CSeq_loc> loc = seh.GetSeq().GetRangeSeq_loc(0,0);//seh.GetSeq().GetInst_Length()-1); 
        id->Assign(*loc->GetId());
        interval->SetInt().SetId(*id);        
        CSeqFeatData::ESubtype subtype =  CSeqFeatData::eSubtype_regulatory;
        CRef<CSeq_feat> feat = CBioseqEditor::MakeDefaultFeature(subtype);
        feat->SetLocation().Assign(*interval);
        
        string regulatory_class;
        switch(evt.GetId())
        {
        case eCmdCreateRegulatory_promoter : regulatory_class = "promoter"; break;
        case eCmdCreateRegulatory_enhancer : regulatory_class = "enhancer"; break;
        case eCmdCreateRegulatory_ribosome_binding_site : regulatory_class = "ribosome_binding_site"; break;
        case eCmdCreateRegulatory_riboswitch : regulatory_class = "riboswitch"; break;
        case eCmdCreateRegulatory_terminator : regulatory_class = "terminator"; break;
        default : break;
        }
        if (!regulatory_class.empty())
        {
            CRef<CGb_qual> q(new CGb_qual());
            q->SetQual("regulatory_class");
            q->SetVal(regulatory_class);
            feat->SetQual().push_back(q);
        }


        CIRef<IEditObject> editor(new CEditObjectSeq_feat(*feat, seh, seh.GetScope(), true));
        CEditObjViewDlgModal edit_dlg(NULL, true);
        wxWindow* editorWindow = editor->CreateWindow(&edit_dlg);
        edit_dlg.SetEditorWindow(editorWindow);
        edit_dlg.SetEditor(editor);
        if (edit_dlg.ShowModal() == wxID_OK) 
        {
            CIRef<IEditCommand> cmd(editor->GetEditCommand());
            if (cmd && x_ExecuteCommand(cmd)) {
                GetFeatures(*loc, seh.GetScope());
                m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand, m_FeatFrames, m_GeneticCode, m_Feat5Partial, m_RealProt, m_ProtFeatRanges);
                m_Panel->Refresh();
                m_Panel->SetFocus();
            }
        } else
            m_Panel->SetFocus();
    }
}

void CEditSequence::OnRetranslate ( wxCommandEvent& event)
{
    CScope &scope = m_seh.GetScope();
    CRef<CSeq_loc> loc = m_seh.GetSeq().GetRangeSeq_loc(0,0);
    CRef<CCmdComposite> cmd(new CCmdComposite("Retranslate CDS"));
    SAnnotSelector sel(CSeqFeatData::eSubtype_cdregion);
    int offset = 1;
    bool create_general_only = false;
    CScope::TTSE_Handles vec_tse;
    scope.GetAllTSEs(vec_tse, CScope::eAllTSEs);
    if (!vec_tse.empty())
        create_general_only = objects::edit::IsGeneralIdProtPresent(vec_tse.front());
    for (CFeat_CI feat_it(scope, *loc, sel); feat_it; ++feat_it)
    {
        cmd->AddCommand(*GetRetranslateCDSCommand(scope, feat_it->GetOriginalFeature(), offset, create_general_only));
    }
    if (cmd && x_ExecuteCommand(cmd)) 
    {
        GetFeatures(*loc, scope);
        m_Panel->UpdateFeatures(m_FeatRanges, m_FeatTypes, m_FeatStrand, m_FeatFrames, m_GeneticCode, m_Feat5Partial, m_RealProt, m_ProtFeatRanges);
        m_Panel->Refresh();
        m_Panel->SetFocus();
    }
}


/*
 * CFindEditSequence type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFindEditSequence, wxDialog )


/*
 * CFindEditSequence event table definition
 */

BEGIN_EVENT_TABLE( CFindEditSequence, wxDialog )

////@begin CFindEditSequence event table entries
EVT_BUTTON( ID_PREV_FIND_EDIT_SEQ, CFindEditSequence::OnPrev)
EVT_BUTTON( ID_NEXT_FIND_EDIT_SEQ, CFindEditSequence::OnNext)
EVT_BUTTON( ID_DISMISS_FIND_EDIT_SEQ, CFindEditSequence::OnDismiss)
EVT_CLOSE(CFindEditSequence::OnClose)

EVT_TEXT( ID_TEXTCTRL_FIND_EDIT_SEQ, CFindEditSequence::OnChange )
EVT_RADIOBUTTON(ID_NUC_FIND_EDIT_SEQ, CFindEditSequence::OnChange )
EVT_RADIOBUTTON(ID_TRANSFRAME_FIND_EDIT_SEQ, CFindEditSequence::OnChange )
EVT_CHOICE(ID_FRAMECHOICE_FIND_EDIT_SEQ, CFindEditSequence::OnChange )
EVT_CHECKBOX(ID_REVCOMP_FIND_EDIT_SEQ, CFindEditSequence::OnChange )

////@end CFindEditSequence event table entries

END_EVENT_TABLE()


/*
 * CFindEditSequence constructors
 */

CFindEditSequence::CFindEditSequence()
{
    Init();
}

CFindEditSequence::CFindEditSequence( wxWindow* parent, CPaintSequence *panel, bool is_nucleotide, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style ) 
    : m_Panel(panel), m_is_nucleotide(is_nucleotide)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CFindEditSequence creator
 */

bool CFindEditSequence::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFindEditSequence creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFindEditSequence creation
    return true;
}


/*
 * CFindEditSequence destructor
 */

CFindEditSequence::~CFindEditSequence()
{
////@begin CFindEditSequence destruction
////@end CFindEditSequence destruction
}


/*
 * Member initialisation
 */

void CFindEditSequence::Init()
{
////@begin CFindEditSequence member initialisation
    m_Pattern = NULL;
    m_ItemOfText = NULL;
    m_Nuc = NULL;
    m_RevComp = NULL;
    m_TransFrame = NULL;
    m_FrameChoice = NULL;
    m_Prev = NULL;
    m_Next = NULL;
////@end CFindEditSequence member initialisation
    m_Num = 0;
    m_Total = 0;
}


/*
 * Control creation for CFindEditSequence
 */

void CFindEditSequence::CreateControls()
{    
////@begin CFindEditSequence content construction
    // Generated by DialogBlocks, 30/01/2015 13:52:15 (unregistered)

    CFindEditSequence* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Find pattern"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    m_Pattern = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_FIND_EDIT_SEQ, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    itemBoxSizer4->Add(m_Pattern, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ItemOfText = new wxStaticText( itemDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_ItemOfText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Search in"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_LEFT|wxALL, 5);

    m_Nuc = new wxRadioButton( itemStaticBoxSizer7->GetStaticBox(), ID_NUC_FIND_EDIT_SEQ, _("Nucleotide sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Nuc->SetValue(true);
    itemBoxSizer8->Add(m_Nuc, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RevComp = new wxCheckBox( itemStaticBoxSizer7->GetStaticBox(), ID_REVCOMP_FIND_EDIT_SEQ, _("reverse complement"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RevComp->SetValue(false);
    itemBoxSizer8->Add(m_RevComp, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer7->Add(itemBoxSizer11, 0, wxALIGN_LEFT|wxALL, 5);

    m_TransFrame = new wxRadioButton( itemStaticBoxSizer7->GetStaticBox(), ID_TRANSFRAME_FIND_EDIT_SEQ, _("Translated frame"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TransFrame->SetValue(false);
    itemBoxSizer11->Add(m_TransFrame, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FrameChoiceStrings;
    m_FrameChoiceStrings.Add(_("Any"));
    m_FrameChoiceStrings.Add(_("+1"));
    m_FrameChoiceStrings.Add(_("+2"));
    m_FrameChoiceStrings.Add(_("+3"));
    m_FrameChoiceStrings.Add(_("-1"));
    m_FrameChoiceStrings.Add(_("-2"));
    m_FrameChoiceStrings.Add(_("-3"));
    m_FrameChoice = new wxChoice( itemStaticBoxSizer7->GetStaticBox(), ID_FRAMECHOICE_FIND_EDIT_SEQ, wxDefaultPosition, wxDefaultSize, m_FrameChoiceStrings, 0 );
    m_FrameChoice->SetStringSelection(_("Any"));
    m_FrameChoice->Disable();
    itemBoxSizer11->Add(m_FrameChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Prev = new wxButton( itemDialog1, ID_PREV_FIND_EDIT_SEQ, _("Previous"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(m_Prev, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Prev->Disable();

    m_Next = new wxButton( itemDialog1, ID_NEXT_FIND_EDIT_SEQ, _("Next"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(m_Next, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Next->Disable();

    wxButton* itemButton17 = new wxButton( itemDialog1, ID_DISMISS_FIND_EDIT_SEQ, _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CFindEditSequence content construction
    if (!m_is_nucleotide)
    {
        itemStaticBoxSizer7Static->Hide();
    }
}


/*
 * Should we show tooltips?
 */

bool CFindEditSequence::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CFindEditSequence::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFindEditSequence bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFindEditSequence bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CFindEditSequence::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFindEditSequence icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFindEditSequence icon retrieval
}



void CFindEditSequence::OnPrev( wxCommandEvent& event )
{
    m_Num--;
    m_CurrentPos--;
    wxString label;
    label << m_Num << " of " << m_Total; 
    m_ItemOfText->SetLabel(label);
    m_Panel->SetPos(*m_CurrentPos+1);
    if (m_Num <= 1)
        m_Prev->Disable();
    if (m_Num < m_Total)
        m_Next->Enable();
}

void CFindEditSequence::OnNext( wxCommandEvent& event )
{
    if (m_Pos.empty())
    {
        string choice;
        int sel = m_FrameChoice->GetSelection();
        if (sel != wxNOT_FOUND)
            choice = m_FrameChoice->GetString(sel).ToStdString();
        m_Pos.clear();
        m_Num = 0;
        if (!m_Nuc->GetValue() && choice == "Any")
        {
            FindMatchPositions("+1");
            FindMatchPositions("+2");
            FindMatchPositions("+3");
            FindMatchPositions("-1");
            FindMatchPositions("-2");
            FindMatchPositions("-3");
        }
        else
        {
            FindMatchPositions(choice);
        }
        m_Total = m_Pos.size();
        m_CurrentPos = m_Pos.begin();
        SetHighlights();
        
    }
    else
    {
        m_CurrentPos++;
    }
    if (!m_Pos.empty())
    {
        m_Num++;
        wxString label;
        label << m_Num << " of " << m_Total; 
        m_ItemOfText->SetLabel(label);
        m_Panel->SetPos(*m_CurrentPos+1);
        if (m_Num >= m_Total)
            m_Next->Disable();
        if (m_Num > 1)
            m_Prev->Enable();
    }
    else
    {
        m_ItemOfText->SetLabel(_("No matches found"));
        m_Next->Disable();
    }
        
}

void CFindEditSequence::OnDismiss( wxCommandEvent& event )
{
    Close();
}

void CFindEditSequence::OnClose(wxCloseEvent& event)
{
    m_Pos.clear();
    SetHighlights();
    m_Panel->Refresh();
    event.Skip();
}

string CFindEditSequence::GetPattern()
{
    string str = m_Pattern->GetValue().ToStdString();
    string res;
    for (size_t i=0; i<str.length(); i++)
    {
        char a = str[i];
        if (isdigit(a) || isspace(a))
            continue;
        res += a;
    }
    return res;
}

void CFindEditSequence::OnChange( wxCommandEvent& event )
{
    m_Num = 0;
    m_Total = 0;
    m_Pos.clear();
    m_ItemOfText->SetLabel(wxEmptyString);
    m_Prev->Disable();
    if (m_Nuc->GetValue())
    {
        m_RevComp->Enable();
        m_FrameChoice->Disable();
    }
    else
    {
        m_RevComp->Disable();
        m_FrameChoice->Enable();
    }

    if (!GetPattern().empty())
    {
        m_Next->Enable();
    }
    else
    {
        m_Next->Disable();
    }
}


void CFindEditSequence::FindMatchPositions(const string &choice)
{  
    int pos = 0;
    string *str = m_Panel->GetFindString(m_Nuc->GetValue(), m_RevComp->GetValue(), choice); 
    string value = GetPattern();
    if ( (m_Nuc->GetValue() && m_RevComp->GetValue()) )//|| (!m_Nuc->GetValue() && (choice == "-1" || choice == "-2" || choice == "-3"))
    {
        reverse(value.begin(),value.end());
    }
    if (str && !value.empty())
    {
        while (pos != NPOS && pos <str->size())
        {
            pos = NStr::FindNoCase(*str,value,pos);
            if (pos != NPOS)
            {
                int pos_actual = pos;
                if (!m_Nuc->GetValue())
                {
                    if (choice == "-1" || choice == "-2" || choice == "-3")
                        pos_actual = str->size() - pos_actual - 1 - (value.size() - 1);
                    int length = m_Panel->GetLength();
                    pos_actual *= 3;
                    int offset = 0;
                    if (choice == "+2")
                        offset = 1;
                    if (choice == "+3")
                        offset = 2;
                    if (choice == "-1")
                        offset = length - str->size()*3;
                    if (choice == "-2")
                        offset = length - str->size()*3 - 1;
                    if (choice == "-3")
                        offset = length - str->size()*3 - 2;
                    if (offset < 0)
                        offset = 0;
                    pos_actual += offset;
                   
                }
                m_Pos.insert(pos_actual); 
                pos++;
            }            
        }
    }
}

void CFindEditSequence::SetHighlights()
{
    unsigned int length = GetPattern().size();
    if (!m_Nuc->GetValue())
    {
        length *= 3;
    }
    set<int> highlights;
    for (set<int>::iterator it = m_Pos.begin(); it != m_Pos.end(); ++it)
    {
        for (unsigned int j = 0; j < length; j++)
            highlights.insert((*it)+j);
    }
    m_Panel->SetHighlights().swap(highlights);
}

END_NCBI_SCOPE
