#ifndef GUI_CORE___PRJ_HELPER__HPP
#define GUI_CORE___PRJ_HELPER__HPP

/*  $Id: prj_helper.hpp 33712 2015-09-09 16:18:24Z katargir $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:  helper for adding items to a GBench project
 *
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <connect/ncbi_types.h>

#include <algo/blast/api/remote_blast.hpp>

BEGIN_NCBI_SCOPE

class CDataLoadingAppJob;

class NCBI_GUICORE_EXPORT  CGBProjectHelper
{
public:
    // BLAST RID statistics collected 
    struct SRIDStatInfo
    {
        string  m_QueryId;
        TSeqPos m_TotalBeg, m_TotalEnd, m_QueryBeg, m_QueryEnd;
        bool    m_QueryRangeSet;
    };

    // creates BLAST result project items from a given RemoteBlast and results set
    // (both must be already loaded)
    // if the results contain any generally unresolvable/local sequences, 
    // adds the sequences to the ProjectItems
    // mostly based on code borrowed from CDLGbprjCreator::CreateProject(rid)
    static void AddProjectItemsFromRID(  // in:
                                       CRef<objects::CScope> scope,
                                       CSeq_align_set& results,
                                       bool isFindComp,
						               CRef<blast::CRemoteBlast> RemoteBlast,
                                       const set<string>& filter,
                                        // out:
                                       CDataLoadingAppJob& job,
                                       SRIDStatInfo& RIDStatInfo);
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_CORE___PRJ_HELPER__HPP
