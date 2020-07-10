/*  $Id: group_alignments_tool_manager.cpp 34155 2015-11-13 17:39:19Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <serial/typeinfo.hpp>

#include <gui/packages/pkg_alignment/group_alignments_tool_manager.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/label.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <algo/sequence/align_group.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CGroupAlignmentsToolManager::CGroupAlignmentsToolManager()
:   CAlgoToolManagerBase("Group Alignments",
                         "",
                         "Group alignments according to sequence properties",
                         "Group alignments according to sequence properties",
                         "GROUP_ALIGNMENTS",
                         "Alignment Creation"),
    m_ParamsPanel(NULL)
{
}


string CGroupAlignmentsToolManager::GetExtensionIdentifier() const
{
    return "group_alignments_tool_manager";
}


string CGroupAlignmentsToolManager::GetExtensionLabel() const
{
    return "Group Alignments Tool";
}


void CGroupAlignmentsToolManager::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_ParamsPanel = NULL;
}


void CGroupAlignmentsToolManager::CleanUI()
{
    m_ParamsPanel = NULL;

    CAlgoToolManagerBase::CleanUI();
}


void CGroupAlignmentsToolManager::x_CreateParamsPanelIfNeeded()
{
    if(m_ParamsPanel == NULL)   {
        x_SelectCompatibleInputObjects();

        m_ParamsPanel = new CGroupAlignmentsParamsPanel();
        m_ParamsPanel->Hide(); // to reduce flicker
        m_ParamsPanel->Create(m_ParentWindow);
        m_ParamsPanel->SetParams(&m_Params, &m_Alignments);

        m_ParamsPanel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_ParamsPanel->LoadSettings();
    }
}


void CGroupAlignmentsToolManager::x_InitProjectParams()
{
    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    m_ProjectParams.SelectProjectByObjects(m_Params.m_Alignments, &*srv);
}


bool CGroupAlignmentsToolManager::x_ValidateParams()
{
    string err;
    if(m_Params.m_Alignments.size() == 0)  {
        err = "Please select at least one set of alignments!";
    }
    if( ! err.empty()) {
        NcbiErrorBox(err, m_Descriptor.GetLabel());
        return false;
    }
    return true;
}


/// select only Seq-aligns
void CGroupAlignmentsToolManager::x_SelectCompatibleInputObjects()
{
    m_Params.m_Alignments.clear();
    m_Alignments.clear();
    ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        ITERATE(TConstScopedObjects, it2, *it) {
            const CSeq_align* align = dynamic_cast<const CSeq_align*>(it2->object.GetPointerOrNull());
            if (align) m_Alignments.push_back(*it2);
        }
    }

    if (m_Alignments.empty()) {
        x_ConvertInputObjects(CSeq_align::GetTypeInfo(), m_Alignments);
    }
}


CAlgoToolManagerParamsPanel* CGroupAlignmentsToolManager::x_GetParamsPanel()
{
    return m_ParamsPanel;
}


IRegSettings* CGroupAlignmentsToolManager::x_GetParamsAsRegSetting()
{
    return &m_Params;
}


CDataLoadingAppJob* CGroupAlignmentsToolManager::x_CreateLoadingJob()
{
    CGroupAlignmentsJob* job = new CGroupAlignmentsJob(m_Params);
    return job;
}


///////////////////////////////////////////////////////////////////////////////
/// CGroupAlignmentsJob

CGroupAlignmentsJob::CGroupAlignmentsJob(const SGroupAlignmentsParams& params)
:   m_Params(params)
{
    CFastMutexGuard lock(m_Mutex);

    m_Descr = "Categorizing alignments"; //TODO
}


static string s_GetAnnotName(const CSeq_annot& annot)
{
    /// determine a base name for the annotation
    string annot_name;
    if (annot.IsSetDesc()) {
        ITERATE (CSeq_annot::TDesc::Tdata, iter, annot.GetDesc().Get()) {
            const CAnnotdesc& desc = **iter;
            if ( !desc.IsName() ) {
                continue;
            }

            annot_name = desc.GetName();
            break;
        }
    }

    return annot_name;
}


void CGroupAlignmentsJob::x_CreateProjectItems()
{
    TConstScopedObjects& aligns = m_Params.m_Alignments;

    ///
    /// assure we're all in one scope
    ///
    CRef<CScope> scope;
    {{
        ITERATE (TConstScopedObjects, iter, aligns) {
            if ( !scope ) {
                scope.Reset(const_cast<CScope*>(&*iter->scope));
            } else if (scope != &*iter->scope) {
                scope.Reset();
                NCBI_THROW(CException, eUnknown,
                       "All alignments must be within the same project");
            }
        }
    }}

    ///
    /// begin by seeding all our alignments in one annotation
    /// we will iteratively operate on this list of annotations
    /// performing all subdivision steps
    ///
    list< CRef<CSeq_annot> > annots;
    {{
        CRef<CSeq_annot> annot(new CSeq_annot);
        ITERATE (TConstScopedObjects, iter, aligns) {
            CObject* obj = const_cast<CObject*>(&*iter->object);
            CRef<CSeq_align> align(dynamic_cast<CSeq_align*>(obj));
            if (align) {
                annot->SetData().SetAlign().push_back(align);
            }
        }

        annots.push_back(annot);
    }}

    CAlignGroup annot_group;

    ///
    /// group by sequence IDs
    ///
    if (m_Params.m_GroupBySeqId) {
        list< CRef<CSeq_annot> > annots_out;
        ITERATE (list< CRef<CSeq_annot> >, iter, annots) {
            const CSeq_annot& annot = **iter;
            annot_group.GroupBySeqIds(annot.GetData().GetAlign(),
                                      annots_out,
                                      s_GetAnnotName(annot),
                                      *scope);
        }
        annots.swap(annots_out);
    }

    ///
    /// group by sequence strands
    ///
    if (m_Params.m_GroupByStrand) {
        list< CRef<CSeq_annot> > annots_out;
        ITERATE (list< CRef<CSeq_annot> >, iter, annots) {
            const CSeq_annot& annot = **iter;
            annot_group.GroupByStrand(annot.GetData().GetAlign(),
                                      annots_out,
                                      s_GetAnnotName(annot),
                                      *scope);
        }
        annots.swap(annots_out);
    }

    ///
    /// group by tax-ids
    /// there is a choice here; group by all tax-ids supercedes group by same tax-id
    ///
    if (m_Params.m_GroupByTaxId) {
        list< CRef<CSeq_annot> > annots_out;
        ITERATE (list< CRef<CSeq_annot> >, iter, annots) {
            const CSeq_annot& annot = **iter;
            annot_group.GroupByTaxIds(annot.GetData().GetAlign(),
                                      annots_out,
                                      s_GetAnnotName(annot),
                                      *scope);
        }
        annots.swap(annots_out);
    } else if (m_Params.m_GroupByLikeTaxId) {
        list< CRef<CSeq_annot> > annots_out;
        ITERATE (list< CRef<CSeq_annot> >, iter, annots) {
            const CSeq_annot& annot = **iter;
            annot_group.GroupByLikeTaxIds(annot.GetData().GetAlign(),
                                          annots_out,
                                          s_GetAnnotName(annot),
                                          *scope);
        }
        annots.swap(annots_out);
    }

    ///
    /// sequence type subdivisions
    ///

    if (m_Params.m_GroupByEST | m_Params.m_GroupByRefSeq |
        m_Params.m_GroupByPredictedRefSeq | m_Params.m_GroupByGenBank |
        m_Params.m_GroupByWGS | m_Params.m_GroupByHTGS |
        m_Params.m_GroupByPatent) {
        CAlignGroup::TSequenceFlags flags = 0;
        if (m_Params.m_GroupByEST) {
            flags |= CAlignGroup::fEST;
        }
        if (m_Params.m_GroupByRefSeq) {
            flags |= CAlignGroup::fRefSeq;
        }
        if (m_Params.m_GroupByPredictedRefSeq) {
            flags |= CAlignGroup::fRefSeqPredicted;
        }
        if (m_Params.m_GroupByGenBank) {
            flags |= CAlignGroup::fGB_EMBL_DDBJ;
        }
        if (m_Params.m_GroupByWGS) {
            flags |= CAlignGroup::fWGS;
        }
        if (m_Params.m_GroupByHTGS) {
            flags |= CAlignGroup::fHTGS;
        }
        if (m_Params.m_GroupByPatent) {
            flags |= CAlignGroup::fPatent;
        }

        list< CRef<CSeq_annot> > annots_out;
        ITERATE (list< CRef<CSeq_annot> >, iter, annots) {
            const CSeq_annot& annot = **iter;
            annot_group.GroupBySequenceType(annot.GetData().GetAlign(),
                                            annots_out,
                                            s_GetAnnotName(annot),
                                            *scope,
                                            flags);
        }
        annots.swap(annots_out);
    }

    ///
    /// final packaging and reporting
    ///
    if (annots.size()) {
        NON_CONST_ITERATE (list< CRef<CSeq_annot> >, iter, annots) {
            CSeq_annot& annot = **iter;
            annot.SetCreateDate(CTime(CTime::eCurrent));

            /// encode the name correctly
            /// we previously used the 'name' not for a temporary computation
            /// we make this the real 'name' that the object manager will understand
            string name;
            string tag = annot.GetName();
            annot.ResetName();
            CLabel::GetLabel(annot, &name, CLabel::eDefault, &*scope);
            if ( !name.empty() ) {
                name += ": ";
            }
            name += tag;
            annot.SetNameDesc(name);

            /// now create a Project Item for the data
            CRef<CProjectItem> item(new CProjectItem());
            item->SetItem().SetAnnot(annot);
            item->SetLabel(name);

            AddProjectItem(*item);
        }
    }
}


END_NCBI_SCOPE

