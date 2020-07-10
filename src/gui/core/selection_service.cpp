/*  $Id: selection_service.cpp 43782 2019-08-29 19:28:56Z katargir $
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

#include <gui/framework/status_bar_service.hpp>
#include <gui/core/selection_service_impl.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/project_view.hpp>
#include <gui/core/commands.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/obj_convert.hpp>
#include <gui/objutils/obj_event.hpp>

#include <gui/widgets/wx/app_status_bar.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <serial/iterator.hpp>

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <wx/dcclient.h>
#include <wx/brush.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
/// CSelectionBroadcastSlot - a Status Bar slot that displays information about
/// tasks in Application Task Service


class NCBI_GUICORE_EXPORT CSelectionBroadcastSlot : public wxPanel, public CEventHandler
{
    DECLARE_EVENT_TABLE();
    DECLARE_EVENT_MAP();
public:
    CSelectionBroadcastSlot();
    ~CSelectionBroadcastSlot() {}

    bool Create(wxWindow* parent);

    void OnPaint(wxPaintEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);

    void    SetSelectionService(CSelectionService* selectionService);

protected:
    wxSize  x_GetTextAreaSize();
    string  x_StatusText(CSelectionEvent::EObjMatchPolicy fObj, CSelectionEvent::EIdMatchPolicy fId);
    string  x_StatusTooltip(CSelectionEvent::EObjMatchPolicy fObj, CSelectionEvent::EIdMatchPolicy fId);

    int     x_GetPendingTasksCount();

    void    x_Update();

protected:
    void    x_OnSelectionEvent(CEvent*);

    wxBoxSizer* m_Sizer;
    wxStaticText* m_TextItem;

    CSelectionService* m_SelectionService;

    wxString m_Text; // current status text

    bool m_Hot; // hot track (mouse inside)
    bool m_Idle; // no running tasks
};

CSelectionService::CSelectionService()
    : m_ActiveClient(NULL)
    , m_SrvLocator(NULL)
    , mf_AutoBroadcast(true)
    , m_StatusBarSlot(nullptr)
{
}


CSelectionService::~CSelectionService()
{
}


void CSelectionService::InitService()
{
    LOG_POST(Info << "Initializing Selection Service...");

    LoadSettings();
    LOG_POST(Info << "Finished initializing Selection Service");
}


void CSelectionService::InitStatusBar(wxWindow* parent)
{
    IStatusBarService* sb_srv = m_SrvLocator->GetServiceByType<IStatusBarService>();

    m_StatusBarSlot = new CSelectionBroadcastSlot();
    m_StatusBarSlot->Create(parent);

    sb_srv->InsertSlot(1, m_StatusBarSlot, m_StatusBarSlot->GetSize().x);
    m_StatusBarSlot->Show();

    m_StatusBarSlot->SetSelectionService(this);
}


void CSelectionService::ShutDownService()
{
    LOG_POST(Info << "Shutting down Selection Service...");

    if (m_StatusBarSlot)
        m_StatusBarSlot->SetSelectionService(nullptr);
    m_StatusBarSlot = 0;

    while (!m_Clients.empty())
        DetachClient(m_Clients.back());

    SaveSettings();

    LOG_POST(Info << "Finished shutting down Selection Service");
}


void CSelectionService::AttachClient( ISelectionClient* client )
{
    TClients::const_iterator it = std::find(m_Clients.begin(), m_Clients.end(), client);
    if( it == m_Clients.end() ){
        m_Clients.push_back(client);
        client->SetSelectionService(this);

        Post(CRef<CEvent>(new CSelectionServiceEvent(
                              CSelectionServiceEvent::eClientAttached,
                              client)));
    } else {
        ERR_POST("CSelectionService::AttachClient() - client already registered");
    }
}


void CSelectionService::DetachClient(ISelectionClient* client)
{
    TClients::iterator it = std::find(m_Clients.begin(), m_Clients.end(), client);
    if(it != m_Clients.end())   {
        m_Clients.erase(it);
        client->SetSelectionService(NULL);
        if(m_ActiveClient == client)
            m_ActiveClient = NULL;

        Post(CRef<CEvent>(new CSelectionServiceEvent(
                              CSelectionServiceEvent::eClientDetached,
                              client)));
    } else {
        ERR_POST("CSelectionService::DetachClient() - client is not attached");
    }
}

bool CSelectionService::IsAttached(ISelectionClient* client) const
{
    TClients::const_iterator it = std::find( m_Clients.begin(), m_Clients.end(), client );
    return it != m_Clients.end();
}


void CSelectionService::OnActiveClientChanged(ISelectionClient* active)
{
    if( active != m_ActiveClient ){
        m_ActiveClient = active;

        Send(CRef<CEvent>(new CSelectionServiceEvent(
                              CSelectionServiceEvent::eActiveChanged,
                              active)));

        if (m_ActiveClient) {
            Post(CRef<CEvent>(new CSelectionServiceEvent(
                                  CSelectionServiceEvent::eSelectionChanged,
                                  m_ActiveClient)));
        }
    }
}

void CSelectionService::GetClients(CSelectionService::TClients& clients)
{
    clients = m_Clients;
}

void CSelectionService::Broadcast(CSelectionEvent& evt, ISelectionClient* source)
{
    static bool fReentry = false;
    if (fReentry)
        return;
    CBoolGuard _guard(fReentry);

    try {
        CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
        _ASSERT(srv);

        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (ws) {
            if (CSelectionEvent::sm_InterDocBroadcast) {
                // broadcast to all documents
                vector<int> loaded = ws->GetLoadedProjects();
                ITERATE(vector<int>, it, loaded) {
                    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(*it));
                    if (doc) {
                        x_Broadcast(evt, source, doc->x_GetViews());
                    }
                }
            }
            else {
                CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(evt.GetScope()));
                // _ASSERT(doc); Search view doesn't have related project
                if (doc)
                    x_Broadcast(evt, source, doc->x_GetViews());
            }
        }
    } NCBI_CATCH("CSelectionService::Broadcast()");
}

void CSelectionService::OnSelectionChanged( ISelectionClient* source )
{
    if (source && mf_AutoBroadcast)
        source->BroadcastSelection();

    Post(CRef<CEvent>(new CSelectionServiceEvent(
                          CSelectionServiceEvent::eSelectionChanged,
                          source)));
}

void CSelectionService::GetCurrentSelection(TConstScopedObjects& sel)
{
    if (m_ActiveClient)
        m_ActiveClient->GetSelection(sel);
}

void CSelectionService::GetActiveObjects(TConstScopedObjects& sel)
{
    if (m_ActiveClient)
        m_ActiveClient->GetActiveObjects(sel);
}

void CSelectionService::GetActiveObjects(vector<TConstScopedObjects>& sel)
{
    if (m_ActiveClient)
        m_ActiveClient->GetActiveObjects(sel);
}

void CSelectionService::SetAutoBroadcast( bool pAuto )
{ 
    mf_AutoBroadcast = pAuto; 
}

void CSelectionService::x_Broadcast(CSelectionEvent& evt, ISelectionClient* source,
                                    CGBDocument::TViews& views)
{
    NON_CONST_ITERATE(CGBDocument::TViews, it, views) {
        IProjectView* view = &**it;
        if(source != static_cast<ISelectionClient*>(view))  {
            ISelectionClient* sel_client = dynamic_cast<ISelectionClient*>(view);
            if(sel_client)    {
                sel_client->SetSelection(evt);
            }
        }
    }
}


void CSelectionService::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


static const char* kObjMatchPolicy = "ObjMatchPolicy";
static const char* kIdMatchPolicy = "IdMatchPolicy";
static const char* kMatchByProduct = "MatchByProduct";
static const char* kAutoBroadcast = "AutoBroadcast";
static const char* kInterDocBroadcast = "InterDocBroadcast";
static const char* kMatchAlnLocs = "MatchAlnLocs";
static const char* kTreeBroadcastProperties = "TreeBroadcastProperties";
static const char* kTreeBroadcastOneToOne = "TreeBroadcastOneToOne";

void CSelectionService::SaveSettings() const
{
    if(! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kObjMatchPolicy, CSelectionEvent::sm_ObjMatchPolicy);
        view.Set(kIdMatchPolicy, CSelectionEvent::sm_IdMatchPolicy);
        view.Set(kMatchByProduct, CSelectionEvent::sm_MatchByProduct);
        //view.Set(kAutoBroadcast, CSelectionEvent::sm_AutoBroadcast);
        view.Set( kAutoBroadcast, mf_AutoBroadcast );
        view.Set(kInterDocBroadcast, CSelectionEvent::sm_InterDocBroadcast);
        view.Set(kMatchAlnLocs, CSelectionEvent::sm_MatchAlnLocs);

        view.Set(kTreeBroadcastProperties, CSelectionEvent::sm_TreeBroadcastProperties);
        view.Set(kTreeBroadcastOneToOne, CSelectionEvent::sm_TreeBroadcastOneToOne);
    }
}


void CSelectionService::LoadSettings()
{
    if(! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        int value = view.GetInt(kObjMatchPolicy, CSelectionEvent::sm_ObjMatchPolicy);
        value = (value < 2) ? value : 0;
        CSelectionEvent::sm_ObjMatchPolicy = (CSelectionEvent::EObjMatchPolicy) value;

        value = view.GetInt(kIdMatchPolicy, CSelectionEvent::sm_IdMatchPolicy);
        value = (value < 2) ? value : 0;
        CSelectionEvent::sm_IdMatchPolicy = (CSelectionEvent::EIdMatchPolicy) value;

        CSelectionEvent::sm_MatchByProduct =
            view.GetBool(kMatchByProduct, CSelectionEvent::sm_MatchByProduct);
        CSelectionEvent::sm_AutoBroadcast =
            view.GetBool(kAutoBroadcast, CSelectionEvent::sm_AutoBroadcast);

        mf_AutoBroadcast = view.GetBool( kAutoBroadcast, mf_AutoBroadcast );
        CSelectionEvent::sm_InterDocBroadcast =
            view.GetBool(kInterDocBroadcast, CSelectionEvent::sm_InterDocBroadcast);
        CSelectionEvent::sm_MatchAlnLocs =
            view.GetBool(kMatchAlnLocs, CSelectionEvent::sm_MatchAlnLocs);

        CSelectionEvent::sm_TreeBroadcastProperties.clear();
        // Set default values if tree broadcast criteria have not yet been added to the registry.
        if (!view.HasField(kTreeBroadcastProperties)) {
            CSelectionEvent::sm_TreeBroadcastProperties.push_back("seq-id");
            CSelectionEvent::sm_TreeBroadcastProperties.push_back("fullasm_id");
            CSelectionEvent::sm_TreeBroadcastProperties.push_back("asm_accession");
            CSelectionEvent::sm_TreeBroadcastProperties.push_back("tax-id");
            CSelectionEvent::sm_TreeBroadcastProperties.push_back("taxid");
            CSelectionEvent::sm_TreeBroadcastProperties.push_back("label");
        }
        else {
            view.GetStringVec(kTreeBroadcastProperties, CSelectionEvent::sm_TreeBroadcastProperties);
        }
        CSelectionEvent::sm_TreeBroadcastOneToOne = view.GetBool(kTreeBroadcastOneToOne, true);
    }
}


void CSelectionService::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}

///////////////////////////////////////////////////////////////////////////////
/// CSelectionBroadcastSlot


BEGIN_EVENT_TABLE(CSelectionBroadcastSlot, wxPanel)
    EVT_PAINT(CSelectionBroadcastSlot::OnPaint)
    EVT_ENTER_WINDOW(CSelectionBroadcastSlot::OnMouseEnter)
    EVT_LEAVE_WINDOW(CSelectionBroadcastSlot::OnMouseLeave)
    EVT_LEFT_DOWN(CSelectionBroadcastSlot::OnLeftDown)
    EVT_LEFT_DCLICK(CSelectionBroadcastSlot::OnLeftDown)
END_EVENT_TABLE()

BEGIN_EVENT_MAP(CSelectionBroadcastSlot, CEventHandler)
    ON_EVENT_RANGE(CSelectionServiceEvent,
                   CSelectionServiceEvent::eClientAttached, CSelectionServiceEvent::eSelectionChanged,
                   &CSelectionBroadcastSlot::x_OnSelectionEvent)
END_EVENT_MAP()

CSelectionBroadcastSlot::CSelectionBroadcastSlot()
    : m_Sizer(NULL)
    , m_TextItem(NULL)
    , m_SelectionService(NULL)
    , m_Hot(false)
    , m_Idle(false)
{
}

void CSelectionBroadcastSlot::SetSelectionService(CSelectionService* selectionService)
{
    if (selectionService) {
        m_SelectionService = selectionService;
        m_SelectionService->AddListener(this);
    } else {
        if (m_SelectionService) {
            m_SelectionService->RemoveListener(this);
            m_SelectionService = 0;
        }
    }
    x_Update();
}

void CSelectionBroadcastSlot::x_OnSelectionEvent(CEvent*)
{
    x_Update();
}

static const int kTextOffX = 4;
static const int kTextOffY = 2;

bool CSelectionBroadcastSlot::Create( wxWindow* parent )
{
    long style = wxCLIP_CHILDREN | wxBORDER_NONE;

    if( wxPanel::Create(parent, wxID_ANY, wxDefaultPosition, wxSize(), style) ){
            Hide();
            SetBackgroundStyle(wxBG_STYLE_CUSTOM);

            /*
            wxSize text_sz = x_GetTextAreaSize();
            m_Sizer = new wxBoxSizer( wxHORIZONTAL );

            // create a Spacer for rendering text
            m_TextItem = m_Sizer->Add( text_sz.x, text_sz.y, 1, wxEXPAND | wxFIXED_MINSIZE );

            SetSizer(m_Sizer);
            */

            m_TextItem = new wxStaticText( 
                this, wxID_ANY, "XXXXXXX", 
                wxDefaultPosition, wxDefaultSize, 
                wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE
            );
            Fit();

            //SetSize(text_sz);
            wxSize sz = GetSize();
            SetMinSize(sz);

            m_TextItem->Bind( wxEVT_ENTER_WINDOW, &CSelectionBroadcastSlot::OnMouseEnter, this );
            m_TextItem->Bind( wxEVT_LEAVE_WINDOW, &CSelectionBroadcastSlot::OnMouseLeave, this );
            m_TextItem->Bind( wxEVT_LEFT_DOWN, &CSelectionBroadcastSlot::OnLeftDown, this );
            m_TextItem->Bind( wxEVT_LEFT_DCLICK, &CSelectionBroadcastSlot::OnLeftDown, this );

            //m_TextItem->Hide();
            return true;
    }
    return false;
}


// calculate the minimal area size sufficient for displaying text
wxSize CSelectionBroadcastSlot::x_GetTextAreaSize()
{
    string s = x_StatusText(CSelectionEvent::eAllIds, CSelectionEvent::eAccOnly );

    wxClientDC dc( this );
    dc.SetFont( GetFont() );
    wxString wx_s = ToWxString( s );

    wxSize size;
    dc.GetTextExtent( wx_s, &size.x, &size.y );

    size.x += 2 * kTextOffX;
    size.y += 2 * kTextOffY;
    return size;
}

// generates a text string representing the current App Task Service state
string CSelectionBroadcastSlot::x_StatusText( CSelectionEvent::EObjMatchPolicy fObj, CSelectionEvent::EIdMatchPolicy fId )
{
    string s;
    /*
    if( fObj == CSelectionEvent::eAllIds ){
        s += "ALL IDS";
    } else {
        s += "ONE ID";
    }

    s += ";";
    */

    if( (fId == CSelectionEvent::eExact) && (fObj == CSelectionEvent::eAllIds) ){
        s += "EXACT";
    } else {
        s += "RELAXED";
    }

    return s;
}

string CSelectionBroadcastSlot::x_StatusTooltip( CSelectionEvent::EObjMatchPolicy fObj, CSelectionEvent::EIdMatchPolicy fId )
{
    string s;

    if( fId == CSelectionEvent::eExact ){
        s += "Exact molecule id matching for selection broadcasting is set";
    } else {
        s += "Molecule versions not used for matching of selection broadcasts";
    }

    return s;
}

void CSelectionBroadcastSlot::x_Update()
{
    m_Text = wxT("");
    
    wxString tooltip;

    if( m_SelectionService ){
        CSelectionEvent::EObjMatchPolicy obj_match = CSelectionEvent::sm_ObjMatchPolicy;
        CSelectionEvent::EIdMatchPolicy id_match = CSelectionEvent::sm_IdMatchPolicy;

        string s = x_StatusText( obj_match, id_match );
        m_Text = ToWxString(s);

        tooltip = ToWxString( x_StatusTooltip( obj_match, id_match ) );
    }
    tooltip += wxT("\n\nClick to see Broadcast Options");


    m_TextItem->SetLabelText( m_Text );
    m_TextItem->SetToolTip( tooltip );

    const wxColour back_cl = CStatusBar::GetBackColor( m_Hot );

    wxColor text_cl = CStatusBar::GetTextColor( m_Hot );
    if( 
        CSelectionEvent::sm_IdMatchPolicy == CSelectionEvent::eExact 
        && CSelectionEvent::sm_ObjMatchPolicy == CSelectionEvent::eAllIds
    ){
        text_cl = *wxRED; 
    }

    m_TextItem->SetBackgroundColour( back_cl );
    m_TextItem->SetForegroundColour( text_cl );

    Layout();
    Refresh();
}


void CSelectionBroadcastSlot::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    // fill background
    wxRect rc = GetRect();
    rc.x = rc.y = 0;
    wxColour back_cl = CStatusBar::GetBackColor(m_Hot);
    wxBrush brush(back_cl);
    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);

    dc.DrawRectangle(rc);

    // draw text
    //rc = m_TextItem->GetRect();
    rc = this->GetRect();

    // set text font
    wxFont font = GetFont();
    font.SetUnderlined(m_Hot);
    dc.SetFont(font);

    // set text color
    wxColor cl = CStatusBar::GetTextColor(m_Hot);
    //wxColor text_cl = m_Idle ? GetAverage(cl, back_cl, 0.5) : cl;
    const wxColor& text_cl = 
        (CSelectionEvent::sm_IdMatchPolicy == CSelectionEvent::eExact && CSelectionEvent::sm_ObjMatchPolicy == CSelectionEvent::eAllIds) ? cl : *wxRED; 
    dc.SetTextForeground(text_cl);

    // prepare and render the text
    int av_w = rc.width - kTextOffX * 2;
    wxString s = TruncateText(dc, m_Text, av_w);

    int w = 0, h = 0;
    dc.GetTextExtent(s, &w, &h);
    int pos_x = rc.x + kTextOffX;
    int pos_y = rc.y + (rc.height - h) / 2;
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    pos_y-=3;
#endif

    dc.DrawText(s, pos_x, pos_y);
}


void CSelectionBroadcastSlot::OnMouseEnter(wxMouseEvent& event)
{
    m_Hot = true;

    x_Update();
}


void CSelectionBroadcastSlot::OnMouseLeave(wxMouseEvent& event)
{
    m_Hot = false;

    x_Update();
}


void CSelectionBroadcastSlot::OnLeftDown(wxMouseEvent& event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, eCmdBroadcastOptions);
    evt.SetEventObject(this);
    GetEventHandler()->ProcessEvent(evt);
}



END_NCBI_SCOPE
