/* $Id: vectortrimpanel.cpp 42821 2019-04-18 19:32:56Z joukovv $
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
 * Authors:  Yoon Choi
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/log.h> 
#include <wx/stattext.h>
#include <wx/radiobox.h>

#include <gui/widgets/edit/vectortrimpanel.hpp>
#include <gui/widgets/edit/vectorscreen.hpp>
#include <gui/widgets/edit/citsub_updater.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/cmd_change_graph.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_del_bioseq_set.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <objects/seq/seqport_util.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqres/Byte_graph.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/sequence.hpp>
#include <corelib/ncbi_autoinit.hpp>

#include <sstream>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void ShowDebugMessage(const wxString& message)
{
    wxMessageDialog msgdlg( NULL, message, _("Debug") );
    msgdlg.ShowModal();
}

// Ids do not need to be globally unique as long as they are
// unique within the local context (e.g., unique per dialog)
enum
{
    MYID_BUTTON_RUN_VECSCREEN    = wxID_HIGHEST + 1,
    MYID_RADIOBOX_SORT,
    MYID_BUTTON_SELECT_ALL,
    MYID_BUTTON_SELECT_STRONG_MODERATE,
    MYID_BUTTON_UNSELECT_ALL,
    MYID_BUTTON_UNSELECT_INTERNAL,
    MYID_CHECKBOX_VIEWGRAPHIC,
    MYID_BUTTON_MAKE_REPORT,
    MYID_BUTTON_TRIM_SELECTED,
    MYID_LISTCTRL_MATCHES,
    MYID_LISTCTRL_LOCATIONS,
    MYID_WEBVIEW_GRAPHIC,
    MYID_BUTTON_FIND_FORWARD,
    MYID_BUTTON_FIND_REVERSE,
    MYID_RADIOBOX_INTERNAL_TRIM_OPTIONS,
    MYID_BUTTON_DISMISS
};

/*!
 * CVectorTrimPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CVectorTrimPanel, wxPanel )


/*!
 * CVectorTrimPanel event table definition
 */

BEGIN_EVENT_TABLE( CVectorTrimPanel, wxPanel )

////@begin CVectorTrimPanel event table entries

////@end CVectorTrimPanel event table entries
    EVT_BUTTON(MYID_BUTTON_RUN_VECSCREEN, CVectorTrimPanel::OnRunVecscreen)
    EVT_RADIOBOX(MYID_RADIOBOX_SORT, CVectorTrimPanel::OnSort)
    EVT_BUTTON(MYID_BUTTON_SELECT_ALL, CVectorTrimPanel::OnSelectAll)
    EVT_BUTTON(MYID_BUTTON_SELECT_STRONG_MODERATE, CVectorTrimPanel::OnSelectStrongModerate)
    EVT_BUTTON(MYID_BUTTON_UNSELECT_ALL, CVectorTrimPanel::OnUnselectAll)
    EVT_BUTTON(MYID_BUTTON_UNSELECT_INTERNAL, CVectorTrimPanel::OnUnselectInternal)
    //EVT_CHECKBOX(MYID_CHECKBOX_VIEWGRAPHIC, CVectorTrimPanel::OnViewAlignmentGraphic)
    EVT_BUTTON(MYID_BUTTON_MAKE_REPORT, CVectorTrimPanel::OnMakeReport)
    EVT_BUTTON(MYID_BUTTON_TRIM_SELECTED, CVectorTrimPanel::OnTrimSelected)
    EVT_BUTTON(MYID_BUTTON_FIND_FORWARD, CVectorTrimPanel::OnFindForward)
    EVT_BUTTON(MYID_BUTTON_FIND_REVERSE, CVectorTrimPanel::OnFindReverse)
    EVT_BUTTON(MYID_BUTTON_DISMISS, CVectorTrimPanel::OnDismiss)

END_EVENT_TABLE()


/*!
 * CVectorTrimPanel constructors
 */

CVectorTrimPanel::CVectorTrimPanel()
{
    m_pParentWindow = NULL;
    Init();
}

CVectorTrimPanel::CVectorTrimPanel( wxWindow* parent, 
                                    objects::CSeq_entry_Handle seh, 
                                    ICommandProccessor* cmdProcessor,
                                    wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
  : m_TopSeqEntry(seh),
    m_CmdProcessor(cmdProcessor),
    m_pParentWindow(parent)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CVectorTrimPanel creator
 */

bool CVectorTrimPanel::Create( wxWindow* parent, 
                               wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CVectorTrimPanel creation
    wxPanel::Create( parent, id, pos, size, style );
    m_pParentWindow = parent;

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CVectorTrimPanel creation
    return true;
}


/*!
 * CVectorTrimPanel destructor
 */

CVectorTrimPanel::~CVectorTrimPanel()
{
////@begin CVectorTrimPanel destruction
////@end CVectorTrimPanel destruction
}


/*!
 * Member initialisation
 */

void CVectorTrimPanel::Init()
{
////@begin CVectorTrimPanel member initialisation
////@end CVectorTrimPanel member initialisation
}


/*!
 * Control creation for CVectorTrimPanel
 */

void CVectorTrimPanel::CreateControls()
{    
////@begin CVectorTrimPanel content construction
    CVectorTrimPanel* itemPanel1 = this;

    // Top level sizer
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

////@end CVectorTrimPanel content construction

    // Row 0 - heading labels
    wxBoxSizer* sizerLabels = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* matches = 
        new wxStaticText( this, wxID_ANY, _( "Sequences with Vector Match" ), 
                          wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_VERTICAL );
    wxStaticText* locations = 
        new wxStaticText( this, wxID_ANY, _( "Location of Vector Match" ), 
                          wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_VERTICAL );
    sizerLabels->Add(matches, 1, wxEXPAND|wxALL, 5);
    sizerLabels->Add(locations, 1, wxEXPAND|wxALL, 5);
    itemBoxSizer2->Add(sizerLabels, 0, wxEXPAND|wxALL, 5);

    // Row 1 - display results
    m_sizerVectorResults = new wxBoxSizer(wxHORIZONTAL);
    m_listMatches = new CMatchesListCtrl( this, MYID_LISTCTRL_MATCHES, wxDefaultPosition, wxDefaultSize, 
                                          wxLC_REPORT | wxLC_SINGLE_SEL );

    // Workaround for Windows only!
    // Highlight the entire row not just the first column in the row.
#ifdef __WXMSW__        
    DWORD dwStyle = ::SendMessage((HWND)m_listMatches->GetHandle(), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
    dwStyle |= LVS_EX_FULLROWSELECT;
    ::SendMessage((HWND)m_listMatches->GetHandle(), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);
#endif


    // This sizer contains 2 elements in the same location.
    // A toggle will show/hide the appropriate element.
    m_sizerVectorResults->Add(m_listMatches, 1, wxEXPAND|wxALL, 5);

    m_viewAlignGraphic = new wxHtmlWindow( this, MYID_WEBVIEW_GRAPHIC );
    m_sizerVectorResults->Add(m_viewAlignGraphic, 1, wxEXPAND|wxALL, 5);
    m_listMatches->SetAlignGraphicView( m_viewAlignGraphic );

    m_listLocations = new CLocationsListCtrl( this, MYID_LISTCTRL_LOCATIONS, wxDefaultPosition, wxDefaultSize, 
                                              wxLC_REPORT | wxLC_SINGLE_SEL );
    m_sizerVectorResults->Add(m_listLocations, 1, wxEXPAND|wxALL, 5);
    m_listMatches->SetRangeView( m_listLocations );

    // By default show m_listLocations, hide m_viewAlignGraphic
    m_sizerVectorResults->Hide(m_viewAlignGraphic);
    m_sizerVectorResults->Layout();


    itemBoxSizer2->Add(m_sizerVectorResults, 1, wxEXPAND|wxALL, 5);

    // Row 2 - run blast vecscreen against UniVec db
    wxBoxSizer* sizerSearchVector = new wxBoxSizer(wxHORIZONTAL);
    m_buttonRunVecscreen = new wxButton( this, MYID_BUTTON_RUN_VECSCREEN, _("Search UniVec_Core") );
    sizerSearchVector->Add(m_buttonRunVecscreen, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonRunVecscreen->Enable(CVectorScreen::IsDbAvailable());
    m_gaugeProgressBar = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH );
    sizerSearchVector->Add(m_gaugeProgressBar, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer2->Add(sizerSearchVector, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Row 3 - Find text
    wxBoxSizer* sizerFindText = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* findtext = 
        new wxStaticText( this, wxID_ANY, _( "Find Text" ), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL );
    sizerFindText->Add(findtext, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_textFind = new wxTextCtrl( this, wxID_ANY );
    sizerFindText->Add(m_textFind, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonFindReverse = new wxButton( this, MYID_BUTTON_FIND_REVERSE, _("<<") );
    sizerFindText->Add(m_buttonFindReverse, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonFindForward = new wxButton( this, MYID_BUTTON_FIND_FORWARD, _(">>") );
    sizerFindText->Add(m_buttonFindForward, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer2->Add(sizerFindText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Row 4 - Sort results
    wxArrayString buttons;
    buttons.Add(_("Internal, 5', 3'"));
    buttons.Add(_("Strength"));
    buttons.Add(_("Marked"));
    buttons.Add(_("Accession"));
    m_radioBoxSort = new wxRadioBox( this, MYID_RADIOBOX_SORT, _("Order By"), wxDefaultPosition, wxDefaultSize,
                                     buttons, 0, wxRA_SPECIFY_COLS );
    itemBoxSizer2->Add(m_radioBoxSort, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Row 5 - Internal Trim Options
    wxArrayString buttons2;
    buttons2.Add(_("Trim to closest end"));
    buttons2.Add(_("Trim to 5' end"));
    buttons2.Add(_("Trim to 3' end"));
    m_radioBoxInternalTrimOptions = new wxRadioBox( this, MYID_RADIOBOX_INTERNAL_TRIM_OPTIONS, 
                                                    _("Internal Trim Options"),
                                                    wxDefaultPosition, wxDefaultSize,
                                                    buttons2, 0, wxRA_SPECIFY_COLS );
    itemBoxSizer2->Add(m_radioBoxInternalTrimOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Row 6 - select results
    wxBoxSizer* sizerSelectResults = new wxBoxSizer(wxHORIZONTAL);
    m_buttonSelectAll = new wxButton( this, MYID_BUTTON_SELECT_ALL, _("Select All") );
    sizerSelectResults->Add(m_buttonSelectAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonUnselectAll = new wxButton( this, MYID_BUTTON_UNSELECT_ALL, _("Unselect All") );
    sizerSelectResults->Add(m_buttonUnselectAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonUnselectInternal = new wxButton( this, MYID_BUTTON_UNSELECT_INTERNAL, _("Unselect Internal") );
    sizerSelectResults->Add(m_buttonUnselectInternal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonSelectStrongModerate = new wxButton( this, MYID_BUTTON_SELECT_STRONG_MODERATE, _("Select Only Strong and Moderate") );
    sizerSelectResults->Add(m_buttonSelectStrongModerate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    //m_checkboxViewGraphic = new wxCheckBox( this, MYID_CHECKBOX_VIEWGRAPHIC, _("View Alignment Graphic") );
    //sizerSelectResults->Add(m_checkboxViewGraphic, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    itemBoxSizer2->Add(sizerSelectResults, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CitSub = new wxCheckBox( this, wxID_ANY, _("Add CitSub Update to trimmed sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CitSub->SetValue(false);
    itemBoxSizer2->Add(m_CitSub, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Row 7 - Trim selected sequences
    wxBoxSizer* sizerTrimSelected = new wxBoxSizer(wxHORIZONTAL);
    m_buttonMakeReport = new wxButton( this, MYID_BUTTON_MAKE_REPORT, _("Make Report") );
    sizerTrimSelected->Add(m_buttonMakeReport, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonTrimSelected = new wxButton( this, MYID_BUTTON_TRIM_SELECTED, _("Trim Selected Sequences") );
    sizerTrimSelected->Add(m_buttonTrimSelected, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_buttonTrimSelected->Disable();
    m_buttonDismiss = new wxButton( this, MYID_BUTTON_DISMISS, _("Dismiss") );
    sizerTrimSelected->Add(m_buttonDismiss, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer2->Add(sizerTrimSelected, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


bool CVectorTrimPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;


    return true;
}


bool CVectorTrimPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;


    return true;
}


/*!
 * Should we show tooltips?
 */

bool CVectorTrimPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CVectorTrimPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CVectorTrimPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CVectorTrimPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CVectorTrimPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CVectorTrimPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CVectorTrimPanel icon retrieval
}


void CVectorTrimPanel::OnRunVecscreen(wxCommandEvent &event)
{
    m_vectorscreen.Run(m_TopSeqEntry, *m_gaugeProgressBar);
    DisplaySummaryBySeqid(m_vectorscreen.GetVecscreenSummaryBySeqid());
    m_buttonTrimSelected->Enable();
}


void CVectorTrimPanel::OnSort(wxCommandEvent &event)
{
    // selection value is 0-based
    int selection = m_radioBoxSort->GetSelection();

    CMatchesListCtrl::ESortOption eSortOption;
    switch (selection) {
    case 0:
        eSortOption = CMatchesListCtrl::eSortInternal5Prime3Prime;
        break;
    case 1:
        eSortOption = CMatchesListCtrl::eSortStrength;
        break;
    case 2:
        eSortOption = CMatchesListCtrl::eSortMarked;
        break;
    case 3:
        eSortOption = CMatchesListCtrl::eSortAccession;
        break;
    default:
        eSortOption = CMatchesListCtrl::eSortInternal5Prime3Prime;
        break;
    }
    
    m_listMatches->Sort(eSortOption, true);
}


void CVectorTrimPanel::OnSelectAll(wxCommandEvent &event)
{
    m_listMatches->SelectAll();
}


void CVectorTrimPanel::OnSelectStrongModerate(wxCommandEvent &event)
{
    m_listMatches->SelectStrongModerate();
}


void CVectorTrimPanel::OnUnselectAll(wxCommandEvent &event)
{
    m_listMatches->UnselectAll();
}


void CVectorTrimPanel::OnUnselectInternal(wxCommandEvent &event)
{
    m_listMatches->UnselectInternal();
}



void CVectorTrimPanel::OnMakeReport(wxCommandEvent &event)
{
    CVecscreenReport* vecscreenReport = new CVecscreenReport(this);
    wxString report;
    m_listMatches->GetReport(report);
    vecscreenReport->SetReport(report);
    vecscreenReport->Show();
}


void CVectorTrimPanel::OnTrimSelected(wxCommandEvent &event)
{
    // Did the user trim or cancel?
    bool bTrimmed = x_TrimSelected();
    m_buttonTrimSelected->Disable();    
    // Was ALL contamination selected?
    bool bAllSelected = m_listMatches->AllSelected();
    
    // If the user trimmed all contamination, then close the window,
    // otherwise update the window with the remaining contamination.
    if (m_pParentWindow && bTrimmed && bAllSelected) {
        m_pParentWindow->Close();
    }
}


void CVectorTrimPanel::OnDismiss(wxCommandEvent &event)
{
    if (m_pParentWindow) {
        m_pParentWindow->Close();
    }
}


void CVectorTrimPanel::OnFindForward(wxCommandEvent &event)
{
    m_listMatches->FindForward(m_textFind->GetValue());
}


void CVectorTrimPanel::OnFindReverse(wxCommandEvent &event)
{
    m_listMatches->FindReverse(m_textFind->GetValue());
}


class CRangeCmp : public binary_function<SRangeMatch, SRangeMatch, bool>
{
public:
    enum ESortOrder
    {
        eAscending,
        eDescending
    };

    explicit CRangeCmp(ESortOrder sortorder = eAscending)
      : m_sortorder(sortorder)
    {
    };

    bool operator()(const SRangeMatch& a1, const SRangeMatch& a2) 
    {
        if (m_sortorder == eAscending)
        {
            // Tiebreaker
            if (a1.m_range.GetTo() == a2.m_range.GetTo()) {
                return a1.m_range.GetFrom() < a2.m_range.GetFrom();
            }
            return a1.m_range.GetTo() < a2.m_range.GetTo();
        }
        else
        {
            // Tiebreaker
            if (a1.m_range.GetTo() == a2.m_range.GetTo()) {
                return a1.m_range.GetFrom() > a2.m_range.GetFrom();
            }
            return a1.m_range.GetTo() > a2.m_range.GetTo();
        }
    };

private:
    ESortOrder m_sortorder;
};


void CVectorTrimPanel::x_SetSeqData(const CBioseq_Handle& bsh,
                                    CRef<objects::CSeq_inst> inst,
                                    const TCuts& sorted_cuts)
{
    // Should be a nuc!
    if (!bsh.IsNucleotide()) {
        return;
    }

    // Add the complete bioseq to scope
    CRef<CBioseq> bseq(new CBioseq);
    bseq->Assign(*bsh.GetCompleteBioseq());
    CScope& scope = bsh.GetTopLevelEntry().GetScope();
    CBioseq_Handle complete_bsh = scope.AddBioseq(*bseq);

    // Determine the "good" range sequence coordinates
    TSeqPos left_pos = 0;
    TSeqPos right_pos = inst->GetLength() - 1;
    for (const auto& cut : sorted_cuts) {
        if (cut.m_range.GetTo() == right_pos) {
            right_pos = cut.m_range.GetFrom() - 1;
        }

        if (cut.m_range.GetFrom() == left_pos) {
            left_pos = cut.m_range.GetTo() + 1;
        }
    }

    // Create a new Delta-ext
    CAutoInitRef<CDelta_ext> pDeltaExt;
    CSeqMap_CI seqmap_ci = complete_bsh.GetSeqMap().ResolvedRangeIterator(&complete_bsh.GetScope(),
        left_pos,
        1 + (right_pos - left_pos));
    for (; seqmap_ci; ++seqmap_ci) {
        switch (seqmap_ci.GetType()) {
        case CSeqMap::eSeqGap: 
        {
            // Sequence gaps
            const TSeqPos uGapLength = seqmap_ci.GetLength();
            const bool bIsLengthKnown = !seqmap_ci.IsUnknownLength();
            CConstRef<CSeq_literal> pOriginalGapSeqLiteral =
                seqmap_ci.GetRefGapLiteral();
            CAutoInitRef<CDelta_seq> pDeltaSeq;
            CAutoInitRef<CSeq_literal> pNewGapLiteral;
            if (pOriginalGapSeqLiteral) {
                pNewGapLiteral->Assign(*pOriginalGapSeqLiteral);
            }
            if (!bIsLengthKnown) {
                pNewGapLiteral->SetFuzz().SetLim(CInt_fuzz::eLim_unk);
            }
            pNewGapLiteral->SetLength(uGapLength);
            pDeltaSeq->SetLiteral(*pNewGapLiteral);
            pDeltaExt->Set().push_back(ncbi::Ref(&*pDeltaSeq));
        }
        break;
        case CSeqMap::eSeqData: 
        {
            // Sequence data
            string new_data;
            CSeqVector seqvec(complete_bsh, CBioseq_Handle::eCoding_Iupac);
            seqvec.GetSeqData(seqmap_ci.GetPosition(), seqmap_ci.GetEndPosition(),
                new_data);
            CRef<CSeq_data> pSeqData(new CSeq_data());
            pSeqData->SetIupacna(*new CIUPACna(new_data));
            CSeqportUtil::Pack(pSeqData);
            CAutoInitRef<CDelta_seq> pDeltaSeq;
            pDeltaSeq->SetLiteral().SetLength(seqmap_ci.GetLength());
            pDeltaSeq->SetLiteral().SetSeq_data(*pSeqData);
            pDeltaExt->Set().push_back(ncbi::Ref(&*pDeltaSeq));
        }
        break;
        default:
            break;
        }
    }

    scope.RemoveBioseq(complete_bsh);

    // Update sequence repr, length and data
    inst->ResetExt();
    inst->ResetSeq_data();
    inst->SetLength(1 + (right_pos - left_pos));
    if (pDeltaExt->Set().size() == 1) {
        // Repr raw
        inst->SetRepr(CSeq_inst::eRepr_raw);
        CRef<CDelta_seq> pDeltaSeq = *pDeltaExt->Set().begin();
        CSeq_data & seq_data = pDeltaSeq->SetLiteral().SetSeq_data();
        inst->SetSeq_data(seq_data);
    }
    else {
        // Repr delta
        inst->SetExt().SetDelta(*pDeltaExt);
    }
}


void CVectorTrimPanel::x_SeqIntervalDelete(CRef<CSeq_interval> interval, 
                                           TSeqPos cut_from, TSeqPos cut_to,
                                           const CSeq_id* seqid,
                                           bool& bCompleteCut,
                                           bool& bTrimmed)
{
    // These are required fields
    if ( !(interval->CanGetFrom() && interval->CanGetTo()) )
    {
        return;
    }

    // Feature location
    TSeqPos feat_from = interval->GetFrom();
    TSeqPos feat_to = interval->GetTo();

    // Size of the cut
    TSeqPos cut_size = cut_to - cut_from + 1;

    // Case 1: feature is located completely before the cut
    if (feat_to < cut_from)
    {
        // Nothing needs to be done - cut does not affect feature
        return;
    }

    // Case 2: feature is completely within the cut
    if (feat_from >= cut_from && feat_to <= cut_to)
    {
        // Feature should be deleted
        bCompleteCut = true;
        return;
    }

    // Case 3: feature is completely past the cut
    if (feat_from > cut_to)
    {
        // Shift the feature by the cut_size
        feat_from -= cut_size;
        feat_to -= cut_size;
        interval->SetFrom(feat_from);
        interval->SetTo(feat_to);
        bTrimmed = true;
        return;
    }

    /***************************************************************************
     * Cases below are partial overlapping cases
    ***************************************************************************/
    // Case 4: Cut is completely inside the feature 
    //         OR
    //         Cut is to the "left" side of the feature (i.e., feat_from is 
    //         inside the cut)
    //         OR
    //         Cut is to the "right" side of the feature (i.e., feat_to is 
    //         inside the cut)
    if (feat_to > cut_to) {
        // Left side cut or cut is completely inside feature
        feat_to -= cut_size;
    }
    else {
        // Right side cut
        feat_to = cut_from - 1;
    }

    // Take care of the feat_from from the left side cut case
    if (feat_from >= cut_from) {
        feat_from = cut_to + 1;
        feat_from -= cut_size;
    }

    interval->SetFrom(feat_from);
    interval->SetTo(feat_to);
    bTrimmed = true;
}


void CVectorTrimPanel::x_SeqLocDelete(CRef<CSeq_loc> loc, 
                                      TSeqPos from, TSeqPos to,
                                      const CSeq_id* seqid,
                                      bool& bCompleteCut,
                                      bool& bTrimmed)
{
    // Given a seqloc and a range, cut the seqloc

    switch(loc->Which())
    {
        // Single interval
        case CSeq_loc::e_Int:
        {
            CRef<CSeq_interval> interval(new CSeq_interval);
            interval->Assign(loc->GetInt());
            x_SeqIntervalDelete(interval, from, to, seqid, 
                                bCompleteCut, bTrimmed);
            loc->SetInt(*interval);
        }
        break;

        // Multiple intervals
        case CSeq_loc::e_Packed_int:
        {
            CRef<CSeq_loc::TPacked_int> intervals(new CSeq_loc::TPacked_int);
            intervals->Assign(loc->GetPacked_int());
            if (intervals->CanGet()) {
                // Process each interval in the list
                CPacked_seqint::Tdata::iterator it;
                for (it = intervals->Set().begin(); 
                     it != intervals->Set().end(); ) 
                {
                    // Initial value: assume that all intervals 
                    // will be deleted resulting in bCompleteCut = true.
                    // Later on if any interval is not deleted, then set
                    // bCompleteCut = false
                    if (it == intervals->Set().begin()) {
                        bCompleteCut = true;
                    }

                    bool bDeleted = false;
                    x_SeqIntervalDelete(*it, from, to, seqid, 
                                        bDeleted, bTrimmed);

                    // Should interval be deleted from list?
                    if (bDeleted) {
                        it = intervals->Set().erase(it);
                    }
                    else {
                        ++it;
                        bCompleteCut = false;
                    }
                }

                // Update the original list
                loc->SetPacked_int(*intervals);
            }
        }
        break;

        // Multiple seqlocs
        case CSeq_loc::e_Mix:
        {
            CRef<CSeq_loc_mix> mix(new CSeq_loc_mix);
            mix->Assign(loc->GetMix());
            if (mix->CanGet()) {
                // Process each seqloc in the list
                CSeq_loc_mix::Tdata::iterator it;
                for (it = mix->Set().begin(); 
                     it != mix->Set().end(); ) 
                {
                    // Initial value: assume that all seqlocs
                    // will be deleted resulting in bCompleteCut = true.
                    // Later on if any seqloc is not deleted, then set
                    // bCompleteCut = false
                    if (it == mix->Set().begin()) {
                        bCompleteCut = true;
                    }

                    bool bDeleted = false;
                    x_SeqLocDelete(*it, from, to, seqid, bDeleted, bTrimmed);

                    // Should seqloc be deleted from list?
                    if (bDeleted) {
                        it = mix->Set().erase(it);
                    }
                    else {
                        ++it;
                        bCompleteCut = false;
                    }
                }

                // Update the original list
                loc->SetMix(*mix);
            }
        }
        break;

        // Other choices not supported yet 
        default:
        {           
        }
        break;
    }
}


void CVectorTrimPanel::x_TrimFeatureLocations(CRef<CSeq_feat> feat,
                                              const TCuts& sorted_cuts,
                                              const CSeq_id* seqid,
                                              bool& bFeatureDeleted,
                                              bool& bFeatureTrimmed,
                                              bool& bProdDeleted,
                                              bool& bProdTrimmed)
{
    for (int ii = 0; ii < sorted_cuts.size(); ++ii)
    {
        const SRangeMatch& cut = sorted_cuts[ii];
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();

        // Update Seqloc "feature made from" 
        if (feat->CanGetLocation())
        {
            CRef<CSeq_feat::TLocation> new_location(new CSeq_feat::TLocation);
            new_location->Assign(feat->GetLocation());
            x_SeqLocDelete(new_location, from, to, seqid, 
                           bFeatureDeleted, bFeatureTrimmed);
            feat->SetLocation(*new_location);

            // No need to cut anymore nor update.  Feature will be completely
            // deleted.  
            if (bFeatureDeleted) {
                return;
            }
        }

        // Update Seqloc "product of process"
        if (feat->CanGetProduct())
        {
            CRef<CSeq_feat::TProduct> new_product(new CSeq_feat::TProduct);
            new_product->Assign(feat->GetProduct());
            x_SeqLocDelete(new_product, from, to, seqid, bProdDeleted, bProdTrimmed);
            feat->SetProduct(*new_product);
        }
    }
}


void CVectorTrimPanel::x_AdjustCdregionFrame(CScope& scope, 
                                             CRef<CSeq_feat> feat,
                                             const TCuts& sorted_cuts,
                                             const CSeq_id* seqid,
                                             bool bFeatureTrimmed,
                                             bool bProdTrimmed)
{
    CBioseq_Handle bsh = scope.GetBioseqHandle(*seqid);

    // Get partialness and strand of location before cutting
    bool bIsPartialStart = false;
    ENa_strand eStrand = eNa_strand_unknown;
    if (feat->CanGetLocation())
    {
        bIsPartialStart = feat->GetLocation().IsPartialStart(eExtreme_Biological);
        eStrand = feat->GetLocation().GetStrand(); 
    }

    for (int ii = 0; ii < sorted_cuts.size(); ++ii)
    {
        const SRangeMatch& cut = sorted_cuts[ii];
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();

        // Adjust Seq-feat.data.cdregion frame
        if (feat->CanGetData() && 
            feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion &&
            feat->GetData().IsCdregion() &&
            (bFeatureTrimmed || bProdTrimmed))
        {
            // Make a copy
            CRef<CCdregion> new_cdregion(new CCdregion);
            new_cdregion->Assign(feat->GetData().GetCdregion());

            // Edit the copy
            if ( (eStrand == eNa_strand_minus &&
                  to == bsh.GetInst().GetLength() - 1 &&
                  bIsPartialStart)
                 ||
                 (eStrand != eNa_strand_minus && 
                  from == 0 && 
                  bIsPartialStart) )
            {
                TSeqPos old_frame = new_cdregion->GetFrame();
                if (old_frame == 0)
                {
                    old_frame = 1;
                }

                TSignedSeqPos new_frame = old_frame - ((to - from + 1) % 3);
                if (new_frame < 1)
                {
                    new_frame += 3;
                }
                new_cdregion->SetFrame((CCdregion::EFrame)new_frame);
            }

            // Update the original
            feat->SetData().SetCdregion(*new_cdregion);
        }
    }
}


void CVectorTrimPanel::x_AdjustInternalCutLocations(TCuts& cuts, 
                                                    TSeqPos seq_length)
{
    EInternalTrimType eInternalTrimType = 
        static_cast<EInternalTrimType>(m_radioBoxInternalTrimOptions->GetSelection());

    for (int ii = 0; ii < cuts.size(); ++ii)
    {
        SRangeMatch& cut = cuts[ii];
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();

        // Is it an internal cut?
        if (from != 0 && to != seq_length-1) {
            if (eInternalTrimType == eTrimToClosestEnd) {
                // Extend the cut to the closest end
                if (from - 0 < seq_length-1 - to) {
                    cut.m_range.SetFrom(0);
                }
                else {
                    cut.m_range.SetTo(seq_length-1);
                }
            }
            else 
            if (eInternalTrimType == eTrimTo5PrimeEnd) {
                // Extend the cut to 5' end
                cut.m_range.SetFrom(0);
            }
            else {
                // Extend the cut to 3' end
                cut.m_range.SetTo(seq_length-1);
            }
        }
    }
}


void CVectorTrimPanel::x_MergeCuts(TCuts& sorted_cuts)
{
    // Assume cuts are sorted in Ascending order.

    // Merge abutting and overlapping cuts
    TCuts::iterator it;
    for (it = sorted_cuts.begin(); it != sorted_cuts.end(); )
    {
        SRangeMatch& cut = *it;
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();

        // Does next cut exist?
        if ( it+1 != sorted_cuts.end() ) {
            SRangeMatch& next_cut = *(it+1);
            TSeqPos next_from = next_cut.m_range.GetFrom();
            TSeqPos next_to = next_cut.m_range.GetTo();

            if ( next_from <= (to + 1) ) {
                // Current and next cuts abut or overlap
                // So adjust current cut and delete next cut
                cut.m_range.SetTo(next_to);
                sorted_cuts.erase(it+1);

                // Post condition after erase:
                // Since "it" is before the erase, "it" stays valid
                // and still refers to current cut
            }
            else {
                ++it;
            }
        }
        else {
            // I'm done
            break;
        }
    }
}


void CVectorTrimPanel::x_RetranslateCDS(CScope& scope, 
                                        CRef<CCmdComposite> command, 
                                        CRef<CSeq_feat> cds)
{
    // Assumption:  cds has been verified to be Cdregion with Product

    // Use Cdregion.Product to get handle to protein bioseq 
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds->GetProduct());

    // Should be a protein!
    if (!prot_bsh.IsProtein())
    {
        return;
    }

    // Make a copy of existing CSeq_inst
    CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
    new_inst->Assign(prot_bsh.GetInst());

    // Make edits to the CSeq_inst copy
    CRef<CBioseq> new_protein_bioseq;
    if (new_inst->IsSetSeq_data())
    {
        // Generate new protein sequence data and length
        new_protein_bioseq = CSeqTranslator::TranslateToProtein(*cds, scope);
        if (new_protein_bioseq->GetInst().GetSeq_data().IsIupacaa()) 
        {
            new_inst->SetSeq_data().SetIupacaa().Set( 
                new_protein_bioseq->GetInst().GetSeq_data().GetIupacaa().Get());
            new_inst->SetLength( new_protein_bioseq->GetInst().GetLength() );
        }
        else if (new_protein_bioseq->GetInst().GetSeq_data().IsNcbieaa()) 
        {
            new_inst->SetSeq_data().SetNcbieaa().Set( 
                new_protein_bioseq->GetInst().GetSeq_data().GetNcbieaa().Get());
            new_inst->SetLength( new_protein_bioseq->GetInst().GetLength() );
        }
    }

    if ( !new_protein_bioseq ) {
        return;
    }

    // Update protein sequence data and length
    CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(prot_bsh, 
                                                                 *new_inst));
    command->AddCommand(*chgInst);

    // If protein feature exists, update it
    SAnnotSelector sel(CSeqFeatData::e_Prot);
    CFeat_CI prot_feat_ci(prot_bsh, sel);
    for ( ; prot_feat_ci; ++prot_feat_ci ) {
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(prot_feat_ci->GetOriginalFeature());

        if ( new_feat->CanGetLocation() &&
             new_feat->GetLocation().IsInt() &&
             new_feat->GetLocation().GetInt().CanGetTo() )
        {
            new_feat->SetLocation().SetInt().SetTo(
                new_protein_bioseq->GetLength() - 1);

            CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*prot_feat_ci,
                                                               *new_feat));
            command->AddCommand(*chgFeat);
        }
    }
}


bool CVectorTrimPanel::x_FindSegment(const CDense_seg& denseg,
                                     CDense_seg::TDim row,
                                     TSeqPos pos, 
                                     CDense_seg::TNumseg& seg,
                                     TSeqPos& seg_start) const
{
    for (seg = 0; seg < denseg.GetNumseg(); ++seg) {
        TSignedSeqPos start = denseg.GetStarts()[seg * denseg.GetDim() + row];
        TSignedSeqPos len   = denseg.GetLens()[seg];
        if (start != -1) {
            if (pos >= start  &&  pos < start + len) {
                seg_start = start;
                return true;
            }
        }
    }
    return false;
}


void CVectorTrimPanel::x_CutDensegSegment(CRef<CSeq_align> align, 
                                          CDense_seg::TDim row,
                                          TSeqPos pos)
{
    // Find the segment where pos occurs for the sequence (identified by 
    // row).
    // If pos is not the start of the segment, cut the segment in two, with 
    // one of the segments using pos as the new start.


    // Find the segment where pos lies
    const CDense_seg& denseg = align->GetSegs().GetDenseg();
    CDense_seg::TNumseg foundseg; 
    TSeqPos seg_start;
    if ( !x_FindSegment(denseg, row, pos, foundseg, seg_start) ) {
        return;
    }

    // Found our segment seg
    // If pos falls on segment boundary, do nothing
    if (pos == seg_start) {
        return;
    }


    // Cut the segment :
    // 1) Allocate a new denseg with numseg size = original size + 1
    // 2) Copy elements before the cut
    // 3) Split segment at pos
    // 4) Copy elements after the cut
    // 5) Replace old denseg with new denseg

    // Allocate a new denseg with numseg size = original size + 1
    CRef<CDense_seg> new_denseg(new CDense_seg);    
    new_denseg->SetDim( denseg.GetDim() );
    new_denseg->SetNumseg( denseg.GetNumseg() + 1 );
    ITERATE( CDense_seg::TIds, idI, denseg.GetIds() ) {
        CSeq_id *si = new CSeq_id;
        si->Assign(**idI);
        new_denseg->SetIds().push_back( CRef<CSeq_id>(si) );
    }

    // Copy elements (starts, lens, strands) before the cut (up to and including
    // foundseg-1 in original denseg)
    for (CDense_seg::TNumseg curseg = 0; curseg < foundseg; ++curseg) {
        // Copy starts
        for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
            TSeqPos index = curseg * denseg.GetDim() + curdim;
            new_denseg->SetStarts().push_back( denseg.GetStarts()[index] );
        }

        // Copy lens
        new_denseg->SetLens().push_back( denseg.GetLens()[curseg] );

        // Copy strands
        if ( denseg.IsSetStrands() ) {
            for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); 
                 ++curdim) 
            {
                TSeqPos index = curseg * denseg.GetDim() + curdim;
                new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
            }
        }
    }

    // Split segment at pos
    // First find the lengths of the split segments, first_len and second_len
    TSeqPos first_len, second_len;
    TSeqPos index = foundseg * denseg.GetDim() + row;
    if ( !denseg.IsSetStrands() || denseg.GetStrands()[index] != eNa_strand_minus )
    {
        first_len  = pos - seg_start;
        second_len = denseg.GetLens()[foundseg] - first_len;
    } 
    else {
        second_len = pos - seg_start;
        first_len  = denseg.GetLens()[foundseg] - second_len;
    }   

    // Set starts, strands, and lens for the split segments (foundseg and foundseg+1)
    // Populate foundseg in new denseg
    for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
        TSeqPos index = foundseg * denseg.GetDim() + curdim;
        if (denseg.GetStarts()[index] == -1) {
            new_denseg->SetStarts().push_back(-1);
        }
        else if (!denseg.IsSetStrands() || denseg.GetStrands()[index] != eNa_strand_minus) {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index]);
        }
        else {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index] + second_len);
        }

        if (denseg.IsSetStrands()) {
            new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
        }
    }    
    new_denseg->SetLens().push_back(first_len);
    // Populate foundseg+1 in new denseg
    for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
        TSeqPos index = foundseg * denseg.GetDim() + curdim;
        if (denseg.GetStarts()[index] == -1) {
            new_denseg->SetStarts().push_back(-1);
        }
        else if (!denseg.IsSetStrands() || denseg.GetStrands()[index] != eNa_strand_minus) {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index] + first_len);
        }
        else {
            new_denseg->SetStarts().push_back(denseg.GetStarts()[index]);
        }

        if (denseg.IsSetStrands()) {
            new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
        }
    }    
    new_denseg->SetLens().push_back(second_len);

    // Copy elements (starts, lens, strands) after the cut (starting from foundseg+1 in 
    // original denseg)
    for (CDense_seg::TNumseg curseg = foundseg+1; curseg < denseg.GetNumseg(); ++curseg) {
        // Copy starts
        for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); ++curdim) {
            TSeqPos index = curseg * denseg.GetDim() + curdim;
            new_denseg->SetStarts().push_back( denseg.GetStarts()[index] );
        }

        // Copy lens
        new_denseg->SetLens().push_back( denseg.GetLens()[curseg] );

        // Copy strands
        if ( denseg.IsSetStrands() ) {
            for (CDense_seg::TDim curdim = 0; curdim < denseg.GetDim(); 
                 ++curdim) 
            {
                TSeqPos index = curseg * denseg.GetDim() + curdim;
                new_denseg->SetStrands().push_back(denseg.GetStrands()[index]);
            }
        }
    }

    // Update 
    align->SetSegs().SetDenseg(*new_denseg);
}


void CVectorTrimPanel::x_AdjustDensegAlignment(CRef<CSeq_align> align, 
                                               CDense_seg::TDim row, 
                                               const TCuts& sorted_cuts)
{
    for (int ii = 0; ii < sorted_cuts.size(); ++ii)
    {
        const SRangeMatch& cut = sorted_cuts[ii];
        TSeqPos cut_from = cut.m_range.GetFrom();
        TSeqPos cut_to = cut.m_range.GetTo();

        TSeqPos cut_len = cut_to - cut_from + 1;
        if (cut_to < cut_from) {
            cut_len = cut_from - cut_to + 1;
            cut_from = cut_to;
        } 

        // Note: row is 0-based

        // May need to cut the segment at both start and stop positions
        // if they do not fall on segment boundaries
        x_CutDensegSegment(align, row, cut_from);
        x_CutDensegSegment(align, row, cut_from + cut_len);

        // Update segment start values for the trimmed sequence row
        const CDense_seg& denseg = align->GetSegs().GetDenseg();
        for (CDense_seg::TNumseg curseg = 0; curseg < denseg.GetNumseg(); ++curseg) {
            TSeqPos index = curseg * denseg.GetDim() + row;
            TSeqPos seg_start = denseg.GetStarts()[index];
            if (seg_start < 0) {
                // This indicates a gap, no change needed
            }
            else if (seg_start < cut_from) {
                // This is before the cut, no change needed
            }
            else if (seg_start >= cut_from && 
                     seg_start + denseg.GetLens()[curseg] <= cut_from + cut_len) {
                // This is in the gap, indicate it with a -1
                align->SetSegs().SetDenseg().SetStarts()[index] = -1;
            }
            else {
                // This is after the cut - subtract the cut_len
                align->SetSegs().SetDenseg().SetStarts()[index] -= cut_len;
            }
        }
    }
}


void CVectorTrimPanel::x_TrimSelected(CRef<CCmdComposite> command,
                                      const CSeq_id* seqid,
                                      const SVecscreenResult& vecres)
{
    CScope& scope = m_TopSeqEntry.GetScope();
    CBioseq_Handle bsh = scope.GetBioseqHandle(*seqid);

    // Should be a nuc!
    if (!bsh.IsNucleotide())
    {
        return;
    }


    TCuts sorted_cuts(vecres.m_arrRangeMatch);
    x_GetSortedCuts(sorted_cuts, seqid);


    /***************************************************************************
     * Trim nuc sequence
    ***************************************************************************/
    // Make a copy of existing CSeq_inst
    CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
    new_inst->Assign(bsh.GetInst());

    // Make edits to the CSeq_inst copy
    x_SetSeqData(bsh, new_inst, sorted_cuts);

    // Swap in the edited CSeq_inst copy against the original 
    CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(bsh, *new_inst));
    command->AddCommand(*chgInst);


    /***************************************************************************
     * Trim all annotation that reference the nuc
    ***************************************************************************/
    // Trim features
    SAnnotSelector feat_sel(CSeq_annot::C_Data::e_Ftable);
    CRef<CSeq_id> copy_id(new CSeq_id());
    copy_id->Assign(*seqid);
    CRef<CSeq_loc> search_loc( new CSeq_loc );
    search_loc->SetWhole(*copy_id);
    CFeat_CI feat_ci(scope, *search_loc, feat_sel);
    for (; feat_ci; ++feat_ci)
    {
        // Make a copy of the feature
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(feat_ci->GetOriginalFeature());

        // Detect complete deletions of feature
        bool bFeatureDeleted = false;

        // Detect case where feature was not deleted but merely trimmed
        bool bFeatureTrimmed = false;

        bool bProdDeleted = false;
        bool bProdTrimmed = false;

        // Make edits to the copy
        x_TrimFeatureLocations(new_feat, sorted_cuts, seqid, 
                               bFeatureDeleted, bFeatureTrimmed,
                               bProdDeleted, bProdTrimmed);

        if (bFeatureDeleted) {
            // Delete original feature
            CIRef<IEditCommand> delFeat(new CCmdDelSeq_feat(*feat_ci));
            command->AddCommand(*delFeat);

            // If this feat is a Cdregion, then delete the protein sequence
            if ( feat_ci->IsSetData() && 
                 feat_ci->GetData().Which() == CSeqFeatData::e_Cdregion &&
                 feat_ci->IsSetProduct() )
            {
                // Use Cdregion feat.product seqloc to get protein bioseq handle
                CBioseq_Handle prot_h = 
                    scope.GetBioseqHandle(feat_ci->GetProduct());

                // Should be a protein!
                if ( prot_h.IsProtein() ) {
                    // NOTE: 
                    // CCmdDelBioseqInst implementation already takes care of
                    // renormalizing the nuc-prot set if necessary.
                    CRef<CCmdDelBioseqInst> 
                        delProt(new CCmdDelBioseqInst(prot_h));
                    command->AddCommand(*delProt);
                }
            }
        }
        else {
            if (bFeatureTrimmed) {
                // If this feat is a Cdregion, then RETRANSLATE the protein
                // sequence AND adjust any protein feature
                if ( new_feat->IsSetData() && 
                     new_feat->GetData().Which() == CSeqFeatData::e_Cdregion &&
                     new_feat->IsSetProduct() )
                {
                    x_AdjustCdregionFrame(scope,
                                          new_feat,
                                          sorted_cuts,
                                          seqid,
                                          bFeatureTrimmed,
                                          bProdTrimmed);

                    // In order to retranslate correctly, we need to create a 
                    // new scope with the trimmed sequence data.

                    // Keep track of original seqinst
                    CRef<objects::CSeq_inst> orig_inst(new objects::CSeq_inst());
                    orig_inst->Assign(bsh.GetInst());

                    // Update the seqinst to the trimmed version, set the scope
                    // and retranslate
                    CBioseq_EditHandle bseh = bsh.GetEditHandle();
                    bseh.SetInst(*new_inst);
                    CScope& new_scope = bseh.GetScope();
                    x_RetranslateCDS(new_scope, command, new_feat);

                    // Restore the original seqinst
                    bseh.SetInst(*orig_inst);
                }

                // Swap edited copy with original feature
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*feat_ci, 
                                                                   *new_feat));
                command->AddCommand(*chgFeat);
            }
        }
    }

    if (m_CitSub->GetValue()) {// add cit-sub update

        CConstRef<CSeqdesc> changedSeqdesc;
        CSeq_entry_Handle seh_for_desc;
        string msg;
        CRef<CSeqdesc> changedORadded_citsub = CCitSubUpdater::s_GetCitSubForTrimmedSequence(bsh, msg, changedSeqdesc, seh_for_desc);
        if (changedORadded_citsub) {
            if (changedSeqdesc && seh_for_desc) {
                CRef<CCmdChangeSeqdesc> change_cmd(new CCmdChangeSeqdesc(seh_for_desc, *changedSeqdesc, *changedORadded_citsub));
                command->AddCommand(*change_cmd);
            }
            else if (!changedSeqdesc) {
                CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
                CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
                if (bssh && 
                    bssh.CanGetClass() && 
                    bssh.GetClass() == CBioseq_set::eClass_nuc_prot) {
                    seh = bssh.GetParentEntry();
                }
                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(seh, *changedORadded_citsub));
                command->AddCommand(*cmdAddDesc);
            }
        }
    }
}


void CVectorTrimPanel::x_GetSortedCuts(TCuts& sorted_cuts,
                                       const CSeq_id* seqid)
{
    CScope& scope = m_TopSeqEntry.GetScope();
    CBioseq_Handle bsh = scope.GetBioseqHandle(*seqid);

    /***************************************************************************
     * Adjust internal cuts to 3' end
     * Merge abutting and overlapping cuts
     ***************************************************************************/
    CRangeCmp asc(CRangeCmp::eAscending);
    sort(sorted_cuts.begin(), sorted_cuts.end(), asc);

    // Adjust internal cuts 
    x_AdjustInternalCutLocations(sorted_cuts, bsh.GetBioseqLength());

    // Merge abutting and overlapping cuts
    x_MergeCuts(sorted_cuts);

    /***************************************************************************
     * Sort the cuts in descending order
    ***************************************************************************/
    // Sort the ranges from greatest to least so that sequence
    // data and metadata will be deleted from greatest loc to smallest loc.
    // That way we don't have to adjust coordinate values after 
    // each delete.
    CRangeCmp descend(CRangeCmp::eDescending);
    sort(sorted_cuts.begin(), sorted_cuts.end(), descend);
}


void CVectorTrimPanel::x_TrimAlignments(CRef<CCmdComposite> command,
                                        const TVecscreenSummaryBySeqid& trimOnly)
{
    // Adjust alignments
    SAnnotSelector align_sel(CSeq_annot::C_Data::e_Align);
    CAlign_CI align_ci(m_TopSeqEntry, align_sel);
    for (; align_ci; ++align_ci)
    {
        // So far, handle DENSEG type only
        const CSeq_align& align = *align_ci;
        if ( align.CanGetSegs() && 
             align.GetSegs().Which() == CSeq_align::C_Segs::e_Denseg )
        {
            // Make sure mandatory fields are present in the denseg
            const CDense_seg& denseg = align.GetSegs().GetDenseg();
            if (! (denseg.CanGetDim() && denseg.CanGetNumseg() && 
                   denseg.CanGetIds() && denseg.CanGetStarts() &&
                   denseg.CanGetLens()) )
            {
                continue;
            }

            // Make a copy of the alignment
            CRef<CSeq_align> new_align(new CSeq_align());
            new_align->Assign(align_ci.GetOriginalSeq_align());

            // Make edits to the copy
            TVecscreenSummaryBySeqid::const_iterator cit;
            for (cit = trimOnly.begin(); cit != trimOnly.end(); ++cit)
            {
                const CSeq_id* seqid = cit->first;
                const SVecscreenResult& result = cit->second;

                // On which "row" does the seqid lie?
                const CDense_seg::TIds& ids = denseg.GetIds();
                CDense_seg::TDim row = -1;
                for (int ii = 0; ii < ids.size(); ++ii) {
                    if ( ids[ii]->Match(*seqid) ) {
                        row = ii;
                        break;
                    }
                }
                if ( row < 0 || !denseg.CanGetDim() || row >= denseg.GetDim() ) {
                    continue;
                }

                TCuts sorted_cuts(result.m_arrRangeMatch);
                x_GetSortedCuts(sorted_cuts, seqid);
                x_AdjustDensegAlignment(new_align, row, sorted_cuts);
            }

            // Swap edited copy with the original alignment
            CIRef<IEditCommand> chgAlign(
                new CCmdChangeAlign( align_ci.GetSeq_align_Handle(), 
                                     *new_align ));
            command->AddCommand(*chgAlign);
        }
        else {
            // For other alignment types, delete them.  This is what
            // C Toolkit does (see AdjustAlignmentsInAnnot)
            CIRef<IEditCommand> delAlign(
                new CCmdDelSeq_align( align_ci.GetSeq_align_Handle() ));
            command->AddCommand(*delAlign);
        }
    }
}


void CVectorTrimPanel::x_UpdateSeqGraphLoc(CRef<CSeq_graph> new_graph, 
                                           const TCuts& sorted_cuts,
                                           const CSeq_id* seqid)
{
    for (int ii = 0; ii < sorted_cuts.size(); ++ii)
    {
        const SRangeMatch& cut = sorted_cuts[ii];
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();

        if (new_graph->CanGetLoc())
        {
            CRef<CSeq_graph::TLoc> new_loc(new CSeq_graph::TLoc);
            new_loc->Assign(new_graph->GetLoc());
            bool bDeleted = false;
            bool bTrimmed = false;
            x_SeqLocDelete(new_loc, from, to, seqid, bDeleted, bTrimmed);
            new_graph->SetLoc(*new_loc);
        }
    }
}


void CVectorTrimPanel::x_GetTrimCoordinates(const TCuts& sorted_cuts, 
                                            const CSeq_id* seqid, 
                                            TSeqPos& trim_start, 
                                            TSeqPos& trim_stop)
{
    CScope& scope = m_TopSeqEntry.GetScope();
    CBioseq_Handle bsh = scope.GetBioseqHandle(*seqid);

    // Set defaults
    trim_start = 0;
    trim_stop = bsh.GetInst().GetLength() - 1;

    // Assumptions :
    // All cuts have been sorted.  Internal cuts were converted to terminal.
    for (int ii = 0; ii < sorted_cuts.size(); ++ii)
    {
        const SRangeMatch& cut = sorted_cuts[ii];
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();

        // Left-side terminal cut.  Update trim_start if necessary.
        if ( from == 0 ) {
            if ( trim_start <= to ) {
                trim_start = to + 1;
            }
        }

        // Right-side terminal cut.  Update trim_stop if necessary.
        if ( to == bsh.GetInst().GetLength() - 1 ) {
            if ( trim_stop >= from ) {
                trim_stop = from - 1;
            }
        }
    }
}


void CVectorTrimPanel::x_TrimSeqGraphData(CRef<CSeq_graph> new_graph, 
                                          const CMappedGraph& orig_graph,
                                          const TCuts& sorted_cuts,
                                          const CSeq_id* seqid)
{
    // Get range that original seqgraph data covers
    TSeqPos graph_start = new_graph->GetLoc().GetStart(eExtreme_Positional);
    TSeqPos graph_stop = new_graph->GetLoc().GetStop(eExtreme_Positional);

    // Get range of trimmed sequence
    TSeqPos trim_start;
    TSeqPos trim_stop;
    x_GetTrimCoordinates(sorted_cuts, seqid, trim_start, trim_stop);

    // Determine range over which to copy seqgraph data from old to new
    TSeqPos copy_start = graph_start;
    if (trim_start > graph_start) {
        copy_start = trim_start;
    }
    TSeqPos copy_stop = graph_stop;
    if (trim_stop < graph_stop) {
        copy_stop = trim_stop;
    }

    // Copy over seqgraph data values.  Handle BYTE type only (see 
    // C Toolkit's GetGraphsProc function in api/sqnutil2.c)
    CSeq_graph::TGraph& dst_data = new_graph->SetGraph();
    dst_data.Reset();
    const CSeq_graph::TGraph& src_data = orig_graph.GetGraph();
    switch ( src_data.Which() ) {
    case CSeq_graph::TGraph::e_Byte:
        // Keep original min, max, axis
        dst_data.SetByte().SetMin(src_data.GetByte().GetMin());
        dst_data.SetByte().SetMax(src_data.GetByte().GetMax());
        dst_data.SetByte().SetAxis(src_data.GetByte().GetAxis());

        // Copy start/stop values are relative to bioseq coordinate system.
        // Change them so that they are relative to graph location.
        copy_start -= graph_start;
        copy_stop -= graph_start;

        // Update data values
        dst_data.SetByte().SetValues();
        dst_data.SetByte().SetValues().insert(
            dst_data.SetByte().SetValues().end(), 
            src_data.GetByte().GetValues().begin() + copy_start,
            src_data.GetByte().GetValues().begin() + copy_stop + 1);

        // Update numvals
        new_graph->SetNumval(copy_stop - copy_start + 1);

        // Update seqloc
        x_UpdateSeqGraphLoc(new_graph, sorted_cuts, seqid);
        break;
    default:
        break;
    }
}


void CVectorTrimPanel::x_TrimSeqGraphs(CRef<CCmdComposite> command,
                                       const TVecscreenSummaryBySeqid& trimOnly)
{
    SAnnotSelector graph_sel(CSeq_annot::C_Data::e_Graph);
    CGraph_CI graph_ci(m_TopSeqEntry, graph_sel);
    for (; graph_ci; ++graph_ci)
    {
        // Only certain types of graphs are supported.
        // See C Toolkit function GetGraphsProc in api/sqnutil2.c
        const CMappedGraph& graph = *graph_ci;
        if ( graph.IsSetTitle() && 
             (NStr::CompareNocase( graph.GetTitle(), "Phrap Quality" ) == 0 ||
              NStr::CompareNocase( graph.GetTitle(), "Phred Quality" ) == 0 ||
              NStr::CompareNocase( graph.GetTitle(), "Gap4" ) == 0) )
        {
            // Make a copy of the graph
            CRef<CSeq_graph> new_graph(new CSeq_graph());
            new_graph->Assign(graph.GetOriginalGraph());

            // Make edits to the copy
            TVecscreenSummaryBySeqid::const_iterator cit;
            for (cit = trimOnly.begin(); cit != trimOnly.end(); ++cit)
            {
                const CSeq_id* seqid = cit->first;
                const SVecscreenResult& result = cit->second;

                // Find matching seqid referred to by the graph
                if ( graph.GetLoc().GetId()->Match(*seqid) ) {
                    TCuts sorted_cuts(result.m_arrRangeMatch);
                    x_GetSortedCuts(sorted_cuts, seqid);

                    x_TrimSeqGraphData(new_graph, graph, sorted_cuts, seqid);

                    // Swap edited copy with the original graph
                    CIRef<IEditCommand> chgGraph(
                        new CCmdChangeGraph( graph.GetSeq_graph_Handle(),
                                             *new_graph ));
                    command->AddCommand(*chgGraph);
                    break;
                }
            }
        }
    }
}


bool CVectorTrimPanel::x_IsCompleteCut(const TCuts& sorted_cuts, 
                                       TSeqPos seq_length)
{
    // Assume cuts are sorted in Ascending order.


    // Special case:  no cuts
    if (sorted_cuts.empty()) {
        return false;
    }


    // Special case:  only one cut
    if (sorted_cuts.size() == 1) {
        const SRangeMatch& cut = sorted_cuts[0];
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();
        if (from == 0 && to == seq_length - 1) {
            return true;
        }
        else {
            return false;
        }
    }


    // 2 or more cuts exist
    TSeqPos min_from = -1;
    TSeqPos max_to = -1;
    for (int ii = 0; ii < sorted_cuts.size(); ++ii)
    {
        const SRangeMatch& cut = sorted_cuts[ii];
        TSeqPos from = cut.m_range.GetFrom();
        TSeqPos to = cut.m_range.GetTo();

        if (ii == 0) {
            min_from = from;
            max_to = to;
        }
        else {
            // If the current cut is discontiguous with previous cut(s)
            // then we do NOT have a complete cut
            if (from > max_to + 1) {
                // Gap exists, so this is a discontiguous cut
                return false;
            }

            min_from = min(min_from, from);
            max_to = max(max_to, to);
        }
    }

    if (min_from == 0 && max_to == seq_length - 1) {
        return true;
    }

    return false;
}


void CVectorTrimPanel::x_FindCompleteDeletions(const TVecscreenSummaryBySeqid& seqidSummary,
                                               TVecscreenSummaryBySeqid& completeDeletions,
                                               TVecscreenSummaryBySeqid& trimOnly)
{
    // Put complete deletions into completeDeletions container
    // Put partial cuts into trimOnly container

    CScope& scope = m_TopSeqEntry.GetScope();

    TVecscreenSummaryBySeqid::const_iterator cit;
    for (cit = seqidSummary.begin(); cit != seqidSummary.end(); ++cit)
    {
        const CSeq_id* seqid = cit->first;
        const SVecscreenResult& vecres = cit->second;

        // Will the sequence be completely deleted by the trim?
        CBioseq_Handle bsh = scope.GetBioseqHandle(*seqid);

        // Should be a nuc!
        if ( !bsh.IsNucleotide() ) {
            continue;
        }

        // Sort ranges in ascending order
        TCuts sorted_cuts(vecres.m_arrRangeMatch);
        CRangeCmp asc(CRangeCmp::eAscending);
        sort(sorted_cuts.begin(), sorted_cuts.end(), asc);

        // Merge abutting and overlapping cuts
        x_MergeCuts(sorted_cuts);

        // Adjust internal cuts 
        TSeqPos nuc_length = bsh.GetBioseqLength();
        x_AdjustInternalCutLocations(sorted_cuts, nuc_length);

        // Is it a complete cut of the entire sequence?
        if (x_IsCompleteCut(sorted_cuts, nuc_length)) {
            completeDeletions.insert(TVecscreenSummaryBySeqid::value_type(seqid, 
                                                                          vecres));
        }
        else {
            trimOnly.insert(TVecscreenSummaryBySeqid::value_type(seqid, 
                                                                 vecres));
        }
    }
}


void CVectorTrimPanel::x_DeleteSelected(CRef<CCmdComposite> command,
                                        const CSeq_id* seqid)
{
    CScope& scope = m_TopSeqEntry.GetScope();
    CBioseq_Handle bsh = scope.GetBioseqHandle(*seqid);

    // Should be a nuc!
    if (!bsh.IsNucleotide())
    {
        return;
    }

    // Is nuc bioseq part of a nucprot set?  If so, delete the nucprot
    // set.  Otherwise delete just the nuc.
    CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
    if (bssh && bssh.CanGetClass() && 
        bssh.GetClass() == CBioseq_set::eClass_nuc_prot) 
    {
        // Delete the nucprot set
        CRef<CCmdDelBioseqSet> delSet(new CCmdDelBioseqSet(bssh));
        command->AddCommand(*delSet);
    }
    else 
    {
        // Delete just the nuc
        CRef<CCmdDelBioseqInst> delSeq(new CCmdDelBioseqInst(bsh));
        command->AddCommand(*delSeq);
    }
}


bool CVectorTrimPanel::x_TrimSelected()
{

    const TVecscreenSummaryBySeqid& seqidSummary = 
        m_listMatches->GetSelectedVecscreenSummaryBySeqid();


    // Detect if some sequences are 100% contamination sequences.
    // Warn user that these sequences will be completely removed.
    // Give user chance to CANCEL.
    TVecscreenSummaryBySeqid completeDeletions;
    TVecscreenSummaryBySeqid trimOnly;
    x_FindCompleteDeletions(seqidSummary, completeDeletions, trimOnly);
    if ( !completeDeletions.empty() ) {
        CListReportDlg* listReport = new CListReportDlg(this);

        wxString header;
        header << "The following " << completeDeletions.size() 
               << " sequences are 100% matches to vector and \n"
               << "will be deleted from your submission:";
        listReport->SetHeader(header);

        wxString list;
        TVecscreenSummaryBySeqid::const_iterator cit;
        for (cit = completeDeletions.begin(); cit != completeDeletions.end(); ++cit)
        {
            const CSeq_id* seqid = cit->first;
            const SVecscreenResult& vecres = cit->second;

            list << seqid->AsFastaString() << "\n";
        }
        listReport->SetList(list);

        wxString footer;
        footer << "Do you want to continue?";
        listReport->SetFooter(footer);

        if ( listReport->ShowModal() == wxID_CANCEL ) {
            // User canceled
            return false;
        }
    }


    CRef<CCmdComposite> command( new CCmdComposite( "Trim Vector" ) );


    // Delete selected sequences
    TVecscreenSummaryBySeqid::const_iterator cit;
    for (cit = completeDeletions.begin(); cit != completeDeletions.end(); ++cit)
    {
        const CSeq_id* seqid = cit->first;

        x_DeleteSelected(command, seqid);
    }


    // Trim selected sequences
    for (cit = trimOnly.begin(); cit != trimOnly.end(); ++cit)
    {
        const CSeq_id* seqid = cit->first;
        const SVecscreenResult& result = cit->second;

        x_TrimSelected(command, seqid, result);
    }

    // Trim alignments
    x_TrimAlignments(command, trimOnly);

    // Trim seq-graphs
    x_TrimSeqGraphs(command, trimOnly);

    
    EInternalTrimType eInternalTrimType = 
        static_cast<EInternalTrimType>(m_radioBoxInternalTrimOptions->GetSelection());
    wxString report;
    m_listMatches->GetSelectedLocations(report, eInternalTrimType);
    if (!report.IsEmpty()) {
        // Popup report of locations trimmed.
        // Set parent window to NULL and add style wxDIALOG_NO_PARENT, so that the report stays up 
        // even when the vector trim dialog is closed.
        CVecscreenReport* vecscreenReport = new CVecscreenReport(GetGrandParent(), 
                                                                 wxID_ANY, 
                                                                 _("Trimmed Location(s)"),
                                                                 wxDefaultPosition,
                                                                 wxDefaultSize,
                                                                 wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX);
        vecscreenReport->SetReport(report);
        vecscreenReport->Show();
    }
    

    // Keep track of ALL commands executed so that later you can
    // unexecute them all if the user cancels
    m_CmdProcessor->Execute(command);
    return true;
}


void CVectorTrimPanel::
DisplaySummaryBySeqid(const TVecscreenSummaryBySeqid& 
                      vecscreen_summary)
{
    m_listMatches->UpdateData(vecscreen_summary);
}



IMPLEMENT_DYNAMIC_CLASS( CMatchesListCtrl, wxCheckedListCtrl )


BEGIN_EVENT_TABLE(CMatchesListCtrl, wxCheckedListCtrl)
    EVT_LIST_COL_CLICK(MYID_LISTCTRL_MATCHES, CMatchesListCtrl::OnColClick)
    EVT_LIST_ITEM_SELECTED(MYID_LISTCTRL_MATCHES, CMatchesListCtrl::OnSelected)
    EVT_LIST_ITEM_CHECKED(MYID_LISTCTRL_MATCHES, CMatchesListCtrl::OnChecked)
END_EVENT_TABLE()


CMatchesListCtrl::CMatchesListCtrl()
{
}


CMatchesListCtrl::CMatchesListCtrl(wxWindow *parent,
                                   const wxWindowID id,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style)
{
    Create(parent, id, pos, size, style);
}


CMatchesListCtrl::~CMatchesListCtrl()
{
}


bool CMatchesListCtrl::Create(wxWindow *parent, 
                              wxWindowID id,
                              const wxPoint &pos,
                              const wxSize &size, 
                              long style)
{
    wxCheckedListCtrl::Create(parent, id, pos, size, style);

    // Init state variables 
    m_SortOption = eSortInternal5Prime3Prime;
    m_SortAscending = true;
    m_SelectedRow = -1;

    // Set line item graphic properties
    m_listItemAttr.SetTextColour(*wxBLUE);
    m_listItemAttr.SetBackgroundColour(*wxLIGHT_GREY);
    m_listItemAttr.SetFont(wxNullFont);

    // Set up the column properties
   
    InsertColumn( 0, _("Select"), wxLIST_FORMAT_LEFT);
    InsertColumn( 1, _("Location"), wxLIST_FORMAT_LEFT);
    InsertColumn( 2, _("Seqid"), wxLIST_FORMAT_LEFT );
    InsertColumn( 3, _("Match"), wxLIST_FORMAT_LEFT );

    return true;
}


void CMatchesListCtrl::SetAlignGraphicView( wxHtmlWindow* view )
{
    m_AlignView = view;
}


void CMatchesListCtrl::SetRangeView( CLocationsListCtrl* view )
{
    m_RangeView = view;
}


bool CMatchesListCtrl::x_GetSelected(const CUVHitLoc& hitloc)
{
    return hitloc.m_Selected;
}


wxString CMatchesListCtrl::x_GetHitLocation(const CUVHitLoc& hitloc)
{
    switch(hitloc.m_HitLocation) {
        case CUVHitLoc::e5Prime:
            return _( "5' End" );
        case CUVHitLoc::eInternal:
        {
            if (hitloc.m_matches.size() != 1) {
                return _( "Internal" );
            }

            TSeqPos start = hitloc.m_matches[0].m_range.GetFrom();
            TSeqPos stop = hitloc.m_matches[0].m_range.GetTo();
            stringstream ss;
            ss << _( "Internal: " ) << start << _( " from 5' end, " )
               << hitloc.m_SeqLen - stop << _( " from 3' end" );
            return _( ss.str() );
        }
        case CUVHitLoc::e3Prime:
            return _( "3' End" );
        default:
            return _( "Unknown" );
    }
}


wxString CMatchesListCtrl::x_GetSeqid(const CUVHitLoc& hitloc)
{
    return _( hitloc.m_FastaSeqid );
}


wxString CMatchesListCtrl::x_GetMatchType(const CUVHitLoc& hitloc)
{
    // Concatenate the match type(s)
    stringstream ssMatchType;
    for (int ii = 0; ii < hitloc.m_matches.size(); ++ii)
    {
        if (ii > 0)
        {
            ssMatchType << "; ";
        }
        ssMatchType << hitloc.m_matches[ii].m_match_type;
    }
    return _( ssMatchType.str() );
}


// Note :
// TVecscreenSummaryBySeqid is CMatchesListCtrl's interface to the outside.
// Internally CMatchesListCtrl uses TVecscreenSummaryByLocation to display 
// data.
const TVecscreenSummaryBySeqid& CMatchesListCtrl::GetSelectedVecscreenSummaryBySeqid()
{
    // Transform internal TVecscreenSummaryByLocation selections to
    // TVecscreenSummaryBySeqid data type for callers
    x_ConvertSelectedToBySeqid(m_seqidSummaryByLocation, m_seqidSummaryBySeqid);
    return m_seqidSummaryBySeqid;
}


bool CMatchesListCtrl::UpdateData(const TVecscreenSummaryBySeqid& 
                                  vecscreen_summary)
{
    // Transform TVecscreenSummaryBySeqid to internal data type
    x_ConvertAllToByLocation(vecscreen_summary, m_seqidSummaryByLocation);

    // Clean slate
    DeleteAllItems();
    if (m_RangeView) {
        m_RangeView->DeleteAllItems();
    }

    // Display message if no vector contamination was found
    if (m_seqidSummaryByLocation.empty()) {
        wxListItem item;
        int row = 0;
        int col = 0;
        item.SetId(row);
        InsertItem(item);
        SetItem(row, col, _("No vector contamination found")); // TODO

        // Adjust the first column width
        wxListItem objcol;
        GetColumn(col, objcol);
        objcol.SetWidth(450);
        SetColumn(col, objcol);

        return true;
    }

    // Re-adjust the first column width
    int col = 0;
    wxListItem objcol;
    GetColumn(col, objcol);
    objcol.SetWidth(50);
    SetColumn(col, objcol);

    x_Redraw();

    return true;
}


void CMatchesListCtrl::x_ConvertSelectedToBySeqid(const TVecscreenSummaryByLocation& seqidSummaryByLocation, 
                                                  TVecscreenSummaryBySeqid& seqidSummaryBySeqid) const
{
    // Input container:  TVecscreenSummaryByLocation
    // Output container:  TVecscreenSummaryBySeqid

    // Combine matches separated by location into a single container for each seqid

    seqidSummaryBySeqid.clear();
    TVecscreenSummaryByLocation::const_iterator cit;
    for (cit = seqidSummaryByLocation.begin(); cit != seqidSummaryByLocation.end(); ++cit) {
        const CUVHitLoc& loc = *cit;

        if (!loc.m_Selected) {
            continue;
        }

        // Insert into output container.
        SVecscreenResult vecres(loc.m_FastaSeqid, loc.m_matches, loc.m_SeqLen);
        pair<TVecscreenSummaryBySeqid::iterator, bool> ins_res = 
            seqidSummaryBySeqid.insert(TVecscreenSummaryBySeqid::value_type(loc.m_Seqid, vecres));

        // If insert fails, append element to existing key's value
        if (!ins_res.second) {
            SVecscreenResult& vecres = ins_res.first->second;
            vecres.m_arrRangeMatch.insert(vecres.m_arrRangeMatch.end(),
                                          loc.m_matches.begin(),
                                          loc.m_matches.end());
        }
    }
}


void CMatchesListCtrl::x_InsertMatches(TVecscreenSummaryByLocation& seqidSummaryByLocation,
                                       const vector<SRangeMatch>& sorted_matches, 
                                       const CSeq_id* seqid,
                                       const SVecscreenResult& vecres) const
{ 
    if (sorted_matches.empty()) {
        return;
    }


    // Use sorted_matches to determine location: 5' or 3' or Internal 
    static const TSeqPos kTerminalLenFromEnd = 50;
    CUVHitLoc::EHitLoc location = CUVHitLoc::eUnknown;
    const TSeqPos loc_start = sorted_matches[0].m_range.GetFrom();
    const TSeqPos loc_stop = sorted_matches[sorted_matches.size()-1].m_range.GetTo();
    if (loc_start < kTerminalLenFromEnd) {
        location = CUVHitLoc::e5Prime;
    }
    else 
    if (vecres.m_SeqLen - loc_stop < kTerminalLenFromEnd) {
        location = CUVHitLoc::e3Prime;
    }
    else {
        location = CUVHitLoc::eInternal;
    }


    // Use location to determine selected: Internal hits should not be selected by default.
    bool selected = true;
    if (location == CUVHitLoc::eInternal) {
        selected = false;
    }


    CUVHitLoc hits(selected, location, vecres, seqid, sorted_matches); 
    seqidSummaryByLocation.push_back(hits);
}


void CMatchesListCtrl::x_ConvertAllToByLocation(const TVecscreenSummaryBySeqid& vecscreen_summary, 
                                                TVecscreenSummaryByLocation& seqidSummaryByLocation) const
{
    // Input container:  TVecscreenSummaryBySeqid
    // Output container:  TVecscreenSummaryByLocation

    // For each seqid, split matches out by location into separate containers.
    // Abutting locations are put into the same container.

    seqidSummaryByLocation.clear();
    TVecscreenSummaryBySeqid::const_iterator cit;
    for (cit = vecscreen_summary.begin(); cit != vecscreen_summary.end(); ++cit) {
        const CSeq_id* seqid = cit->first;
        SVecscreenResult vecres = cit->second;

        // Sort the matches in ascending order
        CRangeCmp asc(CRangeCmp::eAscending);
        sort(vecres.m_arrRangeMatch.begin(), vecres.m_arrRangeMatch.end(), asc);

        // If location is by itself, put into separate container.
        // If location is abutting previous location, put into same container as previous.
        int previous_to = -99;
        vector<SRangeMatch> previous_matches;
        for (int ii = 0; ii < vecres.m_arrRangeMatch.size(); ++ii) {
            const SRangeMatch& match = vecres.m_arrRangeMatch[ii];

            if (match.m_range.GetFrom() <= previous_to+1) {
                // This location abuts previous location!
                // Put into same container as previous.
                previous_matches.push_back(match);
            }
            else {
                // Put into new separate container.
                vector<SRangeMatch> current_matches;
                current_matches.push_back(match);

                // And insert any previous_matches into Output container TVecscreenSummaryByLocation
                x_InsertMatches(seqidSummaryByLocation, previous_matches, seqid, vecres);

                // Update previous_matches container
                previous_matches.swap(current_matches);
            }

            previous_to = match.m_range.GetTo();
        }

        // Insert remaining previous_matches into Output container TVecscreenSummaryByLocation
        x_InsertMatches(seqidSummaryByLocation, previous_matches, seqid, vecres);
    }
}


void CMatchesListCtrl::GetReport(wxString& report)
{
    if (m_seqidSummaryByLocation.empty()) {
        report << _("No vector contamination found");
        return;
    }

    // Generate report of current state of results
    TVecscreenSummaryByLocation::const_iterator cit;
    for (cit = m_seqidSummaryByLocation.begin(); cit != m_seqidSummaryByLocation.end(); ++cit)
    {
        const CUVHitLoc& hitloc = *cit;

        report << x_GetHitLocation(hitloc) << _(";")
               << x_GetSeqid(hitloc) << _(";")
               << x_GetMatchType(hitloc) << _("\n");
    }
}


void CMatchesListCtrl::GetSelectedLocations(wxString& report, CVectorTrimPanel::EInternalTrimType eInternalTrimType)
{
    // Sort a copy of the seqidSummaryByLocation data structure by
    // Accession for reporting purposes.
    CUVHitCmp cmp(eSortAccession, true);
    TVecscreenSummaryByLocation seqidSummaryByLocation(m_seqidSummaryByLocation);
    sort(seqidSummaryByLocation.begin(), seqidSummaryByLocation.end(), cmp);

    TVecscreenSummaryByLocation::const_iterator cit;
    for (cit = seqidSummaryByLocation.begin(); cit != seqidSummaryByLocation.end(); ++cit)
    {
        if (cit->m_Selected && !cit->m_matches.empty()) {
            // Seqid
            report << cit->m_FastaSeqid << _(":");

            const SRangeMatch& firstrange = cit->m_matches[0];
            const SRangeMatch& lastrange = cit->m_matches[cit->m_matches.size()-1];

            // Since abutting hits are kept together in the same container,
            // get the "from" of the first element and get the "to" of the last 
            // element to effectively MERGE abutting hits 
            TSeqPos from = firstrange.m_range.GetFrom();
            TSeqPos to = lastrange.m_range.GetTo();

            // Adjust the cut location reports for any internal cut 
            // adjustments that were made
            TSeqPos seq_length = cit->m_SeqLen;
            if (from != 0 && to != seq_length-1) {
                // We have an internal hit!
                // Internal hits are always adjusted to terminal using an option.

                if (eInternalTrimType == CVectorTrimPanel::eTrimToClosestEnd) {
                    // Extend the cut to the closest end
                    if (from - 0 < seq_length-1 - to) {
                        from = 0;
                    }
                    else {
                        to = seq_length-1;
                    }
                }
                else 
                if (eInternalTrimType == CVectorTrimPanel::eTrimTo5PrimeEnd) {
                    // Extend the cut to 5' end
                    from = 0;
                }
                else {
                    // Extend the cut to 3' end
                    to = seq_length-1;
                }
            }

            // Report the locations that were actually cut
            // Units are 1-based for reporting!
            report << from + 1 << "-" << to + 1 << _("\n");
        }
    }
}


void CMatchesListCtrl::Sort(ESortOption eSortOption, bool bAscending)
{
    m_SortOption = eSortOption;
    m_SortAscending = bAscending;

    x_Redraw();
}


bool CMatchesListCtrl::x_Find(const CUVHitLoc& hitloc, const wxString& searchstr)
{
    if (searchstr.empty()) {
        return false;
    }

    // Search hit location 
    wxString s = x_GetHitLocation(hitloc);
    if (s.Find(searchstr) != wxNOT_FOUND) {
        return true;
    }

    // Search Seqid 
    s = x_GetSeqid(hitloc);
    if (s.Find(searchstr) != wxNOT_FOUND) {
        return true;
    }

    // Search Strength 
    s = x_GetMatchType(hitloc);
    if (s.Find(searchstr) != wxNOT_FOUND) {
        return true;
    }

    return false;
}


bool CMatchesListCtrl::x_IsValidRow(int row)
{
    return row >= 0 && row < m_seqidSummaryByLocation.size();
}


void CMatchesListCtrl::x_DisplaySearchResult(bool bFound, int row)
{
    if (bFound) {
        // If found, select that row as if user clicked on it
        SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        m_SelectedRow = row;
    }
    else {
        // Popup a Not Found! dialog
        wxMessageDialog msg( NULL, _("Text not found!"), _("Info") );
        msg.ShowModal();
    }
}


void CMatchesListCtrl::FindForward(wxString searchstr)
{
    bool bFound = false;

    // Start search from next
    int row = m_SelectedRow + 1;
    if (!x_IsValidRow(row)) {
        // If not valid, init search from first row
        row = 0;
    }

    int initialSearchRow = row;
    for (; row < m_seqidSummaryByLocation.size();) {
        const CUVHitLoc& hitloc = m_seqidSummaryByLocation[row];
        bFound = x_Find(hitloc, searchstr);
        if (bFound) {
            break;
        }

        // If you've reached the end and your initial search row was not the
        // beginning, then wrap around to the beginning and search again
        if (row == m_seqidSummaryByLocation.size() - 1 && initialSearchRow > 0) {
            row = 0;
            initialSearchRow = row;
        }
        else {
            ++row;
        }
    }

    x_DisplaySearchResult(bFound, row);
}


void CMatchesListCtrl::FindReverse(wxString searchstr)
{
    bool bFound = false;
    
    // Start search from previous 
    int row = m_SelectedRow - 1;
    if (!x_IsValidRow(row)) {
        // If not valid, init search from last row
        row = m_seqidSummaryByLocation.size() - 1;
    }

    int initialSearchRow = row;
    for (; row >= 0;) {
        const CUVHitLoc& hitloc = m_seqidSummaryByLocation[row];
        bFound = x_Find(hitloc, searchstr);
        if (bFound) {
            break;
        }

        // If you've reached the beginning and your initial search row was not the
        // end, then wrap around to the end and search again
        if (row == 0 && initialSearchRow < m_seqidSummaryByLocation.size() - 1) {
            row = m_seqidSummaryByLocation.size() - 1;
            initialSearchRow = row;
        }
        else {
            --row;
        }
    }

    x_DisplaySearchResult(bFound, row);
}


bool CMatchesListCtrl::AllSelected()
{
    TVecscreenSummaryByLocation::const_iterator cit;
    for (cit = m_seqidSummaryByLocation.begin(); cit != m_seqidSummaryByLocation.end(); ++cit)
    {
        if (!cit->m_Selected) {
            return false;
        }
    }
    return true;
}


bool CMatchesListCtrl::CUVHitCmp::operator()(const CUVHitLoc& x, const CUVHitLoc& y) const
{
    return m_SortAscending ? 
        x_Compare(x, y) < 0 :
        x_Compare(x, y) > 0;
}


void CMatchesListCtrl::CUVHitCmp::x_FindStrongestMatchType(const vector<SRangeMatch>& matches, 
                                                           SRangeMatch::EMatchType& matchtype) const
{
    // Iterate through container and keep track of the strongest matchtype found 
    for (int ii = 0; ii < matches.size(); ++ii) {
        string matchstr = matches[ii].m_match_type;
        NStr::ToLower(matchstr);

        SRangeMatch::EMatchType current = SRangeMatch::eUnknown;
        if (matchstr == "strong") {
            current = SRangeMatch::eStrong;
        }
        else 
        if (matchstr == "moderate") {
            current = SRangeMatch::eModerate;
        }
        else 
        if (matchstr == "weak") {
            current = SRangeMatch::eWeak;
        }
        else 
        if (matchstr == "suspect") {
            current = SRangeMatch::eSuspect;
        }

        // Update to the strongest match defined by enum order
        if (current < matchtype) {
            matchtype = current;
        }
    }
}


int CMatchesListCtrl::CUVHitCmp::x_CompareMatches(const vector<SRangeMatch>& x, 
                                                  const vector<SRangeMatch>& y) const
{
    SRangeMatch::EMatchType xMatchType = SRangeMatch::eUnknown;
    x_FindStrongestMatchType(x, xMatchType);

    SRangeMatch::EMatchType yMatchType = SRangeMatch::eUnknown;
    x_FindStrongestMatchType(y, yMatchType);

    return xMatchType - yMatchType;
}


int CMatchesListCtrl::CUVHitCmp::x_Compare(const CUVHitLoc& x, 
                                           const CUVHitLoc& y) const
{
    switch (m_SortOption) {
        case eSortInternal5Prime3Prime:
        {
            int res = x.m_HitLocation - y.m_HitLocation;
            if (res != 0) {
                return res;
            }
            //else go to tiebreak
        }
        break;

        case eSortStrength:
        {
            int res = x_CompareMatches(x.m_matches, y.m_matches);
            if (res != 0) {
                return res;
            }
            //else go to tiebreak
        }
        break;

        case eSortMarked:
        {
            // Define ascending to be marked=true before marked=false
            if (x.m_Selected && !y.m_Selected) {
                return -1;
            }
            else if (!x.m_Selected && y.m_Selected) {
                return 1;
            }
            //else go to tiebreak
        }
        break;

        case eSortAccession:
        {
            return x.m_FastaSeqid.compare(y.m_FastaSeqid);
        }
    }

    // Tiebreak!
    return x.m_FastaSeqid.compare(y.m_FastaSeqid);
}


void CMatchesListCtrl::x_Redraw()
{
    // Sort
    CUVHitCmp cmp(m_SortOption, m_SortAscending);
    sort(m_seqidSummaryByLocation.begin(), m_seqidSummaryByLocation.end(), cmp);

    // Clean slate
    DeleteAllItems();

    // Set value in each column of each row
    TVecscreenSummaryByLocation::const_iterator cit;
    int row = 0;
    for (cit = m_seqidSummaryByLocation.begin(); cit != m_seqidSummaryByLocation.end(); ++cit)
    {
        const CUVHitLoc& hitloc = *cit;

        // Insert a row
        wxListItem item;
        item.SetId(row);
        InsertItem(item);

        // Column to indicate selected or not
        SetChecked(row, x_GetSelected(hitloc));
        SetItem( row, 0, wxEmptyString );

        // 5` or 3` End
        SetItem( row, 1, x_GetHitLocation(hitloc) );

        // Seqid
        SetItem( row, 2, x_GetSeqid(hitloc) );

        // Match type
        SetItem( row, 3, x_GetMatchType(hitloc) );

        ++row;
    }
    SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    SetColumnWidth(2, wxLIST_AUTOSIZE);
    SetColumnWidth(3, wxLIST_AUTOSIZE);
}


void CMatchesListCtrl::SelectAll()
{
    TVecscreenSummaryByLocation::iterator it;
    int row = 0;
    for (it = m_seqidSummaryByLocation.begin(); it != m_seqidSummaryByLocation.end(); ++it)
    {
        CUVHitLoc& hitloc = *it;

        hitloc.m_Selected = true;
    }

    x_Redraw();
}


void CMatchesListCtrl::SelectStrongModerate()
{
    TVecscreenSummaryByLocation::iterator it;
    int row = 0;
    for (it = m_seqidSummaryByLocation.begin(); it != m_seqidSummaryByLocation.end(); ++it)
    {
        CUVHitLoc& hitloc = *it;

        hitloc.m_Selected = false;
        for (int ii = 0; ii < hitloc.m_matches.size(); ++ii)
        {
            const SRangeMatch& rangematch = hitloc.m_matches[ii];
            if ( NStr::FindNoCase(rangematch.m_match_type, "strong") != NPOS || 
                 NStr::FindNoCase(rangematch.m_match_type, "moderate") != NPOS )
            {
                hitloc.m_Selected = true;
                break;
            }
        }
    }

    x_Redraw();
}


void CMatchesListCtrl::UnselectAll()
{
    TVecscreenSummaryByLocation::iterator it;
    int row = 0;
    for (it = m_seqidSummaryByLocation.begin(); it != m_seqidSummaryByLocation.end(); ++it)
    {
        CUVHitLoc& hitloc = *it;

        hitloc.m_Selected = false;
    }

    x_Redraw();
}


void CMatchesListCtrl::UnselectInternal()
{
    TVecscreenSummaryByLocation::iterator it;
    int row = 0;
    for (it = m_seqidSummaryByLocation.begin(); it != m_seqidSummaryByLocation.end(); ++it)
    {
        CUVHitLoc& hitloc = *it;

        if (hitloc.m_HitLocation == CUVHitLoc::eInternal) {
            hitloc.m_Selected = false;
        }
    }

    x_Redraw();
}


void CMatchesListCtrl::OnColClick(wxListEvent& event)
{
    // Use this callback to sort.

    // Column integer value is 0 based.
    int col = event.GetColumn();

    ESortOption eSortOption;
    switch (col) {
        case 0:
            eSortOption = eSortMarked;
            break;
        case 1:
            eSortOption = eSortInternal5Prime3Prime;
            break;
        case 2:
            eSortOption = eSortAccession;
            break;
        case 3:
            eSortOption = eSortStrength;
            break;
        default:
            eSortOption = eSortInternal5Prime3Prime;
            break;
    }

    // If sort option did not change, then toggle sort ascending
    if (m_SortOption == eSortOption) {
        m_SortAscending = m_SortAscending ? false : true;
    }
    else {
        m_SortOption = eSortOption;
    }

    x_Redraw();
}


void CMatchesListCtrl::OnSelected(wxListEvent& event)
{
    if ( GetWindowStyle() & wxLC_REPORT )
    {
        int selected_row = event.GetIndex();

        TVecscreenSummaryByLocation::const_iterator cit;
        int count = 0;
        for (cit = m_seqidSummaryByLocation.begin(); cit != m_seqidSummaryByLocation.end(); ++cit)
        {
            if (count == selected_row)
            {
                m_SelectedRow = selected_row;

                if (m_RangeView)
                {
                    m_RangeView->UpdateData(cit->m_FastaSeqid, cit->m_matches);
                }
                break;
            }

            ++count;
        }
    }
}

void CMatchesListCtrl::OnChecked(wxListEvent& event)
{
    int selected_row = event.GetIndex();
    m_seqidSummaryByLocation[selected_row].m_Selected = IsChecked(selected_row);
    event.Skip();
}



IMPLEMENT_DYNAMIC_CLASS( CLocationsListCtrl, wxListCtrl )


BEGIN_EVENT_TABLE(CLocationsListCtrl, wxListCtrl)
    EVT_LIST_ITEM_SELECTED(MYID_LISTCTRL_LOCATIONS, CLocationsListCtrl::OnSelected)
END_EVENT_TABLE()


CLocationsListCtrl::CLocationsListCtrl()
{
}


CLocationsListCtrl::CLocationsListCtrl(wxWindow *parent,
                                       const wxWindowID id,
                                       const wxPoint& pos,
                                       const wxSize& size,
                                       long style)
{
    Create(parent, id, pos, size, style);
}


CLocationsListCtrl::~CLocationsListCtrl()
{
}


bool CLocationsListCtrl::Create(wxWindow *parent, 
                                wxWindowID id,
                                const wxPoint &pos,
                                const wxSize &size, 
                                long style)
{
    wxListCtrl::Create(parent, id, pos, size, style);

    // Set line item graphic properties
    m_listItemAttr.SetTextColour(*wxBLUE);
    m_listItemAttr.SetBackgroundColour(*wxLIGHT_GREY);
    m_listItemAttr.SetFont(wxNullFont);

    // Set up the column properties
    wxListItem col0;
    col0.SetColumn(0);
    col0.SetText( _("Seqid") );
    col0.SetWidth(200);
    InsertColumn( 0, col0 );

    wxListItem col1;
    col1.SetColumn(1);
    col1.SetText( _("Range") );
    col1.SetWidth(220);
    InsertColumn( 1, col1 );

    return true;
}


bool CLocationsListCtrl::UpdateData(const string& fastaseqid,
                                    const vector<SRangeMatch>& matches)
{
    // Clean slate
    DeleteAllItems();

    // Set value in each column of each row
    for (int ii = 0; ii < matches.size(); ++ii)
    {
        const SRangeMatch& rangematch = matches[ii];

        // Insert a row
        wxListItem row;
        row.SetId(ii);
        InsertItem(row);

        // Seqid
        SetItem( ii, 0, _( fastaseqid ) );

        // Range
        stringstream ssRange;
        ssRange << rangematch.m_range.GetFrom() + 1 
                << "-" << rangematch.m_range.GetTo() + 1;
        SetItem( ii, 1, _( ssRange.str() ) );
    }

    return true;
}


void CLocationsListCtrl::OnSelected(wxListEvent& event)
{
    if ( GetWindowStyle() & wxLC_REPORT )
    {
        wxListItem info;
        info.m_itemId = event.GetIndex();

    }
}


IMPLEMENT_DYNAMIC_CLASS( CListReportDlg, wxDialog )


BEGIN_EVENT_TABLE( CListReportDlg, wxDialog )
END_EVENT_TABLE()


CListReportDlg::CListReportDlg()
{
    Init();
}

CListReportDlg::CListReportDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CListReportDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}


CListReportDlg::~CListReportDlg()
{
}


void CListReportDlg::Init()
{
}


void CListReportDlg::CreateControls()
{
    wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(dialogSizer);


    // Display header portion of dialog
    m_textHeader = new wxStaticText( this, 
                                     wxID_ANY,
                                     _( "" ),
                                     wxDefaultPosition,
                                     wxDefaultSize,
                                     wxALIGN_LEFT );
    dialogSizer->Add(m_textHeader, 0, wxEXPAND|wxALL, 5);


    // Display message list in a scrollable text field
    m_textList = new wxTextCtrl( this,
                                 wxID_ANY, 
                                 wxEmptyString, 
                                 wxDefaultPosition, 
                                 wxSize(600, 450),
                                 wxTE_MULTILINE|wxTE_READONLY );
    dialogSizer->Add(m_textList, 1, wxEXPAND|wxALL, 5);


    // Display footer portion of dialog
    m_textFooter = new wxStaticText( this, 
                                     wxID_ANY,
                                     _( "" ),
                                     wxDefaultPosition,
                                     wxDefaultSize,
                                     wxALIGN_LEFT );
    dialogSizer->Add(m_textFooter, 0, wxEXPAND|wxALL, 5);


    // Display Ok/Cancel buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okButton = new wxButton( this, wxID_OK, _( "Ok" ), wxDefaultPosition, wxDefaultSize, 0 );
    wxButton* cancelButton = new wxButton( this, wxID_CANCEL, _( "Cancel" ), wxDefaultPosition, wxDefaultSize, 0 );
    buttonSizer->Add(okButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    buttonSizer->Add(cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    dialogSizer->Add(buttonSizer, 0, wxEXPAND|wxALL, 5);
}


void CListReportDlg::SetHeader(const wxString& header)
{
    m_textHeader->SetLabel(header);
}


void CListReportDlg::SetList(const wxString& list)
{
    m_textList->SetValue(list);
}


void CListReportDlg::SetFooter(const wxString& footer)
{
    m_textFooter->SetLabel(footer);
}


IMPLEMENT_DYNAMIC_CLASS( CVecscreenReportPanel, wxPanel )


BEGIN_EVENT_TABLE( CVecscreenReportPanel, wxPanel )
END_EVENT_TABLE()


CVecscreenReportPanel::CVecscreenReportPanel()
{
    Init();
}

CVecscreenReportPanel::CVecscreenReportPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CVecscreenReportPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}


CVecscreenReportPanel::~CVecscreenReportPanel()
{
}


void CVecscreenReportPanel::Init()
{
}


void CVecscreenReportPanel::CreateControls()
{
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(panelSizer);


    // Display message list in a scrollable text field
    m_textReport = new wxTextCtrl( this,
                                 wxID_ANY, 
                                 wxEmptyString, 
                                 wxDefaultPosition, 
                                 wxSize(600, 450),
                                 wxTE_MULTILINE );
    panelSizer->Add(m_textReport, 1, wxEXPAND|wxALL, 5);
}


void CVecscreenReportPanel::SetReport(const wxString& report)
{
    m_textReport->SetValue(report);
}


IMPLEMENT_DYNAMIC_CLASS( CVecscreenReport, wxDialog )


BEGIN_EVENT_TABLE( CVecscreenReport, wxDialog )
END_EVENT_TABLE()


CVecscreenReport::CVecscreenReport()
{
    Init();
}

CVecscreenReport::CVecscreenReport( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CVecscreenReport::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}


CVecscreenReport::~CVecscreenReport()
{
}


void CVecscreenReport::Init()
{
}


void CVecscreenReport::CreateControls()
{
    wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(dialogSizer);

    m_panel = new CVecscreenReportPanel(this);
    dialogSizer->Add(m_panel, 1, wxEXPAND|wxALL, 5);
}


void CVecscreenReport::SetReport(const wxString& report)
{
    m_panel->SetReport(report);
}


END_NCBI_SCOPE

