/*  $Id: sequin_desktop_view.cpp 41968 2018-11-23 17:42:33Z filippov $
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
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>

#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/framework/status_bar_service.hpp>
#include <objtools/edit/apply_object.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/seq_desktop/seq_desktop_panel.hpp>
#include <gui/widgets/seq_desktop/desktop_canvas.hpp>
#include <gui/widgets/seq_desktop/desktop_event.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/sequin_desktop_view.hpp>

#include <wx/panel.h>
#include <wx/aui/aui.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace {
CProjectViewTypeDescriptor s_ViewTypeDescr(
    "Sequin Desktop View", // type name
    "sequin_desktop_view", // icon alias
    "Create an interactive text representation of data.",
    "The Sequin Desktop View shows Seq-entry hierarchy.",
    "SEQUIN_DESKTOP_VIEW", // help ID
    "Sequence",   // category
    false,      // not a singleton
    "Seq-entry",
    eOneObjectAccepted
);
}


BEGIN_EVENT_MAP(CSequinDesktopView, CProjectView)
    ON_EVENT(CDesktopWidgetEvent, CDesktopWidgetEvent::eRefreshData, &CSequinDesktopView::x_OnRefreshData)
    ON_EVENT(CDesktopWidgetEvent, CDesktopWidgetEvent::eSingleClick, &CSequinDesktopView::x_OnSingleMouseClick)
END_EVENT_MAP()

CSequinDesktopView::CSequinDesktopView()
  : CProjectView(), m_Window(0)
{
}

CSequinDesktopView::~CSequinDesktopView()
{
}

wxWindow* CSequinDesktopView::GetWindow()
{
    _ASSERT(m_Window);
    return m_Window;
}

wxEvtHandler* CSequinDesktopView::GetCommandHandler()
{
    _ASSERT(m_Window);
    return m_Window->GetWidget()->GetEventHandler();
}

// override function to suppress Navigation toolbar
void CSequinDesktopView::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}

const wxMenu* CSequinDesktopView::GetMenu()
{
    return 0;
}

bool CSequinDesktopView::InitView(TConstScopedObjects& objects, const CUser_object* params)
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
        CWorkspaceFolder& root_folder = ws->SetWorkspace();

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
        _ASSERT(doc);

        if (doc) {
            m_Window->SetUndoManager(&doc->GetUndoManager());

            if (!m_Window->InitWidget(objects)) return false;

            x_AttachToProject(*doc);
            x_UpdateContentLabel();

            // When the Flat File view is focused on Nucleotides/All, the visible area
            // is automatically updated to show the selected object.
            // In other circumstances, this automatic update of the visible area does not take place
            IProjectView* projectTextView = srv->FindView(*object, "Text View");
            if (projectTextView) {
                CTextPanel* panel = dynamic_cast<CTextPanel*>(projectTextView->GetWindow());
                panel->GetWidget()->SetTrackSelection(true);
            }
            
            return true;
        }
        return false;
    } else {
        // cannot represent the data
        x_ReportInvalidInputData(objects);
        return false;
    }
}

void CSequinDesktopView::OnProjectChanged(CProjectViewEvent& evt)
{
    if (evt.GetSubtype() == CProjectViewEvent::eDataChanging) {
        if (m_Window) m_Window->DataChanging();
    }
    else if (evt.GetSubtype() == CProjectViewEvent::eData ||
             evt.GetSubtype() == CProjectViewEvent::eBothPropsData) {
        if (m_Window) m_Window->DataChanged();
    }
}


void CSequinDesktopView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_Window);

    m_Window = new CSeqDesktopPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(0, 0), wxTAB_TRAVERSAL | wxNO_BORDER);
    m_Window->SetHost(this);
    m_Window->GetWidget()->SetHost(this);

    m_Window->AddListener(this, ePool_Parent);
}


void CSequinDesktopView::DestroyViewWindow()
{
    if (m_Window) {
        m_Window->GetWidget()->SetHost(nullptr);
        m_Window->SetHost(nullptr);
        m_Window->Destroy();
        m_Window = nullptr;
    }
}


const CViewTypeDescriptor& CSequinDesktopView::GetTypeDescriptor() const
{
    return s_ViewTypeDescr;
}


void CSequinDesktopView::GetSelection(CSelectionEvent& evt) const
{
    if (!m_Window && !x_HasProject()) return;

    TConstObjects objects;
    m_Window->GetSelectedObjects(objects);
    evt.AddObjectSelection(objects);
}


void CSequinDesktopView::GetSelection(TConstScopedObjects& objs) const
{
    if (!m_Window && !x_HasProject()) return;

    CScope* scope = x_GetScope();
    TConstObjects objects;
    m_Window->GetSelectedObjects(objects);
    ITERATE(vector< CConstRef<CObject> >, it, objects)
        objs.push_back(SConstScopedObject(*it, scope));
}


void CSequinDesktopView::GetMainObject(TConstScopedObjects& objs) const
{
    if (!m_Window && !x_HasProject()) return;

    m_Window->GetMainObjects(objs);
}

void CSequinDesktopView::x_OnSetSelection(CSelectionEvent& evt)
{
    if (!m_Window && !x_HasProject()) return;

    TConstObjects objs;
    evt.GetAllObjects(objs);
    evt.GetOther(objs);
    m_Window->SetSelectedObjects(objs);
}


const CObject* CSequinDesktopView::x_GetOrigObject() const
{
    return m_Window ? m_Window->GetOrigObject() : 0;
}

/// IGuiWidgetHost implementation

void CSequinDesktopView::CloseWidget()
{
    CProjectService* prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    prjSrv->RemoveProjectView(*this);
}

void CSequinDesktopView::WidgetSelectionChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
}

void CSequinDesktopView::WidgetActivated()
{
}

void CSequinDesktopView::GetAppSelection(TConstObjects& objects)
{
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    TConstScopedObjects scopedObjects;
    sel_srv->GetCurrentSelection(scopedObjects);
    ITERATE(TConstScopedObjects, it, scopedObjects)
        objects.push_back(it->object);
}

void CSequinDesktopView::OnWidgetCommand(const string& command)
{
}

void CSequinDesktopView::ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat)
{
    if (objects.empty()) return;
    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    prjSrv->ShowView(viewName, widgetFactory, objects, bFloat);
}

void CSequinDesktopView::x_OnRefreshData(CEvent* evt)
{
    CDesktopWidgetEvent* desktop_event(dynamic_cast<CDesktopWidgetEvent*>(evt));
    if (!desktop_event) return;

    TConstScopedObjects objects;
    GetViewObjects(m_Workbench, objects);
    m_Window->InitWidget(objects);
    
}

static CBioseq_Handle s_GetAccession(SConstScopedObject& scoped_object)
{
    const CBioseq* bseq = dynamic_cast<const CBioseq*>(scoped_object.object.GetPointer());
    const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(scoped_object.object.GetPointer());
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(scoped_object.object.GetPointer());

    CBioseq_Handle bsh;
    if (bseq) {
        bsh = scoped_object.scope->GetBioseqHandle(*bseq);
    }
    else if (desc) {
        CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(scoped_object.scope, *desc);
        if (seh) {
            if (seh.IsSeq()) {
                bsh = seh.GetSeq();
            }
            else {
                CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
                if (b_iter) {
                    bsh = *b_iter;
                }
            }
        }
    }
    else if (feat) {
        bsh = GetBioseqForSeqFeat(*feat, *scoped_object.scope);
    }

    return bsh;
}

void CSequinDesktopView::x_OnSingleMouseClick(CEvent* event)
{
    CDesktopWidgetEvent* desktop_event(dynamic_cast<CDesktopWidgetEvent*>(event));
    if (!desktop_event) return;

    // get the flat file view

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    _ASSERT(srv);

    TConstScopedObjects main_object;
    GetMainObject(main_object);
    if (main_object.empty()) {
        return;
    }
    CIRef<IProjectView> text_view(srv->FindView(*(main_object.front().object), "Text View"));
    if (!text_view) {
        // don't open a new text view if there is not one already
        return;
    }

    CIRef<IFlatFileCtrl> FlatFileCtrl(dynamic_cast<IFlatFileCtrl*>(text_view.GetPointerOrNull()));
    if (FlatFileCtrl) {
        TConstScopedObjects objs;
        GetSelection(objs);
        if (!objs.empty()) {
            try {
                CBioseq_Handle bsh = s_GetAccession(objs.front());
                FlatFileCtrl->SetPosition(bsh, objs.front().object.GetPointer());
            }
            catch (const CException&) {}
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CSequinDesktopViewFactory
string  CSequinDesktopViewFactory::GetExtensionIdentifier() const
{
    static string sid("sequin_desktop_view_factory");
    return sid;
}


string CSequinDesktopViewFactory::GetExtensionLabel() const
{
    static string slabel("Sequin Desktop View Factory");
    return slabel;
}


void CSequinDesktopViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("text_view.png"));
}

void CSequinDesktopViewFactory::RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
}

const CProjectViewTypeDescriptor& CSequinDesktopViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_ViewTypeDescr;
}


IView* CSequinDesktopViewFactory::CreateInstance() const
{
    return new CSequinDesktopView();
}


IView* CSequinDesktopViewFactory::CreateInstanceByFingerprint(const TFingerprint&) const
{
    return NULL;
}


int CSequinDesktopViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;

    for(  size_t i = 0;  i < objects.size();  i++)  {
        const CSerialObject* so =
            dynamic_cast<const CSerialObject*>(objects[i].object.GetPointer());
        if (so 
            && (dynamic_cast<const CSeq_submit*>(so)
                || dynamic_cast<const CSeq_entry*>(so)
                || dynamic_cast<const CBioseq*>(so)
                || dynamic_cast<const CBioseq_set*>(so)
                || dynamic_cast<const CSeq_annot*>(so)
                || dynamic_cast<const CSeq_id*>(so))) {
                found_good = true;
        }
        else {
            found_bad = true;
        }
    }

    if (found_good)
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);

    return 0; // can show nothing
}

END_NCBI_SCOPE
