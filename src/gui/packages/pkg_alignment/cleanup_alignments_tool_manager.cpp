/*  $Id: cleanup_alignments_tool_manager.cpp 37257 2016-12-20 19:24:55Z katargir $
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
#include <gui/packages/pkg_alignment/cleanup_alignments_tool_manager.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/label.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <algo/sequence/align_group.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/seqalign/Dense_seg.hpp>

#include <algo/sequence/align_cleanup.hpp>

#include <algo/align/util/blast_tabular.hpp>
#include <algo/align/util/hit_filter.hpp>
#include <algo/align/util/score_builder.hpp>
#include <algo/sequence/align_group.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CCleanupAlignmentsToolManager::CCleanupAlignmentsToolManager()
:   CAlgoToolManagerBase("Clean Up Alignments",
                         "",
                         "Remove redundant elements from alignments",
                         "Merge and remove redundant elements from alignments to "
                         "create a cleaner diagonalized representation",
                         "CLEAN_ALIGNMENTS",
                         "Alignment Creation"),
    m_ParamsPanel(NULL)
{
}


string CCleanupAlignmentsToolManager::GetExtensionIdentifier() const
{
    return "cleanup_alignments_tool_manager";
}


string CCleanupAlignmentsToolManager::GetExtensionLabel() const
{
    return "Clean Up Alignments Tool";
}


void CCleanupAlignmentsToolManager::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_ParamsPanel = NULL;
}


void CCleanupAlignmentsToolManager::CleanUI()
{
    m_ParamsPanel = NULL;

    CAlgoToolManagerBase::CleanUI();
}


void CCleanupAlignmentsToolManager::x_CreateParamsPanelIfNeeded()
{
    if(m_ParamsPanel == NULL)   {
        x_SelectCompatibleInputObjects();

        if( x_AreSelfAlignmentsOnly() ){
            m_Params.m_AlnMgr_PreserveRows = true;
        }

        m_ParamsPanel = new CCleanupAlignmentsParamsPanel();
        m_ParamsPanel->Hide(); // to reduce flicker
        m_ParamsPanel->Create(m_ParentWindow);
        m_ParamsPanel->SetParams(&m_Params, &m_Alignments);

        m_ParamsPanel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_ParamsPanel->LoadSettings();
    }
}


bool CCleanupAlignmentsToolManager::x_ValidateParams()
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
void CCleanupAlignmentsToolManager::x_SelectCompatibleInputObjects()
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

bool CCleanupAlignmentsToolManager::x_AreSelfAlignmentsOnly() const
{

    ITERATE( TConstScopedObjects, obj_ir, m_Alignments ){

        const CSeq_align* aln = dynamic_cast<const CSeq_align*>( obj_ir->object.GetPointer() );
        if( aln ){
            int num_seqs = aln->CheckNumRows();
            if( num_seqs <= 0 ){
                continue;
            }

            set<CSeq_id_Handle> idh_set;
            for( int q = 0; q < num_seqs; q++ ){
                const CSeq_id& seq_id = aln->GetSeq_id( q );
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( seq_id );
                idh_set.insert( idh );
            }

            if( idh_set.size() > 1 ){
                return false;
            }
        }
    }

    return true;
}

CAlgoToolManagerParamsPanel* CCleanupAlignmentsToolManager::x_GetParamsPanel()
{
    return m_ParamsPanel;
}


IRegSettings* CCleanupAlignmentsToolManager::x_GetParamsAsRegSetting()
{
    return &m_Params;
}


CDataLoadingAppJob* CCleanupAlignmentsToolManager::x_CreateLoadingJob()
{
    CCleanupAlignmentsJob* job = new CCleanupAlignmentsJob(m_Params);
    return job;
}




///////////////////////////////////////////////////////////////////////////////
/// CCleanupAlignmentsJob

CCleanupAlignmentsJob::CCleanupAlignmentsJob(const SCleanupAlignmentsParams& params)
:   m_Params(params)
{
    m_Descr = "Cleaning alignments"; //TODO
}

/*
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
 */

void CCleanupAlignmentsJob::x_CreateProjectItems()
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
                NCBI_USER_THROW( "All alignments must be within the same project" );
            }
        }
    }}

    ///
    /// meat goes here
    ///
    TConstScopedObjects aligns_out;

    switch (m_Params.m_Algo) {
    case SCleanupAlignmentsParams::eAlignmentManager:
        ///
        /// alignment manager version is encapsulated in CAlignCleanup
        ///
        {{
            CAlignCleanup::TConstAligns aligns_in;
            CAlignCleanup::TAligns      aligns_out_tmp;

            ITERATE (TConstScopedObjects, iter, aligns) {
                CConstRef<CSeq_align> al
                    (dynamic_cast<const CSeq_align*>(&*iter->object));
                if (al) {
                    aligns_in.push_back(al);
                }
            }

            CAlignCleanup cleanup(*scope);
            cleanup.SortInputsByScore(m_Params.m_AlnMgr_Sort);
            //cleanup.AllowTranslocations(m_Params.m_AlnMgr_Transloc);
            cleanup.PreserveRows(m_Params.m_AlnMgr_PreserveRows);
            cleanup.FillUnaligned(m_Params.m_AlnMgr_FillUnaligned);

            cleanup.Cleanup(aligns_in, aligns_out_tmp,
                            CAlignCleanup::eAnchoredAlign);
            ITERATE (CAlignCleanup::TAligns, iter, aligns_out_tmp) {
                aligns_out.push_back(SConstScopedObject(*iter, scope));
            }
        }}
        break;

    case SCleanupAlignmentsParams::eHitFilter:
        {{
            ///
            /// compose a list of hit refs
            ///

            TSeqPos min_len  = m_Params.m_HitFilter_MinLength;
            double  min_idty = m_Params.m_HitFilter_MinIdentity;

            typedef CBlastTabular          THit;
            typedef CRef<THit>             THitRef;
            typedef vector<THitRef>        THitRefs;

            THitRefs hitrefs;
            ITERATE (TConstScopedObjects, iter, aligns) {
                CConstRef<CSeq_align> al
                    (dynamic_cast<const CSeq_align*>(&*iter->object));
                if (al) {
                    if (al->GetSegs().IsDisc()) {
                        const CSeq_align_set::Tdata &sas =
                            al->GetSegs().GetDisc().Get();
                        ITERATE(CSeq_align_set::Tdata, sa_iter, sas) {
                            CRef<CBlastTabular> hitref (new CBlastTabular(**sa_iter, true));
                            if(hitref->GetIdentity() >= min_idty  &&
                                hitref->GetLength() >= min_len)
                            {
                                if(hitref->GetQueryStrand() == false) {
                                    hitref->FlipStrands();
                                }
                                hitrefs.push_back(hitref);
                            }
                        }
                    } else {
                        CRef<CBlastTabular> hitref (new CBlastTabular(*al, true));
                            if(hitref->GetIdentity() >= min_idty  &&
                                hitref->GetLength() >= min_len)
                        {
                            if(hitref->GetQueryStrand() == false) {
                                hitref->FlipStrands();
                            }
                            hitrefs.push_back(hitref);
                        }
                    }
                }
            }

            if ( !hitrefs.size() ) {
                break;
            }

            ///
            /// run the greedy reconciliation step
            ///
            THitRefs hits_new;
            CHitFilter<THit>::s_RunGreedy(hitrefs.begin(), hitrefs.end(), &hits_new,
                                          min_len, min_idty);
            hitrefs.erase(remove_if(hitrefs.begin(), hitrefs.end(),
                                    CHitFilter<THit>::s_PNullRef), hitrefs.end());
            copy(hits_new.begin(), hits_new.end(), back_inserter(hitrefs));

            ///
            /// format our results as a seq-annot
            ///
            CRef<CSeq_align> seq_align_disc (new CSeq_align);
            seq_align_disc->SetType(CSeq_align::eType_disc);
            CSeq_align_set::Tdata& align_list = seq_align_disc->SetSegs().SetDisc().Set();
            aligns_out.push_back(SConstScopedObject(seq_align_disc, scope));

            ITERATE(THitRefs, ii, hitrefs) {

                const THit& h = **ii;

                CRef<CDense_seg> ds (new CDense_seg);
                const ENa_strand query_strand = h.GetQueryStrand()? eNa_strand_plus:
                    eNa_strand_minus;
                const ENa_strand subj_strand = h.GetSubjStrand()? eNa_strand_plus:
                    eNa_strand_minus;
                const string xcript (CAlignShadow::s_RunLengthDecode(h.GetTranscript()));

                ds->FromTranscript(h.GetQueryStart(), query_strand,
                                   h.GetSubjStart(), subj_strand,
                                   xcript);

                if(query_strand == eNa_strand_plus  && subj_strand == eNa_strand_plus) {
                    ds->ResetStrands();
                }

                vector< CRef< CSeq_id > > &ids = ds->SetIds();
                for(Uint1 where = 0; where < 2; ++where) {

                    CRef<CSeq_id> id (new CSeq_id);
                    id->Assign(*h.GetId(where));
                    ids.push_back(id);
                }

                CDense_seg::TScores& scores = ds->SetScores();
                CRef<CScore> score (new CScore);
                score->SetValue().SetReal(h.GetScore());
                scores.push_back(score);

                CRef<CSeq_align> seq_align (new CSeq_align);
                seq_align->SetType(CSeq_align::eType_disc);
                seq_align->SetSegs().SetDenseg(*ds);
                align_list.push_back(seq_align);
            }

            ///
            /// add some scores to the alignments
            ///
            NON_CONST_ITERATE (CSeq_align_set::Tdata, iter, align_list) {
                try {
                    CScoreBuilder builder;
                    builder.AddScore(*scope, **iter,
                        CSeq_align::eScore_PercentIdentity);
                } catch (CException&) {
                }
            }

        }}
        break;
    }

    //
    // final packaging and reporting
    //
    if (aligns_out.size()) {

        CAlignGroup align_group_sorter;
        CAlignGroup::TAlignList aligns_out_tmp;

        // container format conversion...
        NON_CONST_ITERATE (TConstScopedObjects, iter, aligns_out) {
            CRef<CSeq_align> al
                (const_cast<CSeq_align*>
                 (dynamic_cast<const CSeq_align*>
                  (&*iter->object)
                  ));
            aligns_out_tmp.push_back(al);
        }

        CAlignGroup::TAnnotList annot_list;
        string annot_base_name("Cleaned Alignment: ");

        align_group_sorter.GroupByStrand(aligns_out_tmp,
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

            string name("Cleaned Alignment: ");
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
}



END_NCBI_SCOPE

