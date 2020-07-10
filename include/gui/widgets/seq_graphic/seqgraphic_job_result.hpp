#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SG_JOB_RESULT__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SG_JOB_RESULT__HPP

/*  $Id: seqgraphic_job_result.hpp 39782 2017-11-03 14:53:25Z shkeda $
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
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/seq_graphic/feature_glyph.hpp>
#include <objmgr/seq_annot_handle.hpp>

BEGIN_NCBI_SCOPE


static const int kInvalidJobToken = -1;


///////////////////////////////////////////////////////////////////////////////
/// CJobResultBase -- the basic job result class holding a token.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CJobResultBase : public CObject
{
public:
    typedef int     TJobToken;

    /// the token is recognizable by the job listener.
    /// It may be used when a job listener deals with more than one kind
    /// of jobs.
    TJobToken               m_Token;
};


///////////////////////////////////////////////////////////////////////////////
/// CSGJobResult -- the data structure holding the seqgraphic job results.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGJobResult : public CJobResultBase
{
public:
    CSGJobResult(){}

    virtual ~CSGJobResult()  {};

    size_t Size() { return m_ObjectList.size(); }

    /// the loaded data stored in the form of CSeqGlyph.
    CSeqGlyph::TObjects m_ObjectList;

    /// additional generated layout.
    /// such as hist, smear bar and graph.
    CRef<CSeqGlyph>     m_ExtraObj;

    /// additional comment glyphs.
    /// used for showing labels at an arbitrary position.
    CSeqGlyph::TObjects m_CommentObjs;

    /// Owner of the job result.
    /// Most of the time, the job owner is the listener of the job,
    /// usually the layout track.  So for that case, m_Owner may be void.
    /// For other cases where the job owner is a sub-glyph in a track,
    /// then m_Owner must be specified.
    CRef<CSeqGlyph>     m_Owner;

    /// data description.
    /// optional.
    string              m_Desc;
};

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGAlignJobResult : public CSGJobResult
{
public:
    CSGAlignJobResult() {}
    objects::CTSE_Handle m_DataHandle;
};

struct SFeatBatchJobRequest
{
    SFeatBatchJobRequest(CFeatGlyph* feat,
        const objects::SAnnotSelector& sel)
        : m_Feat(feat)
        , m_Sel(sel)
    {
    }

    /// the layout feature contains both target sequence and top sequence.
    CRef<CFeatGlyph> m_Feat;

    objects::SAnnotSelector m_Sel;
};

typedef vector<SFeatBatchJobRequest> TFeatBatchJobRequests;

///////////////////////////////////////////////////////////////////////////////
/// CBatchJobResult -- the data structure holding the seqgraphic job results
/// for a batch job processing.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CBatchJobResult : public CJobResultBase
{
public:
    typedef vector< CRef<CSGJobResult> > TResults;

    TResults m_Results;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SG_JOB_RESULT__HPP
