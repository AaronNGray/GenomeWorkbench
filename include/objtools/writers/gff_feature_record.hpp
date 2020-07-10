/*  $Id: gff_feature_record.hpp 564651 2018-05-31 16:44:56Z ludwigf $
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
 * Author: Frank Ludwig
 *
 * File Description:
 *   GFF3 transient data structures
 *
 */

#ifndef OBJTOOLS_WRITERS___GFF_FEATURE_RECORD__HPP
#define OBJTOOLS_WRITERS___GFF_FEATURE_RECORD__HPP

#include <objtools/writers/gff_base_record.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ============================================================================
class NCBI_XOBJWRITE_EXPORT CGffFeatureRecord
//  ============================================================================
    : public CGffBaseRecord
{
public:
    CGffFeatureRecord( 
        const string& id="");
    CGffFeatureRecord(
        const CGffFeatureRecord& other);
    virtual ~CGffFeatureRecord();

    void InitLocation(
        const CSeq_loc&);
    void SetLocation(
        const CSeq_interval&);
    void SetEndpoints(
        unsigned int start,
        unsigned int stop,
        ENa_strand strand);

    const CSeq_loc& Location() const {
        return *m_pLoc;
    };
protected:
};

//  ============================================================================
class NCBI_XOBJWRITE_EXPORT CGffSourceRecord
//  ============================================================================
    : public CGffBaseRecord
{
public:
    CGffSourceRecord( 
        const string& id=""): CGffBaseRecord(id) {};
};

//  ============================================================================
class NCBI_XOBJWRITE_EXPORT CGffAlignRecord
//  ============================================================================
    : public CGffBaseRecord
{
public:
    CGffAlignRecord( 
        const string& id="");

    void
    AddInsertion(
        unsigned int);
    void
    AddForwardShift(
        unsigned int);
    void
    AddReverseShift(
        unsigned int);
    void
    AddDeletion(
        unsigned int);
    void
    AddMatch(
        unsigned int);

    void
    FinalizeMatches();

    string StrId() const { return mRecordId; };
    string StrTarget() const;
    string StrGap() const;
    string StrAttributes() const;

protected:
    string mRecordId;
    string mAttrGap;
    bool mGapIsTrivial;
    unsigned int mAccumulatedMatches;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif // OBJTOOLS_WRITERS___GFF_FEATURE_RECORD__HPP
