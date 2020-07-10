/*  $Id: feattable_edit.hpp 593415 2019-09-18 14:52:52Z ludwigf $
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
 * Author: Frank Ludwig, NCBI
 *
 * File Description:
 *   Convenience wrapper around some of the other feature processing finctions
 *   in the xobjedit library.
 */


#ifndef _FEATTABLE_EDIT_H_
#define _FEATTABLE_EDIT_H_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/util/feature.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
class IObjtoolsListener;
BEGIN_SCOPE(edit)

//  ----------------------------------------------------------------------------
class NCBI_XOBJEDIT_EXPORT CFeatTableEdit
    //  ----------------------------------------------------------------------------
{
    typedef list<CRef<CSeq_feat> > FEATS;

public:
    CFeatTableEdit(
        CSeq_annot&,
        const string& = "",
        unsigned int = 1, //starting locus tag
        unsigned int = 1, //starting feature id
        IObjtoolsListener* = nullptr);
    ~CFeatTableEdit();

    void GenerateLocusTags();
    void InferParentMrnas();
    void InferParentGenes();
    void InferPartials();
    void EliminateBadQualifiers();
    void GenerateProteinAndTranscriptIds();
    void InstantiateProducts();
    void InstantiateProductsNames();
    void GenerateLocusIds();
    void SubmitFixProducts();
    void GenerateMissingMrnaForCds();
    void GenerateMissingGeneForMrna();
    void GenerateMissingGeneForCds();
    void GenerateMissingParentFeatures(
        bool forEukaryote);
    void GenerateMissingParentFeaturesForEukaryote();
    void GenerateMissingParentFeaturesForProkaryote();
    unsigned int PendingLocusTagNumber() const {
        return mLocusTagNumber;
    }
    unsigned int PendingFeatureId() const {
        return mNextFeatId;
    }

    bool AnnotHasAllLocusTags() const;

    bool m_use_hypothetic_protein = true;
    string mLocusTagPrefix;
    unsigned int mLocusTagNumber = 1;
    unsigned int mNextFeatId     = 1;

protected:
    void xGenerateLocusIdsUseExisting();
    void xGenerateLocusIdsRegenerate();

    string xNextFeatId();
    string xNextLocusTag();
    string xNextProteinId(
        const CMappedFeat&);
    string xNextTranscriptId(
        const CMappedFeat&);

    void xPutError(const string& message);

    void xPutErrorMissingLocustag(
        CMappedFeat);
    void xPutErrorMissingTranscriptId(
        CMappedFeat);
    void xPutErrorMissingProteinId(
        CMappedFeat);
    void xPutErrorDifferingTranscriptIds(
        const CMappedFeat& mrna);
    void xPutErrorDifferingProteinIds(
        const CMappedFeat& mrna);


    void xFeatureAddQualifier(
        CMappedFeat,
        const std::string&,                 // qual key
        const std::string&);                // qual value
    void xFeatureRemoveQualifier(
        CMappedFeat,                        // qual key
        const std::string&);
    void xFeatureSetQualifier(
        CMappedFeat,
        const std::string&,                 // qual key
        const std::string&);                // qual value

    void xFeatureAddProteinIdMrna(
        CMappedFeat);
    void xFeatureAddProteinIdCds(
        CMappedFeat);
    void xFeatureAddProteinIdDefault(
        CMappedFeat);

    void xFeatureAddTranscriptIdMrna(
        CMappedFeat);
    void xFeatureAddTranscriptIdCds(
        CMappedFeat);
    void xFeatureAddTranscriptIdDefault(
        CMappedFeat);
    void xFeatureSetProduct(
        CMappedFeat,
        const string&);
    std::string xGenerateTranscriptOrProteinId(
        CMappedFeat,
        const std::string&);

    CRef<CSeq_feat> xMakeGeneForFeature(
        const CMappedFeat&);
    void xGenerateMissingGeneForSubtype(
        CSeqFeatData::ESubtype);
    void xGenerateMissingGeneForChoice(
        CSeqFeatData::E_Choice);
    bool xCreateMissingParentGene(
        CMappedFeat);
    bool xAdjustExistingParentGene(
        CMappedFeat);

    static std::string xGetIdStr(
        CMappedFeat);
    std::string xGetCurrentLocusTagPrefix(
        CMappedFeat);

    void xAddTranscriptAndProteinIdsToCdsAndParentMrna(CMappedFeat& cds);
    void xAddTranscriptAndProteinIdsToUnmatchedMrna(CMappedFeat& mrna);
    void xAddTranscriptAndProteinIdsToMrna(const string& cds_transcript_id,
        const string& cds_protein_id,
        CMappedFeat& mrna);
    void xConvertToGeneralIds(const CMappedFeat& mf,
        string& transcript_id,
        string& protein_id);

    void xGenerate_mRNA_Product(CSeq_feat& cd_feature);
    CConstRef<CSeq_feat> xGetLinkedFeature(const CSeq_feat& cd_feature, bool gene);


    CSeq_annot& mAnnot;
    CRef<CScope> mpScope;
    CSeq_annot_Handle mHandle;
    feature::CFeatTree mTree;
    CSeq_annot_EditHandle mEditHandle;
    IObjtoolsListener* mpMessageListener;

    map<string, int> mMapProtIdCounts;

    using TFeatQualMap = map<CMappedFeat, string>;
    set<CMappedFeat> mProcessedMrnas;
};

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif

