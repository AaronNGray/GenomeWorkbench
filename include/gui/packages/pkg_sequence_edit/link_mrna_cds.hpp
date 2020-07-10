#ifndef _GUI_PKG_EDIT__LINK_MRNA_CDS__HPP_
#define _GUI_PKG_EDIT__LINK_MRNA_CDS__HPP_
/*  $Id: link_mrna_cds.hpp 41094 2018-05-22 17:55:16Z katargir $
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
 * Authors:  Andrea Asztalos
 *   Place reciprocal cross-references between mRNA and CDS. The features are selected 
 *   according to some criteria.
 */


#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/utils/command_processor.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_feat;
END_SCOPE(objects)

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CmRNACDSLinker
{
public:
    CmRNACDSLinker(const objects::CSeq_entry_Handle& entry)
        : m_TopSeqEntry(entry) {}
    ~CmRNACDSLinker() {}

    void LinkSelectedFeatures(TConstScopedObjects& objects, ICommandProccessor* cmdProcessor);
    void LinkByOverlap(ICommandProccessor* cmdProcessor);
    void LinkByProduct(ICommandProccessor* cmdProcessor);
    void LinkByLabel(ICommandProccessor* cmdProcessor);
    void LinkByLabelAndLocation(ICommandProccessor* cmdProcessor);
    void CompleteHalfFormedXrefPairs(ICommandProccessor* cmdProcessor);

    bool AssignFeatureIds(CRef<CMacroCmdComposite> cmd);
    static void s_CreateXRefLink(objects::CSeq_feat& from_feat, const objects::CSeq_feat& to_feat);
    static bool s_IsDirectXrefBetween(const objects::CSeq_feat& from_feat, const objects::CSeq_feat& to_feat);

    bool SetReciprocalXrefs(const objects::CSeq_feat& mrna, const objects::CSeq_feat& cds, CRef<CCmdComposite> cmd);
    bool ReplaceExistingXrefs(const objects::CSeq_feat& mrna, const objects::CSeq_feat& cds, CRef<CCmdComposite> cmd);
    
    /// returns the pair of cross-referenced mRNA and CDS features, when one of them is selected
    vector<CConstRef<CObject> > GetReferencedmRNA_CDS(TConstScopedObjects& objects);

private:
    bool x_OkForLinking(CConstRef<objects::CSeq_feat> mrna, CConstRef<objects::CSeq_feat> cds);
    bool x_ReplaceExistingFeatIdXrefs(objects::CSeq_feat& from_feat, const objects::CSeq_feat& to_feat);

    objects::CSeq_entry_Handle m_TopSeqEntry;

    // prohibit copying and assigning
    CmRNACDSLinker(const CmRNACDSLinker&);
    CmRNACDSLinker& operator=(const CmRNACDSLinker&);
};

END_NCBI_SCOPE

#endif
    // _GUI_PKG_EDIT__LINK_MRNA_CDS__HPP_
