#ifndef GUI_WIDGETS_EDIT___MACRO_ACTION_TYPE__HPP
#define GUI_WIDGETS_EDIT___MACRO_ACTION_TYPE__HPP
/*  $Id: macro_action_type.hpp 44990 2020-05-04 17:26:18Z asztalos $
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

#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE

enum class EMActionType {
    eNotSet,
    eApplyBsrc,
    eEditBsrcText,
    eEditBsrcTax,
    eConvertBsrc,
    eCopyBsrc,
    eParseBsrc,
    eSwapBsrc,
    eParseToBsrc,
    eRemoveBsrc,
    eRmvOutsideBsrc,
    eApplyMolinfo,
    eEditMolinfo,
    eRemoveMolinfo,
    eRmvOutsideMolinfo,
    eEditMolinfoFields,
    eApplyMisc,
    eEditMisc,
    eRemoveMisc,
    eRmvOutsideMisc,
    eApplyFeatQual,
    eEditFeatQual,
    eConvertFeatQual,
    eCopyFeatQual,
    eParseFeatQual,
    eSwapFeatQual,
    eRmvFeatQual,
    eRmvOutsideFeatQual,
    eApplyRNA,
    eEditRNA,
    eConvertRNA,
    eCopyRNA,
    eParseRNA,
    eSwapRNA,
    eRmvRNA,
    eRmvOutsideRNA,
    eRmvLineageNotes,
    eRmvOrgName,
    eFixSubsrcFormat,
    eFixSrcQuals,
    eFixMouseStrain,
    eFixFormatPrimer,
    eTrimJunk,
    eApplyPub,
    eEditPub,
    eRmvPub,
    eRmvOutsidePub,
    eApplyAuthors,
    eFixPubTitle,
    eFixPubAuthors,
    eFixPubAffil,
    eFixPubAffilCountry,
    eFixPubAffilExcept,
    eFixPubLastName,
    eISOJTALookup,
    ePMIDLookup,
    eDOILookup,
    eApplyPmidToEntry,
    eApplyDOIToEntry,
    eMoveMiddleName,
    eStripSuffix,
    eReverseAuthorNames,
    eTruncateMI,
    eFixUSAandStates,
    eApplyStructComm,
    eEditStructComm,
    eRmvStructComm,
    eRmvOutsideStructComm,
    eReorderStructComm,
    eAddStructComment,
    eParseBsrcToStructComm,
    eApplyDBLink,
    eEditDBLink,
    eRmvDBLink,
    eRmvOutsideDBLink,
    eRemoveDescr,
    eAdjustConsensusSpliceSite,
    eSynchronizeCDSPartials,
    eReplaceStopsSel,
    eTrimStopsFromCompleteCDS,
    eRetranslateCDS,
    eConvertFeature,
    eRemoveFeature,
    eRemoveDbXref,
    eAddGeneXref,
    eRemoveGeneXref,
    eSetExceptions,
    eSet5Partial,
    eClear5Partial,
    eSet3Partial,
    eClear3Partial,
    eSetBothPartials,
    eClearBothPartials,
    eConvertLocStrand,
    eConvertLocType,
    eExtend5ToEnd,
    eExtend3ToEnd,
    eAddCDS,
    eAddRNA,
    eAddFeature,
    eGeneApply,
    eGeneEdit,
    eGeneConvert,
    eGeneParse,
    eGeneCopy,
    eGeneSwap,
    eRmvGene,
    eRmvOutsideGene,
    eProteinApply,
    eProteinEdit,
    eProteinConvert,
    eProteinParse,
    eProteinCopy,
    eProteinSwap,
    eProteinRmv,
    eProteinRmvOutside,
    eFixProtFormat,
    eRmvInvalidEC,
    eUpdateEC,
    eConvertCdsGeneProt,
    eCopyCdsGeneProt,
    eSwapCdsGeneProt,
    eParseCdsGeneProt,
    eParseToCdsGeneProt,
    eRmvSegGaps,
    eRmvAlign,
    eRmvSequence,
    eTrimTerminalNs,
    eRmvDuplStructComm,
    eRmvDuplFeatures,
    eAddProteinTitle,
    eDiscrAutofix,
    eFixSpelling,
    eBoldXref,
    eAutodefID,
    eAutodef,
    eJointRNA,
    eTaxLookup,
    eAddAssemblyGapByNs,
    eDeltaToRaw,
    eRemoveSingleItem,
    eRenormNucProt,
    eConvertSetClass,
    eCreateProtFeats,
    eApplySrcQualTable,
    eApplyStrCommQualTable,
    eApplyPubQualTable,
    eConvertGapsBySize,
    eApplyDBlinkQualTable,
    eApplyFeatQualTable,
    eApplyMolinfoQualTable,
    eApplyCDSGeneProtQualTable,
    eApplyRNAQualTable,
    eApplyMiscQualTable
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_ACTION_TYPE__HPP
