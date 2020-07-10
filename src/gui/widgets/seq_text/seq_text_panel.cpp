/*  $Id: seq_text_panel.cpp 44433 2019-12-18 15:53:19Z shkeda $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Roman Katargin
 *
 */

#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/textctrl.h>
#include <wx/button.h>

#include <gui/widgets/seq_text/seq_text_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images

/*!
 * CSeqTextPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSeqTextPanel, wxPanel )


/*!
 * CSeqTextPanel event table definition
 */

BEGIN_EVENT_TABLE( CSeqTextPanel, wxPanel )

////@begin CSeqTextPanel event table entries
    EVT_BUTTON( ID_BUTTON, CSeqTextPanel::OnFindBwdClick )
    EVT_BUTTON( ID_BUTTON1, CSeqTextPanel::OnFindFwdClick )
    EVT_BUTTON( ID_BUTTON2, CSeqTextPanel::OnStopClick )
    EVT_CHOICE( ID_CHOICE1, CSeqTextPanel::OnFeatureTypeSelected )
////@end CSeqTextPanel event table entries

END_EVENT_TABLE()


/*!
 * CSeqTextPanel constructors
 */

CSeqTextPanel::CSeqTextPanel()
{
    Init();
}

CSeqTextPanel::CSeqTextPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSeqTextPanel creator
 */

bool CSeqTextPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
#ifdef __WXOSX_COCOA__ // GB-8581
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
#endif
////@begin CSeqTextPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqTextPanel creation
    return true;
}


/*!
 * CSeqTextPanel destructor
 */

CSeqTextPanel::~CSeqTextPanel()
{
////@begin CSeqTextPanel destruction
    // job is canceled if main window is destroyed
    if (m_JobAdapter) {
        m_JobAdapter->Cancel();
    }
////@end CSeqTextPanel destruction
}


/*!
 * Member initialisation
 */

void CSeqTextPanel::Init()
{
////@begin CSeqTextPanel member initialisation
    m_BwdButton = NULL;
    m_FwdButton = NULL;
    m_StopButton = NULL;
    m_MousePos = NULL;
    m_SeqTextWidget = NULL;
////@end CSeqTextPanel member initialisation
}


/*!
 * Control creation for CSeqTextPanel
 */

void CSeqTextPanel::CreateControls()
{
////@begin CSeqTextPanel content construction
    CSeqTextPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Find"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemTextCtrl5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BwdButton = new wxButton( itemPanel1, ID_BUTTON, _("Bwd"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_BwdButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FwdButton = new wxButton( itemPanel1, ID_BUTTON1, _("Fwd"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_FwdButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StopButton = new wxButton( itemPanel1, ID_BUTTON2, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StopButton->Enable(false);
    itemBoxSizer3->Add(m_StopButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Current Position:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MousePos = new wxStaticText( itemPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(100, -1)).x, -1), wxNO_BORDER );
    itemBoxSizer3->Add(m_MousePos, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("Show"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice12Strings;
    itemChoice12Strings.Add(_("None"));
    itemChoice12Strings.Add(_("Gene"));
    itemChoice12Strings.Add(_("CDS"));
    itemChoice12Strings.Add(_("mRNA"));
    itemChoice12Strings.Add(_("misc_RNA"));
    itemChoice12Strings.Add(_("STS"));
    wxChoice* itemChoice12 = new wxChoice( itemPanel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, itemChoice12Strings, 0 );
    itemChoice12->SetStringSelection(_("None"));
    itemBoxSizer3->Add(itemChoice12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SeqTextWidget = new CSeqTextWidget(itemPanel1, ID_WINDOW, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(300, 200)), wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(m_SeqTextWidget, 1, wxGROW|wxALL, 0);

////@end CSeqTextPanel content construction
    m_SeqTextWidget->Create();
    m_SeqTextWidget->SetHost(this);

    int subtype = m_SeqTextWidget->GetCaseFeatureSubtype();
    wxChoice* choice = (wxChoice*)FindWindow(ID_CHOICE1);

    switch (subtype) {
        case objects::CSeqFeatData::eSubtype_gene:
            choice->SetSelection (1);
            break;
        case objects::CSeqFeatData::eSubtype_cdregion:
            choice->SetSelection (2);
            break;
        case objects::CSeqFeatData::eSubtype_mRNA:
            choice->SetSelection (3);
            break;
        case objects::CSeqFeatData::eSubtype_misc_RNA:
            choice->SetSelection (4);
            break;
        case objects::CSeqFeatData::eSubtype_STS:
            choice->SetSelection (5);
            break;
    }
    m_SeqTextWidget->SetFocus();
}


/*!
 * Should we show tooltips?
 */

bool CSeqTextPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSeqTextPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqTextPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqTextPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSeqTextPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqTextPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqTextPanel icon retrieval
}

bool CSeqTextPanel::InitObject(SConstScopedObject& object)
{
    wxBusyCursor wait;

    const CSeq_id* id = dynamic_cast<const CSeq_id*>(object.object.GetPointer());
    if (id) {
        CBioseq_Handle handle = object.scope->GetBioseqHandle(*id);
        if (!handle) {
            string str;
            id->GetLabel(&str);
            ReportIDError(str, id->IsLocal(), "Sequence Text View");
            return false;
        }
        m_DataSource.Reset(new CSeqTextDataSource(handle, *object.scope));
        m_SeqTextWidget->SetDataSource (*m_DataSource);
        return true;
    }

    const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*>(object.object.GetPointer());
    if (seq_entry) {
        m_DataSource.Reset(new CSeqTextDataSource(*const_cast<CSeq_entry*>(seq_entry), *object.scope));
        m_SeqTextWidget->SetDataSource (*m_DataSource);
        return true;
    }

    const CBioseq_Handle* handle = dynamic_cast<const CBioseq_Handle*>(object.object.GetPointer());
    if (handle) {
        m_DataSource.Reset(new CSeqTextDataSource(*handle, *object.scope));
        m_SeqTextWidget->SetDataSource (*m_DataSource);
        return true;
    }

    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(object.object.GetPointer());
    if (loc && loc->GetId()) {
        auto handle = object.scope->GetBioseqHandle(*loc->GetId());
        if (!handle) {
            string str;
            loc->GetId()->GetLabel(&str);
            ReportIDError(str, loc->GetId()->IsLocal(), "Sequence Text View");
            return false;
        }

        m_DataSource.Reset(new CSeqTextDataSource(*const_cast<CSeq_loc*>(loc), *object.scope));
        m_SeqTextWidget->SetDataSource (*m_DataSource);
        return true;
    }

    return false;
}

void CSeqTextPanel::STWH_ReportMouseOverPos(TSeqPos pos)
{
    m_MousePos->SetLabel(ToWxString(NStr::IntToString(pos)));
}


void CSeqTextPanel::OnFeatureTypeSelected( wxCommandEvent& event )
{
    objects::CSeqFeatData::ESubtype subtype = objects::CSeqFeatData::eSubtype_bad;
    switch (event.GetInt()) {
        case 1:
            subtype = objects::CSeqFeatData::eSubtype_gene;
            break;
        case 2:
            subtype = objects::CSeqFeatData::eSubtype_cdregion;
            break;
        case 3:
            subtype = objects::CSeqFeatData::eSubtype_mRNA;
            break;
        case 4:
            subtype = objects::CSeqFeatData::eSubtype_misc_RNA;
            break;
        case 5:
            subtype = objects::CSeqFeatData::eSubtype_STS;
            break;
    }

    m_SeqTextWidget->ChooseCaseFeature (subtype);
}


void CSeqTextPanel::OnFindBwdClick( wxCommandEvent& event )
{
    m_FwdButton->Enable(false);
    m_BwdButton->Enable(false);
    m_StopButton->Enable(true);
    x_StartSearch(false);
}

void CSeqTextPanel::OnFindFwdClick( wxCommandEvent& event )
{
    m_FwdButton->Enable(false);
    m_BwdButton->Enable(false);
    m_StopButton->Enable(true);
    x_StartSearch(true);
}

void CSeqTextPanel::OnStopClick(wxCommandEvent& event)
{
    if (m_JobAdapter) m_JobAdapter->Cancel();
    m_SeqTextWidget->GetPane().ResetTextSearchData();
    x_RestoreButtonStates();
}

void CSeqTextPanel::x_RestoreButtonStates()
{
    m_FwdButton->Enable(true);
    m_BwdButton->Enable(true);
    m_StopButton->Enable(false);
}

namespace {
    struct SSeqSearchInput {
        string m_Term;
        bool m_Forward;
        CSeqTextWidget* m_TextWidget;

        SSeqSearchInput(const string& search_term, bool forward, CSeqTextWidget* widget)
            : m_Term(search_term), m_Forward(forward), m_TextWidget(widget) {}
    };

    struct SSeqSearchOutput {
        CSeqTextPane::CSeqTextSearch m_Result;
        string m_Term;
        bool m_Found;

        SSeqSearchOutput() : m_Term(kEmptyStr), m_Found(false) {}
    };
}

static bool s_SearchInSeqText(SSeqSearchInput& input, SSeqSearchOutput& output, string& error, ICanceled& canceled)
{
    try {
        LOG_POST(Info << "Searching for " << input.m_Term);
        
        output.m_Term = input.m_Term;
        output.m_Found = false;

        if (canceled.IsCanceled())
            return false;

        CSeqTextWidget* widget = input.m_TextWidget;
        CSeqTextDataSource* pDS = widget->GetDS();
        if (!pDS) 
            return false;


        CSeqTextPane& pane = widget->GetPane();
        output.m_Result = pane.GetTextSearchData(); // receive the data related to previous search from the pane
        CSeqTextPane::CSeqTextSearch& res = output.m_Result;

        if (!NStr::EqualNocase(res.m_LastSearch, input.m_Term)) {
            pDS->FindSequenceFragmentList(input.m_Term, res.m_FoundList, &canceled);
            if (canceled.IsCanceled()) {
                return false;
            }
            TSeqPos curr_scroll = pane.STG_GetSequenceByWindow(0, 0);
            res.m_CurrFindPos = 0;
            while (res.m_CurrFindPos < res.m_FoundList.size() && res.m_FoundList[res.m_CurrFindPos] < curr_scroll) {
                res.m_CurrFindPos++;
            }
            if (res.m_CurrFindPos >= res.m_FoundList.size()) {
                res.m_CurrFindPos = 0;
            }
        } else {
            // searching forward for the previous term
            if (input.m_Forward) {
                res.m_CurrFindPos++;
                if (res.m_CurrFindPos >= res.m_FoundList.size()) {
                    res.m_CurrFindPos = 0;
                }
            } else { // searching backwards for the previous term
                if (res.m_CurrFindPos > 0) {
                    --res.m_CurrFindPos;
                } else {
                    res.m_CurrFindPos = res.m_FoundList.size() - 1;
                }
            }
        }

        if (res.m_FoundList.size() > res.m_CurrFindPos) {
            widget->ScrollToPosition(res.m_FoundList[res.m_CurrFindPos]);
            output.m_Found = true;
        }
        
        res.m_LastSearch = input.m_Term;
    }
    catch (const CException& e) {
        LOG_POST(Error << e.GetMsg());
        return false;
    }
    catch (const std::exception& e) {
        LOG_POST(Error << e.what());
        return false;
    }
    return true;
}

void CSeqTextPanel::OnJobResult(CObject* result, CJobAdapter&)
{
    CJobAdapterResult<SSeqSearchOutput>* jobResult = dynamic_cast<CJobAdapterResult<SSeqSearchOutput>*>(result);
    if (jobResult) {
        const SSeqSearchOutput& res = jobResult->GetData();
        m_SeqTextWidget->GetPane().SetTextSearchData(res.m_Result);
        
        if (!res.m_Found) {
            NcbiInfoBox("Search string was not found");
        }
    }
    m_SeqTextWidget->UpdateLastSourcePos();
    m_SeqTextWidget->GetPane().Refresh();

    x_RestoreButtonStates();
}

void CSeqTextPanel::OnJobFailed(const string& errMsg, CJobAdapter&)
{
    string err_msg = "Failed: ";
    if (!errMsg.empty()) {
        err_msg += errMsg;
    }
    else {
        err_msg += "Unknown fatal error";
    }

    NcbiErrorBox(err_msg);
    m_SeqTextWidget->UpdateLastSourcePos();
    m_SeqTextWidget->GetPane().Refresh();
    x_RestoreButtonStates();
}

void CSeqTextPanel::x_StartSearch(bool forward)
{
    wxTextCtrl* find = (wxTextCtrl*)FindWindow(ID_TEXTCTRL1);
    string value = ToStdString(find->GetValue());
    value = NStr::TruncateSpaces(value);
    if (value.empty()) {
        NcbiInfoBox("The search string is empty.");
        x_RestoreButtonStates();
        return;
    }

    if (m_JobAdapter) m_JobAdapter->Cancel();

    SSeqSearchInput input(value, forward, m_SeqTextWidget);
    m_JobAdapter.Reset(LaunchAdapterJob<SSeqSearchInput, SSeqSearchOutput>
        (this, input, s_SearchInSeqText, "CSeqTextSearchJob", "Search for a string in Sequence Text View"));
}

END_NCBI_SCOPE
