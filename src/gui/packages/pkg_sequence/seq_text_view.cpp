/*  $Id: seq_text_view.cpp 44433 2019-12-18 15:53:19Z shkeda $
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

#include <gui/packages/pkg_sequence/seq_text_view.hpp>

#include <wx/msgdlg.h>

#include <objmgr/util/sequence.hpp>
#include <gui/core/project_service.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/objutils/utils.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/widgets/seq_text/seq_text_panel.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectViewTypeDescriptor s_SeqTextViewTypeDescr(
    "Sequence Text View", // type name
    "seq_text_view", // icon alias
    "Sequence Text View",
    "The Sequence Text View provides a text representation of sequence, "
    "with color coding to highlight specific annotation types.",
    "SEQ_TEXT_VIEW", // help ID
    "Generic",   // category
    false,      // not a singleton
    "Seq-loc",
    eSimilarObjectsAccepted
);

CSeqTextView::CSeqTextView()
:   CProjectView(),
    m_Window(0)
{
}


wxWindow* CSeqTextView::GetWindow()
{
    _ASSERT(m_Window);
    return m_Window;
}

wxEvtHandler* CSeqTextView::GetCommandHandler()
{
    CSeqTextPanel* panel = (CSeqTextPanel*) m_Window;
    _ASSERT(panel);
    CSeqTextWidget* widget = panel->GetWidget();
    return widget->GetEventHandler();
}

bool CSeqTextView::InitView(TConstScopedObjects& objects, const objects::CUser_object*)
{
    x_MergeObjects( objects );

    CScope* scope = NULL;
    const CObject* object = NULL;

    if(objects.size() == 1) {
        object = objects[0].object.GetPointer();
        scope = objects[0].scope.GetPointer();
    }

    if(object)  {
        CSeqTextPanel* panel = (CSeqTextPanel*)m_Window;
        try {
            if (!panel->InitObject(objects[0]))
                return false;
        } catch( CException& ex ){
            string view_name = GetTypeDescriptor().GetLabel();
            string title = view_name + " - Invalid Data";

            wxMessageBox(ToWxString(ex.GetMsg()), ToWxString(title), wxOK | wxICON_EXCLAMATION);
            return false;
        }
        m_OrigObj.Reset(objects[0].object.GetPointer());

        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();

        _ASSERT(srv);

        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;
        CWorkspaceFolder& root_folder = ws->SetWorkspace();

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
        _ASSERT(doc);

        if (doc) {
            x_AttachToProject(*doc);
            x_UpdateContentLabel();
            OnProjectChanged();
            return true;
        } else {
            _ASSERT(false);
            return false;
        }
    } else {
        // cannot represent the data
        x_ReportInvalidInputData(objects);
        return false;
    }
}


void CSeqTextView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_Window);
    CSeqTextPanel* panel = new CSeqTextPanel(parent);
    m_Window = panel;
    CSeqTextWidget* widget = panel->GetWidget();

    AddListener(widget, ePool_Child);
    widget->AddListener(this, ePool_Parent);
}


void CSeqTextView::DestroyViewWindow()
{
    if(m_Window) {
        m_Window->Destroy();
        m_Window = NULL;
    }
}


const CViewTypeDescriptor& CSeqTextView::GetTypeDescriptor() const
{
    return s_SeqTextViewTypeDescr;
}


void CSeqTextView::GetSelection(CSelectionEvent& evt) const
{
    if (x_HasProject()) {
        CSeqTextPanel* panel = (CSeqTextPanel*)m_Window;
        const CSeqTextDataSource* ds = panel->GetDataSource();
        CSeqTextWidget* widget = panel->GetWidget();
        if (widget &&  ds)  {
            // Range Selection
            evt.AddRangeSelection(*ds->GetId(), widget->GetSelection());
        }
    }
}

void CSeqTextView::GetSelection(TConstScopedObjects& objs) const
{
    if (m_Window  &&  x_HasProject()) {
        CSeqTextPanel* panel = (CSeqTextPanel*) m_Window;
        CSeqTextWidget* widget = panel->GetWidget();
        const CSeqTextDataSource* ds = panel->GetDataSource();
        if ( ! widget || ! ds)
            return;

        typedef CSeqTextWidget::TRangeColl TRangeColl;
        CScope* scope = x_GetScope();

        // Range selection
        const TRangeColl& coll = widget->GetSelection();
        if ( ! coll.empty()) {
            CRef<CSeq_loc> loc = CSeqUtils::CreateSeq_loc(*ds->GetId(), coll);
            if (loc)    {
                _ASSERT(scope);
                objs.push_back(SConstScopedObject(loc, scope));
            }
        }
    }
}


void CSeqTextView::GetMainObject(TConstScopedObjects& objs) const
{
    if (m_Window  &&  x_HasProject()) {
        CSeqTextPanel* panel = (CSeqTextPanel*)m_Window;
        const CSeqTextDataSource* ds = panel->GetDataSource();
        if (ds) {
            CScope* scope = x_GetScope();
            objs.push_back(SConstScopedObject(ds->GetLoc(), scope));
        }
    }
}


void CSeqTextView::x_OnSetSelection(CSelectionEvent& evt)
{
    CScope* scope = x_GetScope();

    CSeqTextPanel* panel = (CSeqTextPanel*)m_Window;
    CSeqTextWidget* widget = panel->GetWidget();
    const CSeqTextDataSource* ds = panel->GetDataSource();

    if(evt.HasRangeSelection())     { // set Range Selection
        typedef CSeqTextWidget::TRangeColl TRangeColl;
        CSelectionEvent::TRangeColl coll;

        coll.Empty();
        // set Range Selection
        CConstRef<CSeq_id>  id = ds->GetId();
        evt.GetRangeSelection(*id, *scope, coll);
        if (coll.size())
            widget->SetRangeSelection(coll);
    }

    if(evt.HasObjectSelection())    {
        widget->ResetObjectSelection();

        TConstObjects objs;
        evt.GetAllObjects(objs);
        ITERATE(TConstObjects, it, objs)
            widget->SelectObject(*it);
    }
}

const CObject* CSeqTextView::x_GetOrigObject() const
{
    return m_OrigObj.GetPointer();
}

///////////////////////////////////////////////////////////////////////////////
/// CSeqTextViewFactory
string  CSeqTextViewFactory::GetExtensionIdentifier() const
{
    static string sid("seq_text_view_factory");
    return sid;
}


string CSeqTextViewFactory::GetExtensionLabel() const
{
    static string slabel("Sequence Text View Factory");
    return slabel;
}


void CSeqTextViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("sequence_text_view.png"));
}


const CProjectViewTypeDescriptor& CSeqTextViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_SeqTextViewTypeDescr;
}


IView* CSeqTextViewFactory::CreateInstance() const
{
    return new CSeqTextView();
}


IView* CSeqTextViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    /*TFingerprint print(CSeqTextView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CSeqTextView();
    }*/
    return NULL;
}


int CSeqTextViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false, found_bad = false;

    for (size_t i = 0; i < objects.size() && !(found_good&&found_bad); i++) {
        const type_info& type = typeid(*objects[i].object);
        if (typeid(CSeq_id) == type || typeid(CSeq_entry) == type ||
            typeid(CBioseq_Handle) == type || typeid(CSeq_loc) == type)
            found_good = true;
        else
            found_bad = true;
    }

    if (found_good)
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);

    return 0; // can show nothing
}

bool CSeqTextView::x_MergeObjects( TConstScopedObjects& objects )
{
    if( objects.size() < 2 ){
        return true;
    }
    // verify that all the objects has the same seq-id
    typedef map<CSeq_id_Handle, TSeqRange> TRanges;
    TRanges ranges;
    ITERATE( TConstScopedObjects, iter, objects ){
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*> (iter->object.GetPointer());
        if( loc ){
            CScope* scope = const_cast<CScope*>(iter->scope.GetPointer());
            CSeq_id_Handle idh = sequence::GetIdHandle(*loc, scope);
            TSeqRange range = loc->GetTotalRange();
            ranges[idh] += range;
        }
    }
    if (ranges.size() == 1) {
        CRef<objects::CScope> scope = objects[0].scope;
        CRef<CSeq_loc> loc(new CSeq_loc);
        TRanges::iterator iter = ranges.begin();
        loc->SetInt().SetFrom(iter->second.GetFrom());
        loc->SetInt().SetTo(iter->second.GetTo());
        loc->SetId(*iter->first.GetSeqId());

        //! Should be better incapsulation of m_OutputObjects
        objects.clear();
        objects.push_back(
            SConstScopedObject(loc, scope)
        );
        return true;
    } else {
        return false;
    }
}

END_NCBI_SCOPE
