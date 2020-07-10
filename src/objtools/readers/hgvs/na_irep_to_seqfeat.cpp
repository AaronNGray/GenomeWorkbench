#include <ncbi_pch.hpp>
#include <objmgr/feat_ci.hpp>
#include <objtools/readers/hgvs/na_irep_to_seqfeat.hpp>
#include <objtools/readers/hgvs/irep_to_seqfeat_errors.hpp>
#include <objtools/readers/hgvs/irep_to_seqfeat_common.hpp>
#include <objtools/readers/hgvs/variation_ref_utils.hpp>
#include <objtools/readers/hgvs/special_irep_to_seqfeat.hpp>
#include <objtools/readers/hgvs/post_process.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

USING_SCOPE(NHgvsTestUtils);


CIUPACna CHgvsNaIrepReader::x_CreateIUPACna(const string& na_seq) const
{
    string dna_seq = na_seq;
    NStr::ToUpper(dna_seq);
    NStr::ReplaceInPlace(dna_seq, "U", "T");
    return CIUPACna(dna_seq);
}


CRef<CSeq_literal> CHgvsNaIrepReader::x_CreateNtSeqLiteral(const string& raw_seq) const
{
    auto lit = x_CreateNtSeqLiteral(raw_seq.size());
    //lit->SetSeq_data().SetIupacna(CIUPACna(raw_seq));
    lit->SetSeq_data().SetIupacna(x_CreateIUPACna(raw_seq));
    return lit;
}


CRef<CSeq_literal> CHgvsNaIrepReader::x_CreateNtSeqLiteral(TSeqPos length) const 
{
    auto lit = Ref(new CSeq_literal);
    lit->SetLength(length);
    return lit;
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateNaIdentityVarref(const CNaIdentity& identity,
                                                                 const CVariation_ref::EMethod_E method) const
{
    string nucleotide = identity.IsSetNucleotide() ? 
                        identity.GetNucleotide() : "";

    return x_CreateNaIdentityVarref(identity.GetLoc(), nucleotide, method);

}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateNaIdentityVarref(const CNtLocation& nt_loc,
                                                                 const string& nucleotide,
                                                                 const CVariation_ref::EMethod_E method) const
{
    auto nt_literal = Ref(new CSeq_literal());

    auto length = nucleotide.empty() ? 1 : nucleotide.size();

    nt_literal->SetLength(length);
    if (!nucleotide.empty()) {
        nt_literal->SetSeq_data().SetIupacna(x_CreateIUPACna(nucleotide));
    }
    auto offset = CIntronOffsetHelper::GetIntronOffset(nt_loc.GetSite());
    return  g_CreateIdentity(nt_literal, method, offset);
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateSubstVarref(const CNtLocation& nt_loc, 
                                                          const string& initial_nt,
                                                          const string& final_nt,
                                                          const CVariation_ref::EMethod_E method) const 
{

    if (!initial_nt.empty() &&
        (initial_nt.size() != final_nt.size())) {
// LCOV_EXCL_START - This error should never arise. Such cases are interpreted as Indels.
        string err_msg = "Invalid nucleotide substitution.";
        err_msg += " Reference subsequence and variant subsequence differ in length";
        NCBI_THROW(CVariationIrepException, eInvalidVariation, err_msg);
// LCOV_EXCL_STOP
    }


    auto var_ref = Ref(new CVariation_ref());
    auto& var_set = var_ref->SetData().SetSet();
    var_set.SetType(CVariation_ref::TData::TSet::eData_set_type_package);

    CRef<CDelta_item> offset;
    if (nt_loc.IsSite()) {
        offset = CIntronOffsetHelper::GetIntronOffset(nt_loc.GetSite());
    }

    { 

        CSeq_data result;
        result.SetIupacna(x_CreateIUPACna(final_nt));

        CRef<CVariation_ref> subvar_ref;
        if (final_nt.size() > 1) {
            subvar_ref = g_CreateMNP(result,
                                     final_nt.size(),
                                     method,
                                     offset);
        } else {
            subvar_ref = g_CreateSNV(result, method, offset);
        }
        var_set.SetVariations().push_back(subvar_ref);
    }

    {
        CRef<CSeq_literal> initial;
        if (initial_nt.empty() &&
            nt_loc.IsSite()) {
            initial = x_CreateNtSeqLiteral(1); // Single, unknown residue
        } else {
            initial = x_CreateNtSeqLiteral(initial_nt);
        }
        auto subvar_ref = g_CreateIdentity(initial, m_MethodUnknown, offset);
        var_set.SetVariations().push_back(subvar_ref);
    }
    return var_ref;
}



CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateSubstVarref(const CNaSub& sub,
                                                          const CVariation_ref::EMethod_E method) const
{
    const auto& nt_loc = sub.GetLoc();
    const auto initial_nt = sub.GetInitial();
    const auto final_nt = sub.GetFinal();

    return x_CreateSubstVarref(nt_loc, 
                             initial_nt,
                             final_nt,
                             method);
}




CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateDeletionVarref(const CDeletion& del,
                                                               const CVariation_ref::EMethod_E method) const
{
    const auto& nt_loc = del.GetLoc().GetNtloc();

    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc);


    // The case where the HGVS expression includes the deleted sequence explicitly
    if (del.IsSetRaw_seq()) {
        auto var_ref = Ref(new CVariation_ref());
        auto& var_set = var_ref->SetData().SetSet();
        var_set.SetType(CVariation_ref::TData::TSet::eData_set_type_package);
        {
            auto subvar_ref = g_CreateDeletion(method, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        }
        {
            auto raw_seq = x_CreateNtSeqLiteral(del.GetRaw_seq());
            CRef<CVariation_ref> subvar_ref = g_CreateIdentity(raw_seq, m_MethodUnknown, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        }
        return var_ref;
    }
   
    // The case where the HGVS expression does not include the deleted sequence explicitly
    return g_CreateDeletion(method, start_offset, stop_offset);
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateInsertionVarref(const CInsertion& ins,
                                                                const CVariation_ref::EMethod_E method) const
{
    const auto& nt_int = ins.GetInt().GetNtint();

    // Construct the CDeltaItem instances that specify the intronic offsets 
    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_int);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_int);

    // The inserted sequence is specified by a CSeq_literal instance
    auto raw_seq = x_CreateNtSeqLiteral(ins.GetSeqinfo().GetRaw_seq());

    return g_CreateInsertion(*raw_seq, method, start_offset, stop_offset);
}


bool CHgvsNaIrepReader::x_LooksLikePolymorphism(const CDelins& delins) const 
{
    // If the deleted and inserted sequences are the same size, the 
    // variation is a substitution/polymorphism.
    if (delins.IsSetDeleted_raw_seq() && 
        delins.GetInserted_seq_info().IsRaw_seq() &&
        delins.GetDeleted_raw_seq().size() ==
        delins.GetInserted_seq_info().GetRaw_seq().size()) {
        return true;
    }

    // If the deletion is restricted to a single site 
    // and the insertion involves a single nucleotide,
    // this variation is an SNP
    if (delins.GetLoc().GetNtloc().IsSite() &&
        delins.GetInserted_seq_info().IsRaw_seq() &&
        delins.GetInserted_seq_info().GetRaw_seq().size() == 1) {
        return true;
    }

    return false;
}
                                        

CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateDelinsVarref(const CDelins& delins,
                                                             const CVariation_ref::EMethod_E method) const
{



    const auto& nt_loc = delins.GetLoc().GetNtloc();

    if (delins.IsSetDeleted_raw_seq()) {
        return x_CreateDelinsVarref(nt_loc, 
                                    delins.GetDeleted_raw_seq(), 
                                    delins.GetInserted_seq_info().GetRaw_seq(),
                                    method);
    }

/*
    // Construct the CDeltaItem instances specifying the intronic offsets in the limits 
    // of the deleted interval.
    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc);

    // The insertion is specified by a CSeq_literal instance
    auto inserted_seq = x_CreateNtSeqLiteral(delins.GetInserted_seq_info().GetRaw_seq());


    // The following code handles the case where the HGVS expression contains the deleted sequence explicitly
    if (delins.IsSetDeleted_raw_seq()) {
        auto var_ref = Ref(new CVariation_ref());
        auto& var_set = var_ref->SetData().SetSet();
        var_set.SetType(CVariation_ref::TData::TSet::eData_set_type_package);
        {
            CRef<CVariation_ref> subvar_ref = g_CreateDelins(*inserted_seq, method, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        }
        {
            auto deleted_seq = x_CreateNtSeqLiteral(delins.GetDeleted_raw_seq());
            CRef<CVariation_ref> subvar_ref = g_CreateIdentity(deleted_seq, m_MethodUnknown, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        }
        return var_ref;
    }
*/
    // else the HGVS expression does not show the deleted sequence explicitly
 
    // Construct the CDeltaItem instances specifying the intronic offsets in the limits 
    // of the deleted interval.
    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc);
    // The insertion is specified by a CSeq_literal instance
    auto inserted_seq = x_CreateNtSeqLiteral(delins.GetInserted_seq_info().GetRaw_seq());
    return g_CreateDelins(*inserted_seq, m_MethodUnknown, start_offset, stop_offset);
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateDelinsVarref(const CNtLocation& nt_loc,
                                                             const string& deleted_seq,
                                                             const string& inserted_seq,
                                                             const CVariation_ref::EMethod_E method) const
{

    // Construct the CDeltaItem instances specifying the intronic offsets in the limits 
    // of the deleted interval.
    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc);

    // The insertion is specified by a CSeq_literal instance
    auto inserted_lit = x_CreateNtSeqLiteral(inserted_seq);

    auto var_ref = Ref(new CVariation_ref());
    auto& var_set = var_ref->SetData().SetSet();
    var_set.SetType(CVariation_ref::TData::TSet::eData_set_type_package);
    {
            CRef<CVariation_ref> subvar_ref = g_CreateDelins(*inserted_lit, method, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
    }
    {
            auto deleted_lit = x_CreateNtSeqLiteral(deleted_seq);
            CRef<CVariation_ref> subvar_ref = g_CreateIdentity(deleted_lit, m_MethodUnknown, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
    }
    return var_ref;


}

CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateDuplicationVarref(const CDuplication& dup, 
                                                                  const CVariation_ref::EMethod_E method) const
{

    const auto& nt_loc = dup.GetLoc().GetNtloc();

    // Construct the CDelta_item instances specifying the intronic offsets in the limits 
    // of the duplicated interval.
    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc);

    // A HGVS duplication expression may include the reference subsequence that is 
    // duplicated. The following code handles this case.
    if (dup.IsSetRaw_seq()) {
        auto var_ref = Ref(new CVariation_ref());
        auto& var_set = var_ref->SetData().SetSet();
        var_set.SetType(CVariation_ref::TData::TSet::eData_set_type_package);
        {
            CRef<CVariation_ref> subvar_ref = g_CreateDuplication(method, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        }
        {
            auto duplicated_seq = x_CreateNtSeqLiteral(dup.GetRaw_seq());
            auto subvar_ref = g_CreateIdentity(duplicated_seq, m_MethodUnknown, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        }
        return var_ref;
    } 
    
    // method needs to be incorporated into this   
    return g_CreateDuplication(method, start_offset, stop_offset);
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateInversionVarref(const CInversion& inv,
                                                                const CVariation_ref::EMethod_E method) const
{
    // Get the inverted interval
    const auto& nt_int = inv.GetNtint();

    // Construct the CDelta_item instances specifying the intronic offsets in the interval limits.
    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_int);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_int);

    // The HGVS inversion expression may include the reference sequence that is inverted or 
    // the size of the inverted sequence. The following lines of code handles these cases.
    if (inv.IsSetRaw_seq() || inv.IsSetSize()) {
        auto var_ref = Ref(new CVariation_ref());
        auto& var_set = var_ref->SetData().SetSet();
        var_set.SetType(CVariation_ref::TData::TSet::eData_set_type_package);
        {
            CRef<CVariation_ref> subvar_ref = g_CreateInversion(method, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        }
        if (inv.IsSetRaw_seq()) {
            auto inverted_seq = x_CreateNtSeqLiteral(inv.GetRaw_seq());
            CRef<CVariation_ref> subvar_ref = g_CreateIdentity(inverted_seq, m_MethodUnknown, start_offset, stop_offset);
            var_set.SetVariations().push_back(subvar_ref);
        } else if (inv.IsSetSize()) {
            auto inversion_size = x_CreateNtSeqLiteral(inv.GetSize());
            const bool enforce_assert = true;
            CRef<CVariation_ref> subvar_ref = g_CreateIdentity(inversion_size, m_MethodUnknown, start_offset, stop_offset, enforce_assert);
            var_set.SetVariations().push_back(subvar_ref);
        }
        return var_ref;
    } 
   
    // The HGVS expression contains neither raw sequence in the inverted region 
    // nor the size of the inverted sequence.
    return g_CreateInversion(m_MethodUnknown, start_offset, stop_offset);
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateConversionVarref(const CConversion& conv,
                                                                 const CVariation_ref::EMethod_E method) const
{
    {   // Don't have a Seq-feat representation for the case where the conversion origin 
        // involves intronic offsets. Throw an exception in this case.
        const auto& replacement_int = conv.GetOrigin();
        const CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(replacement_int);
        const CRef<CDelta_item> stop_offset  = CIntronOffsetHelper::GetStopIntronOffset(replacement_int);

        if (start_offset || stop_offset) {
            string err_msg = "Nucleotide conversions with origin intervals that begin or end in an intron are not currently supported.";
            err_msg += " Please report to variation-services@ncbi.nlm.nih.gov";
            NCBI_THROW(CVariationIrepException, eUnsupported, err_msg);
        }
    }

    // Construct the Seq-loc for the conversion location. 
    // The code currently assumes that the location is an interval.
    const auto& nt_loc = conv.GetLoc();

    string id_string;
    CSequenceVariant::TSeqtype seq_type;

    const CNtIntLimit& interval_start = conv.GetOrigin().GetInt().GetStart();
    if (interval_start.IsSite()) {
        id_string = interval_start.GetSite().GetSeqid();
        seq_type = interval_start.GetSite().GetSeqtype();
    }
    else 
    if (interval_start.IsRange()) {
        id_string = interval_start.GetRange().GetStart().GetSeqid();
        seq_type = interval_start.GetRange().GetStart().GetSeqtype();
    }


    CConstRef<CSeq_id> seq_id = m_IdResolver->GetAccessionVersion(id_string).GetSeqId();

    CRef<CSeq_loc> seq_loc = CNaSeqlocHelper::CreateSeqloc(*seq_id,
                                                           conv.GetOrigin(),
                                                           seq_type,
                                                           m_Scope);
    
    // Compute the intronic offsets for the interval limits.
    CRef<CDelta_item> start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc);
    CRef<CDelta_item> stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc);

    CRef<CVariation_ref> var_ref = g_CreateConversion(*seq_loc, method, start_offset, stop_offset);
    return var_ref;
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateSSRVarref(const CRepeat& ssr,
                                                          const CVariation_ref::EMethod_E method) const
{
    CRef<CSeq_literal> seq_literal;
    if (ssr.IsSetRaw_seq()) {
        seq_literal = Ref(new CSeq_literal());
        const auto raw_seq = ssr.GetRaw_seq();
        seq_literal->SetLength(raw_seq.size());
        seq_literal->SetSeq_data().SetIupacna(x_CreateIUPACna(raw_seq));
    }

    const auto& nt_loc = ssr.GetLoc().GetNtloc();

    auto start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc);

    auto stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc);

    auto delta = CDeltaHelper::CreateSSR(ssr.GetCount(), seq_literal);
    CRef<CVariation_ref> var_ref = g_CreateMicrosatellite(delta, method, start_offset, stop_offset);
    return var_ref;
}


CRef<CVariation_ref> CHgvsNaIrepReader::x_CreateVarref(const string& var_name,
                                                       const CSimpleVariant& simple_var) const
{
    CRef<CVariation_ref> var_ref;
    const auto& var_type = simple_var.GetType();


    CVariation_ref::EMethod_E method = (simple_var.IsSetFuzzy() && simple_var.GetFuzzy()) ?
        CVariation_ref::eMethod_E_computational :
        CVariation_ref::eMethod_E_unknown;

    switch (var_type.Which()) {
       default:
// LCOV_EXCL_START - Should not arise
           NCBI_THROW(CVariationIrepException, eUnknownVariation, "Unknown variation type");
// LCOV_EXCL_STOP
       case CSimpleVariant::TType::e_Na_identity:
           var_ref = x_CreateNaIdentityVarref(var_type.GetNa_identity(), method);
           break;
       case CSimpleVariant::TType::e_Na_sub: 
           { // Check if the substitution actually describes an identity or a delins
               const auto& na_sub = var_type.GetNa_sub();
               const auto initial_nt = na_sub.GetInitial();
               const auto final_nt = na_sub.GetFinal();
               if (final_nt == initial_nt) {
                   var_ref = x_CreateNaIdentityVarref(na_sub.GetLoc(),
                                                      initial_nt,
                                                      method);
                   break;
               } else 
               if (initial_nt.size() != final_nt.size()) {
                  var_ref = x_CreateDelinsVarref(na_sub.GetLoc(),
                                                 initial_nt,
                                                 final_nt,
                                                 method);
                  break; 
               }
           }
           // Not identity - non-trivial substitution
           var_ref = x_CreateSubstVarref(var_type.GetNa_sub(), method);
           break;
       case CSimpleVariant::TType::e_Dup:
           var_ref = x_CreateDuplicationVarref(var_type.GetDup(), method);
           break;
       case CSimpleVariant::TType::e_Del:
           var_ref = x_CreateDeletionVarref(var_type.GetDel(), method);
           break;
       case CSimpleVariant::TType::e_Ins:
           var_ref = x_CreateInsertionVarref(var_type.GetIns(), method);
           break;
       case CSimpleVariant::TType::e_Delins:
           // If the deleted subsequence and the insertion are the 
           // same size relabel as a substitution.
           if (x_LooksLikePolymorphism(var_type.GetDelins())) {
               const auto& delins = var_type.GetDelins();

               const auto& deleted = delins.IsSetDeleted_raw_seq() ?
                                     delins.GetDeleted_raw_seq() : 
                                     "";

               var_ref = x_CreateSubstVarref(delins.GetLoc().GetNtloc(),
                                           deleted,
                                           delins.GetInserted_seq_info().GetRaw_seq(),
                                           method);
               break;
           }
           var_ref = x_CreateDelinsVarref(var_type.GetDelins(), method);
           break;
       case CSimpleVariant::TType::e_Inv:
           var_ref = x_CreateInversionVarref(var_type.GetInv(), method);
           break;
       case CSimpleVariant::TType::e_Conv:
           var_ref = x_CreateConversionVarref(var_type.GetConv(), method);
           break;
       case CSimpleVariant::TType::e_Repeat:
           var_ref = x_CreateSSRVarref(var_type.GetRepeat(), method);
           break;
    }

    var_ref->SetName(var_name);

    return var_ref;
}



CRef<CSeq_feat> CHgvsNaIrepReader::x_CreateSimpleVariantFeat(const string& var_name,
                                                             const string& id_string,
                                                             const CSequenceVariant::TSeqtype& seq_type,
                                                             const CSimpleVariant& simple_var) const 
{
    auto seq_feat = Ref(new CSeq_feat());
    CRef<CVariation_ref> var_ref = x_CreateVarref(var_name, simple_var);
    seq_feat->SetData().SetVariation(*var_ref);

    auto seq_id = m_IdResolver->GetAccessionVersion(id_string).GetSeqId();

    CRef<CSeq_loc> seq_loc = CNaSeqlocHelper::CreateSeqloc(*seq_id, 
                                                           simple_var,
                                                           seq_type,
                                                           m_Scope);
    seq_feat->SetLocation(*seq_loc);

    return seq_feat;
}

// Could make CreateSeqfeat a base class method
// x_CreateSimpleVariantFeat could be a virtual method, which has to be implemented in both cases, maybe
CRef<CSeq_feat> CHgvsNaIrepReader::CreateSeqfeat(const CVariantExpression& variant_expr) const 
{
    const auto& sequence_variant = variant_expr.GetSequence_variant();

    if (sequence_variant.IsSetComplex()) {
        string message;
        if (sequence_variant.GetComplex() == CSequenceVariant::eComplex_chimera) {
            message = "Chimeras ";
        } else if (sequence_variant.GetComplex() == CSequenceVariant::eComplex_mosaic) {
            message = "Mosaics ";
        }
        message += "are not currently supported.";
        message += " Please report to variation-services@ncbi.nlm.nih.gov";
        NCBI_THROW(CVariationIrepException, eUnsupported, message);
    }


    CRef<CVariant> subvariant = variant_expr.GetSequence_variant().GetSubvariants().front();
    const auto seq_type = variant_expr.GetSequence_variant().GetSeqtype();

    if (seq_type != eVariantSeqType_c &&
        seq_type != eVariantSeqType_g &&
        seq_type != eVariantSeqType_r &&
        seq_type != eVariantSeqType_m &&
        seq_type != eVariantSeqType_n) {
        NCBI_THROW(CVariationIrepException, eInvalidSeqType, "Nucleotide sequence expected"); // LCOV_EXCL_LINE
    }


    if (subvariant->IsSpecial()) {
        CConstRef<CSeq_id> seq_id = m_IdResolver->GetAccessionVersion(variant_expr.GetReference_id()).GetSeqId();
        ESpecialVariant special_type = static_cast<ESpecialVariant>(subvariant->GetSpecial());
        return g_CreateSpecialSeqfeat(special_type, *seq_id, variant_expr.GetInput_expr());
    }

   
    const CSimpleVariant& simple_variant = subvariant->GetSimple();

    CRef<CSeq_feat> unnormalized_seqfeat = x_CreateSimpleVariantFeat(variant_expr.GetInput_expr(),
                                                                     variant_expr.GetReference_id(),
                                                                     seq_type,
                                                                     simple_variant);
    
    return g_NormalizeVariationSeqfeat(*unnormalized_seqfeat, &m_Scope);
}



// Create the CSeq_loc instance for different variant types
CRef<CSeq_loc> CNaSeqlocHelper::CreateSeqloc(const CSeq_id& seq_id,
                                             const CSimpleVariant& simple_var,
                                             const CSequenceVariant::TSeqtype& seq_type,
                                             CScope& scope)
{
    CRef<CSeq_loc> seq_loc;
    const auto& var_type = simple_var.GetType();
    
    switch (var_type.Which()) {
        default:
// LCOV_EXCL_START - Should never arise
            NCBI_THROW(CVariationIrepException, eUnknownVariation, "Unsupported variation type");
// LCOV_EXCL_STOP
        case CSimpleVariant::TType::e_Na_identity:  // No change
            seq_loc = CreateSeqloc(seq_id,
                                   var_type.GetNa_identity().GetLoc(),
                                   seq_type,
                                   scope);
            break;
        case CSimpleVariant::TType::e_Na_sub: // Nucleotide substitution - SNP or MNP 
            seq_loc = CreateSeqloc(seq_id, 
                                   var_type.GetNa_sub().GetLoc(),
                                   seq_type,
                                   scope);
            break;
        case CSimpleVariant::TType::e_Dup: // Duplication     
            seq_loc = CreateSeqloc(seq_id, 
                                   var_type.GetDup().GetLoc().GetNtloc(),
                                   seq_type, 
                                   scope);
            break; 
        case CSimpleVariant::TType::e_Del: // Deletion    
            seq_loc = CreateSeqloc(seq_id, 
                                   var_type.GetDel().GetLoc().GetNtloc(),
                                   seq_type, 
                                   scope);
            break; 
        case CSimpleVariant::TType::e_Ins: // Insertion
            seq_loc = x_CreateSeqloc(seq_id,
                                     var_type.GetIns().GetInt().GetNtint(),
                                     seq_type,
                                     scope);
            break;
        case CSimpleVariant::TType::e_Delins: // Deletion-insertion    
            seq_loc = CreateSeqloc(seq_id, 
                                   var_type.GetDelins().GetLoc().GetNtloc(),
                                   seq_type, 
                                   scope);
            break; 

        case CSimpleVariant::TType::e_Inv:  // Inversion    
            seq_loc = x_CreateSeqloc(seq_id, 
                                    var_type.GetInv().GetNtint(),
                                    seq_type, 
                                    scope);
            break; 

        case CSimpleVariant::TType::e_Conv: // Conversion
            seq_loc = CreateSeqloc(seq_id, 
                                   var_type.GetConv().GetLoc(),
                                   seq_type, 
                                   scope);
            break;
       case CSimpleVariant::TType::e_Repeat: // SSR
            seq_loc = CreateSeqloc(seq_id,
                                   var_type.GetRepeat().GetLoc().GetNtloc(),
                                   seq_type,
                                   scope);
            break;
    }
    return seq_loc;
}



// Create a CSeq_loc instance given a CSeq_id, a CNtLocation object, and the 
// sequence type (CDS, genomic, etc).
CRef<CSeq_loc> CNaSeqlocHelper::CreateSeqloc(const CSeq_id& seq_id,
                                             const CNtLocation& nt_loc,
                                             const CSequenceVariant::TSeqtype& seq_type,
                                             CScope& scope)
{
    if (nt_loc.IsSite()) {
        return x_CreateSeqloc(seq_id,
                              nt_loc.GetSite(),
                              seq_type,
                              scope);
    }

    
    if (nt_loc.IsRange()) {
        return x_CreateSeqloc(seq_id,
                              nt_loc.GetRange(),
                              seq_type,
                              scope);
    }


    if (!nt_loc.IsInt()) {
// LCOV_EXCL_START - Should never arise
        NCBI_THROW(CVariationIrepException, eInvalidLocation, "Invalid nucleotide sequence location");
// LCOV_EXCL_STOP
    }

    return x_CreateSeqloc(seq_id,
                          nt_loc.GetInt(),
                          seq_type,
                          scope);
}


CRef<CSeq_loc> CNaSeqlocHelper::x_CreateSeqloc(const CSeq_id& seq_id,
                                               const CNtSite& nt_site,
                                               const CSequenceVariant::TSeqtype& seq_type,
                                               CScope& scope)
{
    TSeqPos site_index;
    const bool know_site = x_ComputeSiteIndex(seq_id,
                                              nt_site,
                                              seq_type,
                                              scope,
                                              site_index);
    auto seq_loc = Ref(new CSeq_loc());
    if (!know_site) {
        seq_loc->SetEmpty().Assign(seq_id);
        return seq_loc;
    }

    const ENa_strand strand = x_GetStrand(nt_site);
    seq_loc->SetPnt().SetPoint(site_index);
    seq_loc->SetPnt().SetId().Assign(seq_id);
    seq_loc->SetPnt().SetStrand(strand);

    if (nt_site.IsSetFuzzy() &&
        nt_site.GetFuzzy()) {
        seq_loc->SetPnt().SetFuzz().SetLim(CInt_fuzz::eLim_unk);
    }

    return seq_loc;
}



CRef<CSeq_loc> CNaSeqlocHelper::x_CreateSeqloc(const CSeq_id& seq_id,
                                               const CNtSiteRange& nt_range,
                                               const CSequenceVariant::TSeqtype& seq_type,
                                               CScope& scope)
{
    TSeqPos site_index;
    bool know_site = x_ComputeSiteIndex(seq_id, 
                                        nt_range,
                                        seq_type,
                                        scope,
                                        site_index);

    auto seq_loc = Ref(new CSeq_loc());
    if (!know_site) {
// LCOV_EXCL_START - Not expected to arise
        seq_loc->SetEmpty().Assign(seq_id);
        return seq_loc;
// LCOV_EXCL_STOP
    }

    ENa_strand strand = x_GetStrand(nt_range);
    seq_loc->SetPnt().SetPoint(site_index);
    seq_loc->SetPnt().SetId().Assign(seq_id);
    seq_loc->SetPnt().SetStrand(strand);

    auto fuzz = x_CreateIntFuzz(seq_id,
                                nt_range,
                                seq_type,
                                scope);

    if (fuzz.NotNull()) {
        seq_loc->SetPnt().SetFuzz(*fuzz);  
    }    

    return seq_loc;
}


ENa_strand CNaSeqlocHelper::x_GetStrand(const CNtSite& nt_site)
{
    return (nt_site.GetStrand_minus() ? eNa_strand_minus : eNa_strand_plus);
}


ENa_strand CNaSeqlocHelper::x_GetStrand(const CNtSiteRange& nt_range)
{
    ENa_strand strand = eNa_strand_unknown;
    return x_GetStrand(nt_range.GetStart());
}


ENa_strand CNaSeqlocHelper::x_GetStrand(const CNtInterval& nt_int) 
{
    if (nt_int.GetStart().IsSite()) {
        return x_GetStrand(nt_int.GetStart().GetSite());
    }

    // else has to be range
    return x_GetStrand(nt_int.GetStart().GetRange());
}



CRef<CSeq_loc> CNaSeqlocHelper::x_CreateSeqloc(const CSeq_id& seq_id,
                                               const CNtInterval& nt_int,
                                               const CSequenceVariant::TSeqtype& seq_type,
                                               CScope& scope)
{

    if ( !nt_int.IsSetStart() || !nt_int.IsSetStop() ) {
        NCBI_THROW(CVariationIrepException, eInvalidInterval, "Undefined interval limits"); // LCOV_EXCL_LINE
    }                                                                                       // Not expected to arise


    auto seq_loc = Ref(new CSeq_loc());
    seq_loc->SetInt().SetId().Assign(seq_id);
   
    // Attempt to compute the bioseq site index for the beginning of the interval.
    TSeqPos start_index;
    bool know_start = x_ComputeSiteIndex(seq_id,
                                         nt_int.GetStart(),
                                         seq_type,
                                         scope,
                                         start_index);

    // Attempt to compute the bioseq site index for the end of the interval
    TSeqPos stop_index;
    bool know_stop = x_ComputeSiteIndex(seq_id, 
                                        nt_int.GetStop(),
                                        seq_type,
                                        scope,
                                        stop_index);

    if ( (know_start && know_stop) && (start_index > stop_index) ) {
        string err_msg = "Reversed interval limits";
        ERR_POST(Warning << err_msg);
        swap(start_index, stop_index);
    }

    if (know_start) { 
        seq_loc->SetInt().SetFrom(start_index);
        // The following code handles the cases where the start site contains 
        // some uncertainty, or "fuzziness".
        if (nt_int.GetStart().IsSite()) {
            if(nt_int.GetStart().GetSite().IsSetFuzzy() &&
               nt_int.GetStart().GetSite().GetFuzzy()) { 
                seq_loc->SetInt().SetFuzz_from().SetLim(CInt_fuzz::eLim_unk);
            }
        }
        else if (nt_int.GetStart().IsRange() &&
                 !nt_int.GetStart().GetRange().GetStart().IsSetOffset() &&
                 !nt_int.GetStart().GetRange().GetStop().IsSetOffset()) {

            CRef<CInt_fuzz> fuzz_from = CNaSeqlocHelper::x_CreateIntFuzz(seq_id,
                                                                         nt_int.GetStart().GetRange(),
                                                                         seq_type,
                                                                         scope);
            seq_loc->SetInt().SetFuzz_from(*fuzz_from);  
        } 
        else if (nt_int.GetStart().IsRange() &&
                 nt_int.GetStart().GetRange().GetStart().GetBase().IsUnknown()) {
            seq_loc->SetInt().SetFuzz_from().SetLim(CInt_fuzz::eLim_tl);
        }
    }
    else { // Could not determine start site
        seq_loc->SetInt().SetFrom(kInvalidSeqPos);
        seq_loc->SetInt().SetFuzz_from().SetLim(CInt_fuzz::eLim_other);
    }


    if (know_stop) { 
        seq_loc->SetInt().SetTo(stop_index);
        // The following code handles the cases where the stop site contains 
        // some uncertainty, or "fuzziness".
        if (nt_int.GetStop().IsSite()) {
            if(nt_int.GetStop().GetSite().IsSetFuzzy() &&
               nt_int.GetStop().GetSite().GetFuzzy()) { 
                seq_loc->SetInt().SetFuzz_to().SetLim(CInt_fuzz::eLim_unk);
            }
        }
        else if(nt_int.GetStop().IsRange() &&
                !nt_int.GetStop().GetRange().GetStart().IsSetOffset() &&
                !nt_int.GetStop().GetRange().GetStop().IsSetOffset()) {

            CRef<CInt_fuzz> fuzz_to = CNaSeqlocHelper::x_CreateIntFuzz(seq_id,
                                                                       nt_int.GetStop().GetRange(),
                                                                       seq_type,
                                                                       scope);
            seq_loc->SetInt().SetFuzz_to(*fuzz_to); 
        }
        else if (nt_int.GetStop().IsRange() &&
                 nt_int.GetStop().GetRange().GetStop().GetBase().IsUnknown()) {
            seq_loc->SetInt().SetFuzz_to().SetLim(CInt_fuzz::eLim_tr);
        } 
    } 
    else { // Could not determine stop site
        seq_loc->SetInt().SetTo(kInvalidSeqPos);
        seq_loc->SetInt().SetFuzz_to().SetLim(CInt_fuzz::eLim_other);
    }


    const ENa_strand strand = x_GetStrand(nt_int);    
    seq_loc->SetInt().SetStrand(strand);
    
    return seq_loc; 
}



CRef<CInt_fuzz> CNaSeqlocHelper::x_CreateIntFuzz(const CSeq_id& seq_id,
                                                 const CNtSiteRange& nt_range,
                                                 const CSequenceVariant::TSeqtype& seq_type,
                                                 CScope& scope)
{

    auto int_fuzz = Ref(new CInt_fuzz());
    TSeqPos start_index = 0;
    bool know_start = x_ComputeSiteIndex(seq_id,
                                         nt_range.GetStart(),
                                         seq_type,
                                         scope,
                                         start_index);   

    TSeqPos stop_index = 0;
    bool know_stop = x_ComputeSiteIndex(seq_id,
                                        nt_range.GetStop(),
                                        seq_type,
                                        scope,
                                        stop_index);

    if (know_start && know_stop) {
        int_fuzz->SetRange().SetMin(start_index);
        int_fuzz->SetRange().SetMax(stop_index);
    }
    else if (!know_start && know_stop) {
        int_fuzz->SetLim(CInt_fuzz::eLim_lt);
    } 
    else if (know_start && !know_stop) {
        int_fuzz->SetLim(CInt_fuzz::eLim_gt);
    }
    else {
        int_fuzz->SetLim(CInt_fuzz::eLim_other); // LCOV_EXCL_LINE - Possible, but extremely unlikely. Should probably be forbidden.
    }                                            // No reason to have a range with two unknown limits.

    return int_fuzz;
}


bool CNaSeqlocHelper::x_ComputeSiteIndex(const CSeq_id& seq_id,
                                         const CNtIntLimit& nt_limit,
                                         const CSequenceVariant::TSeqtype& seq_type,
                                         CScope& scope,
                                         TSeqPos& site_index)
{
   // Interval limits must be either of type NtSite or NtRange
   if (nt_limit.IsSite()) {
       return x_ComputeSiteIndex(seq_id, 
                                 nt_limit.GetSite(),
                                 seq_type,
                                 scope,
                                 site_index);
   }


   return x_ComputeSiteIndex(seq_id,
                             nt_limit.GetRange(),
                             seq_type,
                             scope,
                             site_index);
}


bool CNaSeqlocHelper::x_ComputeSiteIndex(const CSeq_id& seq_id,
                                         const CNtSiteRange& nt_range,
                                         const CSequenceVariant::TSeqtype& seq_type,
                                         CScope& scope,
                                         TSeqPos& site_index)
{
    const auto& nt_site = nt_range.GetStart().GetBase().IsVal() ? nt_range.GetStart() : nt_range.GetStop();

    if (!nt_site.GetBase().IsVal()) {
        return false; // LCOV_EXCL_LINE
    }                 // In principle, possible, but extremely unlikely to ever arise in practice

    return x_ComputeSiteIndex(seq_id,
                              nt_site,
                              seq_type,
                              scope,
                              site_index);
}


// Given a seq-id and scope, return a reference to the unique CDS on that sequence. 
// Throw an exception if a unique CDS is not found.
const CSeq_feat& CNaSeqlocHelper::x_GetCDS(const CSeq_id& seq_id, CScope& scope) {
    
    auto bioseq_handle = scope.GetBioseqHandle(seq_id);
    if (!bioseq_handle) {
        NCBI_THROW(CVariationIrepException, eInvalidSeqId, "Cannot resolve bioseq from seq ID");
    }
    SAnnotSelector selector;
    selector.SetResolveTSE();
    selector.IncludeFeatType(CSeqFeatData::e_Cdregion);
    CConstRef<CSeq_feat> coding_seq;

    for(CFeat_CI it(bioseq_handle, selector); it; ++it) {
        const auto& mapped_feat = *it;
        if (mapped_feat.GetData().IsCdregion()) {
            if (coding_seq.NotNull()) {
                NCBI_THROW(CVariationIrepException, eCDSError, "Multiple CDS features on sequence");
            }
            coding_seq = mapped_feat.GetSeq_feat();
        }
    }

    if (coding_seq.IsNull()) {
        NCBI_THROW(CVariationIrepException, eCDSError, "Could not find CDS feature");
    }

    return *coding_seq;
}

//
// The following method translates the nucleotide site specified in the HGVS expression
// into a bioseq site index. 
// For non-CDS variants, translation is trivial.
// HGVS convention is to use index 1 to denote the starting nucleotide in a sequence 
// whereas NCBI convention is to assign index 0 to the starting nucleotide.
// Therefore, we simply subtract 1 from the HGVS nucleotide index to obtain the 
// corresponding NCBI site index.
//
// For CDS variants, things are a little complicated since the HGVS indexing scheme makes 
// reference to the beginning and end of the coding region. 
// Hence, index 1 refers to the position of nucleotide A in the start codon. 
// Index -1 refers to the nucleotide site immediately preceding the start codon, 
// and index -2 is the site immediately preceding that. 
// Similarly, *1 refers to the nucleotide site immediately 3' of the stop codon,
// and index *2 is the site immediately 3' of that. 
//  
// On the other hand, the NCBI indexing scheme makes no reference to the CDS location;
// As in the non-CDS case, NCBI index 0 simply refers to the location of the first nucleotide 
// in the underlying bioseq. 
//
// Therefore, for CDS variants, translation of a HGVS index into a bioseq index involves an 
// additional offset, and the value of that offset depends on whether the 
// nucleotide location is 5' of the CDS, 3' prime of the CDS, or falls within the CDS limits.
//
//
bool CNaSeqlocHelper::x_ComputeSiteIndex(const CSeq_id& seq_id,
                                         const CNtSite& nt_site,
                                         const CSequenceVariant::TSeqtype& seq_type,
                                         CScope& scope,
                                         TSeqPos& site_index) 
{
    // Can't do anything if the HGVS base value hasn't been specified
    if (!nt_site.IsSetBase() || !nt_site.GetBase().IsVal()) {
        return false;
    }

    TSeqPos offset = 0;
    if (seq_type == eVariantSeqType_c) {
        const CSeq_feat& coding_seq = x_GetCDS(seq_id, scope);

        if (nt_site.IsSetUtr() && nt_site.GetUtr().IsThree_prime()) {
            offset = coding_seq.GetLocation().GetStop(eExtreme_Biological) + 1;
        }
        else {
            offset = coding_seq.GetLocation().GetStart(eExtreme_Biological);
        }
    }

    site_index = nt_site.GetBase().GetVal();
    if (site_index == 0 ) { // LCOV_EXCL_START - Does not parse
        NCBI_THROW(CVariationIrepException, eInvalidLocation, "Invalid HGVS site index: 0"); 
    } // LCOV_EXCL_STOP

    // If this is a CDS variant and the referenced site lies 5' of the CDS start codon
    if (nt_site.IsSetUtr() && nt_site.GetUtr().IsFive_prime()) {
        // offset is the NCBI index referring to nucleotide A of the start codon
        if (site_index > offset) { // LCOV_EXCL_START
           NCBI_THROW(CVariationIrepException, eInvalidLocation, "Error deducing 5' UTR location"); 
        } // LCOV_EXCL_STOP                                                                                         
        site_index = offset - site_index;
        return true;
    }
    
    // To reach this point, one of the following MUST be true:
    //
    //     1) The reference sequence is not a CDS. 
    //        In this case, offset = 0
    //
    //     2) The referenced site lies within the limits of the CDS.
    //        In this case, offset is the NCBI index for the A nucleotide in the start codon.
    //
    //     3) The reference site lies 3' of the CDS.
    //        In this case, offset is the NCBI index referencing the site immediately 3' of the stop codon.       
    
    site_index = site_index-1 + offset;
    return true;
}


void CIntronOffsetHelper::x_SetDeltaItemOffset(const TSeqPos length, 
                                              const CDelta_item::TMultiplier multiplier, 
                                              const bool fuzzy,
                                              CDelta_item& delta_item)
{
     delta_item.SetAction(CDelta_item::eAction_offset);

     if (length < kInvalidSeqPos) {
         delta_item.SetSeq().SetLiteral().SetLength(length);
         if (multiplier != 1) {
             delta_item.SetMultiplier(multiplier);
         }

         if (fuzzy) {
             delta_item.SetSeq().SetLiteral().SetFuzz().SetLim(CInt_fuzz::eLim_unk);
         }
         return;
     }

     delta_item.SetSeq().SetLiteral().SetLength(0);
     const CInt_fuzz::ELim fuzz_lim = (multiplier>0) ? CInt_fuzz::eLim_gt
                                                     : CInt_fuzz::eLim_lt;

     delta_item.SetSeq().SetLiteral().SetFuzz().SetLim(fuzz_lim);
}


// Return the CDelta_item instance encapsulating the intronic offset at a nucleotide site.
CRef<CDelta_item> CIntronOffsetHelper::GetIntronOffset(const CNtSite& nt_site)
{
    CRef<CDelta_item> delta_item;

    if (nt_site.IsSetOffset()) {
        TSeqPos offset_length = kInvalidSeqPos;
        TSignedSeqPos multiplier = 1;
        bool fuzzy = false;
        delta_item = Ref(new CDelta_item());
        if(nt_site.GetOffset().IsVal()) {
            auto offset = nt_site.GetOffset().GetVal();
            if (offset < 0) {
                offset = -offset;
                multiplier = -1;
            }
            offset_length = offset; 
            if (nt_site.IsSetFuzzy_offset()) {
                fuzzy = nt_site.GetFuzzy_offset();
            }
        } 
        else if (nt_site.GetOffset().IsMinus_unknown()) {
            multiplier = -1;
        }
        x_SetDeltaItemOffset(offset_length, multiplier, fuzzy, *delta_item);
    }

    return delta_item;
}

// Return the CDelta_item instance specifying the intronic offsets appearing in 
// a nucleotide site range.
CRef<CDelta_item> CIntronOffsetHelper::GetIntronOffset(const CNtSiteRange& nt_range)
{
    CRef<CDelta_item> delta_item;

    // If the range does not involve intronic offsets
    // return a null CRef
    if (!nt_range.GetStart().IsSetOffset() &&
        !nt_range.GetStop().IsSetOffset()) {
        return delta_item;
    }

    // The following is due to limitations 
    // in the Seq-feat representation of variants. 
    // The base values in intronic ranges must be equal.
    if (nt_range.GetStart().IsSetBase() &&
        nt_range.GetStart().GetBase().IsVal() &&
        nt_range.GetStop().IsSetBase() &&
        nt_range.GetStop().GetBase().IsVal() &&
        nt_range.GetStart().GetBase().GetVal() != 
        nt_range.GetStop().GetBase().GetVal()) {
        NCBI_THROW(CVariationIrepException, eUnsupported, 
                "Base points in intronic fuzzy location must be equal");
    }

    delta_item = Ref(new CDelta_item());
    delta_item->SetAction(CDelta_item::eAction_offset);
    
    // The following code handles ranges of the form 
    // (?_base+offset). The range starts at an unknown point 
    // and ends at base+offset
    TSignedSeqPos offset_length = 0;
    if (nt_range.GetStart().IsSetBase() &&
        nt_range.GetStart().GetBase().IsUnknown() &&
        nt_range.GetStop().IsSetOffset() &&
        nt_range.GetStop().GetOffset().IsVal()) {
        offset_length = nt_range.GetStop().GetOffset().GetVal();
        delta_item->SetSeq().SetLiteral().SetLength(offset_length);
        return delta_item;
    }

    // The following code handles ranges of the form
    // (base+offset_?). The range starts at base+offset and 
    // ends at an unknown point.
    if (nt_range.GetStop().IsSetBase() &&
        nt_range.GetStop().GetBase().IsUnknown() &&
        nt_range.GetStart().IsSetOffset() &&
        nt_range.GetStart().GetOffset().IsVal()) {
        offset_length = nt_range.GetStart().GetOffset().GetVal();
        delta_item->SetSeq().SetLiteral().SetLength(offset_length);
        return delta_item;
    }

    // At this point, both the beginning and end of the range are 
    // known. Get the values of the offsets on range limits. 
    // The offsets can take positive or negative integer values,
    // but two non-zero offsets are required to have the same sign.
    TSignedSeqPos start_offset=0;
    TSignedSeqPos stop_offset=0;

    if (nt_range.GetStart().IsSetOffset() &&
        nt_range.GetStart().GetOffset().IsVal()) {
        start_offset = nt_range.GetStart().GetOffset().GetVal();
    } 

    if (nt_range.GetStop().IsSetOffset() &&
        nt_range.GetStop().GetOffset().IsVal()) {
        stop_offset = nt_range.GetStop().GetOffset().GetVal();
    }

    // Check to see if start_offset and stop_offset have opposite sign 
    if ((start_offset && stop_offset) &&
        ((start_offset^stop_offset) < 0)) {
        NCBI_THROW(CVariationIrepException, eUnsupported, 
                "Offsets in intronic fuzzy location cannot differ in sign");
    }

    // Construct the CDelta_item instance for the range offsets
    if (start_offset >= 0) {
        delta_item->SetSeq().SetLiteral().SetLength(start_offset);
        delta_item->SetSeq().SetLiteral().SetFuzz().SetRange().SetMin(start_offset);
    }
    else if (start_offset < 0) {
        delta_item->SetSeq().SetLiteral().SetFuzz().SetRange().SetMax(-start_offset);
    }

    if (stop_offset <= 0) {
        delta_item->SetSeq().SetLiteral().SetLength(-stop_offset);
        delta_item->SetSeq().SetLiteral().SetFuzz().SetRange().SetMin(-stop_offset);
    } else if (stop_offset > 0) {
        delta_item->SetSeq().SetLiteral().SetFuzz().SetRange().SetMax(stop_offset);
    }

    if (start_offset < 0 || stop_offset < 0) {
        delta_item->SetMultiplier(-1);
    }

    return delta_item;
}


// Return a CRef to the CDelta_item instance specifying the intron offset at an interval start point
// Return an empty reference if no offset is found
CRef<CDelta_item> CIntronOffsetHelper::GetStartIntronOffset(const CNtInterval& nt_int)
{
    if (nt_int.IsSetStart() &&
        nt_int.GetStart().IsSite()) {
        return CIntronOffsetHelper::GetIntronOffset(nt_int.GetStart().GetSite());
    } 
    else if (nt_int.IsSetStart() &&
             nt_int.GetStart().IsRange()) {
        return CIntronOffsetHelper::GetIntronOffset(nt_int.GetStart().GetRange());
    }
    return Ref(new CDelta_item()); // LCOV_EXCL_LINE - Not expected to arise
}


// Return a CRef to the CDelta_item instance specifying the intron offset an interval end point
// Return an empty reference if no offset is found
CRef<CDelta_item> CIntronOffsetHelper::GetStopIntronOffset(const CNtInterval& nt_int)
{
    if (nt_int.IsSetStop() &&
        nt_int.GetStop().IsSite()) {
        return CIntronOffsetHelper::GetIntronOffset(nt_int.GetStop().GetSite());
    }
    else if (nt_int.IsSetStop() &&
             nt_int.GetStop().IsRange()) {
        return CIntronOffsetHelper::GetIntronOffset(nt_int.GetStop().GetRange());
    }
    return Ref(new CDelta_item()); // LCOV_EXCL_LINE - Not expected to arise
}


CRef<CDelta_item> CIntronOffsetHelper::GetStartIntronOffset(const CNtLocation& nt_loc)
{

    CRef<CDelta_item> start_offset;
    if (nt_loc.IsSite()) {
        start_offset = CIntronOffsetHelper::GetIntronOffset(nt_loc.GetSite());
    } 
    else if (nt_loc.IsInt()) {
        start_offset = CIntronOffsetHelper::GetStartIntronOffset(nt_loc.GetInt());
    }
    return start_offset;
}



CRef<CDelta_item> CIntronOffsetHelper::GetStopIntronOffset(const CNtLocation& nt_loc)
{
    CRef<CDelta_item> stop_offset;
    if (nt_loc.IsInt()) {
        stop_offset = CIntronOffsetHelper::GetStopIntronOffset(nt_loc.GetInt());
    }
    return stop_offset;
}


END_SCOPE(objects)
END_NCBI_SCOPE

