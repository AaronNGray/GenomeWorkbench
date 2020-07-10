/*  $Id: text_view.cpp 39412 2017-09-21 17:44:52Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "text_view.hpp"

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>

#include <gui/objects/GBWorkspace.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/status_bar_service.hpp>
#include <gui/core/selection_service_impl.hpp>

#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/seq/text_ds.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace {
CProjectViewTypeDescriptor s_ViewTypeDescr(
    "Text View", // type name
    "text_view", // icon alias
    "Create an interactive text representation of data.",
    "The Text View shows data in a variety of formats, including GenBank FlatFile, "
    "NCBI ASN.1, and FastA format.",
    "TEXT_VIEW", // help ID
    "Generic",   // category
    false,      // not a singleton
    "SerialObject",
    eOneObjectAccepted
);
}

CTextView::CTextView()
  : CProjectView(), m_Window(0), m_UsingStatusBar(false)
{
}

CTextView::~CTextView()
{
    if (m_UsingStatusBar && m_Workbench) {
        CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
        if (sb_srv) {
            sb_srv->RemoveSlot(1);
        }
    }
}

wxWindow* CTextView::GetWindow()
{
    _ASSERT(m_Window);
    return m_Window;
}

wxEvtHandler* CTextView::GetCommandHandler()
{
    _ASSERT(m_Window);
    return m_Window->GetWidget()->GetEventHandler();
}

// override function to suppress Navigation toolbar
void CTextView::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}

const wxMenu* CTextView::GetMenu()
{
    if (m_Window) {
        CIRef<IMenuContributor> contributor = m_Window->GetMenuContributor();
        if (contributor)
            return contributor->GetMenu();
    }

    return 0;
}

bool CTextView::InitView(TConstScopedObjects& objects, const CUser_object* params)
{
    CScope* scope = NULL;
    const CObject* object = NULL;

    if(objects.size() == 1) {
        object = objects[0].object.GetPointer();
        scope = objects[0].scope.GetPointer();
    }

    if (object)  {
        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        _ASSERT(srv);

        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
        _ASSERT(doc);

        if (doc) {
            m_Window->SetUndoManager(&doc->GetUndoManager());

            if (params) {
                string viewType, initialSequence;
                bool expandAll = false, trackSelection = false;

                const CObject_id& type = params->GetType();
                if (type.IsStr() && type.GetStr() == "TextViewParams") {
                    ITERATE(CUser_object::TData, it, params->GetData()) {
                        const CObject_id& field_id = (*it)->GetLabel();
                        if (!field_id.IsStr())
                            continue;

                        const string& label = field_id.GetStr();
                        const CUser_field::TData& data = (*it)->GetData();

                        if (label == "TextViewType") {
                            if (data.IsStr())
                                viewType = data.GetStr();
                        }
                        else if (label == "ExpandAll") {
                            if (data.IsBool())
                                expandAll = data.GetBool();
                        }
                        else if (label == "TrackSelection") {
                            if (data.IsBool()) {
                                trackSelection = data.GetBool();
                            }
                        }
                        else if (label == "InitialSequence") {
                            if (data.IsStr()) {
                                initialSequence = data.GetStr();
                            }
                        }
                    }
                }

                if (!viewType.empty())
                    m_Window->SetInitialViewType(viewType);

                if (expandAll)
                    m_Window->SetOpenExpanded(true);

                if (!initialSequence.empty())
                    m_Window->SetInitialSequence(initialSequence);

				m_Window->GetWidget()->SetTrackSelection(trackSelection);
            }

            if (!m_Window->InitWidget(objects)) return false;

            x_AttachToProject(*doc);
            x_UpdateContentLabel();
            
            return true;
        }
        return false;
    } else {
        // cannot represent the data
        x_ReportInvalidInputData(objects);
        return false;
    }
}

void CTextView::OnProjectChanged(CProjectViewEvent& evt)
{
    if (evt.GetSubtype() == CProjectViewEvent::eDataChanging) {
        if (m_Window) m_Window->DataChanging();
    }
    else if (evt.GetSubtype() == CProjectViewEvent::eData ||
             evt.GetSubtype() == CProjectViewEvent::eBothPropsData) {
        if (m_Window) m_Window->DataChanged();
    }
}


void CTextView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_Window);

    m_Window = new CTextPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(0, 0), wxTAB_TRAVERSAL|wxNO_BORDER);
    m_Window->SetHost(this);
    m_Window->GetWidget()->SetHost(this);
}


void CTextView::DestroyViewWindow()
{
    if (m_Window) {
        m_Window->GetWidget()->SetHost(0);
        m_Window->SetHost(0);
        m_Window->Destroy();
        m_Window = 0;
    }
}


const CViewTypeDescriptor& CTextView::GetTypeDescriptor() const
{
    return s_ViewTypeDescr;
}


void CTextView::GetSelection(CSelectionEvent& evt) const
{
    if (!m_Window && !x_HasProject()) return;

    TConstObjects objects;
    m_Window->GetSelectedObjects(objects);
    evt.AddObjectSelection(objects);
}


void CTextView::GetSelection(TConstScopedObjects& objs) const
{
    if (!m_Window && !x_HasProject()) return;

    CScope* scope = x_GetScope();
    TConstObjects objects;
    m_Window->GetSelectedObjects(objects);
    ITERATE(vector< CConstRef<CObject> >, it, objects)
        objs.push_back(SConstScopedObject(*it, scope));
}


void CTextView::GetMainObject(TConstScopedObjects& objs) const
{
    if (!m_Window && !x_HasProject()) return;

	m_Window->GetMainObjects(objs);
}

void CTextView::GetActiveObjects(TConstScopedObjects& objects) const
{
    if (!m_Window && !x_HasProject()) return;

    vector<TConstScopedObjects> widgetObjects;
    m_Window->GetActiveObjects(widgetObjects);
    ITERATE(vector<TConstScopedObjects>, it, widgetObjects)
        objects.insert(objects.end(), it->begin(), it->end());
}

void CTextView::GetActiveObjects(vector<TConstScopedObjects>& objects)
{
    if (!m_Window && !x_HasProject()) return;

    m_Window->GetActiveObjects(objects);
}



void CTextView::x_OnSetSelection (CSelectionEvent& evt)
{
    if (!m_Window && !x_HasProject()) return;

    TConstObjects objs;
    evt.GetAllObjects(objs);
	evt.GetOther(objs);
    m_Window->SetSelectedObjects(objs);
}

const CObject* CTextView::x_GetOrigObject() const
{
    return m_Window ? m_Window->GetOrigObject() : 0;
}

/// IFlatFileCtrl implementation

bool CTextView::SetPosition(const string& seq, const CObject* object)
{
    IFlatFileCtrl* ctrl = dynamic_cast<IFlatFileCtrl*>(m_Window);
    return ctrl ? ctrl->SetPosition(seq, object) : false;
}

bool CTextView::SetPosition(const objects::CBioseq_Handle& h, const CObject* object)
{
    IFlatFileCtrl* ctrl = dynamic_cast<IFlatFileCtrl*>(m_Window);
    return ctrl ? ctrl->SetPosition(h, object) : false;
}

/// IGuiWidgetHost implementation

void CTextView::CloseWidget()
{
    CProjectService* prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    prjSrv->RemoveProjectView(*this);
}

void CTextView::WidgetSelectionChanged()
{
    if (m_SelectionService)
        m_SelectionService->OnSelectionChanged(this);
}

void CTextView::WidgetActivated()
{
}

void CTextView::GetAppSelection(TConstObjects& objects)
{
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    TConstScopedObjects scopedObjects;
    sel_srv->GetCurrentSelection(scopedObjects);
    ITERATE(TConstScopedObjects, it, scopedObjects)
        objects.push_back(it->object);
}

void CTextView::OnWidgetCommand(const string& command)
{
}

void CTextView::ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat)
{
    if (objects.empty()) return;
    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    prjSrv->ShowView(viewName, widgetFactory, objects, bFloat);
}

/// ITextWidgetHost implementation

void CTextView::OnTextGotFocus()
{
    CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
    if (!m_UsingStatusBar) {
        sb_srv->InsertSlot(1, NULL, 120);
        m_UsingStatusBar = true;
    }
}

void CTextView::OnTextLostFocus()
{
    if (m_UsingStatusBar) {
        CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
        sb_srv->RemoveSlot(1);
        m_UsingStatusBar = false;
    }
}

void CTextView::OnTextPositionChanged(int row, int col)
{
    CNcbiOstrstream ostr;
    ostr << " Ln " << row + 1 << " Col " << col + 1;
    string text = CNcbiOstrstreamToString(ostr);
    CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
    sb_srv->SetStatusText(ToWxString(text), 1);
}


///////////////////////////////////////////////////////////////////////////////
/// CTextViewFactory
string  CTextViewFactory::GetExtensionIdentifier() const
{
    static string sid("text_view_factory");
    return sid;
}


string CTextViewFactory::GetExtensionLabel() const
{
    static string slabel("Text View Factory");
    return slabel;
}


void CTextViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("text_view.png"));
}

void CTextViewFactory::RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    CTextPanel::RegisterCommands(cmd_reg, provider);
}

const CProjectViewTypeDescriptor& CTextViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_ViewTypeDescr;
}


IView* CTextViewFactory::CreateInstance() const
{
    return new CTextView();
}


IView* CTextViewFactory::CreateInstanceByFingerprint(const TFingerprint&) const
{
    return NULL;
}


int CTextViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;

    for(  size_t i = 0;  i < objects.size();  i++)  {
        const CSerialObject* so =
            dynamic_cast<const CSerialObject*>(objects[i].object.GetPointer());
        if (so)
            found_good = true;
        else
            found_bad = true;
    }

    if (found_good)
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);

    return 0; // can show nothing
}


END_NCBI_SCOPE
