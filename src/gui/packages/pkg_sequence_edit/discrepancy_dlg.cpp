/*  $Id: discrepancy_dlg.cpp 44547 2020-01-17 18:31:19Z kachalos $
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
 * Authors: Sema
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbitime.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cds_edit.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_gene_edit.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_rna_edit.hpp>
#include <gui/packages/pkg_sequence_edit/custom_tree.hpp>
#include <gui/packages/pkg_sequence_edit/discrepancy_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/sequester_sets.hpp>
#include <gui/packages/pkg_sequence_edit/srceditdialog.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/seq_desktop/seq_desktop_panel.hpp>
#include <gui/widgets/seq_desktop/desktop_canvas.hpp>
#include <gui/widgets/seq_desktop/desktop_event.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/apply_object.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/seq_graphic/graphic_panel.hpp>
#include <wx/settings.h>
#include <wx/renderer.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/hyperlink.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(NDiscrepancy);
USING_SCOPE(objects);


class CDiscRepItem : public CCustomTreeItem
{
public:
    CDiscRepItem(CRef<CReportItem> item, const bool* tstnames, bool showfatal, CCustomTreeItem* parent = 0, bool active = false);
    string GetText() const;
    string GetStr() const;
    string GetTestName() const { return m_Item->GetTitle(); };
    CRef<CReportItem> GetItem() { return m_Item; }
    bool IsReal() const { return m_Item->IsReal(); }
    bool IsHead() const { return IsReal() && (!m_Parent || !((CDiscRepItem*)m_Parent)->IsReal()); }
protected:
    CRef<CReportItem> m_Item;
    const bool* m_ShowTestName;
    bool m_ShowFatal;
};


string CDiscRepItem::GetText() const
{
    if (IsHead() && *m_ShowTestName) {
        return m_Item->GetTitle() + ": " + (m_ShowFatal && m_Item->IsFatal() ? "FATAL! " + m_Item->GetMsg() : m_Item->GetMsg());
    }
    return m_ShowFatal && m_Item->IsFatal() ? "FATAL! " + m_Item->GetMsg() : m_Item->GetMsg();
}


string CDiscRepItem::GetStr() const
{
    string s;
    if (IsHead()) {
        s = m_Item->GetTitle() + " " + m_Item->GetStr();
    }
    else {
        s = m_Item->GetMsg();
    }
    while (true) { // remove all "[(] ... [)]"
        size_t n = s.find("[(]");
        if (n == string::npos) {
            break;
        }
        size_t k = s.find("[)]", n + 3);
        if (k != string::npos) {
            s = s.substr(0, n) + "###" + s.substr(k + 3);
        }
        else {
            s = s.substr(0, n) + "###";
            break;
        }
    }
    return s;
}


CDiscRepItem::CDiscRepItem(CRef<CReportItem> item, const bool* tstnames, bool showfatal, CCustomTreeItem* parent, bool active) : CCustomTreeItem(parent), m_ShowTestName(tstnames), m_ShowFatal(showfatal), m_Item(item)
{
    m_Active = active || item->CanAutofix();
    vector<CRef<CReportItem> > sub = item->GetSubitems();
    for (auto it : sub) {
        m_List.push_back(new CDiscRepItem(it, tstnames, showfatal, this));
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(CDiscrepancyPanel, wxPanel)

BEGIN_EVENT_MAP(CDiscrepancyPanel::CDiscrepancyHelper, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CDiscrepancyPanel::CDiscrepancyHelper::OnJobNotification)
END_EVENT_MAP()

BEGIN_EVENT_TABLE(CDiscrepancyPanel, wxPanel)
    EVT_LISTBOX(ID_DISCR_LEFT, CDiscrepancyPanel::OnClick)
    EVT_LISTBOX_DCLICK(ID_DISCR_LEFT, CDiscrepancyPanel::OnDblClk)
    EVT_LISTBOX(ID_DISCR_RIGHT, CDiscrepancyPanel::OnListClk)
    EVT_LISTBOX_DCLICK(ID_DISCR_RIGHT, CDiscrepancyPanel::OnListDblClk)
    EVT_TEXT(ID_DISCR_FIND_TEXT, CDiscrepancyPanel::OnFindText)
    EVT_BUTTON(ID_DISCR_NEXT, CDiscrepancyPanel::OnFindNext)
    EVT_BUTTON(ID_DISCR_PREV, CDiscrepancyPanel::OnFindPrev)
END_EVENT_TABLE()


CDiscrepancyPanel::CDiscrepancyPanel(CDiscrepancyDlg::EReportType type, wxWindow* parent, CDiscrepancyPanelHost* host, IWorkbench* workbench, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: m_Type(type), m_Host(host), m_Workbench(workbench), m_SelectionClient(new CSelectionClient("Discrepancy Report")),  m_Helper(new CDiscrepancyPanel::CDiscrepancyHelper(this))
{
    m_Workbench->GetServiceByType<CSelectionService>()->AttachClient(m_SelectionClient);
    Init();
    Create(parent, id, pos, size, style);
}


CDiscrepancyPanel::~CDiscrepancyPanel() 
{
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) sel_srv->DetachClient(m_SelectionClient);
}


bool CDiscrepancyPanel::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    wxPanel::Create(parent, id, pos, size, style);
    wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
    if (attr.colBg != wxNullColour) {
        SetOwnBackgroundColour(attr.colBg);
    }
    CreateControls();
    return true;
}


void CDiscrepancyPanel::CreateControls()
{
    wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
    SetSizer(box);

    wxBoxSizer* box1 = new wxBoxSizer(wxHORIZONTAL);
    m_Left = new CCustomTree(this, ID_DISCR_LEFT, RunningInsideNCBI(), wxDefaultPosition, wxSize(320, 300), wxBORDER_SUNKEN | wxVSCROLL);
    m_Left->SetScrollbar(wxVERTICAL, 0, 0, 0);
    m_Left->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    box1->Add(m_Left, 1, wxGROW | wxALL, 5);
    m_Right = new wxListBox(this, ID_DISCR_RIGHT, wxDefaultPosition, wxSize(320, 300), 0, 0, wxLB_HSCROLL);
    box1->Add(m_Right, 1, wxGROW | wxALL, 5);

    box->Add(box1, 1, wxGROW | wxALL, 5);

    wxFont font(10, wxFONTFAMILY_MODERN, wxNORMAL, wxNORMAL);
    m_Left->SetFont(font);
    m_Right->SetFont(font);

    wxBoxSizer* box2 = new wxBoxSizer(wxHORIZONTAL);
    box->Add(box2, 0, wxCENTER | wxALL, 5);

    if (!RunningInsideNCBI()) {
        auto label = new wxStaticText(this, 0, "Search", wxDefaultPosition, wxSize(50, -1), 0);
        box2->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    }

    m_PrevBtn = new wxButton(this, ID_DISCR_PREV, "<<", wxDefaultPosition, wxSize(30, -1), 0);
    box2->Add(m_PrevBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_FindText = new wxTextCtrl(this, ID_DISCR_FIND_TEXT, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0);
    box2->Add(m_FindText, 1, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_NextBtn = new wxButton(this, ID_DISCR_NEXT, ">>", wxDefaultPosition, wxSize(30, -1), 0);
    box2->Add(m_NextBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_PrevBtn->Disable();
    m_NextBtn->Disable();

    m_Loading = new wxStaticText(this, 0, " Loading...");
    m_Loading->SetForegroundColour(*wxRED);
    m_Loading->SetFont(font);
    m_Loading->Hide();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(CDiscrepancyDlg, wxFrame)

BEGIN_EVENT_MAP(CDiscrepancyDlg, CEventHandler)
    ON_EVENT(CProjectViewEvent, CViewEvent::eProjectChanged, &CDiscrepancyDlg::OnDataChanged)
END_EVENT_MAP()

BEGIN_EVENT_TABLE(CDiscrepancyDlg, wxFrame)
    EVT_BUTTON(ID_DISCR_EXPAND, CDiscrepancyDlg::OnExpand)
    EVT_BUTTON(ID_DISCR_CONTRACT, CDiscrepancyDlg::OnContract)
    EVT_BUTTON(ID_DISCR_REFRESH, CDiscrepancyDlg::OnRefresh)
    EVT_BUTTON(ID_DISCR_CHECKALL, CDiscrepancyDlg::OnCheckAll)
    EVT_BUTTON(ID_DISCR_CHECKFIXABLE, CDiscrepancyDlg::OnCheckFixable)
    EVT_BUTTON(ID_DISCR_UNCHECKALL, CDiscrepancyDlg::OnUncheckAll)
    EVT_BUTTON(ID_DISCR_AUTOFIX, CDiscrepancyDlg::OnAutofix)
    EVT_BUTTON(ID_DISCR_REPORT, CDiscrepancyDlg::OnReport)
    EVT_BUTTON(ID_DISCR_TESTNAME, CDiscrepancyDlg::OnTestName)
    EVT_BUTTON(ID_DISCR_SETTINGS, CDiscrepancyDlg::OnSettings)
    EVT_BUTTON(ID_DISCR_SEQUESTER, CDiscrepancyDlg::OnSequester)
    EVT_BUTTON(ID_DISCR_CLOSE, CDiscrepancyDlg::OnClose)
    EVT_CHECKBOX(ID_DISCR_REOPEN, CDiscrepancyDlg::OnReopen)
END_EVENT_TABLE()


IAppTask::ETaskState COpenDiscrepancyTask::x_Run()
{
    CDiscrepancyDlg::OpenDiscrepancyDlg(m_Type, *m_Proj.GetScope(), &m_Workbench);
    return eCompleted;
}


CDiscrepancyDlg* CDiscrepancyDlg::OpenDiscrepancyDlg(EReportType type, CScope& scope, IWorkbench* workbench, const wxString& workDir)
{
    CProjectService* prj_srv = workbench->GetServiceByType<CProjectService>();
    CGBProjectHandle* proj = prj_srv->GetGBWorkspace()->GetProjectFromScope(scope);
    if (!proj) {
        return 0;
    }
    wxWindowList children = workbench->GetMainWindow()->GetChildren();
    for (wxWindowList::compatibility_iterator it = children.GetFirst(); it; it = it->GetNext()) {
        wxWindow *current = (wxWindow*)it->GetData();
        CDiscrepancyDlg* dlg = dynamic_cast<CDiscrepancyDlg*>(current);
        if (dlg && dlg->m_Type == type &&  dlg->m_Project->GetScope() == &scope) {
            dlg->Raise();
            return dlg;
        }
    }

    CDiscrepancyDlg* dlg = new CDiscrepancyDlg(type, workbench->GetMainWindow(), workbench);
    dlg->m_Project = proj;
    dlg->m_CmdProccessor = &dynamic_cast<CGBDocument*>(proj)->GetUndoManager();  
    dynamic_cast<CGBDocument*>(proj)->AddListener(dlg);
    dlg->SetWorkDir(workDir);
    dlg->Show();
    dlg->RunTests();
    return dlg;
}


CDiscrepancyDlg::CDiscrepancyDlg(EReportType type, wxWindow* parent,  IWorkbench* workbench, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : m_Type(type), m_Workbench(workbench)
{
    Init();
    Create(parent, id, pos, size, style);
}


CDiscrepancyDlg::~CDiscrepancyDlg()
{   
}


string CDiscrepancyPanel::RegisterRoot(bool separate)
{
    if (separate) {
        switch (m_Type) {
            case CDiscrepancyDlg::eNormal:
                return "Dialogs.Edit.Discrepancy";
            case CDiscrepancyDlg::eOncaller:
                return "Dialogs.Edit.Oncaller";
            case CDiscrepancyDlg::eSubmitter:
                return "Dialogs.Edit.Submitter";
            case CDiscrepancyDlg::eMega:
                return "Dialogs.Edit.Megareport";
            case CDiscrepancyDlg::eCdsTrnaOverlap:
                return "Dialogs.Edit.CdsTrnaOverlap";
        }
    }
    return "Dialogs.Edit.Discrepancy";
}

string CDiscrepancyDlg::RegisterRoot(bool separate)
{
    return m_Panel->RegisterRoot(separate);
}


wxString CDiscrepancyPanel::GetSettingsPath()
{
    return CSysPath::ResolvePath(wxT("<home>/discrepancy_report_settings.asn"));
}


wxString CDiscrepancyDlg::GetSettingsPath()
{
    return CSysPath::ResolvePath(wxT("<home>/discrepancy_report_settings.asn"));
}


static const int REGISTER_MAGIC_NUMBER_DISCREPANCY = CGuiRegistry::ePriority_Local - 3;

void CDiscrepancyDlg::WriteSettings(CGuiRegistry& gui_reg) { m_Panel->WriteSettings(gui_reg);  }

void CDiscrepancyPanel::WriteSettings(CGuiRegistry& gui_reg)
{
    wxString path = GetSettingsPath();
    if (!path.IsEmpty()) {
        CNcbiOfstream ostr(path.fn_str());
        gui_reg.Write(ostr, REGISTER_MAGIC_NUMBER_DISCREPANCY);
    }
}


void CDiscrepancyDlg::LoadSettings()
{
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    wxString path = GetSettingsPath();
    if (!path.IsEmpty() && wxFileName::FileExists(path)) {
        CNcbiIfstream istr(path.fn_str());
        if (istr) {
            reg.AddSite(istr, REGISTER_MAGIC_NUMBER_DISCREPANCY);
        } 
    }

    CRegistryReadView reg_view_common = reg.GetReadView(RegisterRoot(false));
    m_AutoReopen = reg_view_common.GetBool("AutoReopen", m_Type == eOncaller);
    m_Panel->m_ShowTestName = reg_view_common.GetBool("ShowTestNames");
    string fname = reg_view_common.GetString("ProductRules");
    CFile file(fname);
    m_Panel->m_ProductRules = file.Exists() ? fname : "";

    CRegistryReadView reg_view = reg.GetReadView(RegisterRoot());
    reg_view.GetStringVec("Add", m_Panel->m_AddTests);
    reg_view.GetStringVec("Rem", m_Panel->m_RemoveTests);

    CRegistryReadView reg_view_rect = reg.GetReadView(RegisterRoot() + ".Rect");
    m_RectX = reg_view_rect.GetInt("x", m_RectX);
    m_RectY = reg_view_rect.GetInt("y", m_RectY);
    m_RectW = reg_view_rect.GetInt("width", 900);
    m_RectH = reg_view_rect.GetInt("height", 600);

    CRegistryReadView reg_view_tree = reg.GetReadView(RegisterRoot() + ".Tree");
    CRegistryReadView::TKeys keys;
    reg_view_tree.GetKeys(keys);
    for (auto& it : keys) {
        m_Panel->m_TreeState[it.key] = reg_view_tree.GetInt(it.key, 0);
    }
}


static string ProductRulesStr(const string& pr)
{
    return "Product Rules: " + (pr.empty() ? " #####  Built-In  #####" : pr);
}


bool CDiscrepancyDlg::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    string caption;
    switch (m_Type) {
        case CDiscrepancyDlg::eNormal:
            caption = "Discrepancy Report";
            break;
        case CDiscrepancyDlg::eOncaller:
            caption = "Oncaller Report";
            break;
        case CDiscrepancyDlg::eSubmitter:
            caption = "Submitter Report";
            break;
        case CDiscrepancyDlg::eMega:
            caption = "Mega Report";
            break;
        case CDiscrepancyDlg::eCdsTrnaOverlap:
            caption = "CDS tRNA Overlap";
            break;
    }
    wxFrame::Create( parent, id, caption, pos, size, style );
    wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
    if (attr.colBg != wxNullColour) {
        SetOwnBackgroundColour(attr.colBg);
    }
    CreateControls();
    Centre();
    wxRect rc = GetRect();
    m_RectX = rc.x;
    m_RectY = rc.y;
    m_RectW = rc.width;
    m_RectH = rc.height;
    LoadSettings();
    if (m_TestNameBtn) {
        m_TestNameBtn->SetLabel(m_Panel->m_ShowTestName ? "Hide Test Names" : "Show Test Names");
    }

    rc.x = m_RectX;
    rc.y = m_RectY;
    rc.width = m_RectW;
    rc.height = m_RectH;
    SetSize(rc, wxSIZE_USE_EXISTING);

    if (m_Type != CDiscrepancyDlg::eCdsTrnaOverlap && RunningInsideNCBI()) {
        m_ProductRulesLabel->SetLabel(ProductRulesStr(m_Panel->m_ProductRules));
    }
    if (m_Reopen) {
        m_Reopen->SetValue(m_AutoReopen);
    }

    NEditingStats::ReportUsage(caption);
    return true;
}


void CDiscrepancyDlg::Init() 
{
    m_Panel = nullptr;
    m_ExpandBtn = nullptr;
    m_ContractBtn = nullptr;
    m_TestNameBtn = nullptr;
    m_RefreshBtn = nullptr;
    m_UncheckAllBtn = nullptr;
    m_CheckFixableBtn = nullptr;
    m_AutofixBtn = nullptr;
    m_ReportBtn = nullptr;
    m_SettingsBtn = nullptr;
    m_SequesterBtn = nullptr;
    m_TrimBtn = nullptr;
    m_PrevBtn = nullptr;
    m_NextBtn = nullptr;
    m_Reopen = nullptr;
    m_FindText = nullptr;
    m_ProductRulesLabel = nullptr; 
}

void CDiscrepancyPanel::Init()
{
    if (m_Type == CDiscrepancyDlg::eNormal || m_Type == CDiscrepancyDlg::eMega || m_Type == CDiscrepancyDlg::eSubmitter) {
        CRef<CDiscrepancyGroup> G;
        G.Reset(new CDiscrepancyGroup("", "MISSING_GENES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "EXTRA_GENES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_LOCUS_TAGS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DUPLICATE_LOCUS_TAGS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BAD_LOCUS_TAG_FORMAT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INCONSISTENT_LOCUS_TAG_PREFIX")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "NON_GENE_LOCUS_TAG")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COUNT_NUCLEOTIDES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_PROTEIN_ID")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INCONSISTENT_PROTEIN_ID")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FEATURE_LOCATION_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "GENE_PRODUCT_CONFLICT")); m_TGrp.Add(G);
        //  { "Duplicate Gene Locus", "DUPLICATE_GENE_LOCUS", FindDuplicateGeneLocus, NULL },
        //  { "EC Number Note", "EC_NUMBER_NOTE", AddECNumberNoteDiscrepancies, NULL },
        G.Reset(new CDiscrepancyGroup("", "PSEUDO_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "JOINED_FEATURES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "JOINED_FEATURES_NO_EXCEPTION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "RIBOSOMAL_SLIPPAGE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "OVERLAPPING_GENES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "OVERLAPPING_CDS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CONTAINED_CDS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "RNA_CDS_OVERLAP")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHORT_CONTIG")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INCONSISTENT_BIOSOURCE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SUSPECT_PRODUCT_NAMES")); m_TGrp.Add(G);
        //  { "Suspect Product Name Typo", "DISC_PRODUCT_NAME_TYPO", FindSuspectProductNames, FixSuspectProductNameTypos },
        //  { "Suspect Product Name QuickFix", "DISC_PRODUCT_NAME_QUICKFIX", FindSuspectProductNames, FixSuspectProductNameQuickFixes },
        //  { "Inconsistent Source And Definition Line", "INCONSISTENT_SOURCE_DEFLINE", FindInconsistentSourceAndDefline, NULL },
        G.Reset(new CDiscrepancyGroup("", "PARTIAL_CDS_COMPLETE_SEQUENCE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "EC_NUMBER_ON_UNKNOWN_PROTEIN")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TAX_LOOKUP_MISSING")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TAX_LOOKUP_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHORT_SEQUENCES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SUSPECT_PHRASES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SUSPICIOUS_NOTE_TEXT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COUNT_TRNAS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FIND_DUP_TRNAS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FIND_BADLEN_TRNAS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FIND_STRAND_TRNAS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COUNT_RRNAS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FIND_DUP_RRNAS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "RNA_NO_PRODUCT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TRANSL_NO_NOTE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "NOTE_NO_TRANSL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TRANSL_TOO_LONG")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CDS_TRNA_OVERLAP")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COUNT_PROTEINS")); m_TGrp.Add(G);
        //  { "Features Intersecting Source Features", "DISC_FEAT_OVERLAP_SRCFEAT", FindFeaturesOverlappingSrcFeatures, NULL },
        //  { "CDS on GenProdSet without protein", "MISSING_GENPRODSET_PROTEIN", CheckListForGenProdSets, NULL},
        //  { "Multiple CDS on GenProdSet, same protein", "DUP_GENPRODSET_PROTEIN", CheckListForGenProdSets, NULL},
        //  { "mRNA on GenProdSet without transcript ID", "MISSING_GENPRODSET_TRANSCRIPT_ID", CheckListForGenProdSets, NULL},
        //  { "mRNA on GenProdSet with duplicate ID", "DISC_DUP_GENPRODSET_TRANSCRIPT_ID", CheckListForGenProdSets, NULL},
        G.Reset(new CDiscrepancyGroup("", "PERCENT_N")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "N_RUNS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "ZERO_BASECOUNT")); m_TGrp.Add(G);
        //  { "Adjacent PseudoGenes with Identical Text", "ADJACENT_PSEUDOGENES", FindAdjacentPseudoGenes, NULL},
        G.Reset(new CDiscrepancyGroup("", "LONG_NO_ANNOTATION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "NO_ANNOTATION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INFLUENZA_DATE_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHORT_INTRON")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_VIRAL_QUALS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SRC_QUAL_PROBLEM")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_SRC_QUAL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DUP_SRC_QUAL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DUP_SRC_QUAL_DATA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FEATURE_MOLTYPE_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CDS_WITHOUT_MRNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "EXON_INTRON_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FEATURE_COUNT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SPECVOUCHER_TAXNAME_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "GENE_PARTIAL_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FLATFILE_FIND")); m_TGrp.Add(G);
        //  { "Coding region product contains suspect text", "DISC_CDS_PRODUCT_FIND", FindTextInCDSProduct, NULL},
        G.Reset(new CDiscrepancyGroup("", "DUP_DEFLINE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "ATCC_CULTURE_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "USA_STATE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INCONSISTENT_MOLTYPES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SUBMITBLOCK_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "POSSIBLE_LINKER")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TITLE_AUTHOR_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BAD_GENE_STRAND")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MAP_CHROMOSOME_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "RBS_WITHOUT_GENE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CITSUBAFFIL_CONFLICT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "REQUIRED_CLONE")); m_TGrp.Add(G);
        //G.Reset(new CDiscrepancyGroup("", "SOURCE_QUALS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("Source Qualifier Report", "SOURCE_QUALS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "mRNA_ON_WRONG_SEQUENCE_TYPE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "RETROVIRIDAE_DNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CHECK_AUTH_CAPS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CHECK_RNA_PRODUCTS_AND_COMMENTS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MICROSATELLITE_REPEAT_TYPE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MITOCHONDRION_REQUIRED")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNPUB_PUB_WITHOUT_TITLE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "QUALITY_SCORES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INTERNAL_TRANSCRIBED_SPACER_RRNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "PARTIAL_PROBLEMS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BACTERIAL_PARTIAL_NONEXTENDABLE_PROBLEMS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BACTERIAL_PARTIAL_NONEXTENDABLE_EXCEPTION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SUSPECT_RRNA_PRODUCTS")); m_TGrp.Add(G);
        //  { "suspect misc_feature comments", "DISC_SUSPECT_MISC_FEATURES", FindBadMiscFeatures, NULL},
        G.Reset(new CDiscrepancyGroup("", "MISSING_DEFLINES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_AFFIL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BACTERIA_SHOULD_NOT_HAVE_ISOLATE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BACTERIA_SHOULD_NOT_HAVE_MRNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CDS_HAS_NEW_EXCEPTION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TRINOMIAL_SHOULD_HAVE_QUALIFIER")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "METAGENOMIC")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "METAGENOME_SOURCE")); m_TGrp.Add(G);
        //  { "Missing genes", "ONCALLER_GENE_MISSING", OnCallerMissingAndSuperfluousGenes, NULL},
        G.Reset(new CDiscrepancyGroup("", "SUPERFLUOUS_GENE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHORT_RRNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CHECK_AUTHORITY")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CONSORTIUM")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "STRAIN_CULTURE_COLLECTION_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MULTISRC")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MULTIPLE_CULTURE_COLLECTION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SEGSETS_PRESENT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "NONWGS_SETS_PRESENT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FEATURE_LIST")); m_TGrp.Add(G);
        //  { "Category Header", "DISC_CATEGORY_HEADER", NULL, NULL},
        G.Reset(new CDiscrepancyGroup("", "MISMATCHED_COMMENTS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "STRAIN_TAXNAME_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "HUMAN_HOST")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BAD_BACTERIAL_GENE_NAME")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BAD_GENE_NAME")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "ORDERED_LOCATION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COMMENT_PRESENT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DEFLINE_ON_SET")); m_TGrp.Add(G);
        //  { "HIV RNA location or molecule type inconsistent", "ONCALLER_HIV_RNA_INCONSISTENT", FindInconsistentHIVRNA, NULL },
        G.Reset(new CDiscrepancyGroup("", "SHORT_PROT_SEQUENCES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "EXON_ON_MRNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "HAS_PROJECT_ID")); m_TGrp.Add(G);
        //  { "Feature has standard_name qualifier", "ONCALLER_HAS_STANDARD_NAME", FindStandardName, NULL },
        G.Reset(new CDiscrepancyGroup("", "REQUIRED_STRAIN")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_GENOMEASSEMBLY_COMMENTS")); m_TGrp.Add(G);
        //  { "Bacterial taxnames should end with strain", "DISC_BACTERIAL_TAX_STRAIN_MISMATCH", FindBacterialTaxStrainMismatch, NULL },
        G.Reset(new CDiscrepancyGroup("", "CDS_HAS_CDD_XREF")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNUSUAL_NT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "LOW_QUALITY_REGION")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "ORGANELLE_NOT_GENOMIC")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNWANTED_SPACER")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "ORGANELLE_PRODUCTS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SP_NOT_UNCULTURED")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BAD_MRNA_QUAL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNNECESSARY_ENVIRONMENTAL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNNECESSARY_VIRUS_GENE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNWANTED_SET_WRAPPER")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_PRIMER")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNUSUAL_MISC_RNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "AMPLIFIED_PRIMERS_NO_ENVIRONMENTAL_SAMPLE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DUP_GENES_OPPOSITE_STRANDS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SMALL_GENOME_SET_PROBLEM")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "OVERLAPPING_RRNAS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MRNA_SEQUENCE_MINUS_STRAND_FEATURES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TAXNAME_NOT_IN_DEFLINE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COUNT_UNVERIFIED")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHOW_TRANSL_EXCEPT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHOW_HYPOTHETICAL_CDS_HAVING_GENE_NAME")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DEFLINE_PRESENT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MRNA_OVERLAPPING_PSEUDO_GENE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "FIND_OVERLAPPED_GENES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BIOMATERIAL_TAXNAME_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CULTURE_TAXNAME_MISMATCH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CHECK_AUTH_NAME")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "NON_RETROVIRIDAE_PROVIRAL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "RNA_PROVIRAL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHORT_SEQUENCES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "10_PERCENTN")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "N_RUNS_14")); m_TGrp.Add(G);
        //  {"Moltype not mRNA", "MOLTYPE_NOT_MRNA", MoltypeNotmRNA, NULL},
        G.Reset(new CDiscrepancyGroup("", "TECHNIQUE_NOT_TSA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_STRUCTURED_COMMENT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_PROJECT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MULTIPLE_CDS_ON_MRNA")); m_TGrp.Add(G);
        //  {"CBS strain should also appear in culture collection", "DUP_DISC_CBS_CULTURE_CONFLICT", CheckCBSStrainCultureCollConflict, AddCBSStrainToCultureColl},
        G.Reset(new CDiscrepancyGroup("", "DIVISION_CODE_CONFLICTS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "RRNA_NAME_CONFLICTS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "EUKARYOTE_SHOULD_HAVE_MRNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MRNA_SHOULD_HAVE_PROTEIN_TRANSCRIPT_IDS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COUNTRY_COLON")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BIOPROJECT_ID")); m_TGrp.Add(G);
        //  {"Type strain comment in OrgMod does not agree with organism name", "ONCALLER_STRAIN_TAXNAME_CONFLICT", StrainTaxnameConflict, NULL},
        //  {"SubSource collected-by contains more than 3 names", "ONCALLER_MORE_NAMES_COLLECTED_BY", FindMoreNamesInCollectedBy, MarkAndRemoveCollectedItems},
        //  {"SubSource identified-by contains more than 3 names", "ONCALLER_MORE_OR_SPEC_NAMES_IDENTIFIED_BY", FindMoreNamesInIdentifiedBy, MarkAndRemoveIdentifiedItems},
        //  {"Suspected organism in identified-by SubSource", "ONCALLER_SUSPECTED_ORG_IDENTIFIED", FindSuspOrgNameInIdentified, MarkAndRemoveIdentifiedItems},
        //  {"Suspected organism in collected-by SubSource", "ONCALLER_SUSPECTED_ORG_COLLECTED", FindSuspOrgNameInCollected, MarkAndRemoveCollectedItems},
        G.Reset(new CDiscrepancyGroup("", "SWITCH_STRUCTURED_COMMENT_PREFIX")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "CITSUB_AFFIL_DUP_TEXT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DUPLICATE_PRIMER_SET")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "END_COLON_IN_COUNTRY")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "PROTEIN_NAMES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TITLE_ENDS_WITH_SEQUENCE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INCONSISTENT_STRUCTURED_COMMENTS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INCONSISTENT_DBLINK")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "INCONSISTENT_MOLINFO_TECH")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "GAPS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BAD_BGPIPE_QUALS")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "SHORT_LNCRNA")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TERMINAL_NS")); m_TGrp.Add(G);
        //  {"Alignment has score attribute", "TEST_ALIGNMENT_HAS_SCORE", FindAlignmentsWithScores, NULL},
        G.Reset(new CDiscrepancyGroup("", "UNCULTURED_NOTES")); m_TGrp.Add(G);
        //  {"Special phrases of seq ids", "SEQ_ID_PHRASES", FindSeqIdHavingPhrases, NULL},
        G.Reset(new CDiscrepancyGroup("", "NO_PRODUCT_STRING")); m_TGrp.Add(G);
    }
    else if (m_Type == CDiscrepancyDlg::eOncaller) {
        CRef<CDiscrepancyGroup> G, H;
        G.Reset(new CDiscrepancyGroup("", "COUNT_NUCLEOTIDES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DUP_DEFLINE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "MISSING_DEFLINES")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "TAXNAME_NOT_IN_DEFLINE")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "HAS_PROJECT_ID")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "BIOPROJECT_ID")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DEFLINE_ON_SET")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "UNWANTED_SET_WRAPPER")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "COUNT_UNVERIFIED")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "DUP_SRC_QUAL")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("Source Qualifier Report", "SOURCE_QUALS")); m_TGrp.Add(G);

        G.Reset(new CDiscrepancyGroup("Feature Count", "FEATURE_COUNT")); m_TGrp.Add(G);
        G.Reset(new CDiscrepancyGroup("", "NO_ANNOTATION")); m_TGrp.Add(G);

        G.Reset(new CDiscrepancyGroup("Molecule type tests", "")); m_TGrp.Add(G);
            H.Reset(new CDiscrepancyGroup("", "FEATURE_MOLTYPE_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "ORGANELLE_NOT_GENOMIC")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "INCONSISTENT_MOLTYPES")); G->Add(H);

        G.Reset(new CDiscrepancyGroup("Cit-sub type tests", "")); m_TGrp.Add(G);
            H.Reset(new CDiscrepancyGroup("", "CHECK_AUTH_CAPS")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CONSORTIUM")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "UNPUB_PUB_WITHOUT_TITLE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "TITLE_AUTHOR_CONFLICT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "SUBMITBLOCK_CONFLICT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CITSUBAFFIL_CONFLICT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CHECK_AUTH_NAME")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MISSING_AFFIL")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "USA_STATE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CITSUB_AFFIL_DUP_TEXT")); G->Add(H);

        G.Reset(new CDiscrepancyGroup("Source tests", "")); m_TGrp.Add(G);
            //H.Reset(new CDiscrepancyGroup("", "DUP_SRC_QUAL")); G->Add(H);    // SQD-4049
            //DISC_MISSING_SRC_QUAL
            H.Reset(new CDiscrepancyGroup("", "DUPLICATE_PRIMER_SET")); G->Add(H);
            //ONCALLER_MORE_NAMES_COLLECTED_BY
            //ONCALLER_MORE_OR_SPEC_NAMES_IDENTIFIED_BY
            //ONCALLER_SUSPECTED_ORG_IDENTIFIED
            //ONCALLER_SUSPECTED_ORG_COLLECTED
            H.Reset(new CDiscrepancyGroup("", "MISSING_VIRAL_QUALS")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "INFLUENZA_DATE_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "HUMAN_HOST")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "SPECVOUCHER_TAXNAME_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "BIOMATERIAL_TAXNAME_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CULTURE_TAXNAME_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "STRAIN_TAXNAME_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "BACTERIA_SHOULD_NOT_HAVE_ISOLATE")); G->Add(H);
            //ONCALLER_STRAIN_TAXNAME_CONFLICT
            H.Reset(new CDiscrepancyGroup("", "SP_NOT_UNCULTURED")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "REQUIRED_CLONE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "UNNECESSARY_ENVIRONMENTAL")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "AMPLIFIED_PRIMERS_NO_ENVIRONMENTAL_SAMPLE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MULTISRC")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "COUNTRY_COLON")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "END_COLON_IN_COUNTRY")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "ATCC_CULTURE_CONFLICT")); G->Add(H);
            //DUP_DISC_CBS_CULTURE_CONFLICT
            H.Reset(new CDiscrepancyGroup("", "STRAIN_CULTURE_COLLECTION_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MULTIPLE_CULTURE_COLLECTION")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "TRINOMIAL_SHOULD_HAVE_QUALIFIER")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CHECK_AUTHORITY")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MAP_CHROMOSOME_CONFLICT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "METAGENOMIC")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "METAGENOME_SOURCE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "RETROVIRIDAE_DNA")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "NON_RETROVIRIDAE_PROVIRAL")); G->Add(H);
            //ONCALLER_HIV_RNA_INCONSISTENT
            H.Reset(new CDiscrepancyGroup("", "RNA_PROVIRAL")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "BAD_MRNA_QUAL")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MITOCHONDRION_REQUIRED")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "UNWANTED_SPACER")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "SMALL_GENOME_SET_PROBLEM")); G->Add(H);

        G.Reset(new CDiscrepancyGroup("Feature tests", "")); m_TGrp.Add(G);
            H.Reset(new CDiscrepancyGroup("", "SUPERFLUOUS_GENE")); G->Add(H);
            //ONCALLER_GENE_MISSING
            H.Reset(new CDiscrepancyGroup("", "GENE_PARTIAL_CONFLICT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "BAD_GENE_STRAND")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "UNNECESSARY_VIRUS_GENE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "NON_GENE_LOCUS_TAG")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "RBS_WITHOUT_GENE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "ORDERED_LOCATION")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MULTIPLE_CDS_ON_MRNA")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CDS_WITHOUT_MRNA")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MRNA_ON_WRONG_SEQUENCE_TYPE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "BACTERIA_SHOULD_NOT_HAVE_MRNA")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "MRNA_OVERLAPPING_PSEUDO_GENE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "EXON_ON_MRNA")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CDS_HAS_NEW_EXCEPTION")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "SHORT_INTRON")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "EXON_INTRON_CONFLICT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "PSEUDO_MISMATCH")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "RNA_NO_PRODUCT")); G->Add(H);
            //DISC_BADLEN_TRNA
            H.Reset(new CDiscrepancyGroup("", "MICROSATELLITE_REPEAT_TYPE")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "SHORT_RRNA")); G->Add(H);

        G.Reset(new CDiscrepancyGroup("", "POSSIBLE_LINKER")); m_TGrp.Add(G);

        G.Reset(new CDiscrepancyGroup("Suspect text tests", "")); m_TGrp.Add(G);
            H.Reset(new CDiscrepancyGroup("", "FLATFILE_FIND_ONCALLER")); G->Add(H);
            //DISC_CDS_PRODUCT_FIND
            H.Reset(new CDiscrepancyGroup("", "SUSPICIOUS_NOTE_TEXT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "CHECK_RNA_PRODUCTS_AND_COMMENTS")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "INTERNAL_TRANSCRIBED_SPACER_RRNA")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "COMMENT_PRESENT")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "SUSPECT_PRODUCT_NAMES")); G->Add(H);
            H.Reset(new CDiscrepancyGroup("", "UNCULTURED_NOTES")); G->Add(H);
    }
    m_Project = nullptr;
}


void CDiscrepancyDlg::CreateControls()
{
    CDiscrepancyDlg* itemFrame1 = this;
    m_Reopen = 0;

    wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
    SetSizer(box);

    wxPanel* itemDialog1 = new wxPanel(itemFrame1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    box->Add(itemDialog1, 1, wxGROW | wxALL, 0);
     
    wxBoxSizer* box0 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(box0);

    /////////////////// Inserting new panel here
    m_Panel = new CDiscrepancyPanel(m_Type, itemDialog1, this, m_Workbench);

    wxBoxSizer* box_p = new wxBoxSizer(wxHORIZONTAL);
    box_p->Add(m_Panel, 1, wxGROW | wxALL, 0);
    box0->Add(box_p, 1, wxGROW | wxALL);

    if (m_Type == CDiscrepancyDlg::eCdsTrnaOverlap) {
        wxBoxSizer* box5 = new wxBoxSizer(wxHORIZONTAL);
        box0->Add(box5, 0, wxCENTER | wxALL, 5);
        m_TrimBtn = new wxButton(itemDialog1, ID_DISCR_AUTOFIX, _("Trim Selected"), wxDefaultPosition, wxDefaultSize, 0);
        box5->Add(m_TrimBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        m_CheckFixableBtn = new wxButton(itemDialog1, ID_DISCR_CHECKFIXABLE, _("Select All"), wxDefaultPosition, wxDefaultSize, 0);
        box5->Add(m_CheckFixableBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        wxButton* closeButton = new wxButton(itemDialog1, ID_DISCR_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
        box5->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    }
    else {
        if (RunningInsideNCBI()) {
            wxBoxSizer* box5 = new wxBoxSizer(wxHORIZONTAL);
            box0->Add(box5, 0, wxCENTER | wxALL, 5);
            m_ExpandBtn = new wxButton(itemDialog1, ID_DISCR_EXPAND, _("Expand All"), wxDefaultPosition, wxDefaultSize, 0);
            box5->Add(m_ExpandBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            m_ContractBtn = new wxButton(itemDialog1, ID_DISCR_CONTRACT, _("Contract All"), wxDefaultPosition, wxDefaultSize, 0);
            box5->Add(m_ContractBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            m_TestNameBtn = new wxButton(itemDialog1, ID_DISCR_TESTNAME, _("Show/Hide Test Names"), wxDefaultPosition, wxDefaultSize, 0);
            box5->Add(m_TestNameBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            if (!CSysPath::GetInst().empty()) {
                m_Reopen = new wxCheckBox(itemDialog1, ID_DISCR_REOPEN, "Reopen automatically");
                box5->Add(m_Reopen, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            }
        } else {
            m_ExpandBtn = NULL;
            m_ContractBtn = NULL;
            m_TestNameBtn = NULL;
        }

        wxBoxSizer* box6 = new wxBoxSizer(wxHORIZONTAL);
        box0->Add(box6, 0, wxCENTER | wxALL, 5);
        if (RunningInsideNCBI()) {
            m_ReportBtn = new wxButton(itemDialog1, ID_DISCR_REPORT, _("Generate Report"), wxDefaultPosition, wxDefaultSize, 0);
            box6->Add(m_ReportBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        } else {
            m_ReportBtn = NULL;
        }
        m_RefreshBtn = new wxButton(itemDialog1, ID_DISCR_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0);
        box6->Add(m_RefreshBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        if (RunningInsideNCBI()) {
            m_CheckFixableBtn = new wxButton(itemDialog1, ID_DISCR_CHECKFIXABLE, _("Mark Fixable"), wxDefaultPosition, wxDefaultSize, 0);
            box6->Add(m_CheckFixableBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            m_AutofixBtn = new wxButton(itemDialog1, ID_DISCR_AUTOFIX, _("Fix Marked"), wxDefaultPosition, wxDefaultSize, 0);
            box6->Add(m_AutofixBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            m_UncheckAllBtn = new wxButton(itemDialog1, ID_DISCR_UNCHECKALL, _("Unmark All"), wxDefaultPosition, wxDefaultSize, 0);
            box6->Add(m_UncheckAllBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        } else {
            m_CheckFixableBtn = NULL;
            m_AutofixBtn = NULL;
            m_UncheckAllBtn = NULL;
        }
        if (m_Type != CDiscrepancyDlg::eSubmitter) {
            m_SettingsBtn = new wxButton(itemDialog1, ID_DISCR_SETTINGS, _("Settings"), wxDefaultPosition, wxDefaultSize, 0);
            box6->Add(m_SettingsBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            m_SequesterBtn = new wxButton(itemDialog1, ID_DISCR_SEQUESTER, _("Sequester"), wxDefaultPosition, wxDefaultSize, 0);
            box6->Add(m_SequesterBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        }        

        wxButton* closeButton = new wxButton(itemDialog1, ID_DISCR_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
        box6->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( itemDialog1, wxID_HELP, _("Help"), wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual8/#submitter_report"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
        itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
        box6->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        if (RunningInsideNCBI()) {
            wxBoxSizer* box3 = new wxBoxSizer(wxHORIZONTAL);
            m_ProductRulesLabel = new wxStaticText(itemDialog1, 0, "Product Rules", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            box3->Add(m_ProductRulesLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
            box0->Add(box3, 0, wxGROW | wxALL, 5);
        }
    }

    if (!RunningInsideNCBI()) {
        //ExpandAll();
    }
}

void CDiscrepancyDlg::OnClose(wxCommandEvent& evt)
{
    Destroy();
}


void CDiscrepancyDlg::OnDataChanged(CEvent* evt)
{
    CProjectViewEvent* e = dynamic_cast<CProjectViewEvent*>(evt);
    if (e && e->GetSubtype() == CProjectViewEvent::eUnloadProject) {
        Destroy();
    }
    else {
        m_Panel->m_NeedRefresh = true;
        if (m_Type != CDiscrepancyDlg::eCdsTrnaOverlap && RunningInsideNCBI()) {
            m_ProductRulesLabel->SetLabel(ProductRulesStr(m_Panel->m_ProductRules));
        }
        UpdateButtons();
    }
}


bool CDiscrepancyDlg::Destroy()
{
    SaveTreeState();
    wxRect rc = GetRect();
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryWriteView reg_view = reg.GetWriteView(RegisterRoot() + ".Rect", REGISTER_MAGIC_NUMBER_DISCREPANCY);
    reg_view.Set("x", rc.x);
    reg_view.Set("y", rc.y);
    reg_view.Set("width", rc.width);
    reg_view.Set("height", rc.height);
    WriteSettings(reg);   
    dynamic_cast<CGBDocument*>(m_Project)->RemoveListener(this);
    return wxFrame::Destroy();
}

//////////////////////////////////////////////////////////////////


void CDiscrepancyPanel::UpdateTestList()
{
    if (m_Type == CDiscrepancyDlg::eCdsTrnaOverlap) {
        return;
    }
    if (m_DefaultTests.empty()) {
        vector<string> AllTests = GetDiscrepancyNames();
        map<string, bool> DefaultTests;
        map<string, bool> AddTests;
        map<string, bool> RemoveTests;
        for (auto& name : AllTests) {
            TGroup group = GetDiscrepancyGroup(name);
            if ((m_Type == CDiscrepancyDlg::eNormal && (group & NDiscrepancy::eDisc)) ||
                (m_Type == CDiscrepancyDlg::eOncaller && (group & NDiscrepancy::eOncaller)) ||
                (m_Type == CDiscrepancyDlg::eSubmitter && (group & NDiscrepancy::eSubmitter)) ||
                (m_Type == CDiscrepancyDlg::eMega)) {
                m_DefaultTests.push_back(name);
                DefaultTests[name] = true;
            }
        }

        for (auto& name : m_AddTests) {
            AddTests[name] = true;
        }
        for (auto& name : m_RemoveTests) {
            RemoveTests[name] = true;
        }
        for (auto& name : AllTests) {
            if ((DefaultTests[name] || AddTests[name]) && !RemoveTests[name]) {
                m_TestList.push_back(name);
            }
        }
    }
}


void CDiscrepancyPanel::RunTests()
{
    SaveTreeState();
    if (m_TestList.empty()) {
        UpdateTestList();
    }
    m_Left->Clear();
    m_Right->Clear();

    m_Tests = CDiscrepancySet::New(*m_Project->GetScope());
    m_Tests->SetGui(true);
    m_Tests->SetUserData(this);
    m_Tests->SetSuspectRules(m_ProductRules);
    for (auto& tname : m_TestList) {
        m_Tests->AddTest(tname);
    }

    CRef<CDiscrepancyReportJob> job(new CDiscrepancyReportJob(m_Project, &*m_Tests));
    try {
        wxBeginBusyCursor();
        m_Helper->RunTests(job);
        m_Loading->Show();
        m_NeedRefresh = false;
        UpdateButtons();
    } catch(CAppJobException& e)  {
        wxEndBusyCursor();
        ERR_POST("CTextDataSource::LoadFasta() - Failed to start job");
        e.ReportAll();
    }
}

void CDiscrepancyPanel::CDiscrepancyHelper::RunTests(CRef<CDiscrepancyReportJob> job)
{
    if (m_JobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
        m_JobId = CAppJobDispatcher::eInvalidJobID;
    }
    m_JobId =  CAppJobDispatcher::GetInstance().StartJob(*job, "ObjManagerEngine", *this, -1, true);
}

void CDiscrepancyDlg::RunTests() {
    m_Panel->m_WorkDir = m_WorkDir;
    m_Panel->m_Project = m_Project;
    m_Panel->m_CmdProccessor = m_CmdProccessor;
    m_Panel->RunTests();
}


CJobCancelable::EJobState CDiscrepancyReportJob::Run()
{
    const CProjectFolder::TItems& items = m_Proj->GetData().GetItems();
    for (auto& it : items) {
        const CSerialObject* obj = it->GetObject();
        const CSeq_entry* se = dynamic_cast<const CSeq_entry*>(obj);
        if (se) {
            m_Tests->Push(*se);
        }
        const CSeq_submit* ss = dynamic_cast<const CSeq_submit*>(obj);
        if (ss) {
            m_Tests->Push(*ss);
        }
        const CBioseq_set* st = dynamic_cast<const CBioseq_set*>(obj);
        if (st) {
            m_Tests->Push(*st);
        }
        const CSeq_id* si = dynamic_cast<const CSeq_id*>(obj);
        if (si) {
            CBioseq_Handle bsh = m_Proj->GetScope()->GetBioseqHandle(*si);
            if (bsh) {
                m_Tests->Push(*bsh.GetCompleteBioseq());
            }
        }
    }
    m_Tests->Parse();
    m_Tests->Summarize();
    return CJobCancelable::eCompleted;
}


void CDiscrepancyPanel::CDiscrepancyHelper::OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (!notn) return;
    CAppJobDispatcher::TJobID job_id = notn->GetJobID();
    if (m_JobId != job_id) {
        ERR_POST("CTextDataSource::OnJobNotification() - unknown Job ID " << job_id);
        return;
    }
    if (notn->GetState() == IAppJob::eCompleted) {
        CRef<CDiscrepancySet> Tests(dynamic_cast<CDiscrepancySet*>(&*notn->GetResult()));
        TDiscrepancyCaseMap map = Tests->GetTests();
        TReportItemList list = m_Panel->m_TGrp.Collect(map);
        for (auto& it : list) {
            m_Panel->m_Left->Add(new CDiscRepItem(it, &m_Panel->m_ShowTestName, m_Panel->m_Type != CDiscrepancyDlg::eOncaller));
        }
        m_Panel->m_Left->Finish();
        m_Panel->RestoreTreeState();
        m_Panel->m_Left->Refresh();
        m_JobId = CAppJobDispatcher::eInvalidJobID;
        m_Panel->UpdateButtons();
    }
    wxEndBusyCursor();
    m_Panel->m_Loading->Hide();
}


void CDiscrepancyPanel::OnClick(wxCommandEvent& evt)
{
    CDiscRepItem* item = (CDiscRepItem*)m_Left->GetCurrentItem();
    if (!item) {
        return;
    }
    TReportObjectList det = item->GetItem()->GetDetails();
    m_Right->Clear();
    for (auto& obj : det) {
        wxString a[1];
        a[0] = NStr::Replace(obj->GetText(), "\t", "  ");
        m_Right->InsertItems(1, a, m_Right->GetCount());
    }
    UpdateButtons();
}


static string CdsTrnaOverlapStr(const CSeq_feat* cds, vector<const CSeq_feat*> trnas, CScope& scope)
{
    const CSeq_loc& loc_cds = cds->GetLocation();
    CBioseq_Handle bs_cds = scope.GetBioseqHandle(loc_cds);
    CSeq_loc::TRange rr = loc_cds.GetTotalRange();
    bool not_T_or_TA = false;
    bool five_prime = false;
    int overlap = 0;
    int count = 0;
    for (auto trna: trnas) {
        if (trna->GetLocation().IsReverseStrand() != cds->GetLocation().IsReverseStrand()) {
            continue;
        }
        const CSeq_loc& loc_trna = trna->GetLocation();
        CSeq_loc::TRange r = loc_trna.GetTotalRange();
        if (rr.GetFrom() < r.GetFrom()) {
            int n = rr.GetToOpen() - r.GetFrom();
            if (n <= 0) {
                continue;
            }
            if (scope.GetBioseqHandle(loc_trna) != bs_cds) {
                continue;
            }
            count++;
            if (n > overlap) {
                overlap = n;
            }
            if (n >= 1 && n <= 2 && cds->GetLocation().IsReverseStrand()) {
                five_prime = true;
            }
            else if (!trna->GetLocation().IsReverseStrand()) {
                if (n == 1 && !not_T_or_TA) {
                    CSeqVector seq(cds->GetLocation(), scope, CBioseq_Handle::eCoding_Iupac);
                    if (seq[seq.size() - 3] != 'T' || seq[seq.size() - 2] != 'A') {
                        not_T_or_TA = true;
                    }
                }
                else if (n == 2 && !not_T_or_TA) {
                    CSeqVector seq(cds->GetLocation(), scope, CBioseq_Handle::eCoding_Iupac);
                    if (seq[seq.size() - 3] != 'T') {
                        not_T_or_TA = true;
                    }
                }
            }
        }
        else {
            int n = r.GetToOpen() - rr.GetFrom();
            if (n <= 0) {
                continue;
            }
            if (scope.GetBioseqHandle(loc_trna) != bs_cds) {
                continue;
            }
            count++;
            if (n > overlap) {
                overlap = n;
            }
            if (n >= 1 && n <= 2 && !cds->GetLocation().IsReverseStrand()) {
                five_prime = true;
            }
            else if (trna->GetLocation().IsReverseStrand()) {
                if (n == 1 && !not_T_or_TA) {
                    CSeqVector seq(cds->GetLocation(), scope, CBioseq_Handle::eCoding_Iupac);
                    if (seq[seq.size() - 3] != 'T' || seq[seq.size() - 2] != 'A') {
                        not_T_or_TA = true;
                    }
                }
                else if (n == 2 && !not_T_or_TA) {
                    CSeqVector seq(cds->GetLocation(), scope, CBioseq_Handle::eCoding_Iupac);
                    if (seq[seq.size() - 3] != 'T') {
                        not_T_or_TA = true;
                    }
                }
            }
        }
    }
    if (overlap > 2) {
        return (count > 1 ? "CDS is overlapped by more than one tRNA, longest overlap is [(]" : "Overlap is too long: [(]") + NStr::IntToString(overlap) + "[)]";
    }
    if (count > 1) {
        return "CDS is overlapped by more than one tRNA!";
    }
    if (five_prime) {
        return "CDS is overlapped at the 5' end";
    }
    if (not_T_or_TA) {
        return "Base pairs in partial codon before trim are neither 'T' nor 'TA'";
    }
    return "Expected overlap";
}


static bool sortCdsTrnaOverlap(CRef<CReportItem> A, CRef<CReportItem> B)
{
    if (A->GetMsg() == B->GetMsg() || A->GetMsg() == "Expected overlap") {
        return false;
    }
    return B->GetMsg() == "Expected overlap" || A->GetMsg() < B->GetMsg();
}


void CDiscrepancyPanel::recursiveCdsTrnaOverlap(CDiscRepItem* item, vector<CRef<CReportItem>>& reps, CScope& scope)
{
    vector<CCustomTreeItem*> children = item->GetChildren();
    if (children.size()) {
        for (auto child: children) {
            recursiveCdsTrnaOverlap((CDiscRepItem*)child, reps, scope);
        }
    }
    else {
        TReportObjectList det = item->GetItem()->GetDetails();
        vector<const CSeq_feat*> all_trnas;
        for (auto& item: det) {
            const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(m_Tests->FindObject(*item));
            if (!feat->GetData().IsCdregion()) {
                all_trnas.push_back(feat);
            }
        }
        for (auto& item: det) {
            const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(m_Tests->FindObject(*item));
            if (feat->GetData().IsCdregion()) {
                string msg = CdsTrnaOverlapStr(feat, all_trnas, scope);
                CRef<CReportItem> ri = CReportItem::CreateReportItem("_CDS_TRNA_OVERLAP", *item, msg, msg == "Expected overlap");
                reps.push_back(ri);
            }
        }
    }
}


void CDiscrepancyPanel::OnDblClk(wxCommandEvent& evt)
{
    CDiscRepItem* item = (CDiscRepItem*)m_Left->GetCurrentItem();
    if (!item) {
        return;
    }
    TReportObjectList det = item->GetItem()->GetDetails();
    if (!det.size()) {
        return;
    }
    vector<CBioseq_Handle> V_src;
    vector<CSeq_feat_Handle> V_cds;
    vector<CSeq_feat_Handle> V_gen;
    vector<CSeq_feat_Handle> V_rna;
    bool refresh = false;
    CScope& scope = *m_Project->GetScope();

    for (auto it: det) {
        const CSerialObject* ptr = m_Tests->FindObject(*it);
        const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(ptr);
        const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(ptr);
        if ((desc || feat) && InvalidObject(ptr, scope)) {
            refresh = true;
            break;
        }
        if (desc && desc->IsSource()) {
            CConstRef<CBioseq> seq = macro::NMacroUtil::GetBioseqForSeqdescObject(ptr, edit::GetSeqEntryForSeqdesc(CRef<CScope>(&scope), *desc));
            if (!seq.Empty()) {
                V_src.push_back(scope.GetBioseqHandle(*seq));
            }
            continue;
        }
        else if (feat && feat->CanGetData()) {
            const CSeq_feat::TData& data = feat->GetData();
            if (data.IsCdregion()) {
                V_cds.push_back(scope.GetSeq_featHandle(*feat));
                continue;
            }
            else if (data.IsGene()) {
                V_gen.push_back(scope.GetSeq_featHandle(*feat));
                continue;
            }
            else if (data.IsRna()) {
                V_rna.push_back(scope.GetSeq_featHandle(*feat));
                continue;
            }
        }
    }
    if (refresh) {
        wxMessageBox(m_NeedRefresh ? "Please refresh!" : "Oops! We have some problem...");
        return;
    }
    if (RunningInsideNCBI() && item->GetTestName() == "CDS_TRNA_OVERLAP") {
        CDiscrepancyDlg* dlg = CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eCdsTrnaOverlap, scope, m_Workbench);
        vector<CRef<CReportItem>> reps;
        recursiveCdsTrnaOverlap(item, reps, scope);
        sort(reps.begin(), reps.end(), sortCdsTrnaOverlap);
        dlg->m_Panel->m_ShowTestName = false;
        dlg->m_Panel->m_Tests = m_Tests;
        for (auto rep: reps) {
            dlg->m_Panel->m_Left->Add(new CDiscRepItem(rep, &dlg->m_Panel->m_ShowTestName, false, 0, rep->GetMsg() == "Expected overlap"));
        }
        dlg->m_Panel->m_Left->Finish();
        wxString a[1];
        a[0] = NStr::Replace(reps[0]->GetDetails()[0]->GetText(), "\t", "  ");
        dlg->m_Panel->m_Right->InsertItems(1, a, 0);
        return;
    }
    if (V_src.size()) {
        SrcEditDialog dlg(NULL, V_src);
        if (dlg.ShowModal() == wxID_OK) {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            if (cmd) {
                m_CmdProccessor->Execute(cmd);
            }
        }
        return;
    }
    if (V_cds.size()) {
		try {
			CBulkCDS dlg(NULL, V_cds);
			if (dlg.ShowModal() == wxID_OK) {
				CRef<CCmdComposite> cmd = dlg.GetCommand();
				if (cmd) {
					m_CmdProccessor->Execute(cmd);
				}
			}
		}
		catch (CException&) {}
        return;
    }
    if (V_gen.size()) {
		try {
			CBulkGene dlg(NULL, V_gen);
			if (dlg.ShowModal() == wxID_OK) {
				CRef<CCmdComposite> cmd = dlg.GetCommand();
				if (cmd) {
					m_CmdProccessor->Execute(cmd);
				}
			}
		}
		catch (CException&) {}
        return;
    }
    if (V_rna.size()) {
		try {
			CBulkRna dlg(NULL, V_rna);
			if (dlg.ShowModal() == wxID_OK) {
				CRef<CCmdComposite> cmd = dlg.GetCommand();
				if (cmd) {
					m_CmdProccessor->Execute(cmd);
				}
			}
		}
		catch (CException&) {}
        return;
    }
    if (det.size() == 1) {
        OpenEditor(m_Tests->FindObject(*det[0]), scope);
        return;
    }
    wxMessageBox("No bulk editor available for this type!");
}


unsigned char CDiscrepancyPanel::GetStatus()
{
    unsigned char status = 0;
    vector<CCustomTreeItem*> all_items = m_Left->GetAllItems();
    bool checked = false;
    if (m_Type != CDiscrepancyDlg::eSubmitter) {
        for (auto& w : all_items) {
            if (w->IsChecked()) {
                vector<CSeq_entry_Handle> handles;
                m_Project->GetScope()->GetAllTSEs(handles, CScope::eAllTSEs);
                if (handles.size() == 1) {
                    status |= CDiscrepancyDlg::eSequester;
                }
                break;
            }
        }
    }
    return status;
}

bool CDiscrepancyPanel::IsInvalidJob()
{
    return m_Helper->IsInvalidJob();
}

bool CDiscrepancyPanel::CDiscrepancyHelper::IsInvalidJob()
{
    return (m_JobId   == CAppJobDispatcher::eInvalidJobID);
}

void CDiscrepancyDlg::UpdateButtons()
{
    unsigned char status = m_Panel->GetStatus();
    bool invalid_job = m_Panel->IsInvalidJob();
    if (m_Type == eCdsTrnaOverlap) {
        return;
    }
    if (m_ExpandBtn) {
        m_ExpandBtn->Enable(invalid_job);
    }
    if (m_ContractBtn) {
        m_ContractBtn->Enable(invalid_job);
    }
    if (m_TestNameBtn) {
        m_TestNameBtn->Enable(invalid_job);
    }
    m_RefreshBtn->Enable(m_Panel->NeedRefresh());
    if (m_UncheckAllBtn) {
        m_UncheckAllBtn->Enable(invalid_job);
    }
    if (m_CheckFixableBtn) {
        m_CheckFixableBtn->Enable(invalid_job);
    }
    if (m_AutofixBtn) {
        m_AutofixBtn->Enable(invalid_job);
    }
    if (m_ReportBtn) {
        m_ReportBtn->Enable(invalid_job);
    }
    if (m_Type != eSubmitter) {
        m_SettingsBtn->Enable(invalid_job);
        m_SequesterBtn->Enable(invalid_job);
    }

    if ( m_SequesterBtn) {
        m_SequesterBtn->Enable(status & eSequester);
    }
}


void CDiscrepancyDlg::SaveTreeState() { m_Panel->SaveTreeState(); }

void CDiscrepancyPanel::SaveTreeState()
{
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryWriteView reg_view = reg.GetWriteView(RegisterRoot() + ".Tree", REGISTER_MAGIC_NUMBER_DISCREPANCY);
    vector<CCustomTreeItem*> all = m_Left->GetAllItems();
    for (auto& it : all) {
        CDiscRepItem* item = dynamic_cast<CDiscRepItem*>(it);
        m_TreeState[item->GetStr()] = (item->IsExpanded() ? 1 : 0) | (item->IsChecked() ? 2 : 0);
        if (item->IsHead() || !item->IsReal()) {
            reg_view.Set(item->GetStr(), item->IsExpanded() ? 1 : 0);
        }
    }
    WriteSettings(reg);
}


void CDiscrepancyDlg::RestoreTreeState() { m_Panel->RestoreTreeState(); }

void CDiscrepancyPanel::RestoreTreeState()
{
    vector<CCustomTreeItem*> all = m_Left->GetAllItems();
    for (auto& it : all) {
        CDiscRepItem* item = dynamic_cast<CDiscRepItem*>(it);
        unsigned char c = m_TreeState[item->GetStr()];
        if (item->GetTestName() == "SUSPECT_PRODUCT_NAMES") {
            c |= 1;
        }
        m_Left->ExpandItem(item, c & 1);
        //m_Left->CheckItem(item, c & 2);
    }
}


void CDiscrepancyPanel::ExpandAll()
{
    vector<CCustomTreeItem*> all = m_Left->GetAllItems();
    for (auto& it : all) {
        m_Left->ExpandItem(it, true);
    }
    m_Left->Refresh();
}

void CDiscrepancyDlg::OnExpand(wxCommandEvent& event)
{
    m_Panel->ExpandAll();
}


void CDiscrepancyPanel::OnContract(wxCommandEvent& event)
{
    vector<CCustomTreeItem*> all = m_Left->GetAllItems();
    for (auto& it : all) {
        m_Left->ExpandItem(it, false);
    }
    m_Left->Refresh();
}


void CDiscrepancyDlg::OnContract(wxCommandEvent& event)
{
    m_Panel->OnContract(event);
}


void CDiscrepancyPanel::OnCheckAll(wxCommandEvent& event)
{
    m_Left->CheckAll(true);
    m_Left->Refresh();
}


void CDiscrepancyDlg::OnCheckAll(wxCommandEvent& event)
{
    m_Panel->OnCheckAll(event);
    UpdateButtons();
}


void CDiscrepancyPanel::OnUncheckAll(wxCommandEvent& event)
{
    m_Left->CheckAll(false);
    m_Left->Refresh();
}


void CDiscrepancyDlg::OnUncheckAll(wxCommandEvent& event)
{
    m_Panel->OnUncheckAll(event);
    UpdateButtons();
}


void CDiscrepancyPanel::OnCheckFixable(wxCommandEvent& event)
{
    vector<CCustomTreeItem*> all = m_Left->GetAllItems();
    for (auto& it : all) {
        bool check = false;
        if (it->IsActive()) {
            check = true;
            if (m_Type == CDiscrepancyDlg::eOncaller) { // don't mark "SOURCE_QUALS" in OnCaller
                CDiscRepItem& disc = (CDiscRepItem&)*it;
                if (disc.IsReal() && disc.GetItem()->GetTitle() == "SOURCE_QUALS") {
                    check = false;
                    m_Left->CheckItem(it, check, true);
                }
            }
        }
        m_Left->CheckItem(it, check, false);
    }
    m_Left->Refresh();
}


void CDiscrepancyDlg::OnCheckFixable(wxCommandEvent& event)
{
    m_Panel->OnCheckFixable(event);
    UpdateButtons();
}


static void CollectNucProtBioseqs(CScope& scope, const CBioseq* seq, set<CBioseq_Handle>& out)
{
    CConstRef<CBioseq_set> set = seq->GetParentSet();
    if (set && set->GetClass() == CBioseq_set::eClass_nuc_prot) {
        CBioseq_set::TSeq_set ss = set->GetSeq_set();
        for (auto& it : set->GetSeq_set()) {
            if (it->IsSeq()) {
                out.insert(scope.GetBioseqHandle(it->GetSeq()));
            }
        }
    }
    else {
        out.insert(scope.GetBioseqHandle(*seq));
    }
}


static void CollectBioseqForSeqdesc(CScope& scope, const CSeqdesc& seq_desc, set<CBioseq_Handle>& out)
{
    CScope::TTSE_Handles tses;
    scope.GetAllTSEs(tses, CScope::eAllTSEs);
    for (auto& handle : tses) {
        for (CBioseq_CI bioseq_it(handle);  bioseq_it;  ++bioseq_it) {
            for (CSeqdesc_CI desc_it(*bioseq_it); desc_it; ++desc_it) {
                if (&*desc_it == &seq_desc) {
                    CollectNucProtBioseqs(scope, bioseq_it->GetCompleteBioseq(), out);
                }
            }
        }
    }
}


static void CollectBioseqForSeqFeat(CScope& scope, const CSeq_feat& feat, set<CBioseq_Handle>& out)
{
    CScope::TTSE_Handles tses;
    scope.GetAllTSEs(tses, CScope::eAllTSEs);
    for (auto& handle : tses) {
        for (CBioseq_CI bioseq_it(handle);  bioseq_it;  ++bioseq_it) {
            for (CFeat_CI feat_it(*bioseq_it); feat_it; ++feat_it) {
                if (&*feat_it->GetSeq_feat() == &feat) {
                    CollectNucProtBioseqs(scope, bioseq_it->GetCompleteBioseq(), out);
                }
            }
        }
    }
}


void CDiscrepancyPanel::Sequester(CCustomTreeItem& item, set<CBioseq_Handle>& out, CScope& scope, bool all)
{
    CDiscRepItem& disc = (CDiscRepItem&)item;
    vector<CCustomTreeItem*> items = disc.GetChildren();
    if ((all || item.IsChecked()) && disc.IsReal()) {
        if (items.empty()) {
            TReportObjectList det = disc.GetItem()->GetDetails();
            for (auto& obj : det) {
                CConstRef<CSerialObject> so(m_Tests->FindObject(*obj));
                const CBioseq* seq = dynamic_cast<const CBioseq*>(&*so);
                const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&*so);
                const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(&*so);
                if (seq) {
                    CollectNucProtBioseqs(scope, seq, out);
                }
                else if (desc) {
                    CollectBioseqForSeqdesc(scope, *desc, out);
                }
                else if (feat) {
                    CollectBioseqForSeqFeat(scope, *feat, out);
                }
            }
        }
    }
    for (auto& w : items) {
        Sequester(*w, out, scope, all);
    }
}


void CDiscrepancyPanel::OnSequester(wxCommandEvent& event)
{
    vector<CSeq_entry_Handle> handles;
    m_Project->GetScope()->GetAllTSEs(handles, CScope::eAllTSEs);
    if (handles.size() != 1) {
        return;
    }
    bool all = true;
    vector<CCustomTreeItem*> all_items = m_Left->GetAllItems();
    for (auto& w : all_items) {
        if (w->IsChecked()) {
            all = false;
            break;
        }
    }
    //if (all && wxOK != wxMessageBox("No items selected! Sequester all?", "", wxICON_NONE | wxOK | wxCANCEL | wxOK_DEFAULT | wxCENTRE)) {
    //    return;
    //}
    set<CBioseq_Handle> selected;
    vector<CCustomTreeItem*> items = m_Left->GetRootItems();
    for (auto& w : items) {
        Sequester(*w, selected, handles[0].GetScope(), all);
    }
    CConstRef<CSeq_submit> seq_submit;
    for (auto& it : m_Project->GetData().GetItems()) {
        const CSeq_submit* ss = dynamic_cast<const CSeq_submit*>(it->GetObject());
        if (ss) {
            seq_submit.Reset(ss);
        }
    }
    CSequesterSets* dlg = new CSequesterSets(GetParent(), handles[0], seq_submit, m_Workbench);
    dlg->SetSubsets(selected);
    dlg->Show(true);
}


void CDiscrepancyDlg::OnSequester(wxCommandEvent& event)
{
    m_Panel->OnSequester(event);
}


void CDiscrepancyPanel::OnListDblClk(wxCommandEvent& event)
{
    int n = m_Right->GetSelection();
    if (n == wxNOT_FOUND) {
        return;
    }
    CDiscRepItem* item = (CDiscRepItem*)m_Left->GetCurrentItem();
    if (!item) {
        item = (CDiscRepItem*)&*m_Left->GetRootItems()[0];
    }
    TReportObjectList det = item->GetItem()->GetDetails();
    if (n >= det.size()) {
        return;
    }
    OpenEditor(m_Tests->FindObject(*det[n]), *m_Project->GetScope());
}


void CDiscrepancyPanel::OpenEditor(const CSerialObject* ref, CScope& scope)
{
    if (dynamic_cast<const CBioseq*>(ref)) {
        //wxMessageBox("No editor available for this type!");
        return;
    }
    if (InvalidObject(&*ref, scope)) {
        wxMessageBox(m_NeedRefresh ? "Please refresh!" : "Oops! We have some problem...");
        return;
    }

    SConstScopedObject obj(ref, &scope);
    CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(obj);
    CIRef<IEditObject> editor = CreateEditorForObject(obj.object, seh, false);
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(this, false);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    editorWindow->TransferDataToWindow();
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    editorWindow->GetParent()->CenterOnParent();
    edit_dlg->Show(true);
}


bool CDiscrepancyPanel::InvalidObject(const CSerialObject* obj, CScope& scope)
{
    const CSeq_feat* seqFeat = dynamic_cast<const CSeq_feat*>(obj);
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj);
    const CBioseq* seq = dynamic_cast<const CBioseq*>(obj);
    const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(obj);

    if (seq) {
        return !scope.GetBioseqHandle(*seq, CScope::eMissing_Null);
    }
    else if (entry && entry->IsSeq()) {
        return !scope.GetBioseqHandle(entry->GetSeq(), CScope::eMissing_Null);
    }
    else  if (seqFeat) {
        return !scope.GetSeq_featHandle(*seqFeat, CScope::eMissing_Null);
    }
    else if (desc) {
        CRef<objects::CScope> sc(&scope);
        try {
            macro::NMacroUtil::GetBioseqForSeqdescObject(desc, edit::GetSeqEntryForSeqdesc(sc, *desc));
        }
        catch(CException &e) { return true; } catch (exception &e) { return true; }         
        return false;
    }
    return false;
}


CBioseq_Handle CDiscrepancyPanel::GetAccessionForObject(const CSerialObject* obj)
{
    CBioseq_Handle bsh;

    CScope &scope = *m_Project->GetScope();

    const CSeq_feat* seqFeat = dynamic_cast<const CSeq_feat*>(obj);
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj);
    const CBioseq* seq = dynamic_cast<const CBioseq*>(obj);
    const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(obj);

    if (seq) {
        bsh = scope.GetBioseqHandle(*seq);
    }
    else if (entry && entry->IsSeq()) {
        bsh = scope.GetBioseqHandle(entry->GetSeq());
    }
    else  if (seqFeat) {
        bsh = scope.GetBioseqHandle(seqFeat->GetLocation());
    }
    else if (desc) {
        CRef<objects::CScope> sc(&scope);
        CConstRef<CBioseq> seq = macro::NMacroUtil::GetBioseqForSeqdescObject(desc, edit::GetSeqEntryForSeqdesc(sc, *desc));
        bsh = scope.GetBioseqHandle(*seq);
    }

    return bsh;
}


const CSerialObject* CDiscrepancyPanel::GetTopObject(const CSerialObject* obj)
{
    const CBioseq* bs = dynamic_cast<const CBioseq*>(obj);
    const CSeq_feat* sf = dynamic_cast<const CSeq_feat*>(obj);
    const CSeqdesc* sd = dynamic_cast<const CSeqdesc*>(obj);
    const CProjectFolder::TItems& items = m_Project->GetData().GetItems();
    if (bs) {
        for (auto& it : items) {
            const CSerialObject* top = it->GetObject();
            for (CTypeConstIterator<CBioseq> I(*top); I; ++I) {
                if (&*I == bs) {
                    return top;
                }
            }
        }
        return obj;
    }
    if (sf) {
        for (auto& it : items) {
            const CSerialObject* top = it->GetObject();
            for (CTypeConstIterator<CSeq_feat> I(*top); I; ++I) {
                if (&*I == sf) {
                    return top;
                }
            }
        }
        return obj;
    }
    if (sd) {
        for (auto& it : items) {
            const CSerialObject* top = it->GetObject();
            for (CTypeConstIterator<CSeqdesc> I(*top); I; ++I) {
                if (&*I == sd) {
                    return top;
                }
            }
        }
        return obj;
    }
    return obj;
}

IProjectView* CDiscrepancyPanel::FindGraphicalSequenceView(CConstRef<CSeq_id> id)
{ 
    if (!id)
        return nullptr;

    const CGBDocument* doc = dynamic_cast<const CGBDocument*>(m_Project);        
    if (!doc)
        return nullptr;

    CIRef<IProjectView> prj_view;
    const vector< CIRef<IProjectView> >& views =  doc->GetViews();

    for (const auto & view : views)
    {
        if (view->GetLabel(IProjectView::eType) == "Graphical Sequence View")
        {
            prj_view = view;
            break;
        }
    }
    if (!prj_view)
        return nullptr;

    TConstScopedObjects objects;
    prj_view->GetMainObject(objects);
    CConstRef<CSeq_id> other_id(dynamic_cast<const CSeq_id*>(objects.front().object.GetPointer()));
    if (!other_id || !sequence::IsSameBioseq(*id, *other_id, m_Project->GetScope()))
    {
        SConstScopedObject obj(id, m_Project->GetScope());
        prj_view->SetOrigObject(obj);
    }
    return prj_view.GetPointer();
}

void CDiscrepancyPanel::OnListClk(wxCommandEvent& event)
{
    int n = m_Right->GetSelection();
    if (n == wxNOT_FOUND) {
        return;
    }
    CDiscRepItem* item = (CDiscRepItem*)m_Left->GetCurrentItem();
    if (!item) {
        item = (CDiscRepItem*)&*m_Left->GetRootItems()[0];
    }
    TReportObjectList det = item->GetItem()->GetDetails();
    if (n >= det.size()) {
        return;
    }

    CScope& scope = *m_Project->GetScope();
    const CSerialObject* obj = m_Tests->FindObject(*det[n]);
    if (InvalidObject(obj, scope)) {
        wxMessageBox(m_NeedRefresh ? "Please refresh!" : "Oops! We have some problem...");
        return;
    }

    CIRef<IFlatFileCtrl> FlatFile;
    const CSerialObject* top = GetTopObject(obj);
    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    CIRef<IProjectView> pTextView(prjSrv->FindView(*top, "Text View"));
    if (pTextView) 
    {
        FlatFile.Reset(dynamic_cast<IFlatFileCtrl*>(pTextView.GetPointerOrNull()));
    }


    CBioseq_Handle bsh = GetAccessionForObject(obj);
    const CBioseq* seq = dynamic_cast<const CBioseq*>(obj);
    if (seq) {
        obj = sequence::GetMolInfo(scope.GetBioseqHandle(*seq));
    }
    
    if (bsh)
    {
        if (FlatFile)
            FlatFile->SetPosition(bsh, obj);
        
        
        IProjectView* gs_view = FindGraphicalSequenceView(sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId());
        if (gs_view) 
        {
            CGraphicPanel* panel = dynamic_cast<CGraphicPanel*>(gs_view->GetWindow());
            if (panel)
            {
                CSeqGraphicWidget* widget = panel->GetWidget();
                if (widget && bsh == widget->GetCurrentBioseq())
                {
                    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(obj);
                    if (feat && feat->IsSetLocation())
                        widget->ZoomOnRange(feat->GetLocation().GetTotalRange(), CSeqGraphicWidget::fAddMargins);
                }
            }
        }
    }

    TConstScopedObjects objs;
    objs.push_back(SConstScopedObject(obj, &scope));
    m_SelectionClient->SetSelectedObjects(objs);

    if (m_Host) {
        dynamic_cast<wxWindow*>(m_Host)->Raise();
    }
}


void CDiscrepancyPanel::OnRefresh(wxCommandEvent& event)
{
    RefreshData();
}

void CDiscrepancyPanel::RefreshData()
{   
    RunTests();
    if (!RunningInsideNCBI()) {
        ExpandAll();
    }
}


void CDiscrepancyDlg::OnRefresh(wxCommandEvent& event)
{
    m_Panel->RefreshData();
    NEditingStats::ReportUsage(GetLabel());
}


static void ReportAutofix(CAutofixReport& res, map<string, unsigned int>& out)
{
    auto& subs = res.GetSubitems();
    if (!subs.empty()) {
        for (auto r: subs) {
            ReportAutofix(*r, out);
        }
    }
    else {
        out[res.GetS()] += res.GetN();
    }
}


void CDiscrepancyPanel::CollectAutofix(CCustomTreeItem& item, TReportObjectList& tofix)
{
    CDiscRepItem& disc = (CDiscRepItem&)item;
    if (item.IsActive()) {
        if (item.IsChecked() && disc.IsReal()) {
            for (auto obj : disc.GetItem()->GetDetails()) {
                if (obj->CanAutofix()) {
                    tofix.push_back(CRef<CReportObj>(&*obj));
                }
            }
        }
        else {
            vector<CCustomTreeItem*> items = disc.GetChildren();
            for (auto w : items) {
                CollectAutofix(*w, tofix);
            }
        }
    }
}


void CDiscrepancyPanel::OnAutofix(wxCommandEvent& event)
{
    if (m_NeedRefresh) {
        wxMessageBox("Please refresh!");
        return;
    }
    map<string, size_t> Map;
    TReportObjectList tofix;
    vector<CCustomTreeItem*> items = m_Left->GetRootItems();
    for (auto w : items) {
        CollectAutofix(*w, tofix);
    }
    m_Tests->Autofix(tofix, Map);
    if (!Map.size()) {
        return;
    }
    CRef<CCmdComposite> cmd(new CCmdComposite("Discrepancy Autofix"));
    m_CmdProccessor->Execute(cmd);

    string s;
    for (auto J : Map) {
        s += CDiscrepancySet::Format(J.first, J.second) + "\n";
    }
    CGenericReportDlg* report = new CGenericReportDlg(NULL);
    report->SetTitle("Autofix Report");
    report->SetText(s);
    report->Show(true);
    if (m_Type != CDiscrepancyDlg::eCdsTrnaOverlap) {
        RunTests();
    }
}


void CDiscrepancyDlg::OnAutofix(wxCommandEvent& event)
{
    m_Panel->OnAutofix(event);
}


void CDiscrepancyPanel::Report(CCustomTreeItem& item, string& out, bool all)
{
    CDiscRepItem& disc = (CDiscRepItem&)item;
    vector<CCustomTreeItem*> items = disc.GetChildren();
    if ((all || item.IsChecked()) && disc.IsReal()) {
        out += (m_ShowTestName ? disc.GetItem()->GetTitle() + ": " : "") + (m_Type != eOncaller && disc.GetItem()->IsFatal() ? "FATAL! " : "") + disc.GetItem()->GetMsg() + "\n";
        if (items.empty()) {
            TReportObjectList det = disc.GetItem()->GetDetails();
            for (auto& obj : det) {
                out += obj->GetText() + "\n";
            }
            out += "\n";
        }
    }
    for (auto& w : items) {
        Report(*w, out, all);
    }
}


void CDiscrepancyPanel::OnReport(wxCommandEvent& evt)
{
    bool all = true;
    vector<CCustomTreeItem*> all_items = m_Left->GetAllItems();
    for (auto& w : all_items) {
        if (w->IsChecked()) {
            all = false;
            break;
        }
    }
    if (all && wxOK != wxMessageBox("No discrepancies selected! Export all?", "", wxICON_NONE | wxOK | wxCANCEL | wxOK_DEFAULT | wxCENTRE)) {
        return;
    }

    string out;
    vector<CCustomTreeItem*> items = m_Left->GetRootItems();
    for (auto& w : items) {
        Report(*w, out, all);
    }
    if (out.empty()) {
        out = "No items to report...";
    }
    CGenericReportDlg* report = new CGenericReportDlg(NULL);
    report->SetLineSpacing(10);
    report->SetTitle("Discrepancy Report");
    report->SetText(out);
    report->SetWorkDir(m_WorkDir);
    report->Show(true);
}


void CDiscrepancyDlg::OnReport(wxCommandEvent& event)
{
    m_Panel->OnReport(event);
}


void CDiscrepancyPanel::OnFindText(wxCommandEvent& event)
{
    string find = m_FindText->GetLineText(0).Upper().ToStdString();
    if (find.empty()) {
        m_PrevBtn->Disable();
        m_NextBtn->Disable();
        return;
    }
    vector<CCustomTreeItem*> items = m_Left->GetAllItems();
    CCustomTreeItem* item = m_Left->GetCurrentItem();
    size_t current = 0;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i] == item) {
            current = i;
            break;
        }
    }
    size_t count = 0;
    CCustomTreeItem* found = 0;
    for (size_t i = current; i < items.size(); i++) {
        string str = items[i]->GetText();
        NStr::ToUpper(str);
        if (str.find(find) != string::npos) {
            if (!found) {
                found = items[i];
            }
            count++;
        }
    }
    for (size_t i = 0; i < current; i++) {
        string str = items[i]->GetText();
        NStr::ToUpper(str);
        if (str.find(find) != string::npos) {
            if (!found) {
                found = items[i];
            }
            count++;
        }
    }
    if (found && found != item) {
        m_Left->SetCurrentItem(found);
        wxTreeListEvent evt;
        OnClick(evt);
    }
    m_PrevBtn->Enable(count > 1);
    m_NextBtn->Enable(count > 1);
}


void CDiscrepancyPanel::OnFindNext(wxCommandEvent& event)
{
    string find = m_FindText->GetLineText(0).Upper().ToStdString();
    if (find.empty()) {
        m_PrevBtn->Disable();
        m_NextBtn->Disable();
        return;
    }
    vector<CCustomTreeItem*> items = m_Left->GetAllItems();
    CCustomTreeItem* item = m_Left->GetCurrentItem();
    size_t current = 0;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i] == item) {
            current = i + 1;
            break;
        }
    }
    CCustomTreeItem* found = 0;
    for (size_t i = current; i < items.size(); i++) {
        string str = items[i]->GetText();
        NStr::ToUpper(str);
        if (str.find(find) != string::npos) {
            if (!found) {
                found = items[i];
            }
        }
    }
    for (size_t i = 0; i < current; i++) {
        string str = items[i]->GetText();
        NStr::ToUpper(str);
        if (str.find(find) != string::npos) {
            if (!found) {
                found = items[i];
            }
        }
    }
    if (found && found != item) {
        m_Left->SetCurrentItem(found);
        wxTreeListEvent evt;
        OnClick(evt);
    }
}


void CDiscrepancyPanel::OnFindPrev(wxCommandEvent& event)
{
    string find = m_FindText->GetLineText(0).Upper().ToStdString();
    if (find.empty()) {
        m_PrevBtn->Disable();
        m_NextBtn->Disable();
        return;
    }
    vector<CCustomTreeItem*> items = m_Left->GetAllItems();
    CCustomTreeItem* item = m_Left->GetCurrentItem();
    size_t current = 0;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i] == item) {
            current = i;
            break;
        }
    }
    CCustomTreeItem* found = 0;
    for (size_t i = current; i < items.size(); i++) {
        string str = items[i]->GetText();
        NStr::ToUpper(str);
        if (str.find(find) != string::npos) {
            found = items[i];
        }
    }
    for (size_t i = 0; i < current; i++) {
        string str = items[i]->GetText();
        NStr::ToUpper(str);
        if (str.find(find) != string::npos) {
            found = items[i];
        }
    }
    if (found && found != item) {
        m_Left->SetCurrentItem(found);
        wxTreeListEvent evt;
        OnClick(evt);
    }
}


void CDiscrepancyPanel::OnTestName()
{
    m_ShowTestName = !m_ShowTestName;
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    reg.GetWriteView(RegisterRoot(false), REGISTER_MAGIC_NUMBER_DISCREPANCY).Set("ShowTestNames", m_ShowTestName);
    WriteSettings(reg);
    m_Left->Refresh();
}


void CDiscrepancyDlg::OnTestName(wxCommandEvent& event)
{
    m_Panel->OnTestName();
    m_TestNameBtn->SetLabel(m_Panel->m_ShowTestName ? "Hide Test Names" : "Show Test Names");
}


void CDiscrepancyDlg::OnReopen(wxCommandEvent& evt)
{
    m_AutoReopen = m_Reopen->GetValue();
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    reg.GetWriteView(RegisterRoot(false), REGISTER_MAGIC_NUMBER_DISCREPANCY).Set("AutoReopen", m_AutoReopen);
    WriteSettings(reg);
}


void CDiscrepancyDlg::OnSettings(wxCommandEvent& event)
{
    CDiscrepancyConf* dlg = new CDiscrepancyConf(m_Type, &m_Panel->m_TestList, &m_Panel->m_DefaultTests, &m_Panel->m_AddTests, &m_Panel->m_RemoveTests, this);
    dlg->UpdateList();
    dlg->Show(true);
}


/// Discrepancy Settings Dialog

IMPLEMENT_DYNAMIC_CLASS(CDiscrepancyConf, wxDialog)


BEGIN_EVENT_TABLE(CDiscrepancyConf, wxDialog)
    EVT_TREELIST_SELECTION_CHANGED(ID_DISCR_LIST, CDiscrepancyConf::OnTreeListCheck)
    EVT_TREELIST_ITEM_CHECKED(ID_DISCR_LIST, CDiscrepancyConf::OnTreeListCheck)
    EVT_BUTTON(ID_DISCR_ALL, CDiscrepancyConf::OnSelectAll)
    EVT_BUTTON(ID_DISCR_NONE, CDiscrepancyConf::OnSelectNone)
    EVT_BUTTON(ID_DISCR_DEFAULT, CDiscrepancyConf::OnSelectDefault)
    EVT_BUTTON(ID_DISCR_CHECK, CDiscrepancyConf::OnCheck)
    EVT_BUTTON(ID_DISCR_UNCHECK, CDiscrepancyConf::OnUncheck)
    EVT_BUTTON(ID_DISCR_TOGGLE, CDiscrepancyConf::OnToggle)
    EVT_BUTTON(ID_DISCR_DETAILS, CDiscrepancyConf::OnDetails)
    EVT_BUTTON(ID_DISCR_SPR, CDiscrepancyConf::OnProductRules)
    EVT_BUTTON(ID_DISCR_OK, CDiscrepancyConf::OnOk)
    EVT_BUTTON(ID_DISCR_CLOSE, CDiscrepancyConf::OnClose)
    EVT_TEXT(ID_DISCR_FIND_TEXT, CDiscrepancyConf::OnFindText)
    EVT_BUTTON(ID_DISCR_NEXT, CDiscrepancyConf::OnFindNext)
    EVT_BUTTON(ID_DISCR_PREV, CDiscrepancyConf::OnFindPrev)
END_EVENT_TABLE()


CDiscrepancyConf::CDiscrepancyConf()
{
    Init();
}


CDiscrepancyConf::CDiscrepancyConf(CDiscrepancyDlg::EReportType type, vector<string>* t, vector<string>* dt, vector<string>* at, vector<string>* rt, CDiscrepancyDlg* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_Type(type), m_Tests(t), m_DefaultTests(dt), m_AddTests(at), m_RemoveTests(rt), m_DiscrepancyDlg(parent)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CDiscrepancyConf::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);
    m_OldProductRules = m_DiscrepancyDlg->GetProductRules();
    m_NewProductRules = m_OldProductRules;
    CreateControls();
    if (GetSizer()) {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}


void CDiscrepancyConf::Init()
{
};


void CDiscrepancyConf::CreateControls()
{
    wxBoxSizer* box0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(box0);
    m_List = new wxTreeListCtrl(this, ID_DISCR_LIST, wxDefaultPosition, wxSize(500, 800), wxTL_CHECKBOX | wxTL_MULTIPLE | wxTL_NO_HEADER);
    m_List->AppendColumn("");
    m_List->AppendColumn("");
    box0->Add(m_List, 1, wxGROW | wxALL, 0);

    wxBoxSizer* box1 = new wxBoxSizer(wxHORIZONTAL);
    m_PrevBtn = new wxButton(this, ID_DISCR_PREV, "<<", wxDefaultPosition, wxSize(30, -1), 0);
    box1->Add(m_PrevBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_FindText = new wxTextCtrl(this, ID_DISCR_FIND_TEXT, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0);
    box1->Add(m_FindText, 1, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_NextBtn = new wxButton(this, ID_DISCR_NEXT, ">>", wxDefaultPosition, wxSize(30, -1), 0);
    box1->Add(m_NextBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_PrevBtn->Disable();
    m_NextBtn->Disable();
    box0->Add(box1, 0, wxGROW | wxALL, 5);

    wxBoxSizer* box2 = new wxBoxSizer(wxHORIZONTAL);
    m_SelAll = new wxButton(this, ID_DISCR_ALL, "Sel. All", wxDefaultPosition, wxDefaultSize, 0);
    box2->Add(m_SelAll, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_SelNon = new wxButton(this, ID_DISCR_NONE, "Sel. None", wxDefaultPosition, wxDefaultSize, 0);
    box2->Add(m_SelNon, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_SelDef = new wxButton(this, ID_DISCR_DEFAULT, "Sel. Default", wxDefaultPosition, wxDefaultSize, 0);
    box2->Add(m_SelDef, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_Check = new wxButton(this, ID_DISCR_CHECK, "Check Sel.", wxDefaultPosition, wxDefaultSize, 0);
    box2->Add(m_Check, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_Unheck = new wxButton(this, ID_DISCR_UNCHECK, "Uncheck Sel.", wxDefaultPosition, wxDefaultSize, 0);
    box2->Add(m_Unheck, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_Toggle = new wxButton(this, ID_DISCR_TOGGLE, "Toggle Sel.", wxDefaultPosition, wxDefaultSize, 0);
    box2->Add(m_Toggle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    box0->Add(box2, 0, wxGROW | wxALL, 5);

    wxBoxSizer* box3 = new wxBoxSizer(wxHORIZONTAL);
    wxButton* buttonSPR = new wxButton(this, ID_DISCR_SPR, "Change", wxDefaultPosition, wxDefaultSize, 0);
    box3->Add(buttonSPR, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_ProductRulesLabel = new wxStaticText(this, 0, ProductRulesStr(m_NewProductRules), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
    box3->Add(m_ProductRulesLabel, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    box0->Add(box3, 0, wxGROW | wxALL, 5);

    wxBoxSizer* box4 = new wxBoxSizer(wxHORIZONTAL);
    wxButton* buttonOk = new wxButton(this, ID_DISCR_OK, "OK", wxDefaultPosition, wxDefaultSize, 0);
    box4->Add(buttonOk, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    wxButton* closeButton = new wxButton(this, ID_DISCR_CLOSE, "Cancel", wxDefaultPosition, wxDefaultSize, 0);
    box4->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    box0->Add(box4, 0, wxGROW | wxALL, 5);

    switch (m_Type) {
        case CDiscrepancyDlg::eNormal:
            SetLabel("Discrepancy Report Settings");
            break;
        case CDiscrepancyDlg::eOncaller:
            SetLabel("Oncaller Tool Settings");
            break;
        case CDiscrepancyDlg::eSubmitter:
            SetLabel("Submitter Report Settings");
            break;
        case CDiscrepancyDlg::eMega:
            SetLabel("Mega Report Settings");
            break;
    }
}


void CDiscrepancyConf::OnDetails(wxCommandEvent& event)
{
    m_Details = !m_Details;
    m_Show->SetLabel(m_Details ? "Hide Details" : "Show Details");
    UpdateList();
}


void CDiscrepancyConf::OnProductRules(wxCommandEvent& event)
{
    if (m_NewProductRules.empty()) {
        wxFileDialog fd(this, "Product Rules", "", "", "Product rule files (*.prt)|*.prt|Text files (*.txt)|*.txt|All files (*.*)|*.*");
        if (fd.ShowModal() == wxID_OK) {
            m_NewProductRules = fd.GetPath();
        }
    }
    else {
        m_NewProductRules.clear();
    }
    m_ProductRulesLabel->SetLabel(ProductRulesStr(m_NewProductRules));
}


void CDiscrepancyConf::OnOk(wxCommandEvent& event)
{
    vector<string> tests = *m_Tests;
    m_Tests->clear();
    m_AddTests->clear();
    m_RemoveTests->clear();
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        string name = m_List->GetItemText(item).ToStdString();
        if (m_List->GetCheckedState(item) == wxCHK_CHECKED) {
            m_Tests->push_back(name);
            if (!m_MapDefaultTests[name]) {
                m_AddTests->push_back(name);
            }
        }
        else {
            if (m_MapDefaultTests[name]) {
                m_RemoveTests->push_back(name);
            }
        }
    }

    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    CRegistryWriteView reg_view = reg.GetWriteView(m_DiscrepancyDlg->RegisterRoot(), REGISTER_MAGIC_NUMBER_DISCREPANCY);
    reg_view.Set("Add", *m_AddTests);
    reg_view.Set("Rem", *m_RemoveTests);

    bool changed = true;
    if (tests.size() == m_Tests->size()) {
        changed = false;
        for (int i = 0; i < tests.size(); i++) {
            if (tests[i] != (*m_Tests)[i]) {
                changed = true;
                break;
            }
        }
    }
    if (m_OldProductRules != m_NewProductRules) {
        changed = true;
        CRegistryWriteView reg_view = reg.GetWriteView(m_DiscrepancyDlg->RegisterRoot(false), REGISTER_MAGIC_NUMBER_DISCREPANCY);
        m_DiscrepancyDlg->SetProductRules(m_NewProductRules);
        reg_view.Set("ProductRules", m_NewProductRules);
    }
    if (changed) {
        m_DiscrepancyDlg->WriteSettings(reg);
        m_DiscrepancyDlg->OnDataChanged(0);
    }
    Destroy();
}


void CDiscrepancyConf::OnClose(wxCommandEvent& event)
{
    Destroy();
}


void CDiscrepancyConf::OnFindText(wxCommandEvent& event)
{
    string find = m_FindText->GetLineText(0).Upper().ToStdString();
    if (find.empty()) {
        m_PrevBtn->Disable();
        m_NextBtn->Disable();
        return;
    }
    size_t count = 0;
    bool looking = true;
    bool selected = false;
    wxTreeListItem sel;
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        string name = m_List->GetItemText(item).Upper().ToStdString();
        if (m_List->IsSelected(item)) {
            selected = true;
        }
        if (name.find(find) != string::npos) {
            if (!count) {
                sel = item;
            }
            if (looking && selected) {
                sel = item;
                looking = false;
            }
            count++;
        }
    }
    m_List->UnselectAll();
    m_List->Select(sel);
    m_List->EnsureVisible(sel);   // need to upgrade to wsWidgets >=3.1.0
    m_PrevBtn->Enable(count > 1);
    m_NextBtn->Enable(count > 1);
    UpdateButtons();
}


void CDiscrepancyConf::OnFindNext(wxCommandEvent& event)
{
    string find = m_FindText->GetLineText(0).Upper().ToStdString();
    if (find.empty()) {
        return;
    }
    size_t count = 0;
    bool selected = false;
    wxTreeListItem sel;
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        string name = m_List->GetItemText(item).Upper().ToStdString();
        if (name.find(find) != string::npos) {
            if (!count) {
                sel = item;
            }
            if (selected) {
                sel = item;
                break;
            }
            count++;
        }
        if (m_List->IsSelected(item)) {
            selected = true;
        }
    }
    m_List->UnselectAll();
    m_List->Select(sel);
    m_List->EnsureVisible(sel);   // need to upgrade to wsWidgets >=3.1.0
    UpdateButtons();
}


void CDiscrepancyConf::OnFindPrev(wxCommandEvent& event)
{
    string find = m_FindText->GetLineText(0).Upper().ToStdString();
    if (find.empty()) {
        return;
    }
    size_t count = 0;
    bool selected = false;
    wxTreeListItem sel;
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        string name = m_List->GetItemText(item).Upper().ToStdString();
        if (m_List->IsSelected(item) && !selected) {
            selected = true;
            if (count) {
                break;
            }
        }
        if (name.find(find) != string::npos) {
            sel = item;
            count++;
        }
    }
    m_List->UnselectAll();
    m_List->Select(sel);
    m_List->EnsureVisible(sel);   // need to upgrade to wsWidgets >=3.1.0
    UpdateButtons();
}


void CDiscrepancyConf::UpdateList()
{
    if (!m_MapDefaultTests.empty()) {
        return;
    }
    for (auto& name : *m_DefaultTests) {
        m_MapDefaultTests[name] = true;
    }
    for (auto& name : *m_Tests) {
        m_MapTests[name] = true;
    }
    vector<string> AllTests = GetDiscrepancyNames();
    wxTreeListItem root = m_List->GetRootItem();
    for (auto& name : AllTests) {
        if (name[0] == '_') {
            continue;
        }
        wxTreeListItem item = m_List->AppendItem(root, name, -1, -1);
        if (m_MapTests[name]) {
            m_List->CheckItemRecursively(item);
        }
    }
    UpdateButtons();
}


void CDiscrepancyConf::UpdateButtons()
{
    bool scd = false; // selected checked default
    bool scn = false; // selected checked not default
    bool sud = false; // selected unchecked default
    bool sun = false; // selected unchecked not default
    bool ucd = false; // unselected checked default
    bool ucn = false; // unselected checked not default
    bool uud = false; // unselected unchecked default
    bool uun = false; // unselected unchecked not default

    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        string name = m_List->GetItemText(item).ToStdString();
        bool def = m_MapDefaultTests[name];
        bool chk = m_List->GetCheckedState(item) == wxCHK_CHECKED;
        bool sel = m_List->IsSelected(item);
        if (sel) {
            if (chk) {
                (def ? scd : scn) = true;
            }
            else {
                (def ? sud : sun) = true;
            }

        }
        else {
            if (chk) {
                (def ? ucd : ucn) = true;
            }
            else {
                (def ? uud : uun) = true;
            }
        }
    }
    m_SelAll->Enable(ucd || ucn || uud || uun);
    m_SelNon->Enable(scd || scn || sud || sun);
    m_SelDef->Enable(scn || sun || ucd || uud);
    m_Check->Enable(sud || sun);
    m_Unheck->Enable(scd || scn);
    m_Toggle->Enable(scd || scn || sud || sun);
}


void CDiscrepancyConf::OnTreeListCheck(wxTreeListEvent& event)
{
    UpdateButtons();
}


void CDiscrepancyConf::OnSelectAll(wxCommandEvent& event)
{
    m_List->SelectAll();
    UpdateButtons();
}


void CDiscrepancyConf::OnSelectNone(wxCommandEvent& event)
{
    m_List->UnselectAll();
    UpdateButtons();
}


void CDiscrepancyConf::OnSelectDefault(wxCommandEvent& event)
{
    m_List->UnselectAll();
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        string name = m_List->GetItemText(item).ToStdString();
        if (m_MapDefaultTests[name]) {
            m_List->Select(item);
        }
    }
    UpdateButtons();
}


void CDiscrepancyConf::OnCheck(wxCommandEvent& event)
{
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        if (m_List->IsSelected(item)) {
            m_List->CheckItemRecursively(item);
        }
    }
    UpdateButtons();
}


void CDiscrepancyConf::OnUncheck(wxCommandEvent& event)
{
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        if (m_List->IsSelected(item)) {
            m_List->CheckItemRecursively(item, wxCHK_UNCHECKED);
        }
    }
    UpdateButtons();
}


void CDiscrepancyConf::OnToggle(wxCommandEvent& event)
{
    for (wxTreeListItem item = m_List->GetFirstItem(); item.IsOk(); item = m_List->GetNextItem(item)) {
        if (m_List->IsSelected(item)) {
            if (m_List->GetCheckedState(item) == wxCHK_UNCHECKED) {
                m_List->CheckItemRecursively(item, wxCHK_CHECKED);
            }
            else {
                m_List->CheckItemRecursively(item, wxCHK_UNCHECKED);
            }
        }
    }
    UpdateButtons();
}


/// Discrepancy List

IMPLEMENT_DYNAMIC_CLASS(CDiscrepancyList, wxFrame)


BEGIN_EVENT_TABLE(CDiscrepancyList, wxFrame)
    EVT_BUTTON(ID_DISCR_DETAILS, CDiscrepancyList::OnDetails)
    EVT_BUTTON(ID_DISCR_CLOSE, CDiscrepancyList::OnClose)
END_EVENT_TABLE()


CDiscrepancyList::CDiscrepancyList() : m_Details(true)
{
}


CDiscrepancyList::CDiscrepancyList(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) : m_Details(true)
{
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


bool CDiscrepancyList::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create(parent, id, caption, pos, size, style);
    wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
    if (attr.colBg != wxNullColour) {
        SetOwnBackgroundColour(attr.colBg);
    }
    CreateControls();
    if (GetSizer()) {
        GetSizer()->SetSizeHints(this);
    }
    UpdateList();
    Centre();
    return true;
}


void CDiscrepancyList::CreateControls()
{
    wxBoxSizer* box0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(box0);
    m_Text = new wxTextCtrl(this, ID_DISCR_TEXT, wxEmptyString, wxDefaultPosition, wxSize(600, 800), wxTE_MULTILINE | wxTE_READONLY);
    box0->Add(m_Text, 1, wxGROW| wxALL, 0);

    wxBoxSizer* box1 = new wxBoxSizer(wxHORIZONTAL);
    m_Show = new wxButton(this, ID_DISCR_DETAILS, m_Details ? "Hide Details" : "Show Details", wxDefaultPosition, wxDefaultSize, 0);
    box1->Add(m_Show, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    wxButton* closeButton = new wxButton(this, ID_DISCR_CLOSE, "Close", wxDefaultPosition, wxDefaultSize, 0);
    box1->Add(closeButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    box0->Add(box1, 0, wxGROW | wxALL, 5);
}


void CDiscrepancyList::UpdateList()
{
    vector<string> names = GetDiscrepancyNames();
    string s;
    for (size_t i = 0; i < names.size(); i++) {
        if (names[i][0] == '_') {
            continue;
        }
        s += names[i];
        if (m_Details) {
            s += " -- ";
            s += GetDiscrepancyDescr(names[i]);
        }
        s += "\n";
    }
    m_Text->Clear();
    m_Text->AppendText(s);
}


void CDiscrepancyList::OnDetails(wxCommandEvent& event)
{
    m_Details = !m_Details;
    m_Show->SetLabel(m_Details ? "Hide Details" : "Show Details");
    UpdateList();
}


void CDiscrepancyList::OnClose(wxCommandEvent& event)
{
    Destroy();
}


END_NCBI_SCOPE
