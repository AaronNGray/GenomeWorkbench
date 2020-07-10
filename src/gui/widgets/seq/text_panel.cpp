/*  $Id: text_panel.cpp 44278 2019-11-21 19:37:02Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/dcclient.h>
#include <wx/settings.h>

////@begin includes
////@end includes

#include <objmgr/bioseq_ci.hpp>

#include <gui/utils/view_event.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/composite_traverser.hpp>
#include <gui/widgets/text_widget/plain_text_item.hpp>
#include <gui/widgets/seq/fasta_view_params.hpp>

#include <gui/widgets/wx/ieditor_factory.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/aui_toolbar.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/widgets/seq/text_panel.hpp>

#include <gui/widgets/seq/text_ds.hpp>
#include <gui/widgets/seq/asn_view_context.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/expand_traverser.hpp>
#include <gui/widgets/seq/flat_file_header_item.hpp>
#include <gui/widgets/wx/richtextctrl.hpp>
#include <gui/widgets/seq/flat_file_retrieve_job.hpp>
#include <gui/widgets/seq/flat_file_context.hpp>
#include "text_commands.hpp"
#include "fasta_context.hpp"

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/submit/Seq_submit.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/widgets/wx/wx_utils.hpp>


#include "text_view_options_dlg.hpp"
#include "select_sequence_dlg.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CTextPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTextPanel, wxPanel )


/*!
 * CTextPanel event table definition
 */

BEGIN_EVENT_TABLE( CTextPanel, wxPanel )
    EVT_TEXT_ENTER( ID_TB_SearchTerm, CTextPanel::OnSearchTermCtrlEnter )
    EVT_MENU( ID_TB_SearchButton, CTextPanel::OnFindNextClick )
    EVT_CHOICE( ID_TB_SearchModeCtrl, CTextPanel::OnFindModeSelected )
    EVT_CHOICE( ID_TB_SequenceListCtrl, CTextPanel::OnSequenceSelected )
    EVT_MENU( ID_TB_Options, CTextPanel::OnOptionsClick )
    EVT_CONTEXT_MENU( CTextPanel::OnContextMenu )
END_EVENT_TABLE()

static const wxChar* pszFlatFile = wxT("Flat File");
static const wxChar* pszFastA = wxT("FastA");
static const wxChar* pszASN = wxT("ASN");
static const wxChar* pszXML = wxT("XML");

static const wxChar* pszMainToolbar = wxT("Main Toolbar");
static const wxChar* pszSequenceToolbar = wxT("Sequence Toolbar");

static const char* kAllNucleotides = "Nucleotides";
static const char* kAllSequences = "All";
static const char* kMoreSequences = "More...";
const  size_t kMaxSeqNumber = 25;

static wxColor m_BackgroundColor = *wxWHITE;
static bool s_BackgroundColorSet = false;

class CSaveTreeTraverser : public ICompositeTraverser
{
public:
    CSaveTreeTraverser(bm::bvector<>& bVector)
        : m_bVector(bVector), m_Pos(0) {}

    virtual bool ProcessItem(CTextItem& /*textItem*/) { return true; }
    virtual bool ProcessItem(CExpandItem& expandItem)
    {
        m_bVector.set(m_Pos, expandItem.IsExpanded());
        ++m_Pos;
        return true;
    }

private:
    bm::bvector<>& m_bVector;
    int m_Pos;
};

class CRestoreTreeTraverser : public ICompositeTraverser
{
public:
    CRestoreTreeTraverser(CTextPanelContext& context, const bm::bvector<>& bVector)
        : m_Context(context), m_bVector(bVector), m_Pos(0) {}

    virtual bool ProcessItem(CTextItem& /*textItem*/) { return true; }
    virtual bool ProcessItem(CExpandItem& expandItem)
    {
        if ((m_bVector[m_Pos] && !expandItem.IsExpanded()) || (!m_bVector[m_Pos] && expandItem.IsExpanded())) {
            expandItem.Toggle(&m_Context, false);
        }
        ++m_Pos;
        return true;
    }

private:
    CTextPanelContext& m_Context;
    const bm::bvector<>& m_bVector;
    int m_Pos;
};

wxFont CTextPanel::GetFont(int size)
{
    int pointSize = 10;
    switch (size) {
    case 0: pointSize = 8; break;
    case 2: pointSize = 12; break;
    case 3: pointSize = 14; break;
    }

#ifdef __WXOSX_COCOA__
    float mult = GetDisplayPPI().y / (float)wxGetDisplayPPI().y;
    pointSize = (int)(pointSize*mult + .5);
#endif

    return wxFont(pointSize, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
}

void CTextPanel::SetTextBackground(const wxColor& color)
{
    m_BackgroundColor = color;
    s_BackgroundColorSet = true;
}

wxWindow* CTextPanel::CreateForm(wxWindow* parent)
{
    return new CTextPanel(parent);
}

/*!
 * CTextPanel constructors
 */

CTextPanel::CTextPanel()
{
    Init();
}

CTextPanel::CTextPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, pos, size, style);
    CreateControls();
#ifdef _DEBUG
    LOG_POST(Info << "CTextPanel:: Text item count: " << gITextItemCount.Get());
#endif
}

/*!
 * CTextPanel destructor
 */

CTextPanel::~CTextPanel()
{
	UpdateRegistry();
    m_AuiManager.UnInit();

////@begin CTextPanel destruction
////@end CTextPanel destruction

#ifdef _DEBUG
    m_TextWindow->SetMainItem(0, 0);
    LOG_POST(Info << "~CTextPanel:: Text item count: " << gITextItemCount.Get());
#endif
}

/*!
 * Member initialisation
 */

static const char
*kTextViewPath = "GBENCH.TextView",
*kFindMode = "FindMode",
*kViewTypeSeq = "ViewTypeSeq",
*kViewTypeFeat = "ViewTypeFeat",
*kViewTypeGeneric = "ViewTypeGeneric",
*kFontSize = "FontSize",
*kFlatFileMode = "FlatFileMode",
*kShowSequence = "ShowSequence",
*kFlatFileSequence = "kFlatFileSequence",
*kFeatureMode = "FeatureMode",
*kASNShowTypes = "ASNShowTypes",
*kASNShowPathToRoot = "ASNShowPathToRoot",
*kFFHideVariations = "FFHideVariations",
*kFFHideSTS = "FFHideSTS",
*kOpenExpanded = "OpenExpanded";


void CTextPanel::Init()
{
////@begin CTextPanel member initialisation
    m_ViewType = kUninitialized;
    m_FontSize = 1;
    m_FlatFileMode = 0;
    m_ShowSequence = true;
    m_FeatMode = 0;
    m_ShowASNTypes = false;
    m_ShowASNPathToRoot = false;
    m_FindMode = 0;
    m_FFHideVariations = false;
    m_FFHideSTS = false;
    m_OpenExpanded = true;
    m_FindTextCtrl = NULL;
    m_FindModeCtrl = NULL;
    m_SequenceList = NULL;
    m_TextWindow = NULL;
////@end CTextPanel member initialisation
    m_ShowComponentFeats = false;

    m_WidgetHost = 0;
    m_CmdProccessor = 0;
    m_SavedPosX = m_SavedPosY = 0;

    x_ReadRegistry();
}

/*!
 * Control creation for CTextPanel
 */

void CTextPanel::CreateControls()
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    Hide();

    m_DataSource.Reset(new CTextDataSource(*this));

    m_AuiManager.SetManagedWindow(this);

    m_TextWindow = new CTextItemPanel( this, ID_WIDGET, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxHSCROLL|wxVSCROLL );
    m_AuiManager.AddPane(m_TextWindow, wxAuiPaneInfo().CenterPane());

    if (s_BackgroundColorSet)
        m_TextWindow->SetBackgroundColour(m_BackgroundColor);
    else
        m_TextWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    m_TextWindow->SetFocus();

    m_AuiManager.Update();
/*
    wxChoice* textType = (wxChoice*)FindWindow(ID_CHOICE1);
    x_UpdateViewType(textType->GetStringSelection());
 */
    m_TextWindow->SetFont(GetFont(m_FontSize));
}

void CTextPanel::SetFontSize(int value)
{
    if (m_FontSize != value) {
        m_FontSize = value;
        m_TextWindow->SetFont(GetFont(m_FontSize));
    }
}

void CTextPanel::x_CreateMainToolbar()
{
    static bool sImagesRegistered = false;
    if (!sImagesRegistered) {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("text_panel::search"), wxT("search.png"));
        provider->RegisterFileAlias(wxT("text_panel::options"), wxT("options.png"));
        sImagesRegistered = true;
    }

    wxAuiPaneInfo& pane = m_AuiManager.GetPane(pszMainToolbar);
    wxAuiToolBar* toolbar = nullptr;
    wxString searchTerm;

    if (pane.IsOk()) {
        toolbar = (wxAuiToolBar*)pane.window;

        wxControl* ctrl = toolbar->FindControl(ID_TB_SearchModeCtrl);
        if (ctrl) ctrl->Destroy();
        m_FindModeCtrl = 0;

        ctrl = toolbar->FindControl(ID_TB_SearchTerm);
        if (ctrl) {
            searchTerm = ((CRichTextCtrl*)ctrl)->GetValue();
            ctrl->Destroy();
        }
        m_FindTextCtrl = 0;

        ctrl = toolbar->FindControl(ID_TB_SequenceListCtrl);
        if (ctrl) ctrl->Destroy();
        m_SequenceList = 0;

        toolbar->ClearTools();
    }
    else {
        toolbar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
            wxDefaultSize, wxAUI_TB_HORIZONTAL | wxBORDER_NONE);
    }

    toolbar->AddLabel(ID_TB_SearchModeLabel, wxT("Find mode:"));
    wxArrayString strings;
    strings.Add(_("Match case"));
    strings.Add(_("Do not match case"));
    if (m_ViewType == kFlatFile)
        strings.Add(_("Sequence"));
    m_FindModeCtrl = new wxChoice(toolbar, ID_TB_SearchModeCtrl, wxDefaultPosition, wxDefaultSize, strings);
    if (m_FindMode < 0 || m_FindMode >= (int)m_FindModeCtrl->GetCount())
        m_FindMode = 0;
    m_FindModeCtrl->SetSelection(m_FindMode);
    toolbar->AddControl(m_FindModeCtrl);

    m_FindTextCtrl = new CRichTextCtrl(toolbar, ID_TB_SearchTerm, searchTerm, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_FindTextCtrl->SetMaxLength(60);

    wxMenu* menu = new wxMenu();
    menu->Append(wxID_COPY, _("&Copy"));
    menu->Append(wxID_PASTE, _("&Paste"));
    m_FindTextCtrl->SetContextMenu(menu);

    toolbar->AddControl(m_FindTextCtrl, wxT("Enter search term"));

    toolbar->AddTool(ID_TB_SearchButton, wxT("Search"), wxArtProvider::GetBitmap(wxT("text_panel::search")), wxT("Search"));

    toolbar->AddSeparator();
    toolbar->AddTool(ID_TB_Options, wxT("Options"), wxArtProvider::GetBitmap(wxT("text_panel::options")), wxT("Options"));

    if (m_FlatFileSeqs.size() > 0) {
        toolbar->AddSeparator();
        toolbar->AddLabel(ID_TB_SequenceLabel, wxT("Sequence(s):"));
        m_SequenceList = new wxChoice(toolbar, ID_TB_SequenceListCtrl);
        toolbar->AddControl(m_SequenceList);
    }

    toolbar->AddStretchSpacer();

    // As part of auto resize tollbar is resized in vertical position for a moment
    // this is visible on MS windows screen - so hide it
    toolbar->Hide();
    toolbar->Realize();
    toolbar->Show();

    if (!pane.IsOk())
        m_AuiManager.AddPane(toolbar, wxAuiPaneInfo()
        .Name(pszMainToolbar)
        .Caption(pszMainToolbar)
        .ToolbarPane()
        .Top()
        .TopDockable(true)
        .LeftDockable(false)
        .RightDockable(false)
        .BottomDockable(false)
        .Floatable(false)
        .Resizable(true)
        .DockFixed()
        .Gripper(false)
        .DestroyOnClose(true)
        .Position(0));
    else
        pane.BestSize(toolbar->GetClientSize());

    m_AuiManager.Update();
}

void CTextPanel::GetMainObjects(TConstScopedObjects& objs)
{
    SConstScopedObject scoped_object;
    scoped_object.object = m_SO;
    scoped_object.scope = m_Scope;
    objs.push_back(scoped_object);
}

void CTextPanel::Load()
{
    m_TextWindow->ReportLoading();

    m_FlatFileSeqs.clear();

    x_CreateMainToolbar();

    Layout();

    wxBusyCursor wait;

    string errMsg;
    try {
        switch(m_ViewType) {
        case kFlatFile:
            x_LoadFlatFile();
            break;
        case kFastA:
            x_LoadFastA();
            break;
        case kXML:
        default:
            x_LoadASN();
            break;
        }
    } catch (const CException& e) {
        errMsg = e.GetMsg();
    } catch (const exception& e) {
        errMsg = e.what();
    }

    if (!errMsg.empty())
        m_TextWindow->ReportError("Error Loading Data: " + errMsg);
}

static const CSeq_entry* GetSeqEntryFromSeqSubmit(const CSerialObject* so) {
    const CSeq_entry* seq_entry = 0;
    const CSeq_submit* seq_submit = dynamic_cast<const CSeq_submit*>(so);
    if (seq_submit) {
        if (seq_submit->IsSetData()) {
            const CSeq_submit::TData& data = seq_submit->GetData();
            if (data.IsEntrys()) {
                const CSeq_submit::TData::TEntrys& entries = data.GetEntrys();
                if (!entries.empty()) {
                    seq_entry = *entries.begin();
                }
            }
        }
    }
    return seq_entry;
}

static CSeq_entry_Handle GetSeqEntryHandle(CScope& scope, const CSerialObject& so)
{
    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(&so);
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(&so);
    const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*>(&so);
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(&so);
    const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set*>(&so);

    if (!seq_entry) {
        seq_entry = GetSeqEntryFromSeqSubmit(&so);
    }

    CSeq_entry_Handle seh;

    if (seq_entry)
        seh = scope.GetSeq_entryHandle(*seq_entry);
    else if (bioseq) {
        CBioseq_Handle handle = scope.GetBioseqHandle(*bioseq);
        if (handle)
            seh = handle.GetTopLevelEntry();
    }
    else if (bioseq_set) {
        CBioseq_set_Handle handle = scope.GetBioseq_setHandle(*bioseq_set);
        if (handle)
            seh = handle.GetTopLevelEntry();
    }
    else if (seq_id) {
        CBioseq_Handle handle = scope.GetBioseqHandle(*seq_id);
        if (handle)
            seh = handle.GetTopLevelEntry();
    }
    else if (seq_loc) {
        CBioseq_Handle handle = scope.GetBioseqHandle(*seq_loc);
        if (handle)
            seh = handle.GetTopLevelEntry();
    }

    return seh;
}

static CFlatFileConfig::EMode s_FFMode(int val)
{
    switch (val) {
    default: return CFlatFileConfig::eMode_GBench;
    case 1:  return CFlatFileConfig::eMode_Entrez;
    case 2:  return CFlatFileConfig::eMode_Release;
    case 3:  return CFlatFileConfig::eMode_Dump;
    }
}

static CFlatFileConfig::EStyle s_FFStyle(bool showSequence)
{
    return showSequence ? CFlatFileConfig::eStyle_Master : CFlatFileConfig::eStyle_Normal;
}

void CTextPanel::x_LoadFlatFile()
{
    if (!m_SO) return;

    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_SO.GetPointer());
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());
    const CSeq_submit* seq_submit = dynamic_cast<const CSeq_submit*>(m_SO.GetPointer());
	const CSubmit_block* submitBlock = 0;

    CSeq_entry_Handle seh;
	if (seq_submit) {
        if (seq_submit->IsSetData()) {
            const CSeq_submit::TData& data = seq_submit->GetData();
            if (data.IsEntrys()) {
                const CSeq_submit::TData::TEntrys& entries = data.GetEntrys();
                if (!entries.empty()) {
					seh = m_Scope->GetSeq_entryHandle(**entries.begin());
                }
            }
        }
		if (seq_submit->IsSetSub()) {
			submitBlock = &seq_submit->GetSub();
		}
	} else if (!seq_feat) {
		seh = GetSeqEntryHandle(*m_Scope, *m_SO);
	}

    if (!seq_feat && !seh)
        return;

    CFlatFileConfig::EMode mode(s_FFMode(m_FlatFileMode));
    CFlatFileConfig::EStyle style(s_FFStyle(m_ShowSequence));

    if (seq_feat)
        m_DataSource->LoadFlatFile(x_CreateFlatFileContext(false), *m_Scope, *seq_feat, m_FeatMode == 0, mode, style);
    else
        m_DataSource->GetFlatFileSeqs(seh, submitBlock, seq_loc, style);
}

CTextPanelContext* CTextPanel::x_CreateFlatFileContext(bool createEditor)
{
    CFlatFileConfig::EMode mode(s_FFMode(m_FlatFileMode));
    CFlatFileConfig::EStyle style(s_FFStyle(m_ShowSequence));

    CFlatFileConfig config(CFlatFileConfig::eFormat_GenBank, mode, style, 0, CFlatFileConfig::fViewAll);
    CRef<CFlatFileContext> ctx(new CFlatFileContext(config));
    CRef<CFlatItemFormatter> formatter(CFlatItemFormatter::New(config.GetFormat()));
    formatter->SetContext(*ctx);

    CIRef<IBioseqEditor> editor;
    if (m_CmdProccessor && createEditor)
        editor = x_CreateEditor();

    CTextPanelContext* context = new CFlatFileViewContext(*formatter, *m_Scope, m_SO, editor);
    x_InitContext(*context);
    return context;
}

CTextPanelContext* CTextPanel::x_CreateAsnContext(const CSerialObject& so)
{
    CIRef<IBioseqEditor> editor;
    if (m_CmdProccessor)
        editor = x_CreateEditor();

    CAsnViewContext* context = new CAsnViewContext(*m_Scope, &so, editor);
    context->SetShowAsnTypes(GetShowASNTypes());
    context->SetShowPathToRoot(GetShowASNPathToRoot());
    x_InitContext(*context);
    return context;
}

CTextPanelContext* CTextPanel::x_CreateXmlContext(const CSerialObject& so)
{
    CTextPanelContext* context = new CTextPanelContext(1, m_Scope, &so);
    x_InitContext(*context);
    return context;
}

CTextPanelContext* CTextPanel::x_CreateFastaContext()
{
    CTextPanelContext* context = new CFastaViewContext(kFastaTextOffset, m_Scope, m_SO);
    x_InitContext(*context);
    return context;
}

void CTextPanel::x_InitContext(CTextPanelContext& context)
{
    context.SetWorkDir(m_WorkDir);
    context.SetFontDesc(*m_TextWindow);
    wxClientDC dc(this);
    wxFont font(context.GetFontDesc());
    dc.SetFont(font);
    context.CalcWSize(dc);
}

void CTextPanel::RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    cmd_reg.RegisterCommand(kExpandAll,
        "&Expand All\tCtrl+E",
        "Expand All",
        NcbiEmptyString,
        NcbiEmptyString,
        "Command expands(opens) all of a FlatFile view");
    wxAcceleratorEntry accelerator(wxACCEL_CTRL, 'E', kExpandAll);
    cmd_reg.AddAccelerator(accelerator);

    CFlatFileViewContext::RegisterCommands(cmd_reg, provider);

    vector<IEditorFactory*> factories;
    GetExtensionAsInterface(EXT_POINT__EDITOR_FACTORY, factories);

    ITERATE(vector<IEditorFactory*>, it, factories) {
        (*it)->RegisterCommands(cmd_reg, provider);
    }
}

CIRef<IBioseqEditor> CTextPanel::x_CreateEditor()
{
    CIRef<IBioseqEditor> editor;

    vector<IEditorFactory*> factories;
    GetExtensionAsInterface(EXT_POINT__EDITOR_FACTORY, factories);

    ITERATE(vector<IEditorFactory*>, it, factories) {
        editor.Reset(dynamic_cast<IBioseqEditor*>(
            (*it)->CreateEditor(typeid(IBioseqEditor).name(), *m_CmdProccessor, m_WidgetHost)));

        if (editor)
            break;
    }

    return editor;
}

void CTextPanel::x_LoadASN()
{
    if (!m_SO) return;

    CConstRef<CSerialObject> so = m_SO;
    if (!GetSeqEntryFromSeqSubmit(so)) {
        CSeq_entry_Handle seh = GetSeqEntryHandle(*m_Scope, *m_SO);
        if (seh) {
            CConstRef<CSeq_entry> seq_entry = seh.GetCompleteSeq_entry();
            if (seq_entry)
                so = seq_entry;
        }
    }

    m_DataSource->LoadAsn(x_CreateAsnContext(*so), *so, *m_Scope);
}

void CTextPanel::x_LoadXML()
{
    if (!m_SO) return;

    CConstRef<CSerialObject> so = m_SO;
    if (!GetSeqEntryFromSeqSubmit(so)) {
        CSeq_entry_Handle seh = GetSeqEntryHandle(*m_Scope, *m_SO);
        if (seh) {
            CConstRef<CSeq_entry> seq_entry = seh.GetSeq_entryCore();
            if (seq_entry)
                so = seq_entry;
        }
    }

    m_DataSource->LoadXml(x_CreateXmlContext(*so), *so, *m_Scope);
}

void CTextPanel::x_LoadFastA()
{
    if (!m_SO) return;

    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());
    if (seq_loc) {
        m_DataSource->LoadFasta(x_CreateFastaContext(), *seq_loc, *m_Scope);
    }
    else {
        CSeq_entry_Handle seh = GetSeqEntryHandle(*m_Scope, *m_SO);
        if (!seh) return;
        m_DataSource->GetFlatFileSeqs(seh, nullptr, nullptr, CFlatFileConfig::eStyle_Normal);
    }
}

void CTextPanel::SequenceListLoaded(const vector<CFlatFileSeq>& sequenceList)
{
    m_FlatFileSeqs = sequenceList;

    if (m_SequenceShown != kAllNucleotides && m_SequenceShown != kAllSequences) {
        bool found = false;
        for (const auto& i : m_FlatFileSeqs) {
            if (i.m_Name == m_SequenceShown) {
                found = true;
                break;
            }
        }
        if (!found) m_SequenceShown = kAllNucleotides;
    }

    if (m_initial && m_FlatFileSeqs.size() > 100 &&
        (m_SequenceShown == kAllNucleotides || m_SequenceShown == kAllSequences)) {
        m_SequenceShown = m_FlatFileSeqs[0].m_Name;
    }

    m_initial = false;

    x_CreateMainToolbar();
    x_FillSeqList();
    x_SelectFlatFileSequence(m_SequenceShown);
}


void CTextPanel::DataLoaded(CRootTextItem& root, CTextPanelContext& context)
{
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_SO.GetPointer());
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());

    bool sequenceList = (m_ViewType == kFlatFile && !seq_feat) ||
                        (m_ViewType == kFastA && !seq_loc);

    if (sequenceList) {
        root.SetDeleteChildren(false);
        _ASSERT(root.GetItemCount() == m_FlatFileSeqs.size());

        CExpandTraverser expander(context, true);

        for (size_t i = 0; i < root.GetItemCount(); ++i) {
            ITextItem* item = root.GetItem(i);
            if (item) {
                delete m_FlatFileSeqs[i].m_Item;
                m_FlatFileSeqs[i].m_Item = item;

                map<string, bm::bvector<> >::iterator it = m_TreeState.find(m_FlatFileSeqs[i].m_Name);
                if (it != m_TreeState.end()) {
                    CRestoreTreeTraverser trv(context, it->second);
                    item->Traverse(trv);
                    m_TreeState.erase(it);
                }
                else if (m_OpenExpanded) {
                    item->Traverse(expander);
                }
            }
        }
        delete &root;
        delete &context;

        if (!x_SelectFlatFileSequence(m_SequenceShown))
            return;

        if (m_PositionOnLoad) {
            m_TextWindow->MakeObjectVisible(*m_PositionOnLoad);
            m_PositionOnLoad.Reset();
        }
    }
    else {
        map<string, bm::bvector<> >::iterator it = m_TreeState.find("");
        if (it != m_TreeState.end()) {
            CRestoreTreeTraverser trv(*m_TextWindow->GetContext(), it->second);
            root.Traverse(trv);
            root.UpdatePositions();
            m_TreeState.erase(it);
        }
        m_TreeState.clear();

        m_TextWindow->SetMainItem(&root, &context);

        if (m_ViewType == kASN) {
            CAsnViewContext* context = dynamic_cast<CAsnViewContext*>(m_TextWindow->GetContext());
            if (context != NULL) {
                context->SetShowAsnTypes(m_ShowASNTypes);
                context->SetShowPathToRoot(m_ShowASNPathToRoot);
            }
        }
    }

    Layout();

    if (m_SavedPosY > 0) {
        int x, y;
        m_TextWindow->GetVirtualSize(&x, &y);

        int xUnit, yUnit;
        m_TextWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
        if (y / yUnit > m_SavedPosY)
            m_TextWindow->Scroll(m_SavedPosX, m_SavedPosY);
        m_SavedPosX = m_SavedPosY = 0;
    }

    if (m_WidgetHost) {
        TConstObjects objects;
        m_WidgetHost->GetAppSelection(objects);
        SetSelectedObjects(objects);
        m_WidgetHost->WidgetSelectionChanged();
    }
}

void CTextPanel::AddExtensionToolbar(wxAuiToolBar& toolbar, const wxString& name)
{
    m_AuiManager.AddPane(&toolbar, wxAuiPaneInfo()
        .Caption(pszMainToolbar)
        .ToolbarPane()
        .Top()
        .TopDockable(true)
        .LeftDockable(false)
        .RightDockable(false)
        .BottomDockable(false)
        .Floatable(false)
        .Resizable(true)
        .DockFixed()
        .Gripper(false)
        .DestroyOnClose(true)
        .Row(1));
    m_AuiManager.Update();
}

bool CTextPanel::ShowToolTips()
{
    return true;
}


wxBitmap CTextPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTextPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTextPanel bitmap retrieval
}


wxIcon CTextPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTextPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTextPanel icon retrieval
}

void CTextPanel::SetInitialViewType(const string& viewType)
{
    wxString type = ToWxString(viewType);

    if (type == pszFlatFile) {
        m_ViewType = kFlatFile;
    }
    else if (type == pszASN) {
        m_ViewType = kASN;
    }
    else if (type == pszFastA) {
        m_ViewType = kFastA;
    }
    else if (type == pszXML) {
        m_ViewType = kASN;
    }
}

bool CTextPanel::CanShowFlatFile() const
{
    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(m_SO.GetPointer());
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_SO.GetPointer());
    const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*>(m_SO.GetPointer());
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(m_SO.GetPointer());
    const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set*>(m_SO.GetPointer());

    if (!seq_entry) {
        seq_entry = GetSeqEntryFromSeqSubmit(m_SO);
    }

    if (seq_feat) {
        CSeqFeatData::E_Choice type = seq_feat->GetData().Which();
        if (type != CSeqFeatData::e_Gene &&
            type != CSeqFeatData::e_Cdregion &&
            type != CSeqFeatData::e_Rna)
            seq_feat = NULL;
    }

    return seq_id || seq_loc || seq_feat || seq_entry || bioseq || bioseq_set;
}

bool CTextPanel::CanShowFastA() const
{
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());
    if (seq_loc)
        return true;

	try {
		CSeq_entry_Handle seh = GetSeqEntryHandle(const_cast<CScope&>(*m_Scope), *m_SO);
		if (seh)
			return true;
	} NCBI_CATCH("CTextPanel::CanShowFastA()");

    return false;
}

bool CTextPanel::CanShowASN() const
{
    return true;
}

bool CTextPanel::CanShowXML() const
{
    return true;
}

CIRef<IMenuContributor> CTextPanel::GetMenuContributor()
{
    CIRef<IMenuContributor> contributor;
    if (m_DataSource) {
        CIRef<IBioseqEditor> editor = x_CreateEditor();
        contributor.Reset(dynamic_cast<IMenuContributor*>(editor.GetPointerOrNull()));
    }
    return contributor;
}

bool CTextPanel::ObjectIsFeature() const
{
    return dynamic_cast<const CSeq_feat*>(m_SO.GetPointer()) != 0;
}

void CTextPanel::UpdateRegistry()
{
    x_SaveViewType();

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(kTextViewPath);

    view.Set(kFontSize, m_FontSize);
    view.Set(kFlatFileMode, m_FlatFileMode);
    view.Set(kShowSequence, m_ShowSequence);
	view.Set(kFlatFileSequence, m_SequenceShown);
    view.Set(kFeatureMode, m_FeatMode);
    view.Set(kASNShowTypes, m_ShowASNTypes);
    view.Set(kASNShowPathToRoot, m_ShowASNPathToRoot);

    view.Set(kFFHideVariations, m_FFHideVariations);
    view.Set(kFFHideSTS, m_FFHideSTS);
    view.Set(kOpenExpanded, m_OpenExpanded);

    view.Set(kFindMode, m_FindMode);
}

void CTextPanel::x_ReadRegistry()
{
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(kTextViewPath);

    m_FontSize = view.GetInt(kFontSize, m_FontSize);
    m_FlatFileMode = view.GetInt(kFlatFileMode, m_FlatFileMode);
    m_ShowSequence = view.GetBool(kShowSequence, m_ShowSequence);
	m_SequenceShown = view.GetString(kFlatFileSequence, m_SequenceShown);
    m_FeatMode = view.GetInt(kFeatureMode, m_FeatMode);
    m_ShowASNTypes = view.GetBool(kASNShowTypes, m_ShowASNTypes);
    m_ShowASNPathToRoot = view.GetBool(kASNShowPathToRoot, m_ShowASNPathToRoot);
    m_FFHideVariations = view.GetBool(kFFHideVariations, m_FFHideVariations);
    m_FFHideSTS = view.GetBool(kFFHideSTS, m_FFHideSTS);
    m_OpenExpanded = view.GetBool(kOpenExpanded, m_OpenExpanded);

    m_FindMode = view.GetInt(kFindMode, m_FindMode);
}

void CTextPanel::RefreshView()
{
    CAsnViewContext* context = dynamic_cast<CAsnViewContext*>(m_TextWindow->GetContext());
    if (context != NULL) {
        context->SetShowAsnTypes(m_ShowASNTypes);
        context->SetShowPathToRoot(m_ShowASNPathToRoot);
    }
    m_TextWindow->Refresh();
}

void CTextPanel::x_SaveViewType()
{
    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(m_SO.GetPointer());
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_SO.GetPointer());
    const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*>(m_SO.GetPointer());
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(m_SO.GetPointer());
    const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set*>(m_SO.GetPointer());

    if (!seq_entry) {
        seq_entry = GetSeqEntryFromSeqSubmit(m_SO);
    }

    if (seq_feat) {
        CSeqFeatData::E_Choice type = seq_feat->GetData().Which();
        if (type != CSeqFeatData::e_Gene &&
            type != CSeqFeatData::e_Cdregion &&
            type != CSeqFeatData::e_Rna)
            seq_feat = NULL;
    }

    const char* strType = kViewTypeGeneric;

    if (seq_feat)
        strType = kViewTypeFeat;
    else if (seq_id || seq_loc || seq_entry || bioseq || bioseq_set)
        strType = kViewTypeSeq;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(kTextViewPath);
    view.Set(strType, (int)m_ViewType);
}

void CTextPanel::x_UpdateViewType(const wxString& type)
{
    TViewType viewType = kASN;

    if (type == pszFlatFile)
        viewType = kFlatFile;
    else if (type == pszFastA)
        viewType = kFastA;

    if (viewType != m_ViewType) {
        m_ViewType = viewType;
        x_SaveViewType();
        GetSizer()->Layout();
        Load();
    }
}

void CTextPanel::x_FindText()
{
    CTextItemPanel* widget = GetWidget();
    if (widget == NULL)
        return;

    const wxString caption = wxT("Find Text");

    wxString text = m_FindTextCtrl->GetValue();
    if (text.empty()) {
        wxMessageBox(wxT("Search string is empty."), caption, wxOK | wxICON_EXCLAMATION);
        return;
    }

    bool found = false;
    if (m_FindMode == 2) {
        wxBusyCursor wait;
        found = widget->FindSequence(text);
    }
    else {
        wxBusyCursor wait;
        found = widget->FindText(text, (m_FindMode == 0), true);
    }

    if (!found)
        wxMessageBox(wxT("Search string not found."), caption, wxOK | wxICON_EXCLAMATION);
}


void CTextPanel::OnFindNextClick( wxCommandEvent& event )
{
    x_FindText();
}

void CTextPanel::OnSearchTermCtrlEnter( wxCommandEvent& event )
{
    x_FindText();
}

void CTextPanel::OnOptionsClick( wxCommandEvent& event )
{
    CTextViewOptionsDlg dlg(this);
    dlg.SetRegistryPath("Dialogs.CTextViewOptionsDlg");
    if (dlg.ShowModal() == wxID_OK) {
        dlg.ApplyChanges();
    }
}

// block toolbar event propagation 
void CTextPanel::OnContextMenu(wxContextMenuEvent& event)
{
}

void CTextPanel::OnSequenceSelected( wxCommandEvent& event )
{
    if (m_ViewType == kFlatFile || m_ViewType == kFastA) {
        string selSeq = ToStdString(event.GetString());
        x_SelectFlatFileSequence(selSeq);
        if (m_WidgetHost) m_WidgetHost->WidgetSelectionChanged();
    }
}

bool CTextPanel::x_SelectFlatFileSequence(const string& seqName)
{
    if (m_ViewType != kFlatFile && m_ViewType != kFastA)
		return false;

    vector<pair<CBioseq_Handle, string> > handles(m_FlatFileSeqs.size());
    for (size_t i = 0; i < m_FlatFileSeqs.size(); ++i)
        handles[i].second = m_FlatFileSeqs[i].m_Name;

    vector<ITextItem*> items;
    string errMsg;
    bool loaded = true;

    if (seqName.empty() || seqName == kAllNucleotides) {
        m_SequenceShown = kAllNucleotides;
        errMsg = "No nucleotide sequences.";
        for (size_t i = 0; i < m_FlatFileSeqs.size(); ++i) {
            if (m_FlatFileSeqs[i].m_Handle.IsNucleotide()) {
                if (m_FlatFileSeqs[i].m_Item) {
                    items.push_back(m_FlatFileSeqs[i].m_Item);
                }
                else {
                    handles[i].first = m_FlatFileSeqs[i].m_Handle;
                    loaded = false;
                }
            }
        }
    }
    else if (seqName == kAllSequences) {
        m_SequenceShown = kAllSequences;
        errMsg = "No sequences.";
        for (size_t i = 0; i < m_FlatFileSeqs.size(); ++i) {
            if (m_FlatFileSeqs[i].m_Item) {
                items.push_back(m_FlatFileSeqs[i].m_Item);
            } else {
                handles[i].first = m_FlatFileSeqs[i].m_Handle;
                loaded = false;
            }
        }
    }
    else {
        string selSeq = seqName;
        if (seqName == kMoreSequences) {
            m_SequenceList->SetStringSelection(m_SequenceShown);
            CSelectSequenceDlg dlg(this, m_FlatFileSeqs, m_SequenceShown);
            dlg.SetRegistryPath("Dialogs.CSelectSequenceDlg");
            if (dlg.ShowModal() != wxID_OK)
                return false;
            selSeq = dlg.GetSequenceId();
        }

        m_SequenceShown = selSeq;
        errMsg = "Sequence '" + selSeq + "' not found.";

        for (size_t i = 0; i < m_FlatFileSeqs.size(); ++i) {
            if (selSeq == m_FlatFileSeqs[i].m_Name) {
                if (m_FlatFileSeqs[i].m_Item) {
                    CExpandItem* expandItem = dynamic_cast<CExpandItem*>(m_FlatFileSeqs[i].m_Item);
                    if (expandItem)
                        items.push_back(expandItem->GetExpandedItem());
                    else
                        items.push_back(m_FlatFileSeqs[i].m_Item);
                }
                else {
                    handles[i].first = m_FlatFileSeqs[i].m_Handle;
                    loaded = false;
                }
            }
        }
    }

    if (loaded) {
        x_FillSeqList();

        if (items.empty()) {
            m_TextWindow->ReportError(errMsg);
        }
        else {
            CRootTextItem* root = new CRootTextItem();
            root->InsertItems(0, &items[0], items.size());
            root->SetDeleteChildren(false);
            CTextPanelContext* context = (m_ViewType == kFlatFile) ? x_CreateFlatFileContext() : x_CreateFastaContext();
            m_TextWindow->SetMainItem(root, context);
        }
        return true;
    }

    m_TextWindow->ReportLoading();

    if (m_ViewType == kFlatFile) {
        const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());
        const CSeq_submit* seq_submit = dynamic_cast<const CSeq_submit*>(m_SO.GetPointer());
        const CSubmit_block* submitBlock = 0;

        if (seq_submit && seq_submit->IsSetSub())
            submitBlock = &seq_submit->GetSub();

        SFlatFileParams params(m_FFHideVariations, m_FFHideSTS);
        params.m_Mode = s_FFMode(m_FlatFileMode);
        params.m_Style = s_FFStyle(m_ShowSequence);
        params.m_SeqSubmit.Reset(submitBlock);
        params.m_SeqLoc.Reset(seq_loc);
        params.m_ShowComponentFeats = m_ShowComponentFeats;
        m_DataSource->LoadFlatFile(x_CreateFlatFileContext(), handles, params);
    }
    else {
        m_DataSource->LoadFasta(x_CreateFastaContext(), handles);
    }

    return false;
}

void CTextPanel::x_FillSeqList()
{
    if (!m_SequenceList)
        return;

    m_SequenceList->SetMinSize(wxDefaultSize); // resizing wxChoice

    m_SequenceList->Clear();
    m_SequenceList->Append(kAllNucleotides);
    m_SequenceList->Append(kAllSequences);

    size_t seqCount = m_FlatFileSeqs.size();

    int seqIndex = -1;
    for (size_t i = 0; i < seqCount; ++i) {
        string seqId = m_FlatFileSeqs[i].m_Name;
        if (m_FlatFileSeqs[i].m_Name == m_SequenceShown)
            seqIndex = i;
    }

    int listStart = 0;
    if (seqCount > kMaxSeqNumber && seqIndex >= 0) {
        listStart = seqIndex - kMaxSeqNumber / 2;
        if (listStart < 0)
            listStart = 0;
        else if (m_FlatFileSeqs.size() - listStart < kMaxSeqNumber)
            listStart = seqCount - kMaxSeqNumber;
    }

    for (size_t i = listStart; i < min(seqCount, (size_t)listStart + kMaxSeqNumber); ++i)
        m_SequenceList->Append(m_FlatFileSeqs[i].m_Name);

    if (seqCount > kMaxSeqNumber)
        m_SequenceList->Append(kMoreSequences);


    int selection = 0;
    if (seqIndex < 0) {
        if (m_SequenceShown == kAllSequences)
            selection = 1;
    }
    else {
        selection = seqIndex - listStart + 2;
    }
    m_SequenceList->SetSelection(selection);
    m_SequenceList->SetMinSize(m_SequenceList->GetBestSize()); // resizing wxChoice

    // resizing toolbar programmatically
    wxAuiPaneInfo& pane = m_AuiManager.GetPane(pszSequenceToolbar);
    if (!pane.IsOk()) return;
    wxAuiToolBar* toolbar = dynamic_cast<wxAuiToolBar*>(pane.window);
    if (!toolbar) return;
    wxAuiToolBarItem* item = toolbar->FindTool(ID_TB_SequenceListCtrl);
    if (!item) return;

    item->SetMinSize(wxDefaultSize); // reset item size
    toolbar->Realize();

    pane.BestSize(toolbar->GetSize()); // seems no other way to resize programmatically
    m_AuiManager.Update();
}

void CTextPanel::OnFindModeSelected( wxCommandEvent& event )
{
    m_FindMode = event.GetSelection();
}

bool CTextPanel::SetPosition(const CBioseq_Handle& h, const CObject* object)
{
    for (const auto& s : m_FlatFileSeqs) {
        if (s.m_Handle == h) {
            return SetPosition(s.m_Name, object);
        }
    }
    return false;
}

bool CTextPanel::SetPosition(const string& seq, const CObject* object)
{
    if (seq.empty()) return false;

    if (m_ViewType != kFlatFile) {
        ClearState();
        m_ViewType = kFlatFile;
        m_SequenceShown = seq;
        m_PositionOnLoad.Reset(object);
        Load();
        return true;
    }

    if (m_DataSource->IsLoading()) {
        m_SequenceShown = seq;
        m_PositionOnLoad.Reset(object);
        return true;
    }

    if (seq == m_SequenceShown) {
        if (object)
            return m_TextWindow->MakeObjectVisible(*object);
        else
            return true;
    }

    if (x_SelectFlatFileSequence(seq)) {
        if (object)
            return m_TextWindow->MakeObjectVisible(*object);
        else
            return true;
    }
    m_PositionOnLoad.Reset(object);

    return true;
}

/// IGuiWidgetHost implementation

void CTextPanel::SetHost(IGuiWidgetHost* host)
{
    m_WidgetHost = host;
}

bool CTextPanel::InitWidget(TConstScopedObjects& objects)
{
    if (objects.size() != 1) return false;

    wxBusyCursor wait;

    ClearState();

    m_Scope.Reset(objects.front().scope.GetPointer());
    m_SO.Reset(dynamic_cast<const CSerialObject*>(objects.front().object.GetPointer()));

    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(m_SO.GetPointer());
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_SO.GetPointer());
    const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*>(m_SO.GetPointer());
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(m_SO.GetPointer());
    const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set*>(m_SO.GetPointer());


    if (!seq_entry) {
        seq_entry = GetSeqEntryFromSeqSubmit(m_SO);
    }

    if (seq_feat) {
        CSeqFeatData::E_Choice type = seq_feat->GetData().Which();
        if (type != CSeqFeatData::e_Gene &&
            type != CSeqFeatData::e_Cdregion &&
            type != CSeqFeatData::e_Rna)
            seq_feat = NULL;
    }

    if (seq_id) {
        CBioseq_Handle handle = m_Scope->GetBioseqHandle(*seq_id);
        if (!handle) {
            string str("failed to retrieve sequence for id ");
            seq_id->GetLabel(&str);
            NCBI_THROW(CException, eUnknown, str);
        }
    }

    //wxChoice* textType = (wxChoice*)FindWindow(ID_CHOICE1);
    //textType->Clear();

    int viewType = m_ViewType;
    wxString viewTypeString;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(kTextViewPath);

    if (seq_id || seq_loc || seq_feat || seq_entry || bioseq || bioseq_set) {
        //textType->Append(pszFlatFile);
        //if (!seq_feat) textType->Append(pszFastA);
        //textType->Append(pszASN);
        //textType->Append(pszXML);

        if (viewType == kUninitialized) {
            if (seq_feat != NULL)
                viewType = view.GetInt(kViewTypeFeat, m_ViewType);
            else
                viewType = view.GetInt(kViewTypeSeq, m_ViewType);
        }

        switch(viewType) {
        case kFastA:
            viewTypeString = (seq_feat != NULL) ? pszFlatFile : pszFastA;
            break;
        case kXML:
        case kASN:
            viewTypeString = pszASN;
            break;
        case kFlatFile:
        case kUninitialized:
        default:
            viewTypeString = pszFlatFile;
            break;
        }
    }
    else {
        //textType->Append(pszASN);
        //textType->Append(pszXML);

        if (viewType == kUninitialized) {
            viewType = view.GetInt(kViewTypeGeneric, viewType);
        }
        viewTypeString = pszASN;
    }

    //textType->SetStringSelection(viewTypeString);
 
    m_ViewType = kUninitialized;
    x_UpdateViewType(viewTypeString);

    TransferDataToWindow();

    return true;
}

const CObject* CTextPanel::GetOrigObject() const
{
    return m_SO.GetPointerOrNull();
}

void CTextPanel::GetSelectedObjects (TConstObjects& objects) const
{
    CTextItemPanel* widget = GetWidget();
    if (!widget) return;

    CTextPanelContext* context = widget->GetContext();
    if (!context) return;

    context->GetSelectedObjects(objects);
}

void CTextPanel::SetSelectedObjects (const TConstObjects& objects)
{
    CTextItemPanel* widget = GetWidget();
    if (!widget) return;

    CTextPanelContext* context = widget->GetContext();
    if (!context) return;

    context->SetSelectedObjects(objects);
}

void CTextPanel::SetUndoManager(ICommandProccessor* cmdProccessor)
{
    m_CmdProccessor = cmdProccessor;

    if (!m_DataSource)
        m_DataSource.Reset(new CTextDataSource(*this));
}

void CTextPanel::DataChanging()
{
    if (!m_TextWindow) return;

    SaveState();
    m_TextWindow->ReportLoading();
}

void CTextPanel::DataChanged()
{
    Load();
}

void CTextPanel::GetActiveObjects(vector<TConstScopedObjects>& objects)
{
    CTextItemPanel* widget = GetWidget();
    if (!widget) return;

    CTextPanelContext* context = widget->GetContext();
    if (!context) return;

    context->GetActiveObjects(objects);
}

void CTextPanel::SaveState()
{
    ClearState();

    if (!m_TextWindow || m_DataSource->IsLoading())
        return;

    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(m_SO.GetPointer());
    const CSeq_loc* seq_loc = dynamic_cast<const CSeq_loc*>(m_SO.GetPointer());

    bool sequenceList = (m_ViewType == kFlatFile && !seq_feat) ||
                        (m_ViewType == kFastA && !seq_loc);

    if (sequenceList) {
        for (const auto& i : m_FlatFileSeqs) {
            if (i.m_Item) {
                if (m_OpenExpanded)
                    m_TreeState[i.m_Name].set();
                CSaveTreeTraverser trv(m_TreeState[i.m_Name]);
                i.m_Item->Traverse(trv);
            }
        }
    }
    else {
        ITextItem* root = m_TextWindow->GetRootItem();
        if (root) {
            CSaveTreeTraverser trv(m_TreeState[""]);
            root->Traverse(trv);
        }
    }

    m_TextWindow->GetViewStart(&m_SavedPosX, &m_SavedPosY);
}

void CTextPanel::ClearState()
{
    m_TreeState.clear();
    m_SavedPosX = m_SavedPosY = 0;
}

END_NCBI_SCOPE
