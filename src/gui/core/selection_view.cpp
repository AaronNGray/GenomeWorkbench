/*  $Id: selection_view.cpp 38835 2017-06-23 18:15:53Z katargir $
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

#include "selection_view.hpp"
#include "selection_panel.hpp"

#include <gui/core/selection_service_impl.hpp> 

#include <gui/framework/workbench.hpp>

#include <gui/utils/extension_impl.hpp>

#include <gui/objutils/obj_event.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CSelectionView

/// register the factory
static CExtensionDeclaration
    decl( "view_manager_service::view_factory", new CSelectionViewFactory() )
;

BEGIN_EVENT_MAP(CSelectionView, CEventHandler)
        ON_EVENT_RANGE(CSelectionServiceEvent,
                   CSelectionServiceEvent::eClientAttached, CSelectionServiceEvent::eSelectionChanged,
                   &CSelectionView::x_OnSelectionEvent)
END_EVENT_MAP()

CViewTypeDescriptor CSelectionView::m_TypeDescr(
    "Active Objects Inspector", // type name
    "selection_inspector", // icon alias TODO
    "Active Objects Inspector displays information about objects selected in views",
    "Active Objects Inspector displays information about objects selected in views",
    "SELECTION_INSPECTOR", // help ID
    "System",     // category
    true // singleton
);

CSelectionView::CSelectionView()
    : m_Service( NULL )
    , m_Broadcasting( false )
    , m_Panel( NULL )
{
}

CSelectionView::~CSelectionView()
{
    SetWorkbench(NULL);
}

/////////////////////////////////////////
/// IView interface implementation

const CViewTypeDescriptor& CSelectionView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CSelectionView::SetWorkbench( IWorkbench* workbench )
{
    if (workbench) {
        m_Service  = workbench->GetServiceByType<CSelectionService>();
        if (m_Service) {
            m_Service->AddListener(this);
        }
    } else {
        if (m_Service) {
            m_Service->RemoveListener(this);
            m_Service = 0;
        }
    }

    if (m_Panel)
        m_Panel->SetSelectionService(m_Service);
}

void CSelectionView::x_OnSelectionEvent(CEvent* evt)
{
    CSelectionServiceEvent* sse = dynamic_cast<CSelectionServiceEvent*>(evt);
    if (sse)
        m_Panel->OnSelServiceStateChange(*sse);
    else
        m_Panel->Update();
}

void CSelectionView::CreateViewWindow( wxWindow* parent )
{
    m_Panel = new CSelectionPanel(this);
    m_Panel->Create( parent, wxID_ANY );
}


void CSelectionView::DestroyViewWindow()
{
    if( m_Panel ){
        m_Panel->Destroy();
        m_Panel = NULL;
    }
}

/////////////////////////////////////////
/// IWMClient interface implementation

wxWindow* CSelectionView::GetWindow()
{
    return m_Panel;
}


string CSelectionView::GetClientLabel(IWMClient::ELabel) const
{
    return m_TypeDescr.GetLabel();
}


string CSelectionView::GetIconAlias() const
{
    return m_TypeDescr.GetIconAlias();
}


const CRgbaColor* CSelectionView::GetColor() const
{
    return NULL;
}


void CSelectionView::SetColor( const CRgbaColor& /*color*/ )
{
    // do nothing
}


const wxMenu* CSelectionView::GetMenu()
{
    return NULL;
}


void CSelectionView::UpdateMenu( wxMenu& /*root_menu*/ )
{
}


IWMClient::CFingerprint CSelectionView::GetFingerprint() const
{
    return CFingerprint(m_TypeDescr.GetLabel(), true);
}


/////////////////////////////////////////////
/// IRegSettings interface implementation

void CSelectionView::SetRegistryPath( const string& path )
{
    m_Panel->SetRegistryPath( path ); // store for later use
}

void CSelectionView::SaveSettings() const
{
    m_Panel->SaveSettings();
}


void CSelectionView::LoadSettings()
{
    m_Panel->LoadSettings();
}


void CSelectionView::GetSelection(TConstScopedObjects& buf) const
{
    m_Panel->GetSelection(buf);
}


void CSelectionView::SetSelectionService(ISelectionService* service)
{
    CSelectionService* srv = dynamic_cast<CSelectionService*>( service );
    m_Service = srv;
}


string  CSelectionView::GetSelClientLabel()
{
    return "Active Objects Inspector";
}


void CSelectionView::GetSelection(CSelectionEvent& evt) const
{
    TConstScopedObjects sel_objs;
    GetSelection(sel_objs);
    if( ! sel_objs.empty())    {
        TConstObjects objs;
        ITERATE(TConstScopedObjects, it, sel_objs)   {
            objs.push_back(it->object);
        }

        // brodcast everything as Object selection, we cannot distinguish object
        // selection from range selection at this point
        evt.AddObjectSelection(objs);
    }
}


void CSelectionView::SetSelection(CSelectionEvent& evt)
{
    // not supported
}


void CSelectionView::GetMainObject(TConstScopedObjects& /*objects*/) const
{
    // there is no main object here
}


///////////////////////////////////////////////////////////////////////////////
/// CSelectionViewFactory
string CSelectionViewFactory::GetExtensionIdentifier() const
{
    static string sid("selection_inspector_factory");
    return sid;
}


string CSelectionViewFactory::GetExtensionLabel() const
{
    static string slabel("Selection Inspector Factory");
    return slabel;
}


void CSelectionViewFactory::RegisterIconAliases(wxFileArtProvider& /*provider*/)
{
    //TODO
}


const CViewTypeDescriptor& CSelectionViewFactory::GetViewTypeDescriptor() const
{
    return CSelectionView::m_TypeDescr;
}


IView* CSelectionViewFactory::CreateInstance() const
{
    return new CSelectionView();
}


IView* CSelectionViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    TFingerprint print(CSelectionView::m_TypeDescr.GetLabel(), true);
    if(print == fingerprint)   {
        return CreateInstance();
    }

    return NULL;
}


END_NCBI_SCOPE
