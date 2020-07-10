/*  $Id: macro_fn_seq_constr.cpp 41958 2018-11-21 16:26:45Z asztalos $
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
 */

#include <ncbi_pch.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_fn_seq_constr.hpp>
/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(macro)
USING_SCOPE(objects);


//////////////////////////////////////////////////////////////////////////////   
/// class CMacroFunction_FeatStrandednessConstraint
/// Checks for the strandedness of all features
/// ISSTRAND_OF_FEATURES(strandedness) - accepts one parameter, a string corresponding to Feature-strandedness-constraint, except "any"
///
const char* CMacroFunction_FeatStrandednessConstraint::sm_FunctionName = "ISSTRAND_OF_FEATURES";

void CMacroFunction_FeatStrandednessConstraint::TheFunction()
{
    EFeature_strandedness_constraint strandedness = x_GetStrandednessFromName(m_Args[0]->GetString());

    // make sure that the iterator is a sequence iterator
    CObjectInfo oi = m_DataIter->GetEditedObject();
    CBioseq* bseq = CTypeConverter<CBioseq>::SafeCast(oi.GetObjectPtr());
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!bseq || !scope)
        return;

    bool match = x_Match(scope->GetBioseqHandle(*bseq), strandedness);
    m_Result->SetDataType(CMQueryNodeValue::eBool);
    m_Result->SetBool(match);
}

EFeature_strandedness_constraint 
CMacroFunction_FeatStrandednessConstraint::x_GetStrandednessFromName(const string& strand_name)
{
    EFeature_strandedness_constraint strand = eFeature_strandedness_constraint_any;
    if (NStr::EqualNocase(strand_name, "minus-only")) {
        strand = eFeature_strandedness_constraint_minus_only;
    } else if (NStr::EqualNocase(strand_name, "plus-only")) {
        strand = eFeature_strandedness_constraint_plus_only;
    } else if (NStr::EqualNocase(strand_name, "at-least-one-minus")) {
        strand = eFeature_strandedness_constraint_at_least_one_minus;
    } else if (NStr::EqualNocase(strand_name, "at-least-one-plus")) {
        strand = eFeature_strandedness_constraint_at_least_one_plus;
    } else if (NStr::EqualNocase(strand_name, "no-minus")) {
        strand = eFeature_strandedness_constraint_no_minus;
    } else if (NStr::EqualNocase(strand_name, "no-plus")) {
        strand = eFeature_strandedness_constraint_no_plus;
    }
    return strand;
}

bool CMacroFunction_FeatStrandednessConstraint::x_Match(const CBioseq_Handle& bsh, EFeature_strandedness_constraint strandedness)
{	
    if (strandedness == eFeature_strandedness_constraint_any)
        return false; // different 

    unsigned num_minus = 0, num_plus = 0;
    CFeat_CI feat(bsh, SAnnotSelector(CSeqFeatData::eSubtype_any));
    for ( ; feat; ++feat) {
        if (feat->GetLocation().GetStrand() == eNa_strand_minus) {
            num_minus++;
            if (strandedness == eFeature_strandedness_constraint_plus_only
                || strandedness == eFeature_strandedness_constraint_no_minus) {
                return false;
            } else if (strandedness == eFeature_strandedness_constraint_at_least_one_minus) {
                return true;
            }
        } else {
            num_plus++;
            if (strandedness == eFeature_strandedness_constraint_minus_only
                || strandedness == eFeature_strandedness_constraint_no_plus) {
                return false;
            } else if (strandedness == eFeature_strandedness_constraint_at_least_one_plus) {
                return true;
            }
        }
    }

    switch (strandedness) {
        case eFeature_strandedness_constraint_minus_only:
            if (num_minus > 0 && num_plus == 0) return true;
        case eFeature_strandedness_constraint_plus_only:
            if (num_plus > 0 && num_minus == 0)  return true;
        case eFeature_strandedness_constraint_at_least_one_minus:
            if (num_minus > 0)  return true;
        case eFeature_strandedness_constraint_at_least_one_plus:
            if (num_plus > 0)  return true;
        case eFeature_strandedness_constraint_no_minus:
            if (num_minus == 0) return true;
        case eFeature_strandedness_constraint_no_plus:
            if (num_plus == 0) return true;
        default: return false;
    }
    return false;
}

bool CMacroFunction_FeatStrandednessConstraint::x_ValidArguments() const
{
    return (m_Args.size() == 1 
        && m_Args[0]->GetDataType() == CMQueryNodeValue::eString 
        && !m_Args[0]->GetString().empty());
}


//////////////////////////////////////////////////////////////////////////////
/// class CMacroFunction_NumberOfFeatures
/// FEATS_ON_SEQ(feature_type | "all") - returns the number of features of type "feature_type" present on the sequence
/// When the argument is "any" or "all", it returns the total number of features (all kind)
/// 
const char* CMacroFunction_NumberOfFeatures::sm_FunctionName = "FEATS_ON_SEQ";

void CMacroFunction_NumberOfFeatures::TheFunction()
{
    CConstRef<CObject> obj = m_DataIter->GetScopedObject().object;
    CRef<CScope> scope = m_DataIter->GetScopedObject().scope;
    if (!obj || !scope)
        return;

    CBioseq_Handle bsh = m_DataIter->GetBioseqHandle();
    _ASSERT(bsh);
    if (bsh) {
        int count = s_GetFeatTypeCount(bsh, m_Args[0]->GetString());
        m_Result->SetInt(count);
    }
}

int CMacroFunction_NumberOfFeatures::s_GetFeatTypeCount(const CBioseq_Handle& bsh, const string& feat_type)
{
    // count all features on the sequence
    int count = 0;
    CSeqFeatData::ESubtype subtype = NMacroUtil::GetFeatSubtype(feat_type);
    if (subtype != CSeqFeatData::eSubtype_bad) {
        if (CSeqFeatData::GetTypeFromSubtype(subtype) != CSeqFeatData::e_Prot) {
            for (CFeat_CI it(bsh, SAnnotSelector(subtype)); it; ++it) {
                count++;
            }
        }
        else {
            CScope& scope = bsh.GetScope();
            for (CFeat_CI feat_it(bsh, SAnnotSelector(CSeqFeatData::e_Cdregion)); feat_it; ++feat_it) {
                if (feat_it->IsSetProduct()) {
                    CBioseq_Handle product = scope.GetBioseqHandle(feat_it->GetProduct());
                    for (CFeat_CI prot_it(product, SAnnotSelector(subtype)); prot_it; ++prot_it) {
                        count++;
                    }
                }
            }
        }
    }
   
    return count;
}

bool CMacroFunction_NumberOfFeatures::x_ValidArguments() const
{
    return (m_Args.size() == 1 && m_Args[0]->GetDataType() == CMQueryNodeValue::eString);
}

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */
