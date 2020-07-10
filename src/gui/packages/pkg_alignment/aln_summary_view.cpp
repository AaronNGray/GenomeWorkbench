/*  $Id: aln_summary_view.cpp 39258 2017-08-31 15:28:36Z evgeniev $
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

#include <gui/packages/pkg_alignment/aln_summary_view.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/status_bar_service.hpp>

#include <wx/grid.h>

#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/table_data_aln_summary.hpp>
#include <gui/widgets/grid_widget/grid_event.hpp>
#include <gui/widgets/grid_widget/grid.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/utils/app_popup.hpp>

#include <objects/seqalign/Seq_align_set.hpp>

#include <objmgr/util/sequence.hpp>
#include <objects/seqalign/Seq_align.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlnSummaryView

CProjectViewTypeDescriptor s_AlnSummaryViewTypeDescr(
    "Alignment Summary View", // type name
    "aln_summary_view", // icon alias
    "Create an interactive table representation of alignment summary.",
    "The Alignment Summary View shows summary for a list of alignments",
    "ALN_SUMMARY_VIEW", // help ID
    "Alignment",   // category
    false,      // not a singleton
    "Seq-align",
    eAnyObjectsAccepted
);

class CAlnSummaryView : public CTableView
{
public:
    CAlnSummaryView() {}

    /// @name IView implementation
    /// @(
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const
    {
        return s_AlnSummaryViewTypeDescr;
    }
    virtual void    CreateViewWindow(wxWindow* parent);
    /// @}

protected:
    virtual CIRef<ITableData> x_CreateTableData(TConstScopedObjects& objects)
    {
        return CIRef<ITableData>(new CTableDataAlnSummary(objects));
    }
};

void CAlnSummaryView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_Window);

    CGridWidget* widget = new CGridWidget(parent, wxID_ANY, wxDefaultPosition, wxSize(0, 0), wxBORDER_NONE);
    m_Window = widget;
    widget->AddListener(this, ePool_Parent);
}

///////////////////////////////////////////////////////////////////////////////
/// CAlnSummaryViewFactory
string  CAlnSummaryViewFactory::GetExtensionIdentifier() const
{
    static string aln_sid("aln_summary_view_factory");
    return aln_sid;
}


string CAlnSummaryViewFactory::GetExtensionLabel() const
{
    static string aln_slabel("Alignment Summary View Factory");
    return aln_slabel;
}


void CAlnSummaryViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("text_view.png"));
}


const CProjectViewTypeDescriptor& CAlnSummaryViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_AlnSummaryViewTypeDescr;
}


IView* CAlnSummaryViewFactory::CreateInstance() const
{
    return new CAlnSummaryView();
}

IView* CAlnSummaryViewFactory::CreateInstanceByFingerprint(const TFingerprint&) const
{
    return NULL;
}


bool CAlnSummaryViewFactory::IsCompatibleWith(const CObject& object, objects::CScope& scope)
{
    const type_info& type = typeid(object);
    if( typeid(CSeq_align) == type  ||
        typeid(CSeq_align_set) == type) {
            return true;
    }

    if( typeid(CSeq_annot) == type ){
        const CSeq_annot& annot = dynamic_cast<const CSeq_annot&>(object);
        if (annot.IsAlign()) {
            return true;
        }
    }

    return false;
}


int CAlnSummaryViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;

    for(  size_t i = 0;  i < objects.size();  i++)  {
        if (IsCompatibleWith(*objects[i].object, *objects[i].scope)) {
            found_good = true;
        }

        found_bad = true;
    }

    if (found_good)
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);

    return 0; // can show nothing
}

END_NCBI_SCOPE
