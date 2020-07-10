/*  $Id: data_mining_view.cpp 40880 2018-04-25 20:26:58Z katargir $
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
 *  and reliability of the software and m_Data, the NLM and the U.S.
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

#include <gui/core/data_mining_view.hpp>

#include <gui/core/data_mining_panel.hpp>
#include <gui/core/dm_search_tool.hpp>
#include <gui/core/selection_service_impl.hpp>

#include <gui/framework/workbench.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/extension_impl.hpp>

#include <gui/objutils/visible_range.hpp>
#include <gui/objutils/obj_event.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
/// CDataMiningView

enum EDMCmds    {
    eCmdShowHideForm = 10000,
};


BEGIN_EVENT_MAP(CDataMiningView, CEventHandler)
    ON_EVENT(CEvent, CViewEvent::eWidgetSelectionChanged,
             &CDataMiningView::OnWidgetSelChanged)
    ON_EVENT(CCDataMiningEvent, CCDataMiningEvent::eStateChanged, &CDataMiningView::x_OnStateChanged)

    ON_EVENT(CSelectionServiceEvent, CSelectionServiceEvent::eActiveChanged, &CDataMiningView::x_OnSelectionEvent)
    ON_EVENT(CSelectionServiceEvent, CSelectionServiceEvent::eSelectionChanged, &CDataMiningView::x_OnSelectionEvent)
END_EVENT_MAP()

static CExtensionDeclaration decl("view_manager_service::view_factory",
                                  new CDataMiningViewFactory());

CViewTypeDescriptor CDataMiningView::m_TypeDescr(
    "Search View", // type name
    "search_view", // icon alias TODO
    "Search View",
    "Search View",
    "SEARCH_VIEW", // help ID
    "System",     // category
    true // singleton
);


CDataMiningView::CDataMiningView()
:   m_Workbench(NULL),
    m_DataMiningService(),
    m_SelectionService(),
    m_Panel(NULL)
{
}


CDataMiningView::~CDataMiningView()
{
}

void CDataMiningView::x_OnSelectionEvent(CEvent* evt)
{
    CSelectionServiceEvent* selEvent = dynamic_cast<CSelectionServiceEvent*>(evt);
    if (!selEvent) return;

    if (selEvent->GetClient() == (ISelectionClient*)this)
        return;

    TConstScopedObjects objs;
    m_SelectionService->GetCurrentSelection(objs);

    m_Panel->Update();        
    x_UpdateLocation(objs);        
}


void CDataMiningView::x_OnStateChanged(CEvent*)
{
    m_Panel->Update();
}

void CDataMiningView::x_UpdateLocation(TConstScopedObjects & cso)
{
    m_Range.SetLength(0);
    ITERATE (TConstScopedObjects, obj, cso){
        CObject  * co  = const_cast<CObject*>(obj->object.GetPointer());
        CSeq_loc * loc = dynamic_cast<CSeq_loc*>(co);
        if (loc) {
            m_Range = loc->GetTotalRange();
        }
    }
    /*
    if (m_Range.GetLength()) {
        cout << "Range Selected:" << m_Range.GetFrom() << m_Range.GetTo() << endl;
    }
    */
    if (m_Panel) {
        m_Panel->SetRange(m_Range);
    }    
}

/////////////////////////////////////////
/// IView interface implementation

const CViewTypeDescriptor& CDataMiningView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CDataMiningView::SetWorkbench(IWorkbench* workbench)
{
   if(workbench)   {
        // connect to services
        m_DataMiningService = workbench->GetServiceByType<CDataMiningService>();      
        if (m_DataMiningService) {
            m_DataMiningService->AddListener(this);
            m_Panel->SetService(m_DataMiningService);
            m_Panel->Update();
        }

        m_SelectionService = workbench->GetServiceByType<CSelectionService>();      
        if (m_SelectionService) {
            m_SelectionService->AddListener(this);
            m_SelectionService->AttachClient(this);
        }

        CIRef<IVisibleRangeService> vr_srv = workbench->GetServiceByType<IVisibleRangeService>();        
        if(vr_srv) {
            vr_srv->AttachClient(this);
        }

        m_Panel->SetWorkbench(workbench);

        // to receive/send selection change event
        AddListener(m_Panel, ePool_Child);
        m_Panel->AddListener(this, ePool_Parent);
    } else if(m_Workbench)  {
        m_Panel->RemoveListener(this);

        m_Panel->SetWorkbench(NULL);

        // disconnect from services

        CIRef<IVisibleRangeService> vr_srv = m_Workbench->GetServiceByType<IVisibleRangeService>();        
        if(vr_srv) {
            vr_srv->DetachClient(this);
        }

        if (m_SelectionService) {
            m_SelectionService->RemoveListener(this);
            m_SelectionService->DetachClient(this);
            m_SelectionService = NULL;
        }

        if (m_DataMiningService) {
            m_DataMiningService->RemoveListener(this);
            m_DataMiningService = NULL;
            m_Panel->SetService(m_DataMiningService);
        }
    }

    m_Workbench = workbench;
}


void CDataMiningView::CreateViewWindow( wxWindow* parent )
{
    m_Panel = new CDataMiningPanel(this);
    m_Panel->Create( parent, wxID_ANY );    
}


void CDataMiningView::DestroyViewWindow()
{
    if (m_Panel) {
        RemoveListener(m_Panel);
        //m_Panel->RemoveAllListeners();
        m_Panel->Destroy();     
        m_Panel = NULL;
    }
}


/////////////////////////////////////////
/// IWMClient interface implementation

wxWindow* CDataMiningView::GetWindow()
{
    return m_Panel;
}


string CDataMiningView::GetClientLabel(IWMClient::ELabel) const
{
    return m_TypeDescr.GetLabel();
}


string CDataMiningView::GetIconAlias() const
{
    return m_TypeDescr.GetIconAlias();
}


const CRgbaColor* CDataMiningView::GetColor() const
{
    return NULL;
}


void CDataMiningView::SetColor( const CRgbaColor& color )
{
    // do nothing
}


const wxMenu* CDataMiningView::GetMenu()
{
    return NULL;
}


void CDataMiningView::UpdateMenu( wxMenu& root_menu )
{
}


IWMClient::CFingerprint CDataMiningView::GetFingerprint() const
{
    return CFingerprint(m_TypeDescr.GetLabel(), true);
}


/////////////////////////////////////////////
/// IRegSettings interface implementation

void CDataMiningView::SetRegistryPath( const string& path )
{
    m_RegPath = path; // store for later use
    m_Panel->SetRegistryPath(m_RegPath);
}


void CDataMiningView::SaveSettings() const
{
    m_Panel->SaveSettings();    
}


void CDataMiningView::LoadSettings()
{
    m_Panel->LoadSettings();
    m_Panel->Update();
}


void CDataMiningView::GetSelection(TConstScopedObjects& buf) const
{
    m_Panel->GetSelection(buf);
}

void CDataMiningView::SetSelectionService(ISelectionService*)
{
}

void CDataMiningView::SelectToolByName(const string& tool_name)
{
    if(m_Panel) {
        m_Panel->SelectToolByName(tool_name);
    }    
}


void CDataMiningView::OnWidgetSelChanged(CEvent* evt)
{
    _ASSERT(evt);
   
    m_CurrSelScopeDirty = true;
    x_UpdateSelectionScope();

    if (evt->GetID() == CViewEvent::eWidgetSelectionChanged  &&  m_SelectionService) {
        m_SelectionService->OnSelectionChanged(this);
        BroadcastVisibleRange();
    }
}


void CDataMiningView::BroadcastSelection()
{
    if (m_SelectionService && m_CurrSelScope) {
        CSelectionEvent new_evt(*m_CurrSelScope);
        GetSelection(new_evt);
        m_SelectionService->Broadcast(new_evt, this);
    }
}


void CDataMiningView::BroadcastVisibleRange()
{
    CIRef<IVisibleRangeService> vr_srv = m_Workbench->GetServiceByType<IVisibleRangeService>();        

    if (vr_srv && m_CurrSelScope) {
        CHandleRangeMap map;
        TConstScopedObjects sel_objs;
        GetSelection(sel_objs);

        ITERATE(TConstScopedObjects, it, sel_objs)   {
            const CSeq_loc   * loc = dynamic_cast<const CSeq_loc*>(it->object.GetPointer());
            const CSeq_feat  * sft = dynamic_cast<const CSeq_feat*>(it->object.GetPointer());
            const CSeq_annot * ant = dynamic_cast<const CSeq_annot*>(it->object.GetPointer());
            if(loc) {
                map.AddLocation(*loc);
            }
            else if (sft) {
                map.AddLocation(sft->GetLocation());
            }
            else if (ant) {
                const CSeq_annot::C_Data::TFtable &feats =ant->GetData().GetFtable();
                ITERATE (CSeq_annot::C_Data::TFtable, feat, feats) {
                    map.AddLocation((*feat)->GetLocation());
                }
            }
        }

        /// iterate IDs in the map and broacast location for every id
        ITERATE(CHandleRangeMap, it, map)   {
            const CSeq_id_Handle& id_handle = it->first;
            CRef<CSeq_id> br_id(new CSeq_id);
            br_id->Assign(*id_handle.GetSeqId());

            const CHandleRange& range_handle = it->second;
            // CHandleRange allows ranges from circular sequence. That means
            // GetLeft() may be at the right side of GetRight() depending on 
            // which range gets added first for muliple ranges. 
            TSeqPos from = kMax_UInt; // don't use this: range_handle.GetLeft();
            TSeqPos to   = 0; // don't use this: range_handle.GetRight();
            for (CHandleRange::const_iterator r_iter = range_handle.begin();
                r_iter != range_handle.end(); ++r_iter) {
                    from = min(from, r_iter->first.GetFrom());
                    to = max(to, r_iter->first.GetTo());                
            }

            CBioseq_Handle bsh = m_CurrSelScope->GetBioseqHandle(id_handle);
            if (bsh) {
                TSeqPos five_pct = TSeqPos((to - from) * 0.05f);
                from = from > five_pct ? from - five_pct : 0;
                to = to + five_pct < bsh.GetBioseqLength() ?
                    to + five_pct : bsh.GetBioseqLength();
            }

            CRef<CSeq_loc> br_loc(new CSeq_loc(*br_id, from, to));

            CVisibleRange vrange(*m_CurrSelScope);
            vrange.AddLocation(*br_loc);
            vrange.SetVisibleRangePolicy(CVisibleRange::eBasic_Slave);
            vr_srv->BroadcastVisibleRange(vrange, this);
        }
    }
}

void CDataMiningView::x_UpdateSelectionScope()
{
    if(m_CurrSelScopeDirty) {
        m_CurrSelScope.Reset(NULL);
        m_CurrSelScopeDirty = false;

        TConstScopedObjects sel_objs;
        GetSelection(sel_objs);

        CScope* sel_sc = NULL;
        ITERATE(TConstScopedObjects, it, sel_objs)   {
            CScope* sc = const_cast<CScope*>(it->scope.GetPointer());
            if(sel_sc == NULL)  {
                sel_sc = sc;
            } else {
                if(sc  &&  sel_sc != sc)    {
                    // multiple scopes in selection
                    return;
                }
            }
        }
        /// all selected objects have the same scope
        m_CurrSelScope.Reset(sel_sc);
    }
}


string  CDataMiningView::GetSelClientLabel()
{
    return m_TypeDescr.GetLabel();
}


void CDataMiningView::GetSelection(CSelectionEvent& evt) const
{
    TConstScopedObjects sel_objs;
    GetSelection(sel_objs);
    if( ! sel_objs.empty())    {
        ITERATE(TConstScopedObjects, it, sel_objs)   {
            const CSeq_loc   * loc = dynamic_cast<const CSeq_loc*>(it->object.GetPointer());
            const CSeq_feat  * sft = dynamic_cast<const CSeq_feat*>(it->object.GetPointer());
            const CSeq_annot * ant = dynamic_cast<const CSeq_annot*>(it->object.GetPointer());
            if(loc) {
                evt.AddRangeSelection(*loc);
            }
            else if (sft) {
                evt.AddObjectSelection(*sft);
            }
            else if (ant) {
                const CSeq_annot::C_Data::TFtable &feats =ant->GetData().GetFtable();
                ITERATE (CSeq_annot::C_Data::TFtable, feat, feats) {
                    evt.AddRangeSelection((*feat)->GetLocation());
                }
                
            }
            else {
                evt.AddObjectSelection(it->object.GetObject());         
            }
        }
    }
}


void CDataMiningView::SetSelection(CSelectionEvent& evt)
{
    // not supported 
}


void CDataMiningView::GetMainObject(TConstScopedObjects& objects) const
{
    // there is no main object here
}


///////////////////////////////////////////////////////////////////////////////
/// CDataMiningViewFactory
string CDataMiningViewFactory::GetExtensionIdentifier() const
{
    static string sid("data_mining_factory");
    return sid;
}


string CDataMiningViewFactory::GetExtensionLabel() const
{
    static string slabel("Data Mining Factory");
    return slabel;
}


void CDataMiningViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("search.png"));
}


const CViewTypeDescriptor& CDataMiningViewFactory::GetViewTypeDescriptor() const
{
    return CDataMiningView::m_TypeDescr;
}


IView* CDataMiningViewFactory::CreateInstance() const
{
    return new CDataMiningView();
}


IView* CDataMiningViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    TFingerprint print(CDataMiningView::m_TypeDescr.GetLabel(), true);
    if(print == fingerprint)   {
        return CreateInstance();
    }
    return NULL;
}


END_NCBI_SCOPE
