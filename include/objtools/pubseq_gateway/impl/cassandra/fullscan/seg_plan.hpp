/*****************************************************************************
 *  $Id: seg_plan.hpp 581891 2019-03-06 19:46:57Z dmitrie1 $
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
 *  Db Cassandra: class generating execution plans for cassandra table segmented scans.
 *
 */

#ifndef OBJTOOLS__PUBSEQ_GATEWAY__IMPL__CASSANDRA__FULLSCAN__SEG_PLAN_HPP
#define OBJTOOLS__PUBSEQ_GATEWAY__IMPL__CASSANDRA__FULLSCAN__SEG_PLAN_HPP

#include <objtools/pubseq_gateway/impl/cassandra/IdCassScope.hpp>
#include <objtools/pubseq_gateway/impl/cassandra/fullscan/plan.hpp>

BEGIN_IDBLOB_SCOPE
USING_NCBI_SCOPE;

class CCassandraSegscanPlan: public CCassandraFullscanPlan
{
 public:
    CCassandraSegscanPlan();
    CCassandraSegscanPlan& SetSegment(pair<size_t, size_t> segment);

    virtual void Generate() override;
 private:
    pair<size_t, size_t> m_Segment;
};

END_IDBLOB_SCOPE

#endif
