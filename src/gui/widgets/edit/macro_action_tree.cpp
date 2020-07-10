/*  $Id: macro_action_tree.cpp 44990 2020-05-04 17:26:18Z asztalos $
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

 */


#include <ncbi_pch.hpp>
#include <gui/objutils/macro_biodata.hpp>
#include <gui/widgets/edit/macro_action_tree.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CMActionNode::CMActionNode(const string& label, const CTempString& target, const EMActionType type)
    : m_Label(label), m_Target(target), m_ActionType(type)
{
}

CRef<CMActionNode> CMActionNode::AddChild(const string& label, const CTempString& target, const EMActionType type)
{
    CRef<CMActionNode> child(new CMActionNode(label, target, type));
    m_Children.push_back(child);
    return child;
}


CConstRef<CMActionTree> CMActionTreeBuilder::BuildActionTree()
{
    CRef<CMActionNode> root(new CMActionNode(kEmptyStr, kEmptyStr));
    
    CRef<CMActionNode> descr_node = root->AddChild("Descriptors", macro::CMacroBioData::sm_Seqdesc);
    // BioSource
    CRef<CMActionNode> bsrc_node = descr_node->AddChild(macro::CMacroBioData::sm_BioSource, macro::CMacroBioData::sm_BioSource);
    const CTempString& bsrc_target = bsrc_node->GetTarget();
    bsrc_node->AddChild("Apply source qualifier", bsrc_target, EMActionType::eApplyBsrc);
    bsrc_node->AddChild("Edit source text qualifier", bsrc_target, EMActionType::eEditBsrcText);
    bsrc_node->AddChild("Edit source tax qualifier", bsrc_target, EMActionType::eEditBsrcTax);
    bsrc_node->AddChild("Convert source qualifier", bsrc_target, EMActionType::eConvertBsrc);
    bsrc_node->AddChild("Copy source qualifier", bsrc_target, EMActionType::eCopyBsrc);
    bsrc_node->AddChild("Parse source qualifier", bsrc_target, EMActionType::eParseBsrc);
    bsrc_node->AddChild("Swap source qualifier", bsrc_target, EMActionType::eSwapBsrc);
    bsrc_node->AddChild("Parse to source qualifier", bsrc_target, EMActionType::eParseToBsrc);
    bsrc_node->AddChild("Parse source qualifier to structured comment field", bsrc_target, EMActionType::eParseBsrcToStructComm);
    bsrc_node->AddChild("Fix subsource format", bsrc_target, EMActionType::eFixSubsrcFormat);
    bsrc_node->AddChild("Fix caps in source qualifiers", bsrc_target, EMActionType::eFixSrcQuals);
    bsrc_node->AddChild("Fix caps in common Mus Musculus strains", bsrc_target, EMActionType::eFixMouseStrain);
    bsrc_node->AddChild("Apply source qualifier table", bsrc_target, EMActionType::eApplySrcQualTable);

    CRef<CMActionNode> primer_node = bsrc_node->AddChild("Primer fixes", bsrc_target);
    primer_node->AddChild("Fix i in primers", bsrc_target, EMActionType::eFixFormatPrimer);
    primer_node->AddChild("Remove 'junk' strings", bsrc_target, EMActionType::eTrimJunk);

    CRef<CMActionNode> remove_node = bsrc_node->AddChild("Remove", bsrc_target, EMActionType::eNotSet);
    remove_node->AddChild("Remove source qualifier", bsrc_target, EMActionType::eRemoveBsrc);
    remove_node->AddChild("Remove text from source qualifier", bsrc_target, EMActionType::eRmvOutsideBsrc);
    remove_node->AddChild("Remove lineage notes", bsrc_target, EMActionType::eRmvLineageNotes);
    remove_node->AddChild("Remove organism name", bsrc_target, EMActionType::eRmvOrgName);

    // MolInfo
    CRef<CMActionNode> molinfo_node = descr_node->AddChild(macro::CMacroBioData::sm_MolInfo, macro::CMacroBioData::sm_MolInfo);
    const CTempString& mol_target = macro::CMacroBioData::sm_Seq;
    molinfo_node->AddChild("Apply molinfo qualifier", mol_target, EMActionType::eApplyMolinfo);
    molinfo_node->AddChild("Edit molinfo qualifier", mol_target, EMActionType::eEditMolinfo);
    molinfo_node->AddChild("Remove molinfo qualifier", mol_target, EMActionType::eRemoveMolinfo);
    molinfo_node->AddChild("Remove text from molinfo qualifier", mol_target, EMActionType::eRmvOutsideMolinfo);
    molinfo_node->AddChild("Edit molinfo fields", macro::CMacroBioData::sm_SeqNa, EMActionType::eEditMolinfoFields);
    molinfo_node->AddChild("Apply molinfo qualifier table", mol_target, EMActionType::eApplyMolinfoQualTable);

    // Pubdesc
    CRef<CMActionNode> pub_node = descr_node->AddChild(macro::CMacroBioData::sm_Pubdesc, macro::CMacroBioData::sm_Pubdesc);
    const CTempString& pub_target = pub_node->GetTarget();
    pub_node->AddChild("Apply publication field", pub_target, EMActionType::eApplyPub);
    pub_node->AddChild("Apply authors", pub_target, EMActionType::eApplyAuthors);
    pub_node->AddChild("Edit publication field", pub_target, EMActionType::eEditPub);
    pub_node->AddChild("Remove publication field", pub_target, EMActionType::eRmvPub);
    pub_node->AddChild("Remove text from publication field", pub_target, EMActionType::eRmvOutsidePub);
    pub_node->AddChild("ISOJTA lookup", pub_target, EMActionType::eISOJTALookup);
    pub_node->AddChild("PMID lookup", pub_target, EMActionType::ePMIDLookup);
    pub_node->AddChild("DOI lookup", pub_target, EMActionType::eDOILookup);
    pub_node->AddChild("Fix USA and state abbreviations", pub_target, EMActionType::eFixUSAandStates);
    pub_node->AddChild("Apply publication qualifier table", pub_target, EMActionType::eApplyPubQualTable);

    CRef<CMActionNode> fixcaps = pub_node->AddChild("Fix capitalization", pub_target);
    fixcaps->AddChild("Fix pub title", pub_target, EMActionType::eFixPubTitle);
    fixcaps->AddChild("Fix pub authors", pub_target, EMActionType::eFixPubAuthors);
    fixcaps->AddChild("Fix pub affil", pub_target, EMActionType::eFixPubAffil);
    fixcaps->AddChild("Fix pub affil country", pub_target, EMActionType::eFixPubAffilCountry);
    fixcaps->AddChild("Fix pub affil except institute and department", pub_target, EMActionType::eFixPubAffilExcept);
    fixcaps->AddChild("Fix pub author last name", pub_target, EMActionType::eFixPubLastName);

    CRef<CMActionNode> author_node = pub_node->AddChild("Author fixes", pub_target);
    author_node->AddChild("Move middle name to first name", pub_target, EMActionType::eMoveMiddleName);
    author_node->AddChild("Remove author suffix", pub_target, EMActionType::eStripSuffix);
    author_node->AddChild("Truncate middle name initials", pub_target, EMActionType::eTruncateMI);
    author_node->AddChild("Reverse author names", pub_target, EMActionType::eReverseAuthorNames);

    // StrComment
    CRef<CMActionNode> strcomm_node = descr_node->AddChild(macro::CMacroBioData::sm_StrComm, macro::CMacroBioData::sm_StrComm);
    const CTempString& strcomm_target = strcomm_node->GetTarget();
    strcomm_node->AddChild("Apply structured comment field", strcomm_target, EMActionType::eApplyStructComm);
    strcomm_node->AddChild("Edit structured comment field", strcomm_target, EMActionType::eEditStructComm);
    strcomm_node->AddChild("Remove structured comment field", strcomm_target, EMActionType::eRmvStructComm);
    strcomm_node->AddChild("Remove text from structured comment field", strcomm_target, EMActionType::eRmvOutsideStructComm);
    strcomm_node->AddChild("Reorder structured comment", strcomm_target, EMActionType::eReorderStructComm);
    strcomm_node->AddChild("Apply structured comment qualifier table", strcomm_target, EMActionType::eApplyStrCommQualTable);

    // DBLink
    CRef<CMActionNode> dblink_node = descr_node->AddChild(macro::CMacroBioData::sm_DBLink, macro::CMacroBioData::sm_DBLink);
    const CTempString& dblink_target = dblink_node->GetTarget();
    dblink_node->AddChild("Apply DBLink field", macro::CMacroBioData::sm_SeqNa, EMActionType::eApplyDBLink);
    dblink_node->AddChild("Edit DBLink field", dblink_target, EMActionType::eEditDBLink);
    dblink_node->AddChild("Remove DBLink field", dblink_target, EMActionType::eRmvDBLink);
    dblink_node->AddChild("Remove text from DBLink field", dblink_target, EMActionType::eRmvOutsideDBLink);
    dblink_node->AddChild("Apply DBLink field table", macro::CMacroBioData::sm_SeqNa, EMActionType::eApplyDBlinkQualTable);

    // Misc
    CRef<CMActionNode> misc_node = descr_node->AddChild("Misc", macro::CMacroBioData::sm_Seqdesc);
    const CTempString& misc_target = misc_node->GetTarget();
    misc_node->AddChild("Apply misc qualifier", macro::CMacroBioData::sm_SeqNa, EMActionType::eApplyMisc);
    misc_node->AddChild("Edit misc qualifier", misc_target, EMActionType::eEditMisc);
    misc_node->AddChild("Remove misc qualifier", misc_target, EMActionType::eRemoveMisc);
    misc_node->AddChild("Remove text from misc qualifier", misc_target, EMActionType::eRmvOutsideMisc);
    misc_node->AddChild("Apply misc qualifier table", macro::CMacroBioData::sm_SeqNa, EMActionType::eApplyMiscQualTable);

    descr_node->AddChild("Remove descriptor", macro::CMacroBioData::sm_Seqdesc, EMActionType::eRemoveDescr);

    // Gene
    CRef<CMActionNode> feat_node = root->AddChild("Features", macro::CMacroBioData::sm_SeqFeat);
    CRef<CMActionNode> gene_node = feat_node->AddChild(macro::CMacroBioData::sm_Gene, macro::CMacroBioData::sm_Gene);
    const CTempString& gene_target = gene_node->GetTarget();
    gene_node->AddChild("Apply gene qualifier", gene_target, EMActionType::eGeneApply);
    gene_node->AddChild("Edit gene qualifier", gene_target, EMActionType::eGeneEdit);
    gene_node->AddChild("Convert gene qualifier", gene_target, EMActionType::eGeneConvert);
    gene_node->AddChild("Copy gene qualifier", gene_target, EMActionType::eGeneCopy);
    gene_node->AddChild("Parse gene qualifier", gene_target, EMActionType::eGeneParse);
    gene_node->AddChild("Swap gene qualifier", gene_target, EMActionType::eGeneSwap);
    gene_node->AddChild("Remove gene qualifier", gene_target, EMActionType::eRmvGene);
    gene_node->AddChild("Remove text from gene qualifier", gene_target, EMActionType::eRmvOutsideGene);
    // RNA
    CRef<CMActionNode> rna_node = feat_node->AddChild(macro::CMacroBioData::sm_RNA, macro::CMacroBioData::sm_RNA);
    const CTempString& rna_target = rna_node->GetTarget();
    rna_node->AddChild("Apply RNA qualifier", rna_target, EMActionType::eApplyRNA);
    rna_node->AddChild("Edit RNA qualifier", rna_target, EMActionType::eEditRNA);
    rna_node->AddChild("Convert RNA qualifier", rna_target, EMActionType::eConvertRNA);
    rna_node->AddChild("Copy RNA qualifier", rna_target, EMActionType::eCopyRNA);
    rna_node->AddChild("Parse RNA qualifier", rna_target, EMActionType::eParseRNA);
    rna_node->AddChild("Swap RNA qualifier", rna_target, EMActionType::eSwapRNA);
    rna_node->AddChild("Remove RNA qualifier", rna_target, EMActionType::eRmvRNA);
    rna_node->AddChild("Remove text from RNA qualifier", rna_target, EMActionType::eRmvOutsideRNA);
    rna_node->AddChild("Apply RNA qualifier table", rna_target, EMActionType::eApplyRNAQualTable);

    // CDS
    CRef<CMActionNode> cds_node = feat_node->AddChild(macro::CMacroBioData::sm_CdRegion, macro::CMacroBioData::sm_CdRegion);
    const CTempString& cds_target = cds_node->GetTarget();
    cds_node->AddChild("Adjust CDS for consensus splice sites", cds_target, EMActionType::eAdjustConsensusSpliceSite);
    cds_node->AddChild("Synchronize CDS partials", cds_target, EMActionType::eSynchronizeCDSPartials);
    cds_node->AddChild("Replace stops with selenocysteines", cds_target, EMActionType::eReplaceStopsSel);
    cds_node->AddChild("Remove trailing stops from complete CDS", cds_target, EMActionType::eTrimStopsFromCompleteCDS);
    cds_node->AddChild("Retranslate CDS", cds_target, EMActionType::eRetranslateCDS);

    // Protein
    CRef<CMActionNode> protein_node = feat_node->AddChild(macro::CMacroBioData::sm_Protein, macro::CMacroBioData::sm_Protein);
    const CTempString& protein_target = protein_node->GetTarget();
    protein_node->AddChild("Apply protein qualifier", protein_target, EMActionType::eProteinApply);
    protein_node->AddChild("Edit protein qualifier", protein_target, EMActionType::eProteinEdit);
    protein_node->AddChild("Convert protein qualifier", protein_target, EMActionType::eProteinConvert);
    protein_node->AddChild("Copy protein qualifier", protein_target, EMActionType::eProteinCopy);
    protein_node->AddChild("Parse protein qualifier", protein_target, EMActionType::eProteinParse);
    protein_node->AddChild("Swap protein qualifier", protein_target, EMActionType::eProteinSwap);
    protein_node->AddChild("Remove protein qualifier", protein_target, EMActionType::eProteinRmv);
    protein_node->AddChild("Remove text from protein qualifier", protein_target, EMActionType::eProteinRmvOutside);
    protein_node->AddChild("Remove organism names from protein names", protein_target, EMActionType::eFixProtFormat);
    protein_node->AddChild("Remove invalid EC numbers", protein_target, EMActionType::eRmvInvalidEC);
    protein_node->AddChild("Update replaced EC numbers", protein_target, EMActionType::eUpdateEC);

    // CDSGeneProt
    CRef<CMActionNode> cdsgeneprot_node = feat_node->AddChild("CDS-Gene-Prot-mRNA", macro::CMacroBioData::sm_Protein);
    const CTempString& cdsgeneprot_target = cdsgeneprot_node->GetTarget();
    cdsgeneprot_node->AddChild("Convert cds-gene-prot qualifier", cdsgeneprot_target, EMActionType::eConvertCdsGeneProt);
    cdsgeneprot_node->AddChild("Copy cds-gene-prot qualifier", cdsgeneprot_target, EMActionType::eCopyCdsGeneProt);
    cdsgeneprot_node->AddChild("Parse cds-gene-prot qualifier", cdsgeneprot_target, EMActionType::eParseCdsGeneProt);
    cdsgeneprot_node->AddChild("Parse to cds-gene-prot qualifier", cdsgeneprot_target, EMActionType::eParseToCdsGeneProt);
    cdsgeneprot_node->AddChild("Swap cds-gene-prot qualifier", cdsgeneprot_target, EMActionType::eSwapCdsGeneProt);
    cdsgeneprot_node->AddChild("Apply cds-gene-prot qualifier table", cdsgeneprot_target, EMActionType::eApplyCDSGeneProtQualTable);

    const CTempString& feat_target = feat_node->GetTarget();
    CRef<CMActionNode> feat_loc_node = feat_node->AddChild("Edit feature locations", macro::CMacroBioData::sm_SeqFeat);
    feat_loc_node->AddChild("Set 5' partial", feat_target, EMActionType::eSet5Partial);
    feat_loc_node->AddChild("Set 3' partial", feat_target, EMActionType::eSet3Partial);
    feat_loc_node->AddChild("Set both ends partial", feat_target, EMActionType::eSetBothPartials);
    feat_loc_node->AddChild("Clear 5' partial", feat_target, EMActionType::eClear5Partial);
    feat_loc_node->AddChild("Clear 3' partial", feat_target, EMActionType::eClear3Partial);
    feat_loc_node->AddChild("Clear both ends partial", feat_target, EMActionType::eClearBothPartials);
    feat_loc_node->AddChild("Convert location strand", feat_target, EMActionType::eConvertLocStrand);
    feat_loc_node->AddChild("Convert location type", feat_target, EMActionType::eConvertLocType);
    feat_loc_node->AddChild("Extend 5' to end of sequence", feat_target, EMActionType::eExtend5ToEnd);
    feat_loc_node->AddChild("Extend 3' to end of sequence", feat_target, EMActionType::eExtend3ToEnd);

    feat_node->AddChild("Apply feature qualifier", feat_target, EMActionType::eApplyFeatQual);
    feat_node->AddChild("Edit feature qualifier", feat_target, EMActionType::eEditFeatQual);
    feat_node->AddChild("Convert feature qualifier", feat_target, EMActionType::eConvertFeatQual);
    feat_node->AddChild("Copy feature qualifier", feat_target, EMActionType::eCopyFeatQual);
    feat_node->AddChild("Parse feature qualifier", feat_target, EMActionType::eParseFeatQual);
    feat_node->AddChild("Swap feature qualifier", feat_target, EMActionType::eSwapFeatQual);
    feat_node->AddChild("Remove feature qualifier", feat_target, EMActionType::eRmvFeatQual);
    feat_node->AddChild("Remove text from feature qualifier", feat_target, EMActionType::eRmvOutsideFeatQual);
    feat_node->AddChild("Convert features", feat_target, EMActionType::eConvertFeature);
    feat_node->AddChild("Remove features", feat_target, EMActionType::eRemoveFeature);
    feat_node->AddChild("Remove dbXrefs from features", feat_target, EMActionType::eRemoveDbXref);
    feat_node->AddChild("Add Gene Xref to features", feat_target, EMActionType::eAddGeneXref);
    feat_node->AddChild("Remove Gene Xref from features", feat_target, EMActionType::eRemoveGeneXref);
    feat_node->AddChild("Set exceptions", feat_target, EMActionType::eSetExceptions);
    feat_node->AddChild("Apply feature qualifier table", feat_target, EMActionType::eApplyFeatQualTable);

    // SeqAlign
    CRef<CMActionNode> align_node = root->AddChild(macro::CMacroBioData::sm_SeqAlign, macro::CMacroBioData::sm_SeqAlign);
    align_node->AddChild("Remove seg-gaps", align_node->GetTarget(), EMActionType::eRmvSegGaps);
    align_node->AddChild("Remove alignment", align_node->GetTarget(), EMActionType::eRmvAlign);

    // SeqNA
    CRef<CMActionNode> seq_node = root->AddChild("Sequences", macro::CMacroBioData::sm_Seq);
    CRef<CMActionNode> seqna_node = seq_node->AddChild(macro::CMacroBioData::sm_SeqNa, macro::CMacroBioData::sm_SeqNa);
    const CTempString& seqna_target = seqna_node->GetTarget();
    seqna_node->AddChild("Apply structured comment", seqna_target, EMActionType::eAddStructComment);
    seqna_node->AddChild("Remove duplicate structured comments", seqna_target, EMActionType::eRmvDuplStructComm);
    seqna_node->AddChild("Join short tRNAs", seqna_target, EMActionType::eJointRNA);
    seqna_node->AddChild("Trim terminal Ns", seqna_target, EMActionType::eTrimTerminalNs);
    seqna_node->AddChild("Convert gaps by size", seqna_target, EMActionType::eConvertGapsBySize);
    seqna_node->AddChild("Make Barcode Xrefs", seqna_target, EMActionType::eBoldXref);

    CRef<CMActionNode> applyfeat_node = seqna_node->AddChild("Apply features", macro::CMacroBioData::sm_SeqNa);
    applyfeat_node->AddChild("Apply CDS feature", seqna_target, EMActionType::eAddCDS);
    applyfeat_node->AddChild("Apply RNA feature", seqna_target, EMActionType::eAddRNA);
    applyfeat_node->AddChild("Apply other feature", seqna_target, EMActionType::eAddFeature);

    // SeqAA
    CRef<CMActionNode> seqaa_node = seq_node->AddChild(macro::CMacroBioData::sm_SeqAa, macro::CMacroBioData::sm_SeqAa);
    const CTempString& seqaa_target = seqaa_node->GetTarget();
    seqaa_node->AddChild("Instantiate protein titles", seqaa_target, EMActionType::eAddProteinTitle);

    seq_node->AddChild("Remove sequence", seq_node->GetTarget(), EMActionType::eRmvSequence);
    seq_node->AddChild("Remove duplicate features", seq_node->GetTarget(), EMActionType::eRmvDuplFeatures);

    // SeqSet
    CRef<CMActionNode> set_node = root->AddChild(macro::CMacroBioData::sm_SeqSet, macro::CMacroBioData::sm_SeqSet);
    set_node->AddChild("Convert set type", set_node->GetTarget(), EMActionType::eConvertSetClass);

    // Top Seq-entry
    CRef<CMActionNode> entry_node = root->AddChild(macro::CMacroBioData::sm_TSEntry, macro::CMacroBioData::sm_TSEntry);
    const CTempString& entry_target = entry_node->GetTarget();
    entry_node->AddChild("Autodef ID", entry_target, EMActionType::eAutodefID);
    entry_node->AddChild("Autodef with options", entry_target, EMActionType::eAutodef);
    entry_node->AddChild("Fix spelling", entry_target, EMActionType::eFixSpelling);
    entry_node->AddChild("Run discrepancy report autofix", entry_target, EMActionType::eDiscrAutofix);
    entry_node->AddChild("Create protein features", entry_target, EMActionType::eCreateProtFeats);
    entry_node->AddChild("Add assembly gaps by Ns", entry_target, EMActionType::eAddAssemblyGapByNs);
    entry_node->AddChild("Convert delta to raw", entry_target, EMActionType::eDeltaToRaw);

    CRef<CMActionNode> setact_node = entry_node->AddChild("Set actions", macro::CMacroBioData::sm_TSEntry);
    setact_node->AddChild("Remove single item set", entry_target, EMActionType::eRemoveSingleItem);
    setact_node->AddChild("Renormalize nuc-prot set", entry_target, EMActionType::eRenormNucProt);

    entry_node->AddChild("Apply PMID and do PMID lookup", entry_target, EMActionType::eApplyPmidToEntry);
    entry_node->AddChild("Apply DOI and do DOI lookup", entry_target, EMActionType::eApplyDOIToEntry);
    entry_node->AddChild("Tax_fix/Cleanup", entry_target, EMActionType::eTaxLookup);

    CRef<CMActionTree> tree(new CMActionTree);
    tree->SetRoot(*root);
    return ConstRef(tree.GetPointer());
}
   
END_NCBI_SCOPE
