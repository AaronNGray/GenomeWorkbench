/*  $Id: selection_panel.cpp 44904 2020-04-13 17:14:22Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "selection_panel.hpp"

#include <gui/utils/app_job_impl.hpp>

#include <gui/core/selection_service_impl.hpp> 

#include <gui/widgets/wx/fileartprov.hpp>

#include <gui/widgets/data/object_contrib.hpp>

#include <gui/widgets/text_widget/plain_text_item.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>

#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/statline.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSelectionTable

class CSelectionViewEventHandler : public wxEvtHandler
{
public:
    CSelectionViewEventHandler( CSelectionPanel* panel ) 
        : m_Panel( panel )
    {}

    void OnContextMenu( wxContextMenuEvent& anEvent );

private:
    CSelectionPanel* m_Panel;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( CSelectionViewEventHandler, wxEvtHandler )
    EVT_CONTEXT_MENU( CSelectionViewEventHandler::OnContextMenu )
END_EVENT_TABLE()

void CSelectionViewEventHandler::OnContextMenu( wxContextMenuEvent& anEvent )
{
    auto_ptr<wxMenu> menu( CreateContextMenuBackbone() );

    void* data = anEvent.GetClientData();
    if( data ){
        Merge( *menu.get(), *(wxMenu*)data );

        anEvent.SetClientData( NULL );
        delete (wxMenu*)data;
    }

    CleanupSeparators( *menu );

// wxGenericListCtrl with GTK for some reason can't have focus from the point of view of wxWidgets.
// This doesn't allow wxGenericListCtrl to handle commands via CCommandToFocusHandler.
// Here is a fix for the Selection View. JIRA: GB-2792
    m_Panel->GetCurrentCtrl()->PopupMenu( menu.get() );
}


#define ACTIVE_VIEW_TARGET_IX 0
#define ALL_VIEWS_TARGET_IX 1

#define ID_TABLE    10001
#define ID_TEXT     10002
#define ID_TOOLBAR  10003
#define ID_COMBOBOX 10004

BEGIN_EVENT_TABLE( CSelectionPanel, wxPanel )
    EVT_CONTEXT_MENU( CSelectionPanel::OnContextMenu )
    EVT_CHOICE( ID_COMBOBOX, CSelectionPanel::OnTargetChanged )
    EVT_TOOL_RANGE( scm_ModeCmd+eTable, scm_ModeCmd+eText, CSelectionPanel::OnModeChanged )
END_EVENT_TABLE()


CSelectionPanel::CSelectionPanel(ISelectionClient* view)
    : m_SelView(view)
    , m_Service(NULL)
    , m_TargetChoice(NULL)
    , m_DisplayMode(eInvalidDisplayMode)
    , m_ListWidget(NULL)
    , m_TextWidget(NULL)
{
    Init();
}

void CSelectionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size )
{
#ifdef __WXOSX_COCOA__ // GB-8581
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
#endif

    wxPanel::Create( parent, id, pos, size );
    CreateControls();

    PushEventHandler( new CCommandToFocusHandler( this ) );
    PushEventHandler( new CSelectionViewEventHandler( this ) );
}

bool CSelectionPanel::Destroy()
{
    PopEventHandler( true );
    PopEventHandler( true );

    return wxPanel::Destroy();
}

void CSelectionPanel::Init()
{
    m_Service = NULL;
    m_TargetChoice = NULL;
    m_ListWidget = NULL;
    m_TextWidget = NULL;

    static bool s_RegisteredIcons = false;

    // register icons only once
    if( ! s_RegisteredIcons ){
        wxFileArtProvider* provider = GetDefaultFileArtProvider();

        provider->RegisterFileAlias( wxT("menu::insp_table_mode"), wxT("insp_table_mode.png") );
        provider->RegisterFileAlias( wxT("menu::insp_brief_text_mode"), wxT("insp_brief_text_mode.png") );
        provider->RegisterFileAlias( wxT("menu::insp_text_mode"), wxT("insp_text_mode.png") );

        s_RegisteredIcons = true;
    }
}

void CSelectionPanel::CreateControls()
{
    wxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( sizer );

    // Create tool bar
    wxToolBar* toolBar = new wxToolBar(
        this, ID_TOOLBAR, wxDefaultPosition, wxDefaultSize,
        wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_HORZ_LAYOUT
    );

    toolBar->AddRadioTool(scm_ModeCmd + eTable, wxT("Table"),
                          wxArtProvider::GetBitmap(wxT("menu::insp_table_mode")),
                          wxNullBitmap, wxT("Table mode"));
    toolBar->AddRadioTool(scm_ModeCmd + eBriefText, wxT("Brief"),
                          wxArtProvider::GetBitmap(wxT("menu::insp_brief_text_mode")),
                          wxNullBitmap, wxT("Brief Text mode"));
    toolBar->AddRadioTool(scm_ModeCmd + eText, wxT("Full"),
                          wxArtProvider::GetBitmap( wxT("menu::insp_text_mode") ),
                          wxNullBitmap, wxT("Full Text mode"));
    toolBar->AddSeparator();

    // Had used a combo box here but that has a strange error on macos carbon - it
    // becomes disabled when certain other windows (like project view) are selected
    // and can't be easily re-enabled (no standard wx functions, enable, setfocus, raise, etc
    // had any effect, but selecting certain Other views, e.g. tree view, did
    // re-enable combo-boxes on this panel...).
    m_TargetChoice = new wxChoice(toolBar, ID_COMBOBOX, wxDefaultPosition);
    toolBar->AddControl( m_TargetChoice );

    toolBar->Realize();
    sizer->Add( toolBar, 0, wxEXPAND );

    // separation line before the table
    wxStaticLine* line = new wxStaticLine(
        this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL
    );
    sizer->Add( line, 0, wxEXPAND );

    // Create list widget
    m_ListWidget = new CObjectListWidget( this, ID_TABLE, wxDefaultPosition, wxSize(100, 100), wxBORDER_NONE );
    m_ListWidget->SetContextMenuEventPropagation( true );
    sizer->Add( m_ListWidget, 1, wxEXPAND );

    // create text widget
    m_TextWidget = new CTextItemPanel( this, ID_TEXT );
    m_TextWidget->Hide();
    m_TextWidget->SetBackgroundColour( wxColor(wxT("white")) );
    m_TextWidget->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    sizer->Add(m_TextWidget, 1, wxEXPAND);

    m_DisplayMode = eTable;

    Update();
}

wxWindow* CSelectionPanel::GetCurrentCtrl()
{
    if (eTable == m_DisplayMode)
        return m_ListWidget;
    else
        return m_TextWidget;
}

void CSelectionPanel::SetSelectionService( CSelectionService* service )
{
    m_Service = service;
    if( !m_Service ){
         m_CurrentClientWeakPtr = NULL;
    }

    Update();
}

void CSelectionPanel::SetRegistryPath( const string& path )
{
    m_RegPath = path; // store for later use
}

void CSelectionPanel::SaveSettings() const
{
    if( ! m_RegPath.empty() ){
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        _ASSERT(m_ListWidget);

        m_ListWidget->SaveTableSettings(view);
    }
}

void CSelectionPanel::LoadSettings()
{
    if( ! m_RegPath.empty() ){
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        _ASSERT( m_ListWidget );

        m_ListWidget->LoadTableSettings(view);
    }
}

void CSelectionPanel::GetSelection( TConstScopedObjects& buf ) const
{
    if( m_DisplayMode == eTable ){
        m_ListWidget->GetSelection( buf );
    } else {
        CTextPanelContext* context = m_TextWidget->GetContext();
        if (context)
            context->GetSelectedObjects( buf );
    }
}

void CSelectionPanel::OnContextMenu( wxContextMenuEvent& anEvent )
{
    wxMenu* menu;

    void* data = anEvent.GetClientData();
    menu = data ? (wxMenu*)data : new wxMenu();

    if( /*ShouldPropagate()*/ true ){
        anEvent.SetClientData( menu );
        anEvent.Skip();
        return;
    }

    anEvent.SetClientData( NULL );

    CleanupSeparators( *menu );
    PopupMenu( menu );

    delete menu;
}

void CSelectionPanel::OnTargetChanged( wxCommandEvent& /*event*/ )
{
    x_ResetObjectWidget();
}

void CSelectionPanel::OnModeChanged( wxCommandEvent& event )
{
    EDisplayMode new_mode = EDisplayMode(event.GetId() - scm_ModeCmd);

    if( new_mode != m_DisplayMode ){
        m_DisplayMode = new_mode;

        if( new_mode == eTable ){
            m_TextWidget->Hide();
            m_ListWidget->Show();
        } else {
            m_ListWidget->Hide();
            m_TextWidget->Show();
            x_InitTextWidget();
        }

        x_ResetObjectWidget();
    }
}

void CSelectionPanel::x_InitCurrentWidget()
{
    if( m_DisplayMode == eTable ){
        m_ListWidget->SetObjects( m_SelObjects );

    } else {
        if( !( m_TextWidget->GetContext() && m_TextWidget->Selecting() ) ){
            x_InitTextWidget();
        }
    }
}

void CSelectionPanel::OnSelServiceStateChange( CSelectionServiceEvent& update )
{
    switch( update.GetID() ){
    case CSelectionServiceEvent::eClientAttached:
        x_ResetTargetChoice();
        break;

    case CSelectionServiceEvent::eClientDetached:
        {{
            CIRef<ISelectionClient> wic(m_CurrentClientWeakPtr.Lock());
            CIRef<ISelectionClient> updateClient = update.GetClient();

            if (wic.GetPointerOrNull() == updateClient) {
                m_CurrentClientWeakPtr = NULL;
                Update();
            } else {
                x_ResetTargetChoice();
            }
        }}
        break;

    case CSelectionServiceEvent::eActiveChanged:
        {{
        CIRef<ISelectionClient> wic(m_CurrentClientWeakPtr.Lock());
        CIRef<ISelectionClient> updateClient = update.GetClient();
        if (updateClient != wic.GetPointerOrNull() && updateClient != m_SelView) {
            if (updateClient == NULL || (m_Service && m_Service->IsAttached(updateClient))) {
                m_CurrentClientWeakPtr = updateClient;
            }

            Update();
        }
        }}
        break;

    case CSelectionServiceEvent::eSelectionChanged:
        x_ResetObjectWidget();
        break;

    default:
        _ASSERT(false);
    }
}

void CSelectionPanel::Update()
{
    x_ResetTargetChoice();

    x_ResetObjectWidget();
}

void CSelectionPanel::x_ResetTargetChoice()
{
    ISelectionClient* cur_target = NULL;
    int target_ix = m_TargetChoice->GetSelection();

    if( target_ix == wxNOT_FOUND ){
        target_ix = ACTIVE_VIEW_TARGET_IX; 

    } else if ( target_ix > ALL_VIEWS_TARGET_IX ) {
        cur_target = (ISelectionClient*)m_TargetChoice->GetClientData(target_ix);
    }

    m_TargetChoice->Clear();

    string s_active = "Active View";
    {{
    CIRef<ISelectionClient> wic(m_CurrentClientWeakPtr.Lock());
    ISelectionClient* current_client = wic.GetPointerOrNull();

    // add "Active Client" line
    if( current_client ){
        s_active += " - " + current_client->GetSelClientLabel();
    }
    }}

    m_TargetChoice->Append( ToWxString(s_active), (void*)nullptr );

    /// add "All Views" line
    m_TargetChoice->Append( wxT("All Views"), (void*)nullptr);

    CSelectionService::TClients clients;

    if( m_Service ){
        m_Service->GetClients( clients );
    }

    if( clients.empty() ){
        m_TargetChoice->SetSelection( ACTIVE_VIEW_TARGET_IX );
        return;
    }

    if (target_ix > ALL_VIEWS_TARGET_IX)
        target_ix = ACTIVE_VIEW_TARGET_IX;
    /// Add lines for all existing clients
    ITERATE( CSelectionService::TClients, it, clients ){
        ISelectionClient* client = *it;
        if( client == m_SelView ){   // skip myself
            continue;
        }

        string s = client->GetSelClientLabel();
        int index = m_TargetChoice->Append( ToWxString(s), (void*)client );

        if( client == cur_target ){
            target_ix = index;
        }
    }
    m_TargetChoice->SetSelection( target_ix );

    m_TargetChoice->SetMinSize(wxDefaultSize);
    wxSize size = m_TargetChoice->GetBestSize();
    m_TargetChoice->SetMinSize(size);
    m_TargetChoice->SetSize(size);

    wxToolBar* toolbar = (wxToolBar*)m_TargetChoice->GetParent();
    toolbar->Realize();
}

bool CSelectionPanel::x_ResetSelObjects()
{
    int target_ix = m_TargetChoice->GetSelection();

    if( !m_Service || target_ix == wxNOT_FOUND ){
        return false;
    }

    TConstScopedObjects tmp_sel_objs;
    if( target_ix == ALL_VIEWS_TARGET_IX ){   // All Views
        CSelectionService::TClients clients;
        m_Service->GetClients( clients );

        ITERATE( CSelectionService::TClients, it, clients ){
            ISelectionClient* client = *it;
            if( client != m_SelView ){
                client->GetActiveObjects( tmp_sel_objs );
            }
        }
    } else {
        ISelectionClient* client = nullptr;
        if( target_ix == ACTIVE_VIEW_TARGET_IX ){   // Active View
            CIRef<ISelectionClient> wic(m_CurrentClientWeakPtr.Lock());
            ISelectionClient* current_client = wic.GetPointerOrNull();
            client = current_client;
        } else {
            client = (ISelectionClient*)m_TargetChoice->GetClientData(target_ix);
            // the functions can be called after the client was removed 
            // and before m_TargetChoice is updated
            // so we need to check if m_TargetChoice's client still exists 
            CSelectionService::TClients active_clients;
            m_Service->GetClients(active_clients);
            if (find(active_clients.begin(), active_clients.end(), client) == active_clients.end()) {
                client = nullptr;
            }
        }

        if( client ){
            client->GetActiveObjects( tmp_sel_objs );
        }
    }

    bool matched = false;
    if (m_SelObjects.size() == tmp_sel_objs.size()) {
        matched = true;
        set<const CObject*> objs;
        ITERATE (TConstScopedObjects, iter, m_SelObjects) {
            objs.insert(iter->object.GetPointerOrNull());
        }

        ITERATE (TConstScopedObjects, iter, tmp_sel_objs) {
            if (objs.count(iter->object.GetPointerOrNull()) == 0) {
                matched = false;
                break;
            }
        }
    }

    if (matched) {
        // no selection changed
        return false;
    }

    m_SelObjects.swap(tmp_sel_objs);
    return true;
}

void CSelectionPanel::x_ResetObjectWidget()
{
    if (x_ResetSelObjects()) {
        x_InitCurrentWidget();
    }

    Layout();
    Refresh();
}

namespace {
    class CCancelGuard
    {
    public:
        CCancelGuard(ICanceled* canceled, CTextPanelContext& context) : m_Context(context)
        {
            m_SaveCanceled = m_Context.SetCanceled(canceled);
        }
        ~CCancelGuard()
        {
            m_Context.SetCanceled(m_SaveCanceled);
        }
    private:
        CTextPanelContext& m_Context;
        ICanceled* m_SaveCanceled;
    };
}

void CSelectionPanel::x_InitTextWidget()
{
    m_TextWidget->ReportLoading();

    typedef tuple<unique_ptr<CCompositeTextItem>, unique_ptr<CTextPanelContext> > TResult;

    m_Future.reset(job_async(
        [objects = m_SelObjects,
        fontDesc = m_TextWidget->GetFont().GetNativeFontInfoDesc(),
        mode = (m_DisplayMode == eBriefText) ? CObjectTextItem::kBrief : CObjectTextItem::kFull](ICanceled& canceled) mutable
    {
        TResult result;

        auto rootItem = std::make_unique<CCompositeTextItem>();
        auto context = std::make_unique<CTextPanelContext>(1, nullptr, nullptr);

        context->SetFontDesc(fontDesc);

        string logMsg("CTextJob: ");

        try {
            NON_CONST_ITERATE(TConstScopedObjects, it, objects) {
                if (canceled.IsCanceled())
                    return result;

                if (rootItem->GetItemCount() > 0) {
                    CPlainTextItem* itemDiv = new CPlainTextItem();
                    itemDiv->AddLine(" ");
                    rootItem->AddItem(itemDiv, false);
                }
                rootItem->AddItem(CObjectTextItem::CreateTextItem(*it, mode, &canceled), false);
            }
        }
        catch (CException& e) {
            NCBI_REPORT_EXCEPTION(logMsg, e);
            throw;
        }
        catch (std::exception& e) {
            CNcbiDiag() << Error << "[" << logMsg << "] Exception: " << e.what();
            throw;
        }

        if (canceled.IsCanceled())
            return result;

        CCancelGuard guard(&canceled, *context);
        wxMemoryDC dc;
        wxFont font(context->GetFontDesc());
        dc.SetFont(font);
        context->CalcWSize(dc);
        rootItem->CalcSize(dc, context.get());

        if (canceled.IsCanceled())
            return result;

        std::get<0>(result) = std::move(rootItem);
        std::get<1>(result) = std::move(context);

        return result;
    },
        [textWidget = m_TextWidget](job_future<TResult>& fn)
    {
        string errMsg = "Unknown fatal error";
        try {
            auto& result = fn();

            unique_ptr<CCompositeTextItem> rootItem(std::move(std::get<0>(result)));
            unique_ptr<CTextPanelContext> context(std::move(std::get<1>(result)));

            if (!rootItem || !context) {
                textWidget->ReportError("No data");
            }
            else {
                textWidget->SetMainItem(rootItem.release(), context.release());
                textWidget->Layout();
                textWidget->Refresh();
            }
            return;
        }
        catch (const CException& e) {
            errMsg = e.GetMsg();
        }
        catch (const std::exception& e) {
            errMsg = e.what();
        }

        textWidget->ReportError("Failed: " + errMsg);
    }
    , "Generate Selection Text"));
}

END_NCBI_SCOPE
