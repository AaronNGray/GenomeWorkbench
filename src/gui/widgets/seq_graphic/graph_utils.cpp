/*  $Id: graph_utils.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Liangshou Wu
 *
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/graph_utils.hpp>
#include <gui/objutils/utils.hpp>

#include <objects/seqloc/Seq_loc.hpp>

#include <objmgr/graph_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/objmgr_exception.hpp>
#include <objmgr/impl/tse_info.hpp>
#include <objmgr/impl/data_source.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

int CGraphUtils::GetNearestLevel(const set<int>& levels, double zoom)
{
    int zoom_int = (int)zoom;
    set<int>::const_iterator end_it = levels.end();
    set<int>::const_iterator pre_iter = end_it;
    set<int>::const_iterator curr_iter = levels.begin();
    while (curr_iter != end_it  &&  *curr_iter <= zoom_int) {
        pre_iter = curr_iter;
        ++curr_iter;
    }

    if (pre_iter != end_it)
        return *pre_iter;

    return 0;
}

void CGraphUtils::CalcGraphLevels(const string& annotName, objects::CScope& scope, const objects::CSeq_loc& loc, set<int>& levels)
{
    CRegistryReadView view = CSeqUtils::GetSelectorRegistry();
    int max_search_segs = CSeqUtils::GetMaxSearchSegments(view);
		
    SAnnotSelector sel;
    sel.SetResolveAll();
    sel.SetAdaptiveDepth();
    sel.IncludeNamedAnnotAccession(annotName, -1);
    sel.SetCollectNames();
    if (max_search_segs > 0) {
        sel.SetMaxSearchSegments((SAnnotSelector::TMaxSearchSegments) max_search_segs);
        sel.SetMaxSearchSegmentsAction(SAnnotSelector::eMaxSearchSegmentsThrow);
        sel.SetMaxSearchTime((float)CSeqUtils::GetMaxSearchTime(view));
	}

    try {
        CGraph_CI it(scope, loc, sel);

        ITERATE(CGraph_CI::TAnnotNames, i, it.GetAnnotNames()) {
            if (i->IsNamed()) {
                int level;
                if (ExtractZoomLevel(i->GetName(), NULL, &level)) {
                    levels.insert(level);
                }
            }
        }
    }
    catch (const CAnnotSearchLimitException& err) 
    {
        switch(err.GetErrCode()) {
            case CAnnotSearchLimitException::eTimeLimitExceded:
                NCBI_USER_THROW("Search segments time limit exceeded");
            case CAnnotSearchLimitException::eSegmentsLimitExceded:
                NCBI_USER_THROW("Search segments limit exceeded");
            default:
                LOG_POST(Error << "CGraphUtils::CalcGraphLevels: loading annotation failed: " << err);
            
        }
    }
    catch (const CException& err) {
        LOG_POST(Error << "CGraphUtils::CalcGraphLevels: loading annotation failed: " << err);
    }

    if (levels.empty()) {
        return;
    }

    /// Temporary solution to ignore all NAAs from SADB but still
    /// use high level graphs for VDB graphs because the high 
    /// level coverage graphs for NAAs from SADB are incorrect
    /// currently. We need to remove the work-around once the
    /// data in SADB is corrected.
    CRef<CObjectManager> om = CObjectManager::GetInstance();
    CDataLoader* gbloader = om->FindDataLoader("GBLOADER");
    if (!gbloader) {
        return;
    }

    int highest_zoom = *levels.rbegin();
    sel.IncludeNamedAnnotAccession(annotName, highest_zoom);
    sel.ExcludeUnnamedAnnots();
    sel.AddNamedAnnots(CombineWithZoomLevel(annotName, highest_zoom));
    sel.SetMaxSize(1);
    sel.SetCollectNames(false);

    CGraph_CI it2(scope, loc, sel);
    if (it2) {
        CTSE_Handle tse = it2.GetAnnot().GetTSE_Handle();
        if (tse.x_GetTSE_Info().GetDataSource().GetDataLoader() == gbloader) {
            // exclude SADB zoom graphs
            levels.clear();
        }
    }
}

END_NCBI_SCOPE
