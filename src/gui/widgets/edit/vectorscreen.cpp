/* $Id: vectorscreen.cpp 44526 2020-01-13 15:10:28Z choi $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Yoon Choi
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>

#include <objtools/data_loaders/blastdb/bdbloader.hpp>
#include <objtools/align_format/vectorscreen.hpp>

#include <algo/blast/api/blast_options_handle.hpp>
#include <algo/blast/api/blast_nucl_options.hpp>
#include <algo/blast/api/local_blast.hpp>
#include <algo/blast/api/objmgr_query_data.hpp>

#include <algo/blast/format/vecscreen_run.hpp>

#include <gui/widgets/edit/vectorscreen.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <wx/app.h>
#include <wx/dir.h>
#include <wx/filename.h>

#include <sstream>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(blast);


// Relative path to blast databases
#if defined(NCBI_OS_MSWIN)
const string kUniVecDatabase = "screen\\UniVec_Core";
#else
const string kUniVecDatabase = "screen/UniVec_Core";
#endif



/*!
 * CVectorScreen constructors
 */

CVectorScreen::CVectorScreen()
{
}


/*!
 * CVectorScreen destructor
 */

CVectorScreen::~CVectorScreen()
{
}

bool CVectorScreen::IsDbAvailable()
{
    CSearchDatabase db(kUniVecDatabase, CSearchDatabase::eBlastDbIsNucleotide);
    CRef<CSeqDB> refdb = db.GetSeqDb();
    return refdb.NotNull();
}

void CVectorScreen::Run(objects::CSeq_entry_Handle seh,
                        wxGauge& progress)
{
    // Our toplevel blob 
    m_TopSeqEntry = seh;

    // Set up objmgr and scope
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    if (!objmgr) {
        throw std::runtime_error("Could not initialize object manager");
    }
    CRef<CScope> scope( new CScope(*objmgr) );

    // Add UniVec database to scope
    CDataLoader* blast_loader =
        CBlastDbDataLoader::RegisterInObjectManager(
            *objmgr, kUniVecDatabase, CBlastDbDataLoader::eNucleotide, true,
            CObjectManager::eNonDefault,
            CObjectManager::kPriority_NotSet).GetLoader();
    scope->AddDataLoader(blast_loader->GetName());

    // Add toplevel blob to our scope 
    scope->AddScope(m_TopSeqEntry.GetScope());

    // Iterate over all nuc bioseqs and collect Seq_locs
    typedef vector< CRef<CSeq_loc> > TSearchLocVec;
    m_vecSearchLocs.clear();
    CBioseq_CI bioseq_ci(m_TopSeqEntry, CSeq_inst::eMol_na);
    for( ; bioseq_ci; ++bioseq_ci ) {
        CBioseq_Handle bioseq_handle = *bioseq_ci;

        // Select an appropriate seqid 
        CRef<CSeq_id> selected_seq_id = x_SelectSeqId(bioseq_handle);
        ASSERT( !selected_seq_id.IsNull() );

        // Search on the "whole" bioseq
        CRef<CSeq_loc> search_loc( new CSeq_loc );
        search_loc->SetWhole(*selected_seq_id);

        m_vecSearchLocs.push_back( search_loc );
    }

    // Run blast program on UniVec database and post-process via vecscreen
    m_vecscreen_summary.clear();
    m_seqidSummary.clear();
    CRef<CBlastOptionsHandle> 
        opts(CBlastOptionsFactory::CreateTask("vecscreen"));
    opts->SetHitlistSize(100); 

    progress.SetRange(m_vecSearchLocs.size());
    int progressCount = 0;

    CSearchDatabase db(kUniVecDatabase, CSearchDatabase::eBlastDbIsNucleotide);
    NON_CONST_ITERATE( TSearchLocVec, searchLocIter, m_vecSearchLocs ) {
        CRef<CSeq_loc> search_loc = *searchLocIter;

        // Blast against UniVec 
        TSeqLocVector query;
        SSeqLoc ssl(*search_loc, *scope);
        query.push_back(ssl);
        CRef<IQueryFactory> query_factory(new CObjMgr_QueryFactory(query));
        
        CLocalBlast blaster(query_factory, opts, db);
        CRef<CSearchResultSet> results;
        results = blaster.Run();

        // Vecscreen post-process
        TSeqPos seqlen = sequence::GetLength(*search_loc, scope);
        CRef< CSeq_align_set > seq_align_set;
        CVecscreen vecscreen(*((*results)[0].GetSeqAlign()), seqlen);
        seq_align_set = vecscreen.ProcessSeqAlign();

        const list<CVecscreen::AlnInfo*>* aln_info = 
            vecscreen.GetAlnInfoList();

        // Alignment graphic view - html output references gif files
        wxString resPath = CSysPath::GetResourcePath();
        vecscreen.SetImagePath( resPath.ToStdString() + string("/") );
        stringstream ssAlignGraphic;
        ssAlignGraphic << "<HTML>"
                       << "<TITLE>BLAST Search Results</TITLE>"
                       << "<BODY BGCOLOR=\"#FFFFFF\" LINK=\"#0000FF\" VLINK=\"#660099\" ALINK=\"#660099\" >"
                       << "<PRE>";
        vecscreen.VecscreenPrint(ssAlignGraphic);
        ssAlignGraphic << "</PRE>"
                       << "</BODY>"
                       << "</HTML>";

        // Produce summary  
        list<CVecscreen::AlnInfo*>::const_iterator itr = aln_info->begin();
        for ( ; itr != aln_info->end(); ++itr )
        {
            if ((*itr)->type == CVecscreen::eNoMatch)
                continue;

            // Fill in standard summary
            CVecscreenRun::SVecscreenSummary summary;
            summary.seqid = search_loc->GetId();
            summary.range = (*itr)->range;
            summary.match_type = CVecscreen::GetStrengthString((*itr)->type);
            m_vecscreen_summary.push_back(summary);

            // Fill in summary organized around seqid
            SRangeMatch rangematch;
            rangematch.m_range = summary.range;
            rangematch.m_match_type = summary.match_type;
            vector<SRangeMatch> rangematchArr;
            rangematchArr.push_back(rangematch);

            SVecscreenResult vecres(summary.seqid->AsFastaString(),
                                    rangematchArr, seqlen);

            pair<TVecscreenSummaryBySeqid::iterator, bool> ins_res = 
                m_seqidSummary.insert(TVecscreenSummaryBySeqid::value_type(summary.seqid,
                                                                           vecres));
            if ( !ins_res.second )
            {
                // If insert failed, add res to existing container.
                // (Add any "suspect" to the front of the container.)
                SVecscreenResult& vecres = ins_res.first->second;
                if ( NStr::FindNoCase(rangematch.m_match_type, "suspect") != NPOS ) 
                {
                    vecres.m_arrRangeMatch.insert(vecres.m_arrRangeMatch.begin(), rangematch);
                }
                else
                {
                    vecres.m_arrRangeMatch.push_back(rangematch);
                }
            }
        }

        progress.SetValue(++progressCount);
        wxTheApp->Yield();
    }
}


const vector<CVecscreenRun::SVecscreenSummary>& 
CVectorScreen::GetVecscreenSummary() const
{
    return m_vecscreen_summary;
}


const TVecscreenSummaryBySeqid& 
CVectorScreen::GetVecscreenSummaryBySeqid() const
{
    return m_seqidSummary;
}


CRef<CSeq_id> CVectorScreen::x_SelectSeqId(CBioseq_Handle bioseq_handle)
{
    if ( !bioseq_handle.CanGetId() ) {
        return CRef<CSeq_id>();
    }

    // Try accession
    CRef<CSeq_id> seqid = x_FindAccession(bioseq_handle);
    if ( seqid )
    {
        return seqid;
    }

    // Try localid
    seqid = x_FindLocalId(bioseq_handle);
    if ( seqid )
    {
        return seqid;
    }

    // Try type general 
    seqid = x_FindTypeGeneral(bioseq_handle);
    if ( seqid )
    {
        return seqid;
    }

    return CRef<CSeq_id>();
}


CRef<CSeq_id> CVectorScreen::x_FindAccession(CBioseq_Handle bioseq_handle)
{
    const CBioseq_Handle::TId& ids = bioseq_handle.GetId();
    ITERATE( CBioseq_Handle::TId, id_itr, ids ) {
        const CSeq_id_Handle& id_handle = *id_itr;
        CConstRef<CSeq_id> id = id_handle.GetSeqIdOrNull();

        if ( !id ) 
        {
            continue;
        }

        const CTextseq_id* textseq_id = id->GetTextseq_Id();
        if ( textseq_id != NULL && textseq_id->CanGetAccession() ) {
            // Found accession
            return CRef<CSeq_id>(new CSeq_id(textseq_id->GetAccession()));
        }
    }

    return CRef<CSeq_id>();
}


CRef<CSeq_id> CVectorScreen::x_FindTypeGeneral(CBioseq_Handle bioseq_handle)
{
    const CBioseq_Handle::TId& ids = bioseq_handle.GetId();
    ITERATE( CBioseq_Handle::TId, id_itr, ids ) {
        const CSeq_id_Handle& id_handle = *id_itr;
        CConstRef<CSeq_id> id = id_handle.GetSeqIdOrNull();

        if ( !id ) 
        {
            continue;
        }

        if ( id->IsGeneral() )
        {
            // Found type general
            return CRef<CSeq_id>(new CSeq_id(id->GetGeneral()));
        }
    }

    return CRef<CSeq_id>();
}


CRef<CSeq_id> CVectorScreen::x_FindLocalId(CBioseq_Handle bioseq_handle)
{
    const CBioseq_Handle::TId& ids = bioseq_handle.GetId();
    ITERATE( CBioseq_Handle::TId, id_itr, ids ) {
        const CSeq_id_Handle& id_handle = *id_itr;
        CConstRef<CSeq_id> id = id_handle.GetSeqIdOrNull();

        if ( !id ) 
        {
            continue;
        }

        if ( id->IsLocal() )
        {
            // Found localid
            const CObject_id& localid = id->GetLocal();
            if ( localid.IsId() )
            {
                return CRef<CSeq_id>(new CSeq_id(CSeq_id_Base::e_Local, 
                                                 localid.GetId()));
            }
            else
            if ( localid.IsStr() )
            {
                return CRef<CSeq_id>(new CSeq_id(CSeq_id_Base::e_Local, 
                                                 localid.GetStr()));
            }
        }
    }

    return CRef<CSeq_id>();
}


END_NCBI_SCOPE

