/*  $Id: macro_panel_args.cpp 44820 2020-03-23 14:59:16Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 * File Description: Utility functions used in macro editor
 *
 */

#include <ncbi_pch.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/widgets/edit/macro_panel_args.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(macro);

namespace NMacroArgs
{
    // Changes in the function and parameter names require changes in the respective
    // XRC file used in the macro editor

    const TArgumentsVector& GetSetArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kNewValue, CMQueryNodeValue::eString, false)
            , SArgMetaData(kExistingText, CMQueryNodeValue::eString, false)
            , SArgMetaData(kDelimiter, CMQueryNodeValue::eString, true)
        };
        return s_args;
    }
    
    const TArgumentsVector& GetEditArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kFind, CMQueryNodeValue::eString, false)
            , SArgMetaData(kRepl, CMQueryNodeValue::eString, false)
            , SArgMetaData(kLocation, CMQueryNodeValue::eString, false)
            , SArgMetaData(kCaseSens, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kCopy, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kIsRegex, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetCopyArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kFromField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kToField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kExistingText, CMQueryNodeValue::eString, false)
            , SArgMetaData(kDelimiter, CMQueryNodeValue::eString, true)
        };
        return s_args;
    }

    const TArgumentsVector& GetSwapArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kFromField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kToField, CMQueryNodeValue::eString, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetConvertArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kFromField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kToField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kExistingText, CMQueryNodeValue::eString, false)
            , SArgMetaData(kDelimiter, CMQueryNodeValue::eString, true)
            , SArgMetaData(kLeaveOrig, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kNoChange, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kToUpper, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kCapNochange, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kCapAtSpaces, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kToLower, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kFCapLower, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kFlowerNochange, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kCapAtSpacesPunct, CMQueryNodeValue::eBool, false)
        };
    return s_args;
    }

    const TArgumentsVector& GetDelimiterArgs()
    {
        static const TArgumentsVector s_args{
                SArgMetaData(kTextLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kDelLeft, CMQueryNodeValue::eString, false)
            ,   SArgMetaData(kDigitsLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kLetterLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kRmvLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kTextRight, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kDelRight, CMQueryNodeValue::eString, false)
            ,   SArgMetaData(kDigitsRight, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kLetterRight, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kRmvRight, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetParseArgs()
    {
        static const TArgumentsVector s_args{
                SArgMetaData(kFromField, CMQueryNodeValue::eString, false)
            ,   SArgMetaData(kToField, CMQueryNodeValue::eString, false)
            ,   SArgMetaData(kExistingText, CMQueryNodeValue::eString, false)
            ,   SArgMetaData(kDelimiter, CMQueryNodeValue::eString, true)
            ,   SArgMetaData(kJustAfter, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kStartAt, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kTextLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kDelLeft, CMQueryNodeValue::eString, false)
            ,   SArgMetaData(kDigitsLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kLetterLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kRmvLeft, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kUpTo, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kInclude, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kTextRight, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kDelRight, CMQueryNodeValue::eString, false)
            ,   SArgMetaData(kDigitsRight, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kLetterRight, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kRmvRight, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kRmvParse, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kCaseInsensParse, CMQueryNodeValue::eBool, false)
            ,   SArgMetaData(kWholeWordParse, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetRmvOutsideArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kBefore, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kAfter, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kCaseInsensRmv, CMQueryNodeValue::eBool, true)
            , SArgMetaData(kWholeWordRmv, CMQueryNodeValue::eBool, true)
        };
        return s_args;
    }

    const TArgumentsVector& GetOptionalArgs()
    {
        static const TArgumentsVector s_optargs{
              SArgMetaData(kFeatType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kFeatQual, CMQueryNodeValue::eString, false)
            , SArgMetaData(kUpdatemRNA, CMQueryNodeValue::eBool, true)
            , SArgMetaData(kRNAType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kncRNAClass, CMQueryNodeValue::eString, false)
            , SArgMetaData(kStrCommField, CMQueryNodeValue::eString, true)
            , SArgMetaData(kMoveToNote, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kObeyStopCodon, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kRmvOverlapGene, CMQueryNodeValue::eBool, true)
            , SArgMetaData(kGapSizeCnv, CMQueryNodeValue::eString, true)
        };
        return s_optargs;
    }

    const TArgumentsVector& GetBsrcArgs()
    {
        static const TArgumentsVector s_bsrcargs{
              SArgMetaData(kBsrcText, CMQueryNodeValue::eBool, true)
            , SArgMetaData(kBsrcTax, CMQueryNodeValue::eBool, true)
            , SArgMetaData(kBsrcLoc, CMQueryNodeValue::eBool, true)
            , SArgMetaData(kBsrcOrigin, CMQueryNodeValue::eBool, true)
        };
        return s_bsrcargs;
    }

    const TArgumentsVector& GetDualBsrcArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kBsrcTextFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kBsrcTaxFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kBsrcTextTo, CMQueryNodeValue::eString, false)
            , SArgMetaData(kBsrcTaxTo, CMQueryNodeValue::eString, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetEditMolinfoArgs()
    {
        static const TArgumentsVector s_args{
              SArgMetaData(kMolFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kMolTo, CMQueryNodeValue::eString, false)
            , SArgMetaData(kTechFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kTechTo, CMQueryNodeValue::eString, false)
            , SArgMetaData(kComplFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kComplTo, CMQueryNodeValue::eString, false)
            , SArgMetaData(kClassFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kClassTo, CMQueryNodeValue::eString, false)
            , SArgMetaData(kTopFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kTopTo, CMQueryNodeValue::eString, false)
            , SArgMetaData(kStrandFrom, CMQueryNodeValue::eString, false)
            , SArgMetaData(kStrandTo, CMQueryNodeValue::eString, false)
            , SArgMetaData(kApplySeq, CMQueryNodeValue::eString, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetConvertSetArgs()
    {
        static const TArgumentsVector s_args{ 
              SArgMetaData(kFromField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kToField, CMQueryNodeValue::eString, false)
            , SArgMetaData(kInconsTaxa, CMQueryNodeValue::eBool, true)
        };
        return s_args;
    }

    const TArgumentsVector& GetConvertFeatArgs()
    {
        static const TArgumentsVector s_args{
            SArgMetaData(kFromFeat, CMQueryNodeValue::eString, false)
            , SArgMetaData(kToFeat, CMQueryNodeValue::eString, false)
            , SArgMetaData(kConvText, CMQueryNodeValue::eString, false)
            , SArgMetaData(kLeaveFeat, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kRemmRNA, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kRemGene, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kRemTranscriptid, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kSelText, CMQueryNodeValue::eString, false)
            , SArgMetaData(kChoices, CMQueryNodeValue::eString, false)
            , SArgMetaData(kCreateOnProtein, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetSetExceptionArgs()
    {
        static const TArgumentsVector s_args{
            SArgMetaData(kFeatType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kExceptionType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kWhereFeatProduct, CMQueryNodeValue::eString, false)
            , SArgMetaData(kMoveExplanation, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetRemoveGeneXrefArgs()
    {
        static const TArgumentsVector s_args{
            SArgMetaData(kFeatType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kSuppressingType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kNecessaryType, CMQueryNodeValue::eString, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetRmvDuplFeaturesArgs()
    {
        static const TArgumentsVector s_args{
            SArgMetaData(kFeatType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kCheckPartials, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kCaseSens, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kRemoveProteins, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetUpdateRpldECNumbersArgs()
    {
        static const TArgumentsVector s_args{
            SArgMetaData(kDelImproper, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kDelUnrecog, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kDelMultRepl, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }

    const TArgumentsVector& GetApplyTableArgs()
    {
        static const TArgumentsVector s_args{
            SArgMetaData(kExistingText, CMQueryNodeValue::eString, false)
            , SArgMetaData(kDelimiter, CMQueryNodeValue::eString, true)
            , SArgMetaData(kRmvBlank, CMQueryNodeValue::eString, true)
        };
        return s_args;
    }

    const TArgumentsVector& GetRmvDbxrefArgs()
    {
        static const TArgumentsVector s_args{
            SArgMetaData(kFeatType, CMQueryNodeValue::eString, false)
            , SArgMetaData(kAllDbxref, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kIllegalDbxref, CMQueryNodeValue::eBool, false)
            , SArgMetaData(kMoveToNote, CMQueryNodeValue::eBool, false)
        };
        return s_args;
    }
}

END_NCBI_SCOPE

/* @} */
