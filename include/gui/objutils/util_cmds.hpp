#ifndef GUI_OBJUTILS___UTIL_CMDS__HPP
#define GUI_OBJUTILS___UTIL_CMDS__HPP

/*  $Id: util_cmds.hpp 42480 2019-03-07 20:14:48Z bollin $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */


#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CSeq_feat;
    class CScope;
    class CBioseq_Handle;
END_SCOPE(objects);

NCBI_GUIOBJUTILS_EXPORT void GetProductToCDSMap(objects::CScope &scope, map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > &product_to_cds);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetDeleteFeatureCommand(const objects::CSeq_feat_Handle& fh, bool remove_proteins = true);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetDeleteFeatureCommand(const objects::CSeq_feat_Handle& fh, bool remove_proteins, map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > &product_to_cds);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetDeleteSequenceCommand(objects::CBioseq_Handle bsh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetDeleteAllFeaturesCommand(objects::CSeq_entry_Handle seh, size_t& count, bool remove_proteins = true);

NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> TaxonomyLookupCommand(objects::CSeq_entry_Handle seh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> SpecificHostCleanupCommand(objects::CSeq_entry_Handle seh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> CleanupCommand(objects::CSeq_entry_Handle orig_seh, bool extended, bool do_tax);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetDisableStrainForwardingCommand(objects::CSeq_entry_Handle seh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetEnableStrainForwardingCommand(objects::CSeq_entry_Handle seh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetRmCultureNotesCommand(objects::CSeq_entry_Handle seh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetSynchronizeProductMolInfoCommand(objects::CScope& scope, const objects::CSeq_feat& cds);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetSynchronizeProteinPartialsCommand(objects::CScope& scope, const objects::CSeq_feat& cds);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetRetranslateCDSCommand(objects::CScope& scope, const objects::CSeq_feat& cds, bool create_general_only);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetRetranslateCDSCommand(objects::CScope& scope, const objects::CSeq_feat& cds, int& offset, bool create_general_only);

// This second GetRetranslateCDSCommand is for when you are making additional changes to the coding region
// before or after translation (such as truncating or extending for stop codon)
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetRetranslateCDSCommand(objects::CScope& scope, objects::CSeq_feat& cds, bool& cds_change, bool create_general_only);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetRetranslateCDSCommand(objects::CScope& scope, objects::CSeq_feat& cds, bool& cds_change, int& offset, bool create_general_only);

NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> SetTranslExcept(objects::CSeq_entry_Handle seh, const string& comment, bool strict, bool extend, bool adjust_gene);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetReverseComplimentSequenceCommand(objects::CBioseq_Handle bsh);

NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> TrimStopsFromCompleteCDS(const objects::CSeq_feat& cds, objects::CScope& scope);
NCBI_GUIOBJUTILS_EXPORT void SetProteinFeature(objects::CSeq_feat& prot, objects::CBioseq_Handle product, const objects::CSeq_feat& cds);
NCBI_GUIOBJUTILS_EXPORT void AdjustProteinFeature(objects::CSeq_feat& prot, objects::CBioseq_Handle product, objects::CSeq_feat& cds, bool& cds_change);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> CreateOrAdjustProteinFeature(objects::CBioseq_Handle product, objects::CSeq_feat& cds, bool& cds_change);
// runs discrepancy autofix for a specific test
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> AutofixCommand(objects::CSeq_entry_Handle seh, const string& test_name, string* output, const string& suspect_rules = kEmptyStr);

NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetEditLocationCommand(const objects::edit::CLocationEditPolicy& policy, 
                                                                   bool retranslate, bool adjust_gene, const objects::CSeq_feat& orig_feat, 
                                                                   objects::CScope& scope, int& offset, bool create_general_only);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdChangeSeq_feat> AdjustGene(const objects::CSeq_feat& orig_feat, const objects::CSeq_feat& new_feat, objects::CScope& scope);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetNormalizeGeneQualsCommand(objects::CSeq_entry_Handle seh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> GetRemoveDuplicateGOTermsCommand(objects::CSeq_entry_Handle seh);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> UpdatemRNAProduct(const objects::CSeq_feat& protein, objects::CScope& scope, string& message);
NCBI_GUIOBJUTILS_EXPORT CRef<CCmdComposite> UpdatemRNAProduct(const string& prot_product, CConstRef<CObject> object, objects::CScope& scope);

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___UTIL_CMDS__HPP



