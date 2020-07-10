/*  $Id: align_tab_export_page2.cpp 43773 2019-08-29 17:37:48Z katargir $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/checklst.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/settings.h>

#include "align_tab_export_page2.hpp"

#include <objmgr/align_ci.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/na_utils.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CAlignTabExportPage2, wxPanel )

BEGIN_EVENT_TABLE( CAlignTabExportPage2, wxPanel )

////@begin CAlignTabExportPage2 event table entries
////@end CAlignTabExportPage2 event table entries

    EVT_IDLE(CAlignTabExportPage2::OnIdle)
END_EVENT_TABLE()

CAlignTabExportPage2::CAlignTabExportPage2()
{
    Init();
}

CAlignTabExportPage2::CAlignTabExportPage2( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CAlignTabExportPage2::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAlignTabExportPage2 creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAlignTabExportPage2 creation
    return true;
}

CAlignTabExportPage2::~CAlignTabExportPage2()
{
////@begin CAlignTabExportPage2 destruction
////@end CAlignTabExportPage2 destruction
}

void CAlignTabExportPage2::Init()
{
////@begin CAlignTabExportPage2 member initialisation
    m_NoAlignments = true;
    m_Sizer = NULL;
    m_AlignList = NULL;
    m_FieldList = NULL;
    m_LoadingSizer = NULL;
////@end CAlignTabExportPage2 member initialisation
}

static const wxString sFields[] =
{
    wxT("Alignment"),
    wxT("Anchor"),
    wxT("Query"),
    wxT("Strand"),
    wxT("Aligned bases"),
    wxT("Segments"),
    wxT("Coverage"),
    wxT("Identity"),
    wxT("Mismatches"),
    wxT("Gaps")
};

void CAlignTabExportPage2::CreateControls()
{    
////@begin CAlignTabExportPage2 content construction
    CAlignTabExportPage2* itemPanel1 = this;

    m_Sizer = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(m_Sizer);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    m_Sizer->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Select alignment:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Select fields:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxArrayString m_AlignListStrings;
    m_AlignList = new wxListBox( itemPanel1, ID_LISTBOX, wxDefaultPosition, wxDefaultSize, m_AlignListStrings, wxLB_SINGLE );
    itemFlexGridSizer3->Add(m_AlignList, 0, wxGROW|wxGROW|wxALL, 5);

    wxArrayString m_FieldListStrings;
    m_FieldList = new wxCheckListBox( itemPanel1, ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, m_FieldListStrings, wxLB_SINGLE );
    itemFlexGridSizer3->Add(m_FieldList, 0, wxGROW|wxGROW|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(1);
    itemFlexGridSizer3->AddGrowableCol(0);
    itemFlexGridSizer3->AddGrowableCol(1);

    m_LoadingSizer = new wxBoxSizer(wxHORIZONTAL);
    m_Sizer->Add(m_LoadingSizer, 0, wxGROW|wxALL, 0);

////@end CAlignTabExportPage2 content construction

    wxStaticText* loadingText = new wxStaticText(this, ID_LOADING_TEXT, wxT("Loading alignments from Named Annotations ..."), wxDefaultPosition, wxDefaultSize, 0);
    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    loadingText->SetFont(bold);
    loadingText->SetForegroundColour(*wxBLACK);
    m_LoadingSizer->Add(loadingText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    CIndProgressBar* progress = new CIndProgressBar(this, ID_LOADING_PROGRESS, wxDefaultPosition, 100);
    m_LoadingSizer->Add(progress, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStaticText* errorText = new wxStaticText(this, ID_LOADING_ERROR, "NA DB error", wxDefaultPosition, wxDefaultSize, 0);
    errorText->SetForegroundColour(*wxRED);
    errorText->SetFont(bold);
    m_LoadingSizer->Add(errorText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_LoadingSizer->Hide(errorText);

    m_SaveTextClr = m_AlignList->GetForegroundColour();
    m_FieldList->Append(sizeof(sFields)/sizeof(sFields[0]), &sFields[0]);
}

bool CAlignTabExportPage2::ShowToolTips()
{
    return true;
}
wxBitmap CAlignTabExportPage2::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAlignTabExportPage2 bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAlignTabExportPage2 bitmap retrieval
}
wxIcon CAlignTabExportPage2::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAlignTabExportPage2 icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAlignTabExportPage2 icon retrieval
}

bool CAlignTabExportPage2::TransferDataToWindow()
{
    vector<string> fields = GetData().GetFields();
    if (fields.empty()) {
        for (size_t i = 0; i < m_FieldList->GetCount(); ++ i) {
            m_FieldList->Check(i, true);
        }
    } else {
        set<string> checked(fields.begin(), fields.end());
        for (size_t i = 0; i < m_FieldList->GetCount(); ++ i) {
            string str = ToStdString(m_FieldList->GetString(i));
            m_FieldList->Check(i, checked.find(str) != checked.end());
        }
    }

    m_NoAlignments = true;
    m_AlignList->Clear();
    m_AlignList->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    x_StartAlignNamesJob();

    return wxPanel::TransferDataToWindow();
}

void CAlignTabExportPage2::x_StartAlignNamesJob()
{
    m_TitleToAccession.clear();

    wxStaticText* loadingText = dynamic_cast<wxStaticText*>(FindWindow(ID_LOADING_TEXT));
    loadingText->SetLabelText("Loading alignment names ...");

    m_LoadingSizer->Show(FindWindow(ID_LOADING_TEXT));
    m_LoadingSizer->Show(FindWindow(ID_LOADING_PROGRESS));
    m_LoadingSizer->Hide(FindWindow(ID_LOADING_ERROR));

    m_Sizer->Layout();

    m_FutureNames = job_async([obj = GetData().GetObject()](ICanceled& canceled) mutable
    {
        static const string errTitle("Failed to get alignement names for ");
        string seqIdLabel;

        try {
            const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(obj.object.GetPointerOrNull());
            if (!seqLoc)
                NCBI_THROW(CException, eUnknown, "Invalid input object (Seq-loc expected)");

            CScope* scope = obj.scope;
            if (!scope)
                NCBI_THROW(CException, eUnknown, "Scope is NULL");

            CConstRef<CSeq_id> seqId(seqLoc->GetId());
            seqIdLabel = seqId->GetSeqIdString(true);

            SAnnotSelector sel(CSeqUtils::GetAnnotSelector(vector<string>()));
            CSeqUtils::SetResolveDepth(sel, true, -1);
            sel.SetCollectNames();

            vector<string> names;

            CAlign_CI aln_iter(*scope, *seqLoc, sel);

            if (canceled.IsCanceled())
                return names;

            ITERATE(CAlign_CI::TAnnotNames, iter, aln_iter.GetAnnotNames()) {
                if (iter->IsNamed()) {
                    if (iter->GetName().find("@@") == string::npos) {
                        names.emplace_back(iter->GetName());
                    }
                }
                else {
                    names.emplace_back(CSeqUtils::GetUnnamedAnnot());
                }
            }

            return names;
        }
        catch (const CException& e) {
            NCBI_REPORT_EXCEPTION(errTitle + seqIdLabel, e);
            throw;
        }
        catch (const exception& e) {
            CNcbiDiag() << Error << "[" << errTitle + seqIdLabel << "] Exception: " << e.what();
            throw;
        }
    },  "Get alignment names");
}

void CAlignTabExportPage2::x_StartNAAlignNamesJob()
{
    wxStaticText* loadingText = dynamic_cast<wxStaticText*>(FindWindow(ID_LOADING_TEXT));
    loadingText->SetLabelText("Loading Named Annotations alignments...");
    m_Sizer->Layout();

    m_FutureNANames = job_async([obj = GetData().GetObject()](ICanceled& canceled) mutable
    {
        typedef map<string, CRef<CAnnotMetaData> > TAnnotMap;

        static const string errTitle("Failed to NA meta-data for ");
        string seqIdLabel;

        try {
            const CSeq_loc* seqLoc = dynamic_cast<const CSeq_loc*>(obj.object.GetPointerOrNull());
            if (!seqLoc)
                NCBI_THROW(CException, eUnknown, "Invalid input object (Seq-loc expected)");

            CScope* scope = obj.scope;
            if (!scope)
                NCBI_THROW(CException, eUnknown, "Scope is NULL");

            static CFastMutex mutex;
            CFastMutexGuard lock(mutex);

            CConstRef<CSeq_id> seqId(seqLoc->GetId());
            seqIdLabel = seqId->GetSeqIdString(true);

            static TAnnotMap cachedAnnots;
            static CConstRef<CSeq_id> cachedSeqId;

            if (!cachedSeqId || CSeq_id_Handle::GetHandle(*cachedSeqId) != CSeq_id_Handle::GetHandle(*seqId)) {
                cachedSeqId.Reset();
                cachedAnnots.clear();

                LOG_POST(Info << "Retrieve NA meta-data for " << seqIdLabel);
                CStopWatch sw;
                sw.Start();
                CNAUtils::TNAMetaDataSet annots;
                CNAUtils na_util(*seqId, *scope);
                na_util.GetAllNAMetaData(annots);
                LOG_POST(Info << "Time for retrieving NA meta-data: " << sw.Elapsed() << " seconds");

                TAnnotMap& cacheRef = cachedAnnots;

                for_each(annots.begin(), annots.end(), [&cacheRef](CNAUtils::TNAMetaDataSet::value_type& v) {
                    if (v.second->m_AnnotType == "align") cacheRef[v.second->m_Name] = v.second;
                });

                cachedSeqId = seqId;
            }

            vector<tuple<string, string> > names;

            if (canceled.IsCanceled())
                return names;

            vector<string> alignNames;
            SAnnotSelector sel(CSeqUtils::GetAnnotSelector(vector<string>({ "NA*" }), true, -1));
            sel.SetCollectNames();
            CAlign_CI aln_iter(*scope, *seqLoc, sel);

            if (canceled.IsCanceled())
                return names;

            ITERATE(CAlign_CI::TAnnotNames, iter, aln_iter.GetAnnotNames()) {
                if (iter->IsNamed() && iter->GetName().find("@@") == string::npos) {
                    string name = iter->GetName();
                    TAnnotMap::const_iterator it = cachedAnnots.find(name);
                    if (it != cachedAnnots.end())
                        names.emplace_back(name, it->second->m_Title);
                    else
                        names.emplace_back(name, name);
                }
            }

            return names;
        }
        catch (const CException& e) {
            NCBI_REPORT_EXCEPTION(errTitle + seqIdLabel, e);
            throw;
        }
        catch (const exception& e) {
            CNcbiDiag() << Error << "[" << errTitle + seqIdLabel << "] Exception: " << e.what();
            throw;
        }
    }, "Get NA DB alignment names");
}

void CAlignTabExportPage2::x_FinishLoading()
{
    m_LoadingSizer->Hide(FindWindow(ID_LOADING_TEXT));
    m_LoadingSizer->Hide(FindWindow(ID_LOADING_PROGRESS));
    m_Sizer->Layout();
}

void CAlignTabExportPage2::OnIdle(wxIdleEvent& event)
{
    if (m_FutureNames.IsComplete()) {
        string errMsg;

        try {
            wxArrayString annotNames;
            string selAlignment, selAcc = GetData().GetAlignmentName();
            for (const string& i : m_FutureNames()) {
                annotNames.push_back(i);
                m_TitleToAccession[i] = i;
                if (selAcc == i)
                    selAlignment = i;
            }

            if (!annotNames.empty()) {
                m_AlignList->Append(annotNames);
                if (!selAlignment.empty())
                    m_AlignList->SetStringSelection(selAlignment);
                m_NoAlignments = false;
            }

            x_StartNAAlignNamesJob();
        }
        catch (const CException& e) {
            errMsg = e.GetMsg();
        }
        catch (const exception& e) {
            errMsg = e.what();
        }

        m_FutureNames.reset();

        if (!errMsg.empty())
            x_ReportError("Alignments error: " + errMsg);
    }
    else if (m_FutureNANames.IsComplete()) {
        string errMsg;

        try {
            wxArrayString annotNames;
            string selAlignment, selAcc = GetData().GetAlignmentName();

            for (const auto& i : m_FutureNANames()) {
                const string& accession = std::get<0>(i);
                const string& title = std::get<1>(i);

                if (m_TitleToAccession.find(title) != m_TitleToAccession.end())
                    continue;
                annotNames.push_back(title);
                m_TitleToAccession[title] = accession;
                if (selAcc == accession)
                    selAlignment = title;
            }

            if (!annotNames.empty()) {
                m_AlignList->Append(annotNames);
                if (!selAlignment.empty())
                    m_AlignList->SetStringSelection(selAlignment);
                m_NoAlignments = false;
            }

            if (m_AlignList->GetCount() == 0) {
                m_AlignList->SetForegroundColour(*wxRED);
                m_AlignList->Append(wxT("No alignments available"));
            }

            x_FinishLoading();
        }
        catch (const CException& e) {
            errMsg = e.GetMsg();
        }
        catch (const exception& e) {
            errMsg = e.what();
        }

        m_FutureNANames.reset();

        if (!errMsg.empty())
            x_ReportError("Named Annotations error: " + errMsg);
    }
    else {
        event.Skip();
    }
}

void CAlignTabExportPage2::x_ReportError(const string errMsg)
{
    m_LoadingSizer->Hide(FindWindow(ID_LOADING_TEXT));
    m_LoadingSizer->Hide(FindWindow(ID_LOADING_PROGRESS));

    wxStaticText* errorText = dynamic_cast<wxStaticText*>(FindWindow(ID_LOADING_ERROR));
    errorText->SetLabelText(errMsg);
    m_LoadingSizer->Show(FindWindow(ID_LOADING_ERROR));

    if (m_AlignList->GetCount() == 0) {
        m_AlignList->SetForegroundColour(*wxRED);
        m_AlignList->Append(wxT("No alignments available"));
    }

    m_Sizer->Layout();
}

bool CAlignTabExportPage2::TransferDataFromWindow()
{
    if (m_NoAlignments) {
        wxMessageBox(wxT("The location doesn't contain alignments.\n")
                     wxT("Please, go back and select another location."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        return false;
    }

    string alignName = m_TitleToAccession[ToStdString(m_AlignList->GetStringSelection())];
    if (alignName.empty()) {
        wxMessageBox(wxT("Please select an alignment"), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_AlignList->SetFocus();
        return false;
    }

    GetData().SetAlignmentName(alignName);

    vector<string> checked;
    for (size_t i = 0; i < m_FieldList->GetCount(); ++ i) {
        string str = ToStdString(m_FieldList->GetString(i));
        if (m_FieldList->IsChecked(i))
            checked.push_back(ToStdString(m_FieldList->GetString(i)));
    }

    if (checked.empty()) {
        wxMessageBox(wxT("Please, select some fields for export."), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        m_FieldList->SetFocus();
        return false;
    }

    GetData().SetFields(checked);

    return wxPanel::TransferDataFromWindow();
}

void CAlignTabExportPage2::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

void CAlignTabExportPage2::SaveSettings() const
{
}

void CAlignTabExportPage2::LoadSettings()
{
}

END_NCBI_SCOPE
