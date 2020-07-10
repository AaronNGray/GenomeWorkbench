/*  $Id: seq_desktop_panel.cpp 44457 2019-12-19 19:25:12Z asztalos $
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
 * Authors:  Roman Katargin, Andrea Asztalos
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objects/submit/Seq_submit.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/ieditor_factory.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/utils/menu_item.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/seq_desktop/desktop_ds.hpp>
#include <gui/widgets/seq_desktop/desktop_event.hpp>
#include <gui/widgets/seq_desktop/desktop_canvas.hpp>
#include <gui/widgets/seq_desktop/seq_desktop_panel.hpp>

#include <wx/srchctrl.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS(CSeqDesktopPanel, wxPanel)

BEGIN_EVENT_TABLE(CSeqDesktopPanel, wxPanel)
    EVT_MENU(ID_TB_ZOOMIN, CSeqDesktopPanel::OnZoomInClick)
    EVT_MENU(ID_TB_ZOOMOUT, CSeqDesktopPanel::OnZoomOutClick)
    EVT_UPDATE_UI(ID_TB_ZOOMIN, CSeqDesktopPanel::UpdateZoomIn)
    EVT_UPDATE_UI(ID_TB_ZOOMOUT, CSeqDesktopPanel::UpdateZoomOut)

    EVT_MENU(wxID_CUT, CSeqDesktopPanel::OnCutSelection)
    EVT_MENU(wxID_COPY, CSeqDesktopPanel::OnCopySelection)
    EVT_LEFT_DOWN(CSeqDesktopPanel::OnMouseClick)

    EVT_SEARCHCTRL_SEARCH_BTN(ID_SEARCHCTRL, CSeqDesktopPanel::OnSearchCtrlClick)
END_EVENT_TABLE()

/*!
* CSeqDesktopPanel constructors
*/

CSeqDesktopPanel::CSeqDesktopPanel()
{
    Init();
}


CSeqDesktopPanel::CSeqDesktopPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, pos, size, style);
    CreateControls();
}

/*!
* CSeqDesktopPanel destructor
*/

CSeqDesktopPanel::~CSeqDesktopPanel()
{
    m_AuiManager.UnInit();
}


void CSeqDesktopPanel::Init()
{
    m_SearchCtrl = NULL;
    m_TextWindow = NULL;
    m_MaxExpansion = 2;
    m_SavedStartX = m_SavedStartY = 0;
    m_CmdProccessor = nullptr;
    m_SavedObject.Reset();
    m_SavedType = CDesktopViewContext::eSavedObjectType_not_set;
}

/*!
* Control creation for CSeqDesktopPanel
*/

static const wxChar* pszMainToolbar = wxT("Main Toolbar");

void CSeqDesktopPanel::CreateControls()
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    Hide();

    m_AuiManager.SetManagedWindow(this);

    m_TextWindow = new CDesktopCanvas(this, ID_WIDGET, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxHSCROLL | wxVSCROLL | wxFULL_REPAINT_ON_RESIZE);
    m_AuiManager.AddPane(m_TextWindow, wxAuiPaneInfo().CenterPane());

    m_TextWindow->SetBackgroundColour(wxColor(wxT("white")));
    m_TextWindow->SetFocus();

    m_AuiManager.Update();

    // m_TextWindow->SetFont();
}

void CSeqDesktopPanel::x_CreateToolbar()
{
    static bool sImagesRegistered = false;
    if (!sImagesRegistered) {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("seq_desktop_panel::zoom_in"), wxT("zoom_in.png"));
        provider->RegisterFileAlias(wxT("seq_desktop_panel::zoom_out"), wxT("zoom_out.png"));
        sImagesRegistered = true;
    }

    // create the toolbar
    m_Toolbar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORZ_LAYOUT | wxBORDER_NONE);
    m_Toolbar->AddTool(ID_TB_ZOOMIN, wxT("Zoom In"), wxArtProvider::GetBitmap(wxT("seq_desktop_panel::zoom_in")), wxT("Zoom In"));
    m_Toolbar->AddTool(ID_TB_ZOOMOUT, wxT("Zoom Out"), wxArtProvider::GetBitmap(wxT("seq_desktop_panel::zoom_out")), wxT("Zoom Out"));
    m_Toolbar->EnableTool(ID_TB_ZOOMOUT, false);

    m_Toolbar->AddSeparator();
    m_SearchCtrl = new wxSearchCtrl(m_Toolbar, ID_SEARCHCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_SearchCtrl->SetDescriptiveText(_("Search the view"));
    //m_SearchCtrl->Bind(wxEVT_TEXT_ENTER, &CSeqDesktopPanel::OnSearchCtrlEnter, this);
    m_SearchCtrl->Bind(wxEVT_CHAR_HOOK, &CSeqDesktopPanel::OnSearchCtrlEnter, this); // a hack, the wxEVT_TEXT_ENTER event should be bound to the control
    m_Toolbar->AddControl(m_SearchCtrl);

    m_Toolbar->Realize();

    m_AuiManager.AddPane(m_Toolbar, wxAuiPaneInfo()
        .Name(pszMainToolbar)
        .Caption(pszMainToolbar)
        .ToolbarPane()
        .Top()
        .TopDockable(true)
        .LeftDockable(false)
        .RightDockable(false)
        .BottomDockable(false)
        .Floatable(false)
        .Resizable(false)
        .DestroyOnClose(true)
        .Position(0));

    m_AuiManager.Update();
}

void CSeqDesktopPanel::SetHost(IGuiWidgetHost* host)
{
    m_WidgetHost = host;
}

bool CSeqDesktopPanel::InitWidget(TConstScopedObjects& objects)
{
    if (objects.size() != 1) return false;

    m_Scope.Reset(objects.front().scope.GetPointer());
    m_SO.Reset(dynamic_cast<const CSerialObject*>(objects.front().object.GetPointer()));

    x_SetMaxExpansionLevel();
    Load();
    return true;
}

void CSeqDesktopPanel::Load(int state)
{
    if (!m_SO) return;

    if (!m_AuiManager.GetPane(pszMainToolbar).IsOk())
        x_CreateToolbar();

    CDesktopDataBuilder builder(m_SO.GetObject(), *m_Scope);
    CIRef<IDesktopItem> root = builder.GetRootDesktopItem();
    m_TextWindow->SetMainItem(root, state);
    CDesktopViewContext* context = x_CreateDesktopViewContext();
    m_TextWindow->SetContext(context);
}


void CSeqDesktopPanel::x_SetMaxExpansionLevel(void)
{
    if (!m_SO) return;
    m_MaxExpansion = 2;

    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(m_SO.GetPointer());
    const CSeq_submit* submit = dynamic_cast<const CSeq_submit*>(m_SO.GetPointer());
    const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(m_SO.GetPointer());
    const CSeq_annot* seq_annot = dynamic_cast<const CSeq_annot*>(m_SO.GetPointer());

    CSeq_entry_Handle seh;
    if (submit && submit->IsSetData()) {
        const CSeq_submit::TData& data = submit->GetData();
        if (data.IsEntrys()) {
            const CSeq_submit::TData::TEntrys& entries = data.GetEntrys();
            if (!entries.empty()) {
                entry = *entries.begin();
            }
        }
    }

    if (seq_id) {
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*seq_id);
        if (bsh) {
            seh = bsh.GetTopLevelEntry();
        }
    }
    else if (entry) {
        seh = m_Scope->GetSeq_entryHandle(*entry);
    }

    if (seh) {
        CFeat_CI feat_it(seh);
        if (feat_it) {
            m_MaxExpansion = 4;
        }
    }
    else if (seq_annot) {
        CSeq_annot_Handle ah = m_Scope->GetSeq_annotHandle(*seq_annot);
        if (ah) {
            CFeat_CI feat_it(ah);
            if (feat_it) {
                m_MaxExpansion = 4;
            }
        }
    }
}

void CSeqDesktopPanel::GetMainObjects(TConstScopedObjects& objs) const
{
    if (m_SO) {
        SConstScopedObject scoped_object;
        scoped_object.object = m_SO;
        scoped_object.scope = m_Scope;
        objs.push_back(scoped_object);
    }
}

const CObject* CSeqDesktopPanel::GetOrigObject() const
{
    return m_SO.GetPointerOrNull();
}

void CSeqDesktopPanel::GetSelectedObjects(TConstObjects& objects) const
{
    m_TextWindow->GetSelectedObjects(objects);
}

void CSeqDesktopPanel::SetSelectedObjects(const TConstObjects& objects)
{
    m_TextWindow->SetSelectedObjects(objects);
}

void CSeqDesktopPanel::SetUndoManager(ICommandProccessor* cmdProccessor)
{
    m_CmdProccessor = cmdProccessor;
}

void CSeqDesktopPanel::DataChanging()
{
    m_TextWindow->GetViewStart(&m_SavedStartX, &m_SavedStartY);
    m_SavedExpansion = m_TextWindow->GetExpansionLevel();
}

void CSeqDesktopPanel::DataChanged()
{
    x_SetMaxExpansionLevel();
    Load(m_SavedExpansion);
    
    if (m_WidgetHost) {
        TConstObjects objects;
        m_WidgetHost->GetAppSelection(objects);
        SetSelectedObjects(objects);
        // the view should notify the selection service about selection change
        m_WidgetHost->WidgetSelectionChanged();
    }
    
    if (m_SavedStartY > 0) {
        int x, y;
        m_TextWindow->GetVirtualSize(&x, &y);

        int xUnit, yUnit;
        m_TextWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
        if (y / yUnit > m_SavedStartY)
            m_TextWindow->Scroll(m_SavedStartX, m_SavedStartY);
    }
    m_SavedStartX = m_SavedStartY = m_SavedExpansion = 0;
}

void CSeqDesktopPanel::OnZoomInClick(wxCommandEvent& event)
{
    m_TextWindow->ZoomIn();
}

void CSeqDesktopPanel::OnZoomOutClick(wxCommandEvent& event)
{
    m_TextWindow->ZoomOut();
}

void CSeqDesktopPanel::UpdateZoomIn(wxUpdateUIEvent& event)
{
    m_Toolbar->EnableTool(event.GetId(), (m_TextWindow->GetExpansionLevel() < m_MaxExpansion));
}

void CSeqDesktopPanel::UpdateZoomOut(wxUpdateUIEvent& event)
{
    m_Toolbar->EnableTool(event.GetId(), (m_TextWindow->GetExpansionLevel() > 0));
}

CDesktopViewContext* CSeqDesktopPanel::x_CreateDesktopViewContext()
{
    CIRef<IBioseqEditor> editor;
    if (m_CmdProccessor) {
        editor = x_CreateEditor();
    }

    _ASSERT(m_CmdProccessor);
    CDesktopViewContext* context = new CDesktopViewContext(editor, *m_CmdProccessor, m_SavedObject, m_SavedType);
    return context;
}

CIRef<IBioseqEditor> CSeqDesktopPanel::x_CreateEditor()
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

void CSeqDesktopPanel::OnCutSelection(wxCommandEvent& event)
{
    x_SaveSelection();
}

void CSeqDesktopPanel::OnCopySelection(wxCommandEvent& event)
{
    x_SaveSelection();
}

void CSeqDesktopPanel::x_SaveSelection()
{
    m_SavedObject.Reset(m_TextWindow->GetContext()->GetSavedObject());
    m_SavedType = m_TextWindow->GetContext()->GetSavedObjectType();
}

void CSeqDesktopPanel::OnMouseClick(wxMouseEvent& event)
{
    switch (event.GetId()) {
    case ID_WIDGET:
    {
        // redirect event handling to the view
        CDesktopWidgetEvent click_event(CEvent::eEvent_Message, CDesktopWidgetEvent::eSingleClick);
        Send(&click_event, ePool_Parent);
        break;
    }
    default:
        break;
    }

    event.Skip();
}

void CSeqDesktopPanel::OnSearchCtrlClick(wxCommandEvent& event)
{
    x_FindText(ToStdString(event.GetString()));
}

/*
void CSeqDesktopPanel::OnSearchCtrlEnter(wxCommandEvent& event)
{
    x_FindText(ToStdString(event.GetString()));
}
*/

void CSeqDesktopPanel::OnSearchCtrlEnter(wxKeyEvent& event)
{
    switch (event.GetKeyCode()) {
    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
        x_FindText(ToStdString(m_SearchCtrl->GetValue()));
        break;
    default:
        break;
    }
    event.Skip();
}

void CSeqDesktopPanel::x_FindText(const string& search)
{
    if (search.empty()) {
        NcbiMessageBox("Search string is empty");
        return;
    }

    // for now, every search is case insensitive
    bool found = m_TextWindow->FindText(search);
    if (!found) {
        NcbiInfoBox("Search string was not found.");
    }
}

END_NCBI_SCOPE
