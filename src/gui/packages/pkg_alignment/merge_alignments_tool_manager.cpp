/*  $Id: merge_alignments_tool_manager.cpp 34155 2015-11-13 17:39:19Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/merge_alignments_tool_manager.hpp>

#include <gui/packages/pkg_alignment/merge_alignments_params_panel.hpp>

#include <gui/framework/app_job_task.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/label.hpp>

#include <objects/gbproj/ProjectItem.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Dense_seg.hpp>

#include <objtools/alnmgr/aln_container.hpp>
#include <objtools/alnmgr/aln_tests.hpp>
#include <objtools/alnmgr/aln_stats.hpp>
#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/aln_builders.hpp>
#include <objtools/alnmgr/aln_user_options.hpp>
#include <objtools/alnmgr/aln_generators.hpp>
#include <objtools/alnmgr/seqids_extractor.hpp>

#include <algo/sequence/align_group.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CMergeAlignmentsToolManager
CMergeAlignmentsToolManager::CMergeAlignmentsToolManager()
:   CAlgoToolManagerBase("Merge Alignments",
                         "",
                         "Combine alignments using many possible strategies",
                         "Create a pseudo-multiple alignment using many possible strategies",
                         "MERGE_ALIGNMENTS",
                         "Alignment Creation"),
    m_ParamsPanel(NULL)
{
}


string CMergeAlignmentsToolManager::GetExtensionIdentifier() const
{
    return "merge_alignments_tool_manager";
}


string CMergeAlignmentsToolManager::GetExtensionLabel() const
{
    return "Merge Alignments Tool";
}


void CMergeAlignmentsToolManager::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_ParamsPanel = NULL;
}


void CMergeAlignmentsToolManager::CleanUI()
{
    m_ParamsPanel = NULL;

    CAlgoToolManagerBase::CleanUI();
}


void CMergeAlignmentsToolManager::x_CreateParamsPanelIfNeeded()
{
    if(m_ParamsPanel == NULL)   {
        x_SelectCompatibleInputObjects();

        m_ParamsPanel = new CMergeAlignmentsParamsPanel();
        m_ParamsPanel->Hide(); // to reduce flicker
        m_ParamsPanel->Create(m_ParentWindow);
        m_ParamsPanel->SetParams(&m_Params, &m_Alignments);

        m_ParamsPanel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_ParamsPanel->LoadSettings();
    }
}


bool CMergeAlignmentsToolManager::x_ValidateParams()
{
    if( m_Params.m_Alignments.size() < 1 ){
        string err = "Please select at least one alignment!";

        NcbiErrorBox( err, m_Descriptor.GetLabel() );
        return false;
    }

    return true;
}


/// select only Seq-aligns
void CMergeAlignmentsToolManager::x_SelectCompatibleInputObjects()
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


CAlgoToolManagerParamsPanel* CMergeAlignmentsToolManager::x_GetParamsPanel()
{
    return m_ParamsPanel;
}


IRegSettings* CMergeAlignmentsToolManager::x_GetParamsAsRegSetting()
{
    return &m_Params;
}


CDataLoadingAppJob* CMergeAlignmentsToolManager::x_CreateLoadingJob()
{
    CMergeAlignmentsJob* job = new CMergeAlignmentsJob(m_Params);
    return job;
}


void CMergeAlignmentsToolManager::x_InitProjectParams()
{
    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    m_ProjectParams.SelectProjectByObjects(m_Params.m_Alignments, &*srv);
}


///////////////////////////////////////////////////////////////////////////////
/// CMergeAlignmentsJob
CMergeAlignmentsJob::CMergeAlignmentsJob(const SMergeAlignmentsParams& params)
:   m_Params(params)
{
    m_Descr = "Merging alignments"; //TODO
}


void CMergeAlignmentsJob::x_CreateProjectItems()
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
    /// do our work
    ///

    CAlnContainer aln_container;

    ///
    /// step 1: add to alignment container
    ///
    int count = 0;
    int count_invalid = 0;
    //bool all_pairwise = true;
    ITERATE (TConstScopedObjects, iter, aligns) {

        try {
            ++count;
            CConstRef<CSeq_align> aln(dynamic_cast<const CSeq_align*>(iter->object.GetPointer()));

            ///
            /// validation is optional!
            aln->Validate(true);

            // if (aln->GetSegs().IsDenseg()  &&
            //     aln->GetSegs().GetDenseg().GetDim() != 2) {
            //     all_pairwise = false;
            // }

            aln_container.insert(*aln);
        }
        catch (CException& e) {
            LOG_POST(Error
                << "CMergeAlignmentsJob::x_CreateProjectItems(): "
                << "failed to validate: " << e.GetMsg());
            ++count_invalid;
        }
    }

    if (count_invalid) {
        string msg;
        msg += NStr::IntToString(count_invalid);
        msg += "/";
        msg += NStr::IntToString(count);
        msg += " alignments failed validation.";
        if (count_invalid == count) {
            NCBI_THROW(CException, eUnknown, msg);
        } else {
            LOG_POST(Warning << msg);
        }
    }

    /// Types we use here:
    typedef CSeq_align::TDim TDim;

    /// Create a vector of seq-ids per seq-align
    TIdExtract id_extract;
    TAlnIdMap aln_id_map(id_extract, aln_container.size());
    size_t count_accepted = 0;
    ITERATE(CAlnContainer, aln_it, aln_container) {
        try {
            aln_id_map.push_back(**aln_it);
            ++count_accepted;
        }
        catch (CAlnException& e) {
            LOG_POST(Error
                << "CMergeAlignmentsJob::x_CreateProjectItems(): "
                << "failed to extract IDs: " << e.GetMsg());
        }
    }

    if (count_accepted != aln_container.size()) {
        if (count_accepted == 0) {
            NCBI_THROW(CException, eUnknown, 
                       "No valid alignments found");
            return;
        }

        LOG_POST(Warning
            << count_accepted << "/" << aln_container.size()
            << " alignments had no IDs to extract.");
    }


    ///
    /// gather statistics about our alignment
    ///
    TAlnStats aln_stats(aln_id_map);


    // auto-detect self-alignments
    // if the input set of sequences correspond to one and only one sequence,
    // force row preservation
    // bool preserve_rows = false;
    {{
         set<CSeq_id_Handle> ids;
         ITERATE (TAlnStats::TIdVec, i, aln_stats.GetIdVec()) {
             CSeq_id_Handle idh = CSeq_id_Handle::GetHandle((*i)->GetSeqId());
             ids.insert(idh);
         }
         // if (ids.size() == 1) {
         //     preserve_rows = true;
         // }
     }}

    CAlnUserOptions opts;


    /// always merge both directions
    opts.m_Direction = (m_Params.m_Direction == CAlnUserOptions::eReverse) ?
        CAlnUserOptions::eReverse : CAlnUserOptions::eBothDirections;

    ///
    /// create a set of anchored alignments
    ///
    TAnchoredAlnVec anchored_aln_vec;
    CreateAnchoredAlnVec(aln_stats, anchored_aln_vec, opts);

    opts.m_MergeAlgo = (m_Params.m_MergeAlgo == CAlnUserOptions::eQuerySeqMergeOnly) ?
        CAlnUserOptions::eQuerySeqMergeOnly : CAlnUserOptions::eMergeAllSeqs;

    CAlnUserOptions::TMergeFlags flags = m_Params.m_MergeFlags;

    opts.SetMergeFlags(flags, true);

    ///
    /// now, build
    ///
    CAnchoredAln out_anchored_aln;
    BuildAln(anchored_aln_vec, out_anchored_aln, opts);

    vector< CRef<CSeq_align> > ds_aligns;
    CreateSeqAlignFromEachPairwiseAln
        (out_anchored_aln.GetPairwiseAlns(), out_anchored_aln.GetAnchorRow(),
         ds_aligns, CSeq_align::TSegs::e_Denseg);

    typedef list< CRef<CSeq_align> > TAligns;
    TAligns aligns_out;

    NON_CONST_ITERATE (vector< CRef<CSeq_align> >, it, ds_aligns) {
        (*it)->SetType(CSeq_align::eType_partial);
        aligns_out.push_back(*it);
    }

    /// fill unaligned regions
    if (m_Params.m_FillUnaligned) {
        NON_CONST_ITERATE (TAligns, align_iter, aligns_out) {
            CRef<CDense_seg> ds = (*align_iter)->SetSegs().SetDenseg().FillUnaligned();
            (*align_iter)->SetSegs().SetDenseg(*ds);
        }
    }

    if (aligns_out.size() == 0)
        return;

    string annot_base_name("Merged Alignment: ");
    CAlignGroup::TAnnotList annot_list;

    CAlignGroup align_group_sorter;
    align_group_sorter.GroupByStrand(aligns_out,
                                     annot_list,
                                     annot_base_name,
                                     *scope);

    // now create a Project Item for the data

    ITERATE(CAlignGroup::TAnnotList, iter, annot_list) {
        CRef<objects::CSeq_annot> annot = *iter;

        annot->SetCreateDate(CTime(CTime::eCurrent));

        // encode the name correctly
        // we previously used the 'name' not for a temporary computation
        // we make this the real 'name' that the object manager will understand

        string name("Merged Alignment: ");
        CLabel::GetLabel(*annot, &name, CLabel::eDefault, &*scope);
        if ( !name.empty() ) {
            annot->SetNameDesc(name);
            annot->SetTitleDesc(name);
        }

        CRef<CProjectItem> pitem(new CProjectItem());

        pitem->SetItem().SetAnnot(*annot);
        pitem->SetLabel(name);

        AddProjectItem(*pitem);
    }
}


END_NCBI_SCOPE
