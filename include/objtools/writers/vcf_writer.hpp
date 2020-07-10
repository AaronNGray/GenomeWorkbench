/*  $Id: vcf_writer.hpp 542403 2017-07-31 14:10:26Z ludwigf $
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
 * Authors:  Frank Ludwig
 *
 * File Description:  Write vcf file
 *
 */

#ifndef OBJTOOLS_WRITERS___VCF_WRITER__HPP
#define OBJTOOLS_WRITERS___VCF_WRITER__HPP

#include <objects/seqfeat/Variation_inst.hpp>
#include <objtools/writers/writer.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE


//  ============================================================================
class NCBI_XOBJWRITE_EXPORT CVcfWriter :
    public CWriterBase, public CFeatWriter
//  ============================================================================
{
public:
    typedef enum {
        fNormal =       0,
    } TFlags;
    
public:
    CVcfWriter(
        CScope&,
        CNcbiOstream&,
        TFlags = fNormal );

    virtual ~CVcfWriter();

    bool WriteAnnot( 
        const CSeq_annot&,
        const string& = "",
        const string& = "" ) override;

    virtual SAnnotSelector& SetAnnotSelector(void) override;

protected:
    void x_GetTypeRefAlt(const CVariation_inst &inst, int &rtype, string &ref, list<int>& alt_types, vector<string> &alt);

    bool x_WriteInit(
        const CSeq_annot& );
    bool x_WriteMeta(
        const CSeq_annot& );
    bool x_WriteHeader(
        const CSeq_annot& );
    bool x_WriteData(
        const CSeq_annot& );

    bool x_WriteMetaCreateNew(
        const CSeq_annot& );

    bool xWriteFeature(
        CFeat_CI feat_it) override;

    bool x_WriteFeature(
        const CMappedFeat& );

    bool x_WriteFeatureChrom(
        const CMappedFeat& );
        
    bool x_WriteFeaturePos(
        const CMappedFeat&,
        unsigned int start,
        const int type
        );
        
    bool x_WriteFeatureId(
        const CMappedFeat& );
        
    bool x_WriteFeatureRef(
        const unsigned int start,
        const int type,
        const string &anchor,
        const string &ref
        );
        
    bool x_WriteFeatureAlt(
        const unsigned int start,
        const int type,
        const string &anchor,
        const list<int>& alt_types,
        const vector<string> &alt
        );
        
    bool x_WriteFeatureQual(
        const CMappedFeat&);
        
    bool x_WriteFeatureFilter(
        const CMappedFeat& );
        
    bool x_WriteFeatureInfo(
        const CMappedFeat& );
        
    bool x_WriteFeatureGenotypeData(
        const CMappedFeat& );

    CScope& m_Scope;
    vector<string> m_GenotypeHeaders;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif  // OBJTOOLS_WRITERS___VCF_WRITER__HPP
