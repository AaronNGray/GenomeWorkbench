/*  $Id: assembly_list_panel.cpp 43934 2019-09-20 20:19:13Z katargir $
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
 * Authors: Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/widgets/loaders/assembly_list_panel.hpp>
//#include <gui/widgets/loaders/assembly_term_completer.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>

#include <gui/objects/assembly_info.hpp>
#include <gui/objects/DL_AssmInfoResponse.hpp>

#include <objmgr/util/sequence.hpp>

#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/objects/DL_Assembly.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/widgets/wx/auto_complete_text_ctrl.hpp>
#include <gui/widgets/wx/string_history.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <wx/radiobut.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CAssemblyListPanel, wxPanel )
BEGIN_EVENT_TABLE( CAssemblyListPanel, wxPanel )

////@begin CAssemblyListPanel event table entries
    EVT_TEXT_ENTER( ID_TEXTCTRL7, CAssemblyListPanel::OnFindEnter )

    EVT_BUTTON( ID_BUTTON4, CAssemblyListPanel::OnFindGenomes )

    EVT_RADIOBUTTON( ID_ALLBUTTON, CAssemblyListPanel::OnAllbuttonSelected )

    EVT_RADIOBUTTON( ID_REFSEQBUTTON, CAssemblyListPanel::OnRefseqbuttonSelected )

    EVT_RADIOBUTTON( ID_GENBANKBUTTON, CAssemblyListPanel::OnGenbankbuttonSelected )

////@end CAssemblyListPanel event table entries
END_EVENT_TABLE()


static CStringHistory s_History(50, "Dialogs.AssemblyListPanel");

CAssemblyListPanel::CAssemblyListPanel()
{
    Init();
}

CAssemblyListPanel::CAssemblyListPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CAssemblyListPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAssemblyListPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAssemblyListPanel creation
    return true;
}

CAssemblyListPanel::~CAssemblyListPanel()
{
////@begin CAssemblyListPanel destruction
////@end CAssemblyListPanel destruction
}

void CAssemblyListPanel::Init()
{
////@begin CAssemblyListPanel member initialisation
    m_Term = NULL;
    m_ItemList = NULL;
    m_ReleaseTypeAll = NULL;
    m_ReleaseTypeRefSeq = NULL;
    m_ReleaseTypeGenBank = NULL;
////@end CAssemblyListPanel member initialisation

    m_ReleaseType = 0;  /// All
    m_MultiSelection = true;
}

void CAssemblyListPanel::CreateControls()
{
////@begin CAssemblyListPanel content construction
    CAssemblyListPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Enter a search term , then select assembly(ies) from the list."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Search term:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Term = new CAutoCompleteTextCtrl( itemPanel1, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(100, -1)).x, -1), 0 );
    if (CAssemblyListPanel::ShowToolTips())
        m_Term->SetToolTip(_("                  \"A valid search term can be one of the following:\n                  1. Organism keyword,  e.g. human, zebra fish, zebra*\n                  2. Taxonomy id,  e.g. 9606, 10116\n                  3. Assembly name,  e.g. hg19, GRCH37, NCBI*\n                  4. Sequence accession,  e.g. NC_000001.10, nt_077402.2\"\n                "));
    itemBoxSizer4->Add(m_Term, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemPanel1, ID_BUTTON4, _("Find Assemblies"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton7->SetDefault();
    itemBoxSizer4->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Found Assemblies"));
    wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer8, 1, wxGROW|wxALL, 5);

    m_ItemList = new CwxTableListCtrl( itemPanel1, ID_WINDOW, wxDefaultPosition, itemPanel1->ConvertDialogToPixels(wxSize(66, 61)), wxSIMPLE_BORDER );
    itemStaticBoxSizer8->Add(m_ItemList, 1, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer10Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Release type"));
    wxStaticBoxSizer* itemStaticBoxSizer10 = new wxStaticBoxSizer(itemStaticBoxSizer10Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer10, 0, wxALIGN_LEFT|wxALL, 5);

    m_ReleaseTypeAll = new wxRadioButton( itemPanel1, ID_ALLBUTTON, _("All"), wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(53, -1)).x, -1), wxRB_GROUP );
    m_ReleaseTypeAll->SetValue(true);
    itemStaticBoxSizer10->Add(m_ReleaseTypeAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ReleaseTypeRefSeq = new wxRadioButton( itemPanel1, ID_REFSEQBUTTON, _("RefSeq"), wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(53, -1)).x, -1), 0 );
    m_ReleaseTypeRefSeq->SetValue(false);
    itemStaticBoxSizer10->Add(m_ReleaseTypeRefSeq, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ReleaseTypeGenBank = new wxRadioButton( itemPanel1, ID_GENBANKBUTTON, _("GenBank"), wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(53, -1)).x, -1), 0 );
    m_ReleaseTypeGenBank->SetValue(false);
    itemStaticBoxSizer10->Add(m_ReleaseTypeGenBank, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAssemblyListPanel content construction
}

void CAssemblyListPanel::x_SetTextCompleter(wxTextCompleter* textCompleter)
{
    static bool autoCompleteSupported = true;
    if (autoCompleteSupported) {
        if (!m_Term->AutoComplete(textCompleter)) {
            autoCompleteSupported = false;
            LOG_POST(Error << "The platform doesn't support text autocomplete");
        }
    }
    else {
        delete textCompleter;
    }
}

void CAssemblyListPanel::SetMainTitle(const wxString& title)
{
    //m_MainBox->SetLabel(title);
}


void CAssemblyListPanel::SetDescr(const wxString& descr)
{
    //m_Descr->SetLabel(descr);
}

string CAssemblyListPanel::GetSearchTerm() const
{
    return ToStdString(m_Term->GetValue());
}


void CAssemblyListPanel::SetSearchTerm(const string& term)
{
    m_SearchTerm = term;
}


void CAssemblyListPanel::SetMultiSelection(bool flag)
{
    m_MultiSelection = flag;
    if ( !m_MultiSelection ) {
        m_ItemList->SetSingleStyle(wxLC_SINGLE_SEL);
    }
}


vector<string> CAssemblyListPanel::GetSelectedAssemblies() const
{
    vector<string> assms;
    vector<int> rows_selected;
    m_ItemList->GetDataRowsSelected(rows_selected);
    ITERATE (vector<int>, r_iter, rows_selected) {
        assms.push_back(m_Assms[*r_iter]);
    }

    return assms;
}

void CAssemblyListPanel::GetAssmNameAndDesc(const string& accession,
                                            string& name,
                                            string& description) const
{
    // Iterate over the whole list to find the selected accession (could go
    // by selected index too).  When found, get the (possibly concatenated)
    // name and the description.
    name.clear();
    description.clear();
    ITERATE (CDL_AssmInfo::TAssemblies, iter, m_OrigItems) {
        const CDL_Assembly& assm = **iter;

        if (assm.GetAccession() == accession) {
            name = assm.GetName();
            if (assm.CanGetOther_names()) {
                string other_names;
                bool first = true;
                ITERATE (CDL_Assembly::TOther_names, name_iter, assm.GetOther_names()) {
                    if (first) {
                        first = false;
                    } else {
                        other_names += "; ";
                    }
                    size_t pos = name_iter->find_first_of(":");
                    if (pos != string::npos) {
                        other_names += NStr::TruncateSpaces(name_iter->substr(pos + 1));
                    }
                }
                if ( !other_names.empty() ) {
                    name += " (" + other_names + ")";
                }
            }

            if (assm.CanGetDescr()) {
                description = assm.GetDescr();
            }
        }
    }
}


bool CAssemblyListPanel::IsInputValid()
{
    if (!m_ItemList->GetModel() || m_ItemList->GetModel()->GetNumColumns() <= 1) {
        wxMessageBox(wxT("Genome List is empty!"),
            wxT("Assembly selection - no selection"),
            wxOK | wxICON_ERROR, this);
        return false;
    }

    if (m_MultiSelection  &&  m_ItemList->GetSelectedItemCount() == 0) {
        wxMessageBox(wxT("Please select one or more assemblies!"),
            wxT("Assembly selection - no selection"),
            wxOK | wxICON_ERROR, this);
        return false;
    } else if ( !m_MultiSelection  &&  m_ItemList->GetSelectedItemCount() != 1) {
        wxMessageBox(wxT("Please select one assembly!"),
            wxT("Assembly selection - no selection"),
            wxOK | wxICON_ERROR, this);
        return false;
    }

    return true;
}


void CAssemblyListPanel::SetRegistryPath( const string& path )
{
    m_RegPath = path;
}

static const char* kReleaseTypeTag = "ReleaseType";

void CAssemblyListPanel::LoadSettings()
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        m_ReleaseType = view.GetInt(kReleaseTypeTag, m_ReleaseType);
        if (m_ReleaseType > 2  ||  m_ReleaseType < 0) {
            m_ReleaseType = 0;
        }
        switch (m_ReleaseType) {
            case 0:
                m_ReleaseTypeAll->SetValue(true);
                break;
            case 1:
                m_ReleaseTypeRefSeq->SetValue(true);
                break;
            case 2:
                m_ReleaseTypeGenBank->SetValue(true);
                break;
            default:
                break;
        }
    }

    s_History.Load();
    if (s_History.empty())
        s_History.AddString(wxT("human"));

    if (m_SearchTerm.empty())
        m_Term->SetValue(s_History.front());
    else
        m_Term->SetValue(m_SearchTerm);

    x_SetTextCompleter(s_History.CreateCompleter());

    x_UpdateSearchTerm();
}


void CAssemblyListPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        if (m_ItemList->GetModel() && m_ItemList->GetModel()->GetNumColumns() > 1) {
            m_ItemList->SaveTableSettings(view);
        }
        view.Set(kReleaseTypeTag, m_ReleaseType);
    }
}

void CAssemblyListPanel::x_ShowStatus(const string& status)
{
    if (!m_RegPath.empty() && 
        m_ItemList->GetModel() && m_ItemList->GetModel()->GetNumColumns() > 1) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        m_ItemList->SaveTableSettings(view);
    }

    CTextTableModel* model = new CTextTableModel();
    vector<wxString> columns;
    columns.push_back(wxT("Status"));

    model->Init(columns, 1);
    model->SetStringValueAt(0, 0, ToWxString(status));
    m_ItemList->SetModel(model, true);
    m_ItemList->SetColumnWidth(0, m_ItemList->GetClientSize().GetWidth());
}

void CAssemblyListPanel::x_UpdateListPanel()
{
    CDL_AssmInfo::TAssemblies items = m_OrigItems;
    if (m_ReleaseType != 0) {
        string release_str = "refseq";
        if (m_ReleaseType == 2) {
            release_str = "genbank";
        }
        CDL_AssmInfo::TAssemblies::iterator iter = items.begin();
        while (iter != items.end()) {
            if ( !(*iter)->IsSetRelease_type()  ||
                (*iter)->GetRelease_type() != release_str) {
                iter = items.erase(iter);
            } else {
                ++iter;
            }
        }
    }

    m_Assms.clear();
    ITERATE (CDL_AssmInfo::TAssemblies, iter, items) {
        m_Assms.push_back((*iter)->GetAccession());
    }

    vector<wxString> columns;
    columns.push_back(wxT("Name"));
    columns.push_back(wxT("Accession"));
    columns.push_back(wxT("Organism"));
    columns.push_back(wxT("Description"));
    columns.push_back(wxT("Class"));
    columns.push_back(wxT("Release type"));
    columns.push_back(wxT("Release date"));

    CTextTableModel* model = new CTextTableModel();
    model->Init(columns, (int)(items.size()));

    int row = 0;
    ITERATE (CDL_AssmInfo::TAssemblies, iter, items) {
        const CDL_Assembly& assm = **iter;
        int col = 0;
        string name = assm.GetName();
        if (assm.CanGetOther_names()) {
            string other_names;
            bool first = true;
            ITERATE (CDL_Assembly::TOther_names, name_iter, assm.GetOther_names()) {
                if (first) {
                    first = false;
                } else {
                    other_names += "; ";
                }
                size_t pos = name_iter->find_first_of(":");
                if (pos != string::npos) {
                    other_names += NStr::TruncateSpaces(name_iter->substr(pos + 1));
                }
            }
            if ( !other_names.empty() ) {
                name += " (" + other_names + ")";
            }
        }

        model->SetStringValueAt(row, col++, ToWxString(name));
        model->SetStringValueAt(row, col++, ToWxString(assm.GetAccession()));

        if (assm.CanGetOrganism()) {
            model->SetStringValueAt(row, col, ToWxString(assm.GetOrganism()));
        }
        ++col;
        if (assm.CanGetDescr()) {
            model->SetStringValueAt(row, col, ToWxString(assm.GetDescr()));
        }
        ++col;
        if (assm.CanGetClass()) {
            model->SetStringValueAt(row, col, ToWxString(assm.GetClass()));
        }
        ++col;
        if (assm.CanGetRelease_type()) {
            model->SetStringValueAt(row, col, ToWxString(assm.GetRelease_type()));
        }
        ++col;
        if (assm.CanGetRelease_date()) {
            model->SetStringValueAt(row, col, ToWxString(assm.GetRelease_date()));
        }

        ++row;
    }

    m_ItemList->Freeze();
    m_ItemList->SetModel(model, true);
    x_LoadTableSettings();

    if (items.size() == 1) {
        m_ItemList->SelectAll();
    }
    m_ItemList->TransferDataToWindow();
    m_ItemList->Thaw();
}


void CAssemblyListPanel::x_LoadTableSettings()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
    if (m_ItemList->GetModel() && m_ItemList->GetModel()->GetNumColumns() > 1) {
        m_ItemList->LoadTableSettings(view);
    }
}

void CAssemblyListPanel::OnAllbuttonSelected( wxCommandEvent& event )
{
    m_ReleaseType = 0;
    SaveSettings();
    if (m_ItemList->GetModel() && m_ItemList->GetModel()->GetNumColumns() > 1) {
        x_UpdateListPanel();
    }
}

void CAssemblyListPanel::OnRefseqbuttonSelected( wxCommandEvent& event )
{
    m_ReleaseType = 1;
    SaveSettings();
    if (m_ItemList->GetModel() && m_ItemList->GetModel()->GetNumColumns() > 1) {
        x_UpdateListPanel();
    }
}

void CAssemblyListPanel::OnGenbankbuttonSelected( wxCommandEvent& event )
{
    m_ReleaseType = 2;
    SaveSettings();
    if (m_ItemList->GetModel() && m_ItemList->GetModel()->GetNumColumns() > 1) {
        x_UpdateListPanel();
    }
}

void CAssemblyListPanel::OnFindGenomes( wxCommandEvent& event )
{
    x_OnFindGenomes();
}

void CAssemblyListPanel::OnFindEnter( wxCommandEvent& event )
{
    x_OnFindGenomes();
}

void CAssemblyListPanel::x_OnFindGenomes()
{
    if (m_Term->GetValue().IsEmpty()) {
        wxMessageBox(wxT("Please fill in the search term!"),
            wxT("Assembly query input - invalid input"),
            wxOK | wxICON_ERROR, this);
        return;
    }
    x_UpdateSearchTerm();
}

void CAssemblyListPanel::x_UpdateSearchTerm()
{
    wxString text = m_Term->GetValue();
    if (text.empty())
        return;

    m_SearchingFor = ToStdString(text);
    x_ShowStatus("Searching genomes for \"" + m_SearchingFor + "\"");

    m_Future.reset(job_async(
        [term=m_SearchingFor](ICanceled& canceled)
    {
        std::exception_ptr excp;
        CRef<CDL_AssmInfo> assm;

        LOG_POST(Info << "Retrieve assembly information for: " << term);
        try {
            assm = CAssemblyInfo::GetAssms_Term(term, NcbiEmptyString, &canceled);
            if (assm)
                return assm;
        }
        catch (const CException&) {
            excp = std::current_exception();
        }

        if (canceled.IsCanceled())
            return assm;

        try {
            CScope scope(*CObjectManager::GetInstance());
            scope.AddDefaults();

            CRef<CSeq_id> seq_id(new CSeq_id);
            seq_id->Set(term);
            CBioseq_Handle bsh = scope.GetBioseqHandle(*seq_id);
            if (bsh) {
                CSeq_id_Handle idh_gi = bsh.GetSeq_id_Handle();
                idh_gi = sequence::GetId(idh_gi, scope, sequence::eGetId_ForceGi);
                if (idh_gi) {
                    // get all GC links associated with the gi
                    assm = CAssemblyInfo::GetAssms_Gi(idh_gi.GetGi());
                }
            }
        }
        catch (const CException&) {
            if (!excp)
                excp = std::current_exception();
        }

        if (excp)
            std::rethrow_exception(excp);

        return assm;
    },
        [this](job_future<CRef<CDL_AssmInfo> >& future)
    {
        CRef<CDL_AssmInfo> assm;
        string errMsg;
        try {
            assm = future();
        }
        catch (const CException& e) {
            errMsg = e.GetMsg();
        }
        catch (const exception&) {
        }

        if (assm) {
            m_OrigItems = assm->GetAssemblies();
            x_UpdateListPanel();
            m_SearchTerm = m_SearchingFor;
            s_History.AddString(ToWxString(m_SearchingFor));
        }
        else {
            if (errMsg.empty())
                errMsg = "Can't find any assembly related to \"" + m_SearchingFor + "\"";
            x_ShowStatus(errMsg);
        }
    }
    , "Search Assemblies"));
}

bool CAssemblyListPanel::ShowToolTips()
{
    return true;
}

wxBitmap CAssemblyListPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAssemblyListPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAssemblyListPanel bitmap retrieval
}
wxIcon CAssemblyListPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAssemblyListPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAssemblyListPanel icon retrieval
}

END_NCBI_SCOPE
