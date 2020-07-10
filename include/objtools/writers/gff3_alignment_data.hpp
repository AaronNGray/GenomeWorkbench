/*  $Id: gff3_alignment_data.hpp 564759 2018-06-01 15:26:45Z ludwigf $
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

#ifndef OBJTOOLS_WRITERS___GFF3ALIGNMENTDATA__HPP
#define OBJTOOLS_WRITERS___GFF3ALIGNMENTDATA__HPP

#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objtools/alnmgr/alnmap.hpp>
#include <objtools/writers/gff2_write_data.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ----------------------------------------------------------------------------
class NCBI_XOBJWRITE_EXPORT CGffAlignmentRecord
//  ----------------------------------------------------------------------------
    : public CGffWriteRecord
{
public:
    CGffAlignmentRecord(
            unsigned int uFlags =0,
            unsigned int uRecordId =0):
        CGffWriteRecord(sDummyContext, NStr::UIntToString(uRecordId)),
        m_uFlags(uFlags),
        m_bIsTrivial(true)
    {
        mType = "match";
        mIdAttribute = string( "ID=" ) + NStr::UIntToString( uRecordId );
    };

    virtual ~CGffAlignmentRecord() {};

    virtual void SetMatchType(
        const CSeq_id&, //source
        const CSeq_id&); //target

    void SetScore(
        const CScore& );

    string StrAttributes() const;

protected:
    static CGffFeatureContext sDummyContext;
    unsigned int m_uFlags;
    string mIdAttribute;
    string m_strAlignment;
    string m_strOtherScores;

    bool m_bIsTrivial;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif // OBJTOOLS_WRITERS___GFF3ALIGNMENTDATA__HPP
