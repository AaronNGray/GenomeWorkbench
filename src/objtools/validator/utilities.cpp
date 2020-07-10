/*  $Id: utilities.cpp 593456 2019-09-18 20:32:03Z kans $
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
 * Author:  Mati Shomrat
 *
 * File Description:
 *      Implementation of utility classes and functions.
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbistr.hpp>

#include <serial/enumvalues.hpp>
#include <serial/serialimpl.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/taxon3/T3Data.hpp>
#include <objects/taxon3/Taxon3_reply.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <objects/taxon3/taxon3.hpp>
#include <objects/taxon1/taxon1.hpp>
#include <objtools/validator/utilities.hpp>
#include <objtools/validator/splice_problems.hpp>
#include <objtools/validator/translation_problems.hpp>
#include <objtools/validator/tax_validation_and_cleanup.hpp>

#include <vector>
#include <algorithm>
#include <list>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(validator)


// =============================================================================
//                                    Functions
// =============================================================================


bool IsClassInEntry(const CSeq_entry& se, CBioseq_set::EClass clss)
{
    for ( CTypeConstIterator <CBioseq_set> si(se); si; ++si ) {
        if ( si->GetClass() == clss ) {
            return true;
        }
    }
    return false;
}


bool IsDeltaOrFarSeg(const CSeq_loc& loc, CScope* scope)
{
    CBioseq_Handle bsh = BioseqHandleFromLocation(scope, loc);
    const CSeq_entry& se = *bsh.GetTopLevelEntry().GetCompleteSeq_entry();

    if ( bsh.IsSetInst_Repr() ) {
        CBioseq_Handle::TInst::TRepr repr = bsh.GetInst_Repr();
        if ( repr == CSeq_inst::eRepr_delta ) {
            if ( !IsClassInEntry(se, CBioseq_set::eClass_nuc_prot) ) {
                return true;
            }
        }
        if ( repr == CSeq_inst::eRepr_seg ) {
            if ( !IsClassInEntry(se, CBioseq_set::eClass_parts) ) {
                return true;
            }
        }
    }

    return false;
}


// Check if string is either empty or contains just white spaces
bool IsBlankStringList(const list< string >& str_list)
{
    ITERATE( list< string >, str, str_list ) {
        if ( !NStr::IsBlank(*str) ) {
            return false;
        }
    }
    return true;
}


TGi GetGIForSeqId(const CSeq_id& id)
{
    TGi gi = ZERO_GI;
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    try {
        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
        gi = scope->GetGi (idh);
    } catch (CException &) {
    } catch (std::exception &) {
    }
    return gi;
}



CScope::TIds GetSeqIdsForGI(TGi gi)
{
    CScope::TIds id_list;
    CSeq_id tmp_id;
    tmp_id.SetGi(gi);
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDefaults();

    try {
        id_list = scope->GetIds(tmp_id);

    } catch (CException &) {
    } catch (std::exception &) {
    }
    return id_list;
}

bool IsFarLocation(const CSeq_loc& loc, const CSeq_entry_Handle& seh)
{
    CScope& scope = seh.GetScope();
    for ( CSeq_loc_CI citer(loc); citer; ++citer ) {
        CConstRef<CSeq_id> id(&citer.GetSeq_id());
        if ( id ) {
            CBioseq_Handle near_seq = scope.GetBioseqHandleFromTSE(*id, seh);
            if ( !near_seq ) {
                return true;
            }
        }
    }

    return false;
}

string GetSequenceStringFromLoc
(const CSeq_loc& loc,
 CScope& scope)
{
    CNcbiOstrstream oss;
    CFastaOstream fasta_ostr(oss);
    fasta_ostr.SetFlag(CFastaOstream::fAssembleParts);
    fasta_ostr.SetFlag(CFastaOstream::fInstantiateGaps);
    string s;

    try {
        for (CSeq_loc_CI citer (loc); citer; ++citer) {
            const CSeq_loc& part = citer.GetEmbeddingSeq_loc();
            CBioseq_Handle bsh = BioseqHandleFromLocation (&scope, part);
            if (bsh) {
                fasta_ostr.WriteSequence (bsh, &part);
            }
        }
        s = CNcbiOstrstreamToString(oss);
        NStr::ReplaceInPlace(s, "\n", "");
    } catch (CException&) {
        s = kEmptyStr;
    }

    return s;
}


CSeqVector GetSequenceFromLoc
(const CSeq_loc& loc,
 CScope& scope,
 CBioseq_Handle::EVectorCoding coding)
{
    CConstRef<CSeqMap> map = 
        CSeqMap::CreateSeqMapForSeq_loc(loc, &scope);
    return CSeqVector(*map, scope, coding, eNa_strand_plus);
}


CSeqVector GetSequenceFromFeature
(const CSeq_feat& feat,
 CScope& scope,
 CBioseq_Handle::EVectorCoding coding,
 bool product)
{

    if ( (product   &&  !feat.CanGetProduct())  ||
         (!product  &&  !feat.CanGetLocation()) ) {
        return CSeqVector();
    }

    const CSeq_loc* loc = product ? &feat.GetProduct() : &feat.GetLocation();
    return GetSequenceFromLoc(*loc, scope, coding);
}


/***** Calculate Accession for a given object *****/


static string s_GetBioseqAcc(const CSeq_id& id, int* version)
{
    try {
        string label;
        id.GetLabel(&label, version, CSeq_id::eFasta);
        return label;
    } catch (CException&) {
        return kEmptyStr;
    }
}


static string s_GetBioseqAcc(const CBioseq_Handle& handle, int* version)
{
    if (handle) {
        CConstRef<CSeq_id> seqid = sequence::GetId(handle, sequence::eGetId_Best).GetSeqId();
        if (seqid) {
            return s_GetBioseqAcc(*seqid, version);
        }
    }
    return kEmptyStr;
}


static string s_GetSeq_featAcc(const CSeq_feat& feat, CScope& scope, int* version)
{
    CBioseq_Handle seq = BioseqHandleFromLocation (&scope, feat.GetLocation());
    if (seq) {
        CBioseq_set_Handle parent = seq.GetParentBioseq_set();
        if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_parts) {
            parent = parent.GetParentBioseq_set();
            if (parent && parent.IsSetClass() && parent.GetClass() == CBioseq_set::eClass_segset) {
                CBioseq_CI m(parent);
                if (m) {
                    return s_GetBioseqAcc(*m, version);
                }
            }
        }
    }

    return s_GetBioseqAcc(seq, version);
}


static string s_GetBioseqAcc(const CBioseq& seq, CScope& scope, int* version)
{
    CBioseq_Handle handle = scope.GetBioseqHandle(seq);
    return s_GetBioseqAcc(handle, version);
}

static const CBioseq* s_GetSeqFromSet(const CBioseq_set& bsst, CScope& scope)
{
    const CBioseq* retval = NULL;

    switch (bsst.GetClass()) {
        case CBioseq_set::eClass_gen_prod_set:
            // find the genomic bioseq
            FOR_EACH_SEQENTRY_ON_SEQSET (it, bsst) {
                if ((*it)->IsSeq()) {
                    const CSeq_inst& inst = (*it)->GetSeq().GetInst();
                    if (inst.IsSetMol()  &&  inst.GetMol() == CSeq_inst::eMol_dna) {
                        retval = &(*it)->GetSeq();
                        break;
                    }
                }
            }
            break;
        case CBioseq_set::eClass_nuc_prot:
            // find the nucleotide bioseq
            FOR_EACH_SEQENTRY_ON_SEQSET (it, bsst) {
                if ((*it)->IsSeq()  &&  (*it)->GetSeq().IsNa()) {
                    retval = &(*it)->GetSeq();
                    break;
                } else if ((*it)->IsSet()  &&
                           (*it)->GetSet().IsSetClass() &&
                           (*it)->GetSet().GetClass() == CBioseq_set::eClass_segset) {
                    retval = s_GetSeqFromSet((*it)->GetSet(), scope);
                    break;
                }
            }
            if (!retval) {
                FOR_EACH_SEQENTRY_ON_SEQSET (it, bsst) {
                    if ((*it)->IsSeq()) {
                        retval = &(*it)->GetSeq();
                        break;
                    }
                }
            }
            break;
        case CBioseq_set::eClass_segset:
            // find the master bioseq
            FOR_EACH_SEQENTRY_ON_SEQSET (it, bsst) {
                if ((*it)->IsSeq()) {
                    retval = &(*it)->GetSeq();
                    break;
                }
            }
            break;

        default:
            // find the first bioseq
            CTypeConstIterator<CBioseq> seqit(ConstBegin(bsst));
            if (seqit) {
                retval = &(*seqit);
            }
            break;
    }

    return retval;
}


static bool s_IsDescOnSeqEntry (const CSeq_entry& entry, const CSeqdesc& desc)
{
    if (entry.IsSetDescr()) {
        const auto& descs = entry.GetDescr();
        for (auto it : descs.Get()) {
            if ((*it).Equals(desc)) {
                return true;
            }
        }
    }
    return false;
}



static string s_GetAccessionForSeqdesc (const CSeq_entry_Handle& seh, const CSeqdesc& desc, CScope& scope, int* version)
{
    if (!seh) {
        return kEmptyStr;\
    } else if (seh.IsSeq()) {
        return s_GetBioseqAcc(*(seh.GetSeq().GetCompleteBioseq()), scope, version);
    } else if (s_IsDescOnSeqEntry (*(seh.GetCompleteSeq_entry()), desc)) {
        if (seh.IsSeq()) {
            return s_GetBioseqAcc(*(seh.GetSeq().GetCompleteBioseq()), scope, version);
        } else if (seh.IsSet()) {
            const CBioseq* seq = s_GetSeqFromSet(*(seh.GetSet().GetCompleteBioseq_set()), scope);
            if (seq != NULL) {
                return s_GetBioseqAcc(*seq, scope, version);
            }
        }
    } else {
        CSeq_entry_Handle parent = seh.GetParentEntry();
        if (parent) {
            return s_GetAccessionForSeqdesc(parent, desc, scope, version);
        }
    }
    return kEmptyStr;
}


bool IsBioseqInSameSeqEntryAsAlign(const CBioseq_Handle& bsh, const CSeq_align& align, CScope& scope)
{
    CSeq_entry_Handle seh = bsh.GetTopLevelEntry();
    for (CAlign_CI align_it(seh); align_it; ++align_it) {
        if (&(*align_it) == &align) {
            return true;
        }
    }
    return false;
}


CConstRef<CSeq_id> GetReportableSeqIdForAlignment(const CSeq_align& align, CScope& scope)
{
    // temporary - to match C Toolkit
    if (align.IsSetSegs() && align.GetSegs().IsStd()) {
        return CConstRef<CSeq_id>(NULL);
    }
    try {
        if (align.IsSetDim()) {
            for (int i = 0; i < align.GetDim(); ++i) {
                const CSeq_id& id = align.GetSeq_id(i);
                CBioseq_Handle bsh = scope.GetBioseqHandle(id);
                if (bsh && IsBioseqInSameSeqEntryAsAlign(bsh, align, scope)) {
                    return CConstRef<CSeq_id>(&id);
                }
            }
        } else if (align.IsSetSegs() && align.GetSegs().IsDendiag()) {
            const CSeq_id& id = *(align.GetSegs().GetDendiag().front()->GetIds()[0]);
            return CConstRef<CSeq_id>(&id);
        }
        // failed to find resolvable ID, use bare ID
        const CSeq_id& id = align.GetSeq_id(0); 
        return CConstRef<CSeq_id>(&id);
    } catch (CException& ) {
    }
    return CConstRef<CSeq_id>(NULL);
}


string GetAccessionFromObjects(const CSerialObject* obj, const CSeq_entry* ctx, CScope& scope, int* version)
{
    string empty_acc = "";

    if (obj && obj->GetThisTypeInfo() == CSeqdesc::GetTypeInfo() && ctx) {
        CSeq_entry_Handle seh = scope.GetSeq_entryHandle(*ctx);
        const CSeqdesc& desc = dynamic_cast<const CSeqdesc&>(*obj);
        string acc = s_GetAccessionForSeqdesc(seh, desc, scope, version);
        if (!NStr::IsBlank(acc)) {
            return acc;
        }
    }

    if (ctx) {
        if (ctx->IsSeq()) {
            return s_GetBioseqAcc(ctx->GetSeq(), scope, version);
        } else if (ctx->IsSet()) {
            const CBioseq* seq = s_GetSeqFromSet(ctx->GetSet(), scope);
            if (seq != NULL) {
                return s_GetBioseqAcc(*seq, scope, version);
            }
        }
    } else if (obj) {
        if (obj->GetThisTypeInfo() == CSeq_feat::GetTypeInfo()) {
            const CSeq_feat& feat = dynamic_cast<const CSeq_feat&>(*obj);
            return s_GetSeq_featAcc(feat, scope, version);
        } else if (obj->GetThisTypeInfo() == CBioseq::GetTypeInfo()) {
            const CBioseq& seq = dynamic_cast<const CBioseq&>(*obj);
            return s_GetBioseqAcc(seq, scope, version);
        } else if (obj->GetThisTypeInfo() == CBioseq_set::GetTypeInfo()) {
            const CBioseq_set& bsst = dynamic_cast<const CBioseq_set&>(*obj);
            const CBioseq* seq = s_GetSeqFromSet(bsst, scope);
            if (seq != NULL) {
                return s_GetBioseqAcc(*seq, scope, version);
            }
        } else if (obj->GetThisTypeInfo() == CSeq_entry::GetTypeInfo()) {
            const CSeq_entry& entry = dynamic_cast<const CSeq_entry&>(*obj);
            if (entry.IsSeq()) {
                return s_GetBioseqAcc(entry.GetSeq(), scope, version);
            } else if (entry.IsSet()) {
                const CBioseq* seq = s_GetSeqFromSet(entry.GetSet(), scope);
                if (seq != NULL) {
                    return s_GetBioseqAcc(*seq, scope, version);
                }
            }
        } else if (obj->GetThisTypeInfo() == CSeq_annot::GetTypeInfo()) {
            CSeq_annot_Handle ah = scope.GetSeq_annotHandle (dynamic_cast<const CSeq_annot&>(*obj));
            if (ah) {
                CSeq_entry_Handle seh = ah.GetParentEntry();
                if (seh) {
                    if (seh.IsSeq()) {
                        return s_GetBioseqAcc(seh.GetSeq(), version);
                    } else if (seh.IsSet()) {
                        CBioseq_set_Handle bsh = seh.GetSet();
                        const CBioseq_set& bsst = *(bsh.GetCompleteBioseq_set());
                        const CBioseq* seq = s_GetSeqFromSet(bsst, scope);
                        if (seq != NULL) {
                            return s_GetBioseqAcc(*seq, scope, version);
                        }
                    }
                }
            }
        } else if (obj->GetThisTypeInfo() == CSeq_align::GetTypeInfo()) {
            const CSeq_align& align = dynamic_cast<const CSeq_align&>(*obj);
            CConstRef<CSeq_id> id = GetReportableSeqIdForAlignment(align, scope);
            if (id) {
                CBioseq_Handle bsh = scope.GetBioseqHandle(*id);
                if (bsh) {
                    return s_GetBioseqAcc(bsh, version);
                } else {
                    return s_GetBioseqAcc(*id, version);
                }
            }
        } else if (obj->GetThisTypeInfo() == CSeq_graph::GetTypeInfo()) {
            const CSeq_graph& graph = dynamic_cast<const CSeq_graph&>(*obj);
            try {
                const CSeq_loc& loc = graph.GetLoc();
                const CSeq_id *id = loc.GetId();
                if (id) {
                    return s_GetBioseqAcc (*id, version);
                }
            } catch (CException& ) {
            }
        }
    }
    return empty_acc;
}


CBioseq_set_Handle GetSetParent (const CBioseq_set_Handle& set, CBioseq_set::TClass set_class)
{
    CBioseq_set_Handle gps;

    CSeq_entry_Handle parent = set.GetParentEntry();
    if (!parent) {
        return gps;
    } else if (!(parent = parent.GetParentEntry())) {
        return gps;
    } else if (!parent.IsSet()) {
        return gps;
    } else if (parent.GetSet().IsSetClass() && parent.GetSet().GetClass() == set_class) {
        return parent.GetSet();
    } else {
        return GetSetParent (parent.GetSet(), set_class);
    }    
}


CBioseq_set_Handle GetSetParent (const CBioseq_Handle& bioseq, CBioseq_set::TClass set_class)
{
    CBioseq_set_Handle set;

    CSeq_entry_Handle parent = bioseq.GetParentEntry();
    if (!parent) {
        return set;
    } else if (!(parent = parent.GetParentEntry())) {
        return set;
    } else if (!parent.IsSet()) {
        return set;
    } else if (parent.GetSet().IsSetClass() && parent.GetSet().GetClass() == set_class) {
        return parent.GetSet();
    } else {
        return GetSetParent (parent.GetSet(), set_class);
    }
}


CBioseq_set_Handle GetGenProdSetParent (const CBioseq_set_Handle& set)
{
    return GetSetParent (set, CBioseq_set::eClass_gen_prod_set);
}

CBioseq_set_Handle GetGenProdSetParent (const CBioseq_Handle& bioseq)
{
    return GetSetParent(bioseq, CBioseq_set::eClass_gen_prod_set);
}


CBioseq_set_Handle GetNucProtSetParent (const CBioseq_Handle& bioseq)
{
    return GetSetParent(bioseq, CBioseq_set::eClass_nuc_prot);
}


CBioseq_Handle GetNucBioseq (const CBioseq_set_Handle& bioseq_set)
{
    CBioseq_Handle nuc;

    if (!bioseq_set) {
        return nuc;
    }
    CBioseq_CI bit(bioseq_set, CSeq_inst::eMol_na);
    if (bit) {
        nuc = *bit;
    } else {
        CSeq_entry_Handle parent = bioseq_set.GetParentEntry();
        if (parent && (parent = parent.GetParentEntry())
            && parent.IsSet()) {
            nuc = GetNucBioseq (parent.GetSet());
        }
    }
    return nuc;
}
       

CBioseq_Handle GetNucBioseq (const CBioseq_Handle& bioseq)
{
    CBioseq_Handle nuc;

    if (bioseq.IsNucleotide()) {
        return bioseq;
    }
    CSeq_entry_Handle parent = bioseq.GetParentEntry();
    if (parent && (parent = parent.GetParentEntry())
        && parent.IsSet()) {
        nuc = GetNucBioseq (parent.GetSet());
    }
    return nuc;
}


EAccessionFormatError ValidateAccessionString (const string& accession, bool require_version)
{
    if (NStr::IsBlank (accession)) {
        return eAccessionFormat_null;
    } else if (accession.length() >= 16) {
        return eAccessionFormat_too_long;
    } else if (accession.length() < 3 
               || ! isalpha (accession.c_str()[0]) 
               || ! isupper (accession.c_str()[0])) {
        return eAccessionFormat_no_start_letters;
    }
    
    string str = accession;
    if (NStr::StartsWith (str, "NZ_")) {
        str = str.substr(3);
    }
    
    const char *cp = str.c_str();
    int numAlpha = 0;

    while (isalpha (*cp)) {
        numAlpha++;
        cp++;
    }

    int numUndersc = 0;

    while (*cp == '_') {
        numUndersc++;
        cp++;
    }

    int numDigits = 0;
    while (isdigit (*cp)) {
        numDigits++;
        cp++;
    }

    if ((*cp != '\0' && *cp != ' ' && *cp != '.') || numUndersc > 1) {
        return eAccessionFormat_wrong_number_of_digits;
    }

    if (require_version) {
        if (*cp != '.') {
            return eAccessionFormat_missing_version;
        }
        cp++;
        int numVersion = 0;
        while (isdigit (*cp)) {
            numVersion++;
            cp++;
        }
        if (numVersion < 1) {
            return eAccessionFormat_missing_version;
        } else if (*cp != '\0' && *cp != ' ') {
            return eAccessionFormat_bad_version;
        }
    }


    if (numUndersc == 0) {
        if ((numAlpha == 1 && numDigits == 5) 
            || (numAlpha == 2 && numDigits == 6)
            || (numAlpha == 3 && numDigits == 5)
            || (numAlpha == 4 && numDigits == 8)
            || (numAlpha == 5 && numDigits == 7)) {
            return eAccessionFormat_valid;
        } 
    } else {
        if (numAlpha != 2 || (numDigits != 6 && numDigits != 8 && numDigits != 9)) {
            return eAccessionFormat_wrong_number_of_digits;
        }
        char first_letter = accession.c_str()[0];
        char second_letter = accession.c_str()[1];
        if (first_letter == 'N' || first_letter == 'X' || first_letter == 'Z') { 
            if (second_letter == 'M' || second_letter == 'C'
                || second_letter == 'T' || second_letter == 'P'
                || second_letter == 'G' || second_letter == 'R'
                || second_letter == 'S' || second_letter == 'W'
                || second_letter == 'Z') {
                return eAccessionFormat_valid;
            }
        }
        if ((first_letter == 'A' || first_letter == 'Y')
            && second_letter == 'P') {
            return eAccessionFormat_valid;
        }
    }

    return eAccessionFormat_wrong_number_of_digits;
}


bool s_FeatureIdsMatch (const CFeat_id& f1, const CFeat_id& f2)
{
    if (!f1.IsLocal() || !f2.IsLocal()) {
        return false;
    }

    return 0 == f1.GetLocal().Compare(f2.GetLocal());
}


bool s_StringHasPMID (const string& str)
{
    if (NStr::IsBlank (str)) {
        return false;
    }

    size_t pos = NStr::Find (str, "(PMID ");
    if (pos == string::npos) {
        return false;
    }

    const char *ptr = str.c_str() + pos + 6;
    unsigned int numdigits = 0;
    while (*ptr != 0 && *ptr != ')') {
        if (isdigit (*ptr)) {
            numdigits++;
        }
        ptr++;
    }

    if (*ptr == ')' && numdigits > 0) {
        return true;
    } else {
        return false;
    }
}


bool HasBadCharacter (const string& str)
{
    if (NStr::Find (str, "?") != string::npos
        || NStr::Find (str, "!") != string::npos
        || NStr::Find (str, "~") != string::npos
        || NStr::Find(str, "|") != string::npos) {
        return true;
    } else {
        return false;
    }
}


bool EndsWithBadCharacter (const string& str)
{
    if (NStr::EndsWith (str, "_") || NStr::EndsWith (str, ".") 
        || NStr::EndsWith (str, ",") || NStr::EndsWith (str, ":")
        || NStr::EndsWith (str, ";")) {
        return true;
    } else {
        return false;
    }
}


int CheckDate (const CDate& date, bool require_full_date)
{
    int rval = eDateValid_valid;

    if (date.IsStr()) {
        if (NStr::IsBlank (date.GetStr()) || NStr::Equal (date.GetStr(), "?")) {
            rval |= eDateValid_bad_str;
        }
    } else if (date.IsStd()) {
        const auto& sdate = date.GetStd();
        if (!sdate.IsSetYear() || sdate.GetYear() == 0) {
            rval |= eDateValid_bad_year;
        }
        if (sdate.IsSetMonth() && sdate.GetMonth() > 12) {
            rval |= eDateValid_bad_month;
        }
        if (sdate.IsSetDay() && sdate.GetDay() > 31) {
            rval |= eDateValid_bad_day;
        }
        if (require_full_date) {
            if (!sdate.IsSetMonth() || sdate.GetMonth() == 0) {
                rval |= eDateValid_bad_month;
            }
            if (!sdate.IsSetDay() || sdate.GetDay() == 0) {
                rval |= eDateValid_bad_day;
            }
        }
        if (sdate.IsSetSeason() && !NStr::IsBlank (sdate.GetSeason())) {
            const char * cp = sdate.GetSeason().c_str();
            while (*cp != 0) {
                if (isalpha (*cp) || *cp == '-') {
                    // these are the only acceptable characters
                } else {
                    rval |= eDateValid_bad_season;
                    break;
                }
                ++cp;
            }
        }
    } else {
        rval |= eDateValid_bad_other;
    }
    return rval;
}


bool IsDateInPast(const CDate& date)
{
    time_t t;
    time(&t);
    struct tm *tm;
    tm = localtime(&t);

    bool in_past = false;
    if (!date.IsStd()) {
        return false;
    }
    const auto & sdate = date.GetStd();
    if (sdate.GetYear() < tm->tm_year + 1900) {
        in_past = true;
    } else if (sdate.GetYear() == tm->tm_year + 1900
        && sdate.IsSetMonth()) {
        if (sdate.GetMonth() < tm->tm_mon + 1) {
            in_past = true;
        } else if (sdate.GetMonth() == tm->tm_mon + 1
            && sdate.IsSetDay()) {
            if (sdate.GetDay() < tm->tm_mday) {
                in_past = true;
            }
        }
    }
    return in_past;
}


string GetDateErrorDescription (int flags)
{
    string reasons = "";

    if (flags & eDateValid_empty_date) {
        reasons += "EMPTY_DATE ";
    }
    if (flags & eDateValid_bad_str) {
        reasons += "BAD_STR ";
    }
    if (flags & eDateValid_bad_year) {
        reasons += "BAD_YEAR ";
    }
    if (flags & eDateValid_bad_month) {
        reasons += "BAD_MONTH ";
    }
    if (flags & eDateValid_bad_day) {
        reasons += "BAD_DAY ";
    }
    if (flags & eDateValid_bad_season) {
        reasons += "BAD_SEASON ";
    }
    if (flags & eDateValid_bad_other) {
        reasons += "BAD_OTHER ";
    }
    return reasons;
}


bool IsBioseqTSA (const CBioseq& seq, CScope* scope) 
{
    if (!scope) {
        return false;
    }
    bool is_tsa = false;
    CBioseq_Handle bsh = scope->GetBioseqHandle(seq);
    if (bsh) {
        CSeqdesc_CI desc_ci(bsh, CSeqdesc::e_Molinfo);
        while (desc_ci && !is_tsa) {
            if (desc_ci->GetMolinfo().IsSetTech() && desc_ci->GetMolinfo().GetTech() == CMolInfo::eTech_tsa) {
                is_tsa = true;
            }
            ++desc_ci;
        }
    }
    return is_tsa;
}


#if 0
// disabled for now
bool IsNCBIFILESeqId (const CSeq_id& id)
{
    if (!id.IsGeneral() || !id.GetGeneral().IsSetDb()
        || !NStr::Equal(id.GetGeneral().GetDb(), "NCBIFILE")) {
        return false;
    } else {
        return true;
    }
}
#endif


bool IsAccession(const CSeq_id& id)
{
    if (id.GetTextseq_Id() != NULL) {
        return true;
    } else {
        return false;
    }
}


static void UpdateToBestId(CSeq_loc& loc, CScope& scope)
{
    bool any_change = false;
    CSeq_loc_I it(loc);
    for (; it; ++it) {
        const CSeq_id& id = it.GetSeq_id();
        if (!IsAccession(id)) {
            CConstRef<CSeq_id> best_id(NULL);
            CBioseq_Handle bsh = scope.GetBioseqHandle(id);
            if (bsh) {
                const auto & ids = bsh.GetCompleteBioseq()->GetId();
                for (auto id_it : ids) {
                    if (IsAccession(*id_it)) {
                        best_id = id_it;
                        break;
                    }
                }
            }
            if (best_id) {
                it.SetSeq_id(*best_id);
                any_change = true;
            }
        }
    }
    if (any_change) {
        loc.Assign(*it.MakeSeq_loc());
    }
}


string GetValidatorLocationLabel (const CSeq_loc& loc, CScope& scope)
{
    string loc_label = "";
    if (loc.IsWhole()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(loc.GetWhole());
        if (bsh) {
            loc_label = GetBioseqIdLabel(*(bsh.GetCompleteBioseq()));
            NStr::ReplaceInPlace(loc_label, "[", "");
            NStr::ReplaceInPlace(loc_label, "]", "");
        }
    }
    if (NStr::IsBlank(loc_label)) {
        CSeq_loc tweaked_loc;
        tweaked_loc.Assign(loc);
        UpdateToBestId(tweaked_loc, scope);
        tweaked_loc.GetLabel(&loc_label);
        NStr::ReplaceInPlace(loc_label, "[", "(");
        NStr::ReplaceInPlace(loc_label, "]", ")");
    }
    return loc_label;
}



string GetBioseqIdLabel(const CBioseq& sq, bool limited)
{
    string content = "";
    int num_ids_found = 0;
    bool id_found = false;

    const auto& id_list = sq.GetId();

    /* find first gi */
    for (auto id_it : id_list) {
        if (id_it->IsGi()) {
            CNcbiOstrstream os;
            id_it->WriteAsFasta(os);
            string s = CNcbiOstrstreamToString(os);
            content += s;
            num_ids_found ++;
            break;
        }
    }
    /* find first accession */
    for (auto id_it : id_list) {
        if (id_it->IsGenbank()
            || id_it->IsDdbj()
            || id_it->IsEmbl()
            || id_it->IsSwissprot()
            || id_it->IsOther()
            || id_it->IsTpd()
            || id_it->IsTpe()
            || id_it->IsTpg()) {
            if (num_ids_found > 0) {
                content += "|";
            }
            CNcbiOstrstream os;
            id_it->WriteAsFasta(os);
            string s = CNcbiOstrstreamToString(os);
            content += s;
            num_ids_found++;
            break;
        }
    }

    if (num_ids_found == 0) {
        /* find first general */
        for (auto id_it : id_list) {
            if (id_it->IsGeneral()) {
                if (num_ids_found > 0) {
                    content += "|";
                }
                CNcbiOstrstream os;
                id_it->WriteAsFasta(os);
                string s = CNcbiOstrstreamToString(os);
                content += s;
                num_ids_found++;
                break;
            }
        }
    }
    // didn't find any?  print them all, but only the first local
    if (num_ids_found == 0) {
        bool found_local = false;
        for (auto id_it : id_list) {
            if (id_it->IsLocal()) {
                if (found_local) {
                    continue;
                } else {
                    found_local = true;
                }
            }
            if (id_found) {
                content += "|";
            }
            CNcbiOstrstream os;
            id_it->WriteAsFasta(os);
            string s = CNcbiOstrstreamToString(os);
            content += s;
            id_found = true;
        }
    }

    return content;
}


void AppendBioseqLabel(string& str, const CBioseq& sq, bool supress_context)
{
    str += "BIOSEQ: ";

    string content = GetBioseqIdLabel (sq);

    if (!supress_context) {
        if (!content.empty()) {
            content += ": ";
        }

        const CEnumeratedTypeValues* tv;
        tv = CSeq_inst::GetTypeInfo_enum_ERepr();
        const CSeq_inst& inst = sq.GetInst();
        content += tv->FindName(inst.GetRepr(), true) + ", ";
        tv = CSeq_inst::GetTypeInfo_enum_EMol();
        content += tv->FindName(inst.GetMol(), true);
        if (inst.IsSetLength()) {
            content += string(" len= ") + NStr::IntToString(inst.GetLength());
        }
    }
    str += content;
}

bool HasECnumberPattern (const string& str)
{
    bool rval = false;
    if (NStr::IsBlank(str)) {
        return false;
    }

    bool is_ambig = false;
    int  numdashes = 0;
    int  numdigits = 0;
    int  numperiods = 0;

    string::const_iterator sit = str.begin();
    while (sit != str.end() && !rval) {
        if (isdigit (*sit)) {
            numdigits++;
            if (is_ambig) {
                is_ambig = false;
                numperiods = 0;
                numdashes = 0;
            }
        } else if (*sit == '-') {
            numdashes++;
            is_ambig = true;
        } else if (*sit == 'n') {
            numdashes++;
            is_ambig = true;
        } else if (*sit == '.') {
            numperiods++;
            if (numdigits > 0 && numdashes > 0) {
                is_ambig = false;
                numperiods = 0;
            } else if (numdigits == 0 && numdashes == 0) {
                is_ambig = false;
                numperiods = 0;
            } else if (numdashes > 1) {
                is_ambig = false;
                numperiods = 0;
            }
            numdigits = 0;
            numdashes = 0;
        } else {
            if (numperiods == 3) {
                if (numdigits > 0 && numdashes > 0) {
                    is_ambig = false;
                } else if (numdigits > 0 || numdashes == 1) {
                    rval = true;
                }
            }
            is_ambig = false;
            numperiods = 0;
            numdigits = 0;
            numdashes = 0;
        }
        ++sit;
    }
    if (numperiods == 3) {
        if (numdigits > 0 && numdashes > 0) {
            rval = false;
        } else if (numdigits > 0 || numdashes == 1) {
            rval = true;
        }
    }
    return rval;
}


bool SeqIsPatent (const CBioseq& seq)
{
    bool is_patent = false;

    // some tests are suppressed if a patent ID is present
    FOR_EACH_SEQID_ON_BIOSEQ (id_it, seq) {
        if ((*id_it)->IsPatent()) {
            is_patent = true;
            break;
        }
    }
    return is_patent;
}


bool SeqIsPatent (const CBioseq_Handle& seq)
{
    return SeqIsPatent (*(seq.GetCompleteBioseq()));
}


bool s_PartialAtGapOrNs (
    CScope* scope,
    const CSeq_loc& loc,
    unsigned int tag,
    bool only_gap
)

{
    if ( tag != sequence::eSeqlocPartial_Nostart && tag != sequence::eSeqlocPartial_Nostop ) {
        return false;
    }

    CSeq_loc_CI first, last;
    for ( CSeq_loc_CI sl_iter(loc); sl_iter; ++sl_iter ) { // EQUIV_IS_ONE not supported
        if ( !first ) {
            first = sl_iter;
        }
        last = sl_iter;
    }

    if ( first.GetStrand() != last.GetStrand() ) {
        return false;
    }
    CSeq_loc_CI temp = (tag == sequence::eSeqlocPartial_Nostart) ? first : last;

    if (!scope) {
        return false;
    }

    CConstRef<CSeq_loc> slp = temp.GetRangeAsSeq_loc();
    if (!slp) {
        return false;
    }
    const CSeq_id* id = slp->GetId();
    if (id == NULL) return false;
    CBioseq_Handle bsh = scope->GetBioseqHandle(*id);
    if (!bsh) {
        return false;
    }
    
    TSeqPos acceptor = temp.GetRange().GetFrom();
    TSeqPos donor = temp.GetRange().GetTo();
    TSeqPos start = acceptor;
    TSeqPos stop = donor;

    CSeqVector vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac,
        temp.GetStrand());
    TSeqPos len = vec.size();

    if ( temp.GetStrand() == eNa_strand_minus ) {
        swap(acceptor, donor);
        stop = len - donor - 1;
        start = len - acceptor - 1;
    }

    bool result = false;

    try {
        if (tag == sequence::eSeqlocPartial_Nostop && stop < len - 1 && vec.IsInGap(stop + 1)) {
            return true;
        } else if (tag == sequence::eSeqlocPartial_Nostart && start > 0 && start < len && vec.IsInGap(start - 1)) {
            return true;
        }
    } catch ( exception& ) {
        
        return false;
    }
    if (only_gap) {
        return false;
    }

    if ( (tag == sequence::eSeqlocPartial_Nostop)  &&  (stop < len - 2) ) {
        try {
            CSeqVector::TResidue res = vec[stop + 1];

            if ( IsResidue(res)  &&  isalpha (res)) {
                if ( res == 'N' ) {
                    result = true;
                }
            }
        } catch ( exception& ) {
            return false;
        }
    } else if ( (tag == sequence::eSeqlocPartial_Nostart)  &&  (start > 1) ) {
        try {
            CSeqVector::TResidue res = vec[start - 1];
        
            if ( IsResidue(res)  &&  isalpha (res)) {
                if ( res == 'N' ) {
                    result = true;
                }
            }
        } catch ( exception& ) {
            return false;
        }
    }

    return result;    
}


CBioseq_Handle BioseqHandleFromLocation (CScope* m_Scope, const CSeq_loc& loc)

{
    CBioseq_Handle bsh;
    for ( CSeq_loc_CI citer (loc); citer; ++citer) {
        const CSeq_id& id = citer.GetSeq_id();
        CSeq_id_Handle sih = CSeq_id_Handle::GetHandle(id);
        bsh = m_Scope->GetBioseqHandle (sih, CScope::eGetBioseq_All);
        if (bsh) {
            return bsh;
        }
    }
    return bsh;
}


bool s_PosIsNNotGap(const CSeqVector& vec, int pos)
{
    if (pos >= vec.size()) {
        return false;
    } else if (vec[pos] != 'N' && vec[pos] != 'n') {
        return false;
    } else if (vec.IsInGap(pos)) {
        return false;
    } else {
        return true;
    }
}


bool ShouldCheckForNsAndGap(const CBioseq_Handle& bsh)
{
    if (!bsh || bsh.GetInst_Length() < 10 || (bsh.IsSetInst_Topology() && bsh.GetInst_Topology() == CSeq_inst::eTopology_circular)) {
        return false;
    } else {
        return true;
    }
}


void CheckBioseqEndsForNAndGap
(const CSeqVector& vec,
EBioseqEndIsType& begin_n,
EBioseqEndIsType& begin_gap,
EBioseqEndIsType& end_n,
EBioseqEndIsType& end_gap,
bool& begin_ambig,
bool& end_ambig)
{
    begin_n = eBioseqEndIsType_None;
    begin_gap = eBioseqEndIsType_None;
    end_n = eBioseqEndIsType_None;
    end_gap = eBioseqEndIsType_None;
    begin_ambig = false;
    end_ambig = false;

    if (vec.size() < 10) {
        return;
    }

    try {

        // check for gap at begining of sequence
        if (vec.IsInGap(0) /* || vec.IsInGap(1) */) {
            begin_gap = eBioseqEndIsType_All;
            for (int i = 0; i < 10; i++) {
                if (!vec.IsInGap(i)) {
                    begin_gap = eBioseqEndIsType_Last;
                    break;
                }
            }
        }

        // check for gap at end of sequence
        if ( /* vec.IsInGap (vec.size() - 2) || */ vec.IsInGap(vec.size() - 1)) {
            end_gap = eBioseqEndIsType_All;
            for (int i = vec.size() - 11; i < vec.size(); i++) {
                if (!vec.IsInGap(i)) {
                    end_gap = eBioseqEndIsType_Last;
                    break;
                }
            }
        }

        if (vec.IsNucleotide()) {
            // check for N bases at beginning of sequence
            if (s_PosIsNNotGap(vec, 0) /* || s_PosIsNNotGap(vec, 1) */) {
                begin_n = eBioseqEndIsType_All;
                for (int i = 0; i < 10; i++) {
                    if (!s_PosIsNNotGap(vec, i)) {
                        begin_n = eBioseqEndIsType_Last;
                        break;
                    }
                }
            }

            // check for N bases at end of sequence
            if ( /* s_PosIsNNotGap(vec, vec.size() - 2) || */ s_PosIsNNotGap(vec, vec.size() - 1)) {
                end_n = eBioseqEndIsType_All;
                for (int i = vec.size() - 10; i < vec.size(); i++) {
                    if (!s_PosIsNNotGap(vec, i)) {
                        end_n = eBioseqEndIsType_Last;
                        break;
                    }
                }
            }

            // check for ambiguous concentration
            size_t check_len = 50;
            if (vec.size() < 50) {
                check_len = vec.size();
            }
            size_t num_ns = 0;
            for (size_t i = 0; i < check_len; i++) {
                if (vec[i] == 'N') {
                    num_ns++;
                    if (num_ns >= 5 && i < 10) {
                        begin_ambig = true;
                        break;
                    } else if (num_ns >= 15) {
                        begin_ambig = true;
                        break;
                    }
                }
            }
            num_ns = 0;
            for (int i = 0; i < check_len; i++) {
                if (vec[vec.size() - i - 1] == 'N') {
                    num_ns++;
                    if (num_ns >= 5 && i < 10) {
                        end_ambig = true;
                        break;
                    } else if (num_ns >= 15) {
                        end_ambig = true;
                        break;
                    }
                }
            }
        }
    } catch (exception&) {
        // if there are exceptions, cannot perform this calculation
    }
}


void CheckBioseqEndsForNAndGap 
(const CBioseq_Handle& bsh,
 EBioseqEndIsType& begin_n,
 EBioseqEndIsType& begin_gap,
 EBioseqEndIsType& end_n,
 EBioseqEndIsType& end_gap,
 bool& begin_ambig,
 bool& end_ambig)
{
    begin_n = eBioseqEndIsType_None;
    begin_gap = eBioseqEndIsType_None;
    end_n = eBioseqEndIsType_None;
    end_gap = eBioseqEndIsType_None;
    begin_ambig = false;
    end_ambig = false;
    if (!ShouldCheckForNsAndGap(bsh)) {
        return;
    }

    try {
        // check for gap at begining of sequence
        CSeqVector vec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        CheckBioseqEndsForNAndGap(vec, begin_n, begin_gap, end_n, end_gap, begin_ambig, end_ambig);
    } catch ( exception& ) {
        // if there are exceptions, cannot perform this calculation
    }
}


bool IsLocFullLength (const CSeq_loc& loc, const CBioseq_Handle& bsh)
{
    if (loc.IsInt() 
        && loc.GetInt().GetFrom() == 0
        && loc.GetInt().GetTo() == bsh.GetInst_Length() - 1) {
        return true;
    } else {
        return false;
    }
}


bool PartialsSame (const CSeq_loc& loc1, const CSeq_loc& loc2)
{
    bool loc1_partial_start =
        loc1.IsPartialStart(eExtreme_Biological);
    bool loc1_partial_stop =
        loc1.IsPartialStop(eExtreme_Biological);
    bool loc2_partial_start =
        loc2.IsPartialStart(eExtreme_Biological);
    bool loc2_partial_stop =
        loc2.IsPartialStop(eExtreme_Biological);
    if (loc1_partial_start == loc2_partial_start  &&
        loc1_partial_stop == loc2_partial_stop) {
        return true;
    } else {
        return false;
    }
}




// Code for finding duplicate features
bool s_IsSameStrand(const CSeq_loc& l1, const CSeq_loc& l2, CScope& scope)
{
    ENa_strand s1 = sequence::GetStrand(l1, &scope);
    ENa_strand s2 = sequence::GetStrand(l2, &scope);
    if ((s1 == eNa_strand_minus && s2 == eNa_strand_minus)
        || (s1 != eNa_strand_minus && s2 != eNa_strand_minus)) {
        return true;
    } else {
        return false;
    }
}


inline
bool s_IsSameSeqAnnot(const CSeq_feat_Handle& f1, const CSeq_feat_Handle& f2, bool& diff_descriptions)
{
    const auto& annot1 = f1.GetAnnot();
    const auto& annot2 = f2.GetAnnot();
    bool rval = annot1 == annot2;
    diff_descriptions = false;
    if (!rval) {
        if ((!annot1.Seq_annot_IsSetDesc() || annot1.Seq_annot_GetDesc().Get().empty()) &&
            (!annot2.Seq_annot_IsSetDesc() || annot2.Seq_annot_GetDesc().Get().empty())) {
            // neither is set
            diff_descriptions = false;
        } else if (annot1.Seq_annot_IsSetDesc() && annot2.Seq_annot_IsSetDesc()) {
            // both are set - are they different?
            const auto d1 = annot1.Seq_annot_GetDesc().Get().front();
            const auto d2 = annot2.Seq_annot_GetDesc().Get().front();
            if (d1->Which() != d2->Which()) {
                diff_descriptions = true;
            } else {
                if (d1->IsName() 
                    && NStr::EqualNocase(d1->GetName(), d2->GetName())) {
                    diff_descriptions = false;
                } else if (d1->IsTitle()
                    && NStr::EqualNocase(d1->GetTitle(), d2->GetTitle())) {
                    diff_descriptions = false;
                } else {
                    diff_descriptions = true;
                }
            }
        } else {
            diff_descriptions = true;
        }
    }
    return rval;
}


bool s_AreGBQualsIdentical(const CSeq_feat_Handle& feat1, const CSeq_feat_Handle& feat2, bool case_sensitive)
{
    if (!feat1.IsSetQual() || !feat2.IsSetQual()) {
        return true;
    }

    bool rval = true;

    CSeq_feat::TQual::const_iterator gb1 = feat1.GetQual().begin();
    CSeq_feat::TQual::const_iterator gb1_end = feat1.GetQual().end();
    CSeq_feat::TQual::const_iterator gb2 = feat2.GetQual().begin();
    CSeq_feat::TQual::const_iterator gb2_end = feat2.GetQual().end();

    while ((gb1 != gb1_end) && (gb2 != gb2_end) && rval) {
        if (!(*gb1)->IsSetQual()) {
            if ((*gb2)->IsSetQual()) {
                rval = false;
            }
        } else if (!(*gb2)->IsSetQual()) {
            rval = false;
        } else if (!NStr::Equal ((*gb1)->GetQual(), (*gb2)->GetQual())) {
            rval = false;
        }
        if (rval) {
            string v1 = (*gb1)->IsSetVal() ? (*gb1)->GetVal() : "";
            string v2 = (*gb2)->IsSetVal() ? (*gb2)->GetVal() : "";
            NStr::TruncateSpacesInPlace(v1);
            NStr::TruncateSpacesInPlace(v2);
            rval = NStr::Equal(v1, v2, case_sensitive ? NStr::eCase : NStr::eNocase);
        }
        ++gb1;
        ++gb2;
    }
    if (gb1 != gb1_end || gb2 != gb2_end) {
        rval = false;
    }

    return rval;
}


bool s_AreFeatureLabelsSame(const CSeq_feat_Handle& feat, const CSeq_feat_Handle& prev, bool case_sensitive)
{
    if (!feat.GetData().Equals(prev.GetData())) {
        return false;
    }

    // compare labels and comments
    bool same_label = true;
    const string& curr_comment =
        feat.IsSetComment() ? feat.GetComment() : kEmptyStr;
    const string& prev_comment =
        prev.IsSetComment() ? prev.GetComment() : kEmptyStr;
    string curr_label = "";
    string prev_label = "";

    feature::GetLabel(*(feat.GetSeq_feat()),
        &curr_label, feature::fFGL_Content, &(feat.GetScope()));
    feature::GetLabel(*(prev.GetSeq_feat()),
        &prev_label, feature::fFGL_Content, &(prev.GetScope()));

    bool comments_same = NStr::Equal(curr_comment, prev_comment, case_sensitive ? NStr::eCase : NStr::eNocase);
    bool labels_same = NStr::Equal(curr_label, prev_label, case_sensitive ? NStr::eCase : NStr::eNocase);

    if (!comments_same || !labels_same) {
        same_label = false;
    } else if (!s_AreGBQualsIdentical(feat, prev, case_sensitive)) {
        same_label = false;
    }
    return same_label;
}


bool s_IsDifferentDbxrefs(const TDbtags& list1, const TDbtags& list2)
{
    if (list1.empty()  ||  list2.empty()) {
        return false;
    } else if (list1.size() != list2.size()) {
        return true;
    }

    TDbtags::const_iterator it1 = list1.begin();
    TDbtags::const_iterator it2 = list2.begin();
    for (; it1 != list1.end(); ++it1, ++it2) {
        if (!NStr::EqualNocase((*it1)->GetDb(), (*it2)->GetDb())) {
            return true;
        }
        string str1 =
            (*it1)->GetTag().IsStr() ? (*it1)->GetTag().GetStr() : "";
        string str2 =
            (*it2)->GetTag().IsStr() ? (*it2)->GetTag().GetStr() : "";
        if ( str1.empty()  &&  str2.empty() ) {
            if (!(*it1)->GetTag().IsId()  &&  !(*it2)->GetTag().IsId()) {
                continue;
            } else if ((*it1)->GetTag().IsId()  &&  (*it2)->GetTag().IsId()) {
                if ((*it1)->GetTag().GetId() != (*it2)->GetTag().GetId()) {
                    return true;
                }
            } else {
                return true;
            }
        } else if (!str1.empty() && !str2.empty() && !NStr::EqualNocase(str1, str2)) {
            return true;
        }
    }
    return false;
}


bool s_AreFullLengthCodingRegionsWithDifferentFrames (const CSeq_feat_Handle& f1, const CSeq_feat_Handle& f2)
{
    const auto & f1data = f1.GetData();
    const auto & f2data = f2.GetData();
    if (!f1data.IsCdregion() || !f2data.IsCdregion()) {
        return false;
    }
    const auto & cd1 = f1data.GetCdregion();
    const auto & cd2 = f2data.GetCdregion();

    int frame1 = 1, frame2 = 1;
    if (cd1.IsSetFrame()) {
        frame1 = cd1.GetFrame();
        if (frame1 == 0) {
            frame1 = 1;
        }
    }
    if (cd2.IsSetFrame()) {
        frame2 = cd2.GetFrame();
        if (frame2 == 0) {
            frame2 = 1;
        }
    }
    if (frame1 == frame2) {
        return false;
    }

    CBioseq_Handle bsh1 = f1.GetScope().GetBioseqHandle(f1.GetLocation());
    if (!IsLocFullLength (f1.GetLocation(), bsh1)) {
        return false;
    }
    CBioseq_Handle bsh2 = f2.GetScope().GetBioseqHandle(f2.GetLocation());
    if (!IsLocFullLength (f2.GetLocation(), bsh2)) {
        return false;
    }

    return true;
}


//LCOV_EXCL_START
// never used, because different variations generate different labels
string s_ReplaceListFromQuals(const CSeq_feat::TQual& quals)
{
    string replace = "";
    ITERATE(CSeq_feat::TQual, q, quals) {
        if ((*q)->IsSetQual() && NStr::Equal((*q)->GetQual(), "replace") && (*q)->IsSetVal()) {
            if (NStr::IsBlank((*q)->GetVal())) {
                replace += " ";
            } else {
                replace += (*q)->GetVal();
            }
            replace += ".";
        }
    }
    return replace;
}


bool s_AreDifferentVariations(const CSeq_feat_Handle& f1, const CSeq_feat_Handle& f2)
{
    if (f1.GetData().GetSubtype() != CSeqFeatData::eSubtype_variation
        || f2.GetData().GetSubtype() != CSeqFeatData::eSubtype_variation) {
        return false;
    }
    if (!f1.IsSetQual() || !f2.IsSetQual()) {
        return false;
    }
    string replace1 = s_ReplaceListFromQuals(f1.GetQual());
    string replace2 = s_ReplaceListFromQuals(f2.GetQual());

    if (!NStr::Equal(replace1, replace2)) {
        return true;
    } else {
        return false;
    }
}
//LCOV_EXCL_STOP


typedef vector<CConstRef<CObject_id> > TFeatIdVec;
static bool s_AreLinkedToDifferentFeats (const CSeq_feat_Handle& f1, const CSeq_feat_Handle& f2, CSeqFeatData::ESubtype s1, CSeqFeatData::ESubtype s2)
{
    bool rval = false;
    
    if (f1.GetData().GetSubtype() == s1 && f2.GetData().GetSubtype() == s1) {
        CScope& scope = f1.GetScope();
        const CSeq_loc& loc = f1.GetLocation();
        CBioseq_Handle bsh = BioseqHandleFromLocation (&scope, loc);
        if (bsh) {
            const CTSE_Handle& tse = bsh.GetTSE_Handle();
            TFeatIdVec mrna1_id;
            TFeatIdVec mrna2_id;
            list<CSeq_feat_Handle> mrna1;
            list<CSeq_feat_Handle> mrna2;

            FOR_EACH_SEQFEATXREF_ON_SEQFEAT (itx, *(f1.GetSeq_feat())) {
                if ((*itx)->IsSetId() && (*itx)->GetId().IsLocal()) {
                    const CObject_id& feat_id = (*itx)->GetId().GetLocal();
                    vector<CSeq_feat_Handle> handles = tse.GetFeaturesWithId(CSeqFeatData::e_not_set, feat_id);
                    ITERATE( vector<CSeq_feat_Handle>, feat_it, handles ) {
                        if (feat_it->IsSetData() 
                            && feat_it->GetData().GetSubtype() == s2) {
                            mrna1.push_back(*feat_it);
                            CConstRef<CObject_id> f(&feat_id);
                            mrna1_id.push_back (f);
                            break;
                        }
                    }
                }
            }
            FOR_EACH_SEQFEATXREF_ON_SEQFEAT (itx, *(f2.GetSeq_feat())) {
                if ((*itx)->IsSetId() && (*itx)->GetId().IsLocal()) {
                    const CObject_id& feat_id = (*itx)->GetId().GetLocal();
                    vector<CSeq_feat_Handle> handles = tse.GetFeaturesWithId(CSeqFeatData::e_not_set, feat_id);
                    ITERATE( vector<CSeq_feat_Handle>, feat_it, handles ) {
                        if (feat_it->IsSetData() 
                            && feat_it->GetData().GetSubtype() == s2) {
                            mrna2.push_back(*feat_it);
                            CConstRef<CObject_id> f(&feat_id);
                            mrna2_id.push_back (f);
                        }
                    }
                }
            }

            if (mrna1_id.size() > 0 && mrna2_id.size() > 0) {
                rval = true;
                for (auto i1 = mrna1_id.begin(); i1 != mrna1_id.end(); ++i1) {
                    for (auto i2 = mrna2_id.begin(); i2 != mrna2_id.end(); ++i2) {
                        if ((*i1)->Equals(**i2)) {
                            rval = false;
                            break;
                        }
                    }
                    if (!rval) {
                        break;
                    }
                }

                if (rval) { // Check that locations aren't the same
                    const CSeq_feat_Handle fh1 = mrna1.front();
                    const CSeq_feat_Handle fh2 = mrna2.front();


                    if (s_IsSameStrand(fh1.GetLocation(),
                                       fh2.GetLocation(),
                                       fh1.GetScope()) 
                      && (sequence::Compare(fh1.GetLocation(), 
                                           fh2.GetLocation(),
                                           &(fh1.GetScope()),
                                           sequence::fCompareOverlapping) == sequence::eSame)) {
                        rval = false;
                    }
                }
            }
        }
    }
    return rval;
}




static bool s_AreCodingRegionsLinkedToDifferentmRNAs (const CSeq_feat_Handle& f1, const CSeq_feat_Handle& f2)
{
    return s_AreLinkedToDifferentFeats (f1, f2, CSeqFeatData::eSubtype_cdregion, CSeqFeatData::eSubtype_mRNA);
}


bool s_AremRNAsLinkedToDifferentCodingRegions (const CSeq_feat_Handle& f1, const CSeq_feat_Handle& f2)
{
    return s_AreLinkedToDifferentFeats (f1, f2, CSeqFeatData::eSubtype_mRNA, CSeqFeatData::eSubtype_cdregion);
}


bool IsDicistronicGene(const CSeq_feat_Handle& f)
{
    if ( f.GetData().GetSubtype() != CSeqFeatData::eSubtype_gene ) return false;
    return IsDicistronic(f);
}


bool IsDicistronic(const CSeq_feat_Handle& f)
{
    if (!f.IsSetExcept()) return false;
    if (!f.IsSetExcept_text()) return false;

    const string& except_text = f.GetExcept_text();
    if (NStr::FindNoCase(except_text, "dicistronic gene") == NPOS) return false;

    return true;
}


EDuplicateFeatureType 
IsDuplicate 
(const CSeq_feat_Handle& f1,
 const CSeq_feat_Handle& f2,
 bool check_partials,
 bool case_sensitive)
{

    EDuplicateFeatureType dup_type = eDuplicate_Not;

    // subtypes
    CSeqFeatData::ESubtype feat1_subtype = f1.GetData().GetSubtype();
    CSeqFeatData::ESubtype feat2_subtype = f2.GetData().GetSubtype();

    // not duplicates if not the same subtype
    if (feat1_subtype != feat2_subtype) {
        return eDuplicate_Not;
    }

    // locations
    const CSeq_loc& feat1_loc = f1.GetLocation();
    const CSeq_loc& feat2_loc = f2.GetLocation();

    // not duplicates if not the same location and strand
    if (!s_IsSameStrand(feat1_loc, feat2_loc, f1.GetScope())  ||
        sequence::Compare(feat1_loc, feat2_loc, &(f1.GetScope()),
                            sequence::fCompareOverlapping) != sequence::eSame) {
        return eDuplicate_Not;
    }

    // same annot?
    bool diff_annot_desc = false;
    bool same_annot = s_IsSameSeqAnnot(f1, f2, diff_annot_desc);

    if (diff_annot_desc) {
        // don't report if features on different annots with different titles or names
        return eDuplicate_Not;
    }

    // compare labels and comments
    bool same_label = s_AreFeatureLabelsSame (f1, f2, case_sensitive);

    // compare dbxrefs
    bool different_dbxrefs = (f1.IsSetDbxref() && f2.IsSetDbxref() && 
                        s_IsDifferentDbxrefs(f1.GetDbxref(), f2.GetDbxref()));

    if ( feat1_subtype == CSeqFeatData::eSubtype_region && different_dbxrefs) {
        return eDuplicate_Not;
    }

    // check for frame difference
    bool full_length_coding_regions_with_different_frames = 
                      s_AreFullLengthCodingRegionsWithDifferentFrames(f1, f2);
    if (!same_label && full_length_coding_regions_with_different_frames) {
        // do not report if both coding regions are full length, have different products,
        // and have different frames
        return eDuplicate_Not;
    }

    if ((feat1_subtype == CSeqFeatData::eSubtype_variation && !same_label) || s_AreDifferentVariations(f1, f2)) {
        // don't report variations if replace quals are different or labels are different
        return eDuplicate_Not;
    }


    if (s_AreCodingRegionsLinkedToDifferentmRNAs(f1, f2)) {
        // do not report if features are coding regions linked to different mRNAs
        return eDuplicate_Not;
    }


    if (s_AremRNAsLinkedToDifferentCodingRegions(f1, f2)) {
        // do not report if features are mRNAs linked to different coding regions
        return eDuplicate_Not;
    }


    // only report pubs if they have the same label
    if (feat1_subtype == CSeqFeatData::eSubtype_pub && !same_label) {
        return eDuplicate_Not;
    }

    bool partials_ok = (!check_partials || PartialsSame(feat1_loc, feat2_loc));

    if (!partials_ok) {
        return eDuplicate_Not;
    }

    if ( same_annot ) {
        if (same_label) {
            dup_type = eDuplicate_Duplicate;
        } else {
            dup_type = eDuplicate_SameIntervalDifferentLabel;
        }
    } else {
        if (same_label) {
            dup_type = eDuplicate_DuplicateDifferentTable;
        } else if ( feat2_subtype != CSeqFeatData::eSubtype_pub ) {
            dup_type = eDuplicate_SameIntervalDifferentLabelDifferentTable;
        }
    }

    return dup_type;        
}

// specific-host functions

bool IsCommonName (const CT3Data& data)
{
    bool is_common = false;
    
    if (data.IsSetStatus()) {
        ITERATE (CT3Reply::TData::TStatus, status_it, data.GetStatus()) {
            if ((*status_it)->IsSetProperty() 
                && NStr::Equal((*status_it)->GetProperty(), "old_name_class", NStr::eNocase)) {
                if ((*status_it)->IsSetValue() && (*status_it)->GetValue().IsStr()) {
                    string value_str = (*status_it)->GetValue().GetStr();
                    if (NStr::Equal(value_str, "common name", NStr::eCase) 
                        || NStr::Equal(value_str, "genbank common name", NStr::eCase)) {
                        is_common = true;
                        break;
                    }
                }
            }
        }
    }
    return is_common;
}

bool HasMisSpellFlag (const CT3Data& data)
{
    bool has_misspell_flag = false;

    if (data.IsSetStatus()) {
        ITERATE (CT3Reply::TData::TStatus, status_it, data.GetStatus()) {
            if ((*status_it)->IsSetProperty()) {
                string prop = (*status_it)->GetProperty();
                if (NStr::EqualNocase(prop, "misspelled_name")) {
                    has_misspell_flag = true;
                    break;
                }
            }
        }
    }
    return has_misspell_flag;
}


bool FindMatchInOrgRef (const string& str, const COrg_ref& org)
{
    string match = "";

    if (NStr::IsBlank(str)) {
        // do nothing;
    } else if (org.IsSetTaxname() && NStr::EqualNocase(str, org.GetTaxname())) {
        match = org.GetTaxname();
    } else if (org.IsSetCommon() && NStr::EqualNocase(str, org.GetCommon())) {
        match = org.GetCommon();
    } else {
        FOR_EACH_SYN_ON_ORGREF (syn_it, org) {
            if (NStr::EqualNocase(str, *syn_it)) {
                match = *syn_it;
                break;
            }
        }
        if (NStr::IsBlank(match) && org.IsSetOrgname()) {
            const COrgName& orgname = org.GetOrgname();
            if (orgname.IsSetMod()) {
                for (const auto mod_it : orgname.GetMod()) {
                    if (mod_it->IsSetSubtype()
                        && (mod_it->GetSubtype() == COrgMod::eSubtype_gb_synonym
                            || mod_it->GetSubtype() == COrgMod::eSubtype_old_name)
                        && mod_it->IsSetSubname()
                        && NStr::EqualNocase(str, mod_it->GetSubname())) {
                        match = mod_it->GetSubname();
                        break;
                    }
                }
            }
        }
    }
    return NStr::EqualCase(str, match);
}


static const string sIgnoreHostWordList[] = {
  " cf.",
  " cf ",
  " aff ",
  " aff.",
  " near",
  " nr.",
  " nr "
};


static const int kNumIgnoreHostWordList = sizeof (sIgnoreHostWordList) / sizeof (string);

void AdjustSpecificHostForTaxServer (string& spec_host)
{
    for (int i = 0; i < kNumIgnoreHostWordList; i++) {
        NStr::ReplaceInPlace(spec_host, sIgnoreHostWordList[i], " ");
    }
    NStr::ReplaceInPlace(spec_host, "  ", " ");
    NStr::TruncateSpacesInPlace(spec_host);
}


string SpecificHostValueToCheck(const string& val)
{
    if (NStr::IsBlank(val)) {
        return val;
#if 0
    } else if (! isupper (val.c_str()[0])) {
        return kEmptyStr;
#endif
    }

    string host = val;
    // ignore portion after semicolon
    size_t pos = NStr::Find(host, ";");
    if (pos != string::npos) {
        host = host.substr(0, pos);
    }
    NStr::TruncateSpacesInPlace(host);
    // must have at least two words to check
    pos = NStr::Find(host, " "); // combine with next line
    if (pos == string::npos) {
        return kEmptyStr;
    }

    AdjustSpecificHostForTaxServer(host);
    pos = NStr::Find(host, " ");
    if (NStr::StartsWith(host.substr(pos + 1), "hybrid ")) {
        pos += 7;
    } else if (NStr::StartsWith(host.substr(pos + 1), "x ")) {
        pos += 2;
    }
    if (! NStr::StartsWith(host.substr(pos + 1), "sp.")
        && ! NStr::StartsWith(host.substr(pos + 1), "(")) {
        pos = NStr::Find(host, " ", pos + 1);
        if (pos != string::npos) {
            host = host.substr(0, pos);
        }
    } else {
        host = host.substr(0, pos);
    }
    return host;
}


string InterpretSpecificHostResult(const string& host, const CT3Reply& reply, const string& orig_host)
{
    string err_str = "";
    if (reply.IsError()) {
        err_str = "?";
        if (reply.GetError().IsSetMessage()) {
            err_str = reply.GetError().GetMessage();
        }
        if(NStr::FindNoCase(err_str, "ambiguous") != string::npos) {
            err_str = "Specific host value is ambiguous: " + 
                (NStr::IsBlank(orig_host) ? host : orig_host);
        } else {
            err_str = "Invalid value for specific host: " + 
                (NStr::IsBlank(orig_host) ? host : orig_host);
        }
    } else if (reply.IsData()) {
        const auto& rdata = reply.GetData();
        if (HasMisSpellFlag(rdata)) {
            err_str = "Specific host value is misspelled: " + 
                (NStr::IsBlank(orig_host) ? host : orig_host);
        } else if (rdata.IsSetOrg()) {
            const auto& org = rdata.GetOrg();
            if (NStr::StartsWith(org.GetTaxname(), host)) {
                // do nothing, all good
            } else if (IsCommonName(rdata)) {
                // not actionable
            } else if (FindMatchInOrgRef(host, org)) {
                // replace with synonym
                err_str = "Specific host value is alternate name: " + 
                    orig_host + " should be " + 
                    org.GetTaxname();
            } else {
                err_str = "Specific host value is incorrectly capitalized: " + 
                    (NStr::IsBlank(orig_host) ? host : orig_host);
            }
        } else {
            err_str = "Invalid value for specific host: " + 
                (NStr::IsBlank(orig_host) ? host : orig_host);
        }
    }
    return err_str;
}


bool IsCommon(const COrg_ref& org, const string& val)
{
    bool is_common = false;
    if (org.IsSetCommon() && NStr::EqualNocase(val, org.GetCommon())) {
        // common name, not genus
        is_common = true;
    } else if (org.IsSetOrgMod()) {
        for (auto it : org.GetOrgname().GetMod()) {
            if (it->IsSetSubtype() &&
                it->GetSubtype() == COrgMod::eSubtype_common &&
                it->IsSetSubname() &&
                NStr::EqualNocase(it->GetSubname(), val)) {
                is_common = true;
                break;
            }
        }
    }
    return is_common;
}


bool IsLikelyTaxname(const string& val)
{
    if (!isalpha(val[0])) {
        return false;
    }
    size_t pos = NStr::Find(val, " ");
    if (pos == NPOS) {
        return false;
    }

    CTaxon1 taxon1;
    taxon1.Init();
    int taxid = taxon1.GetTaxIdByName(val.substr(0, pos));
    if (taxid == 0 || taxid == -1) {
        return false;
    }

    bool is_species = false;
    bool is_uncultured = false;
    string blast_name;
    CConstRef<COrg_ref> org = taxon1.GetOrgRef(taxid, is_species, is_uncultured, blast_name);
    if (org && IsCommon(*org, val.substr(0, pos))) {
        return false;
    } else {
        return true;
    }
}


//LCOV_EXCL_START
//not used by asnvalidate but used by other applications
bool IsSpecificHostValid(const string& val, string& error_msg)
{
    CTaxValidationAndCleanup tval;
    return tval.IsOneSpecificHostValid(val, error_msg);
}


string FixSpecificHost(const string& val)
{
    string hostfix = val;
    validator::CTaxValidationAndCleanup tval;
    tval.FixOneSpecificHost(hostfix);
    
    return hostfix;
}


static char s_ConvertChar(char ch)
{
    if (ch < 0x02 || ch > 0x7F) {
        // no change
    }
    else if (isalpha(ch)) {
        ch = tolower(ch);
    }
    else if (isdigit(ch)) {
        // no change
    }
    else if (ch == '\'' || ch == '/' || ch == '@' || ch == '`' || ch == ',') {
        // no change
    }
    else {
        ch = 0x20;
    }
    return ch;
}


void ConvertToEntrezTerm(string& title)
{
    string::iterator s = title.begin();
    char p = ' ';
    while (s != title.end()) {
        *s = s_ConvertChar(*s);
        if (isspace(*s) && isspace(p)) {
            s = title.erase(s);
        }
        else {
            p = *s;
            ++s;
        }
    }
    NStr::TruncateSpacesInPlace(title);
}
//LCOV_EXCL_STOP


void FixGeneticCode(CCdregion& cdr)
{
    if (!cdr.IsSetCode()) {
        return;
    }
    const auto& gcode = cdr.GetCode();
    CGenetic_code::C_E::TId genCode = 0;
    for (auto it : gcode.Get()) {
        if (it->IsId()) {
            genCode = it->GetId();
        }
    }

    if (genCode == 7) {
        genCode = 4;
    } else if (genCode == 8) {
        genCode = 1;
    } else if (genCode == 0) {
        genCode = 1;
    }
    cdr.ResetCode();
    CRef<CGenetic_code::C_E> new_code(new CGenetic_code::C_E());
    new_code->SetId(genCode);
    cdr.SetCode().Set().push_back(new_code);
}


string TranslateCodingRegionForValidation(const CSeq_feat& feat, CScope &scope, bool& alt_start)
{
    string transl_prot = kEmptyStr;
    CRef<CSeq_feat> tmp_cds(new CSeq_feat());
    tmp_cds->Assign(feat);
    FixGeneticCode(tmp_cds->SetData().SetCdregion());
    const CCdregion& cdregion = tmp_cds->GetData().GetCdregion();
    const CSeq_loc& cds_loc = tmp_cds->GetLocation();
    if (cds_loc.IsWhole()) {
        CBioseq_Handle bsh = scope.GetBioseqHandle(cds_loc.GetWhole());
        if (!bsh) {
            return kEmptyStr;
        }
        size_t start = 0;
        if (cdregion.IsSetFrame()) {
            if (cdregion.GetFrame() == 2) {
                start = 1;
            } else if (cdregion.GetFrame() == 3) {
                start = 2;
            }
        }
        const CGenetic_code* genetic_code = NULL;
        if (cdregion.IsSetCode()) {
            genetic_code = &(cdregion.GetCode());
        }
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(cds_loc.GetWhole());
        CRef<CSeq_loc> tmp(new CSeq_loc(*id, start, bsh.GetInst_Length() - 1));
        CSeqTranslator::Translate(*tmp, scope, transl_prot, genetic_code, true, false, &alt_start);
    } else {
        CSeqTranslator::Translate(*tmp_cds, scope, transl_prot,
            true,   // include stop codons
            false,  // do not remove trailing X/B/Z
            &alt_start);
    }

    return transl_prot;
}


bool HasBadStartCodon(const CSeq_loc& loc, const string& transl_prot)
{
    bool got_dash = (transl_prot[0] == '-');
    bool got_x = (transl_prot[0] == 'X'
        && !loc.IsPartialStart(eExtreme_Biological));

    if (!got_dash && !got_x) {
        return false;
    }
    return true;
}


static const char * kUnclassifiedTranslationDiscrepancy = "unclassified translation discrepancy";

static const char* const sc_BypassCdsTransCheckText[] = {
    "RNA editing",
    "adjusted for low-quality genome",
    "annotated by transcript or proteomic data",
    "rearrangement required for product",
    "reasons given in citation",
    "translated product replaced",
    kUnclassifiedTranslationDiscrepancy
};
typedef CStaticArraySet<const char*, PCase_CStr> TBypassCdsTransCheckSet;
DEFINE_STATIC_ARRAY_MAP(TBypassCdsTransCheckSet, sc_BypassCdsTransCheck, sc_BypassCdsTransCheckText);

static const char* const sc_ForceCdsTransCheckText[] = {
    "artificial frameshift",
    "mismatches in translation"
};
typedef CStaticArraySet<const char*, PCase_CStr> TForceCdsTransCheckSet;
DEFINE_STATIC_ARRAY_MAP(TForceCdsTransCheckSet, sc_ForceCdsTransCheck, sc_ForceCdsTransCheckText);

bool ReportTranslationErrors(const string& except_text)
{
    bool report = true;
    ITERATE(TBypassCdsTransCheckSet, it, sc_BypassCdsTransCheck) {
        if (NStr::FindNoCase(except_text, *it) != NPOS) {
            report = false;
        }
    }
    if (!report) {
        ITERATE(TForceCdsTransCheckSet, it, sc_ForceCdsTransCheck) {
            if (NStr::FindNoCase(except_text, *it) != NPOS) {
                report = true;
            }
        }
    }
    return report;
}


//LCOV_EXCL_START
//not used by asnvalidate but used by other applications
bool HasBadStartCodon(const CSeq_feat& feat, CScope& scope, bool ignore_exceptions)
{
    if (!feat.IsSetData() || !feat.GetData().IsCdregion()) {
        return false;
    }
    // do not validate for pseudo gene
    FOR_EACH_GBQUAL_ON_FEATURE(it, feat) {
        if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), "pseudo")) {
            return false;
        }
    }

    if (!ignore_exceptions && feat.CanGetExcept() && feat.GetExcept() &&
        feat.CanGetExcept_text()) {
        if (!ReportTranslationErrors(feat.GetExcept_text())) {
            return false;
        }
    }

    bool alt_start = false;
    string transl_prot;
    try {
        transl_prot = TranslateCodingRegionForValidation(feat, scope, alt_start);
    } catch (CException& ) {
        return false;
    }
    return HasBadStartCodon(feat.GetLocation(), transl_prot);
}
//LCOV_EXCL_STOP


size_t CountInternalStopCodons(const string& transl_prot)
{
    if (NStr::IsBlank(transl_prot)) {
        return 0;
    }
    // count internal stops and Xs
    size_t internal_stop_count = 0;

    ITERATE(string, it, transl_prot) {
        if (*it == '*') {
            ++internal_stop_count;
        }
    }
    // if stop at end, reduce count by one (since one of the stops counted isn't internal)
    if (transl_prot[transl_prot.length() - 1] == '*') {
        --internal_stop_count;
    }
    return internal_stop_count;
}


//LCOV_EXCL_START
//not used by asnvalidate but used by other applications
bool HasInternalStop(const CSeq_feat& feat, CScope& scope, bool ignore_exceptions)
{
    if (!feat.IsSetData() || !feat.GetData().IsCdregion()) {
        return false;
    }
    // do not validate for pseudo gene
    FOR_EACH_GBQUAL_ON_FEATURE(it, feat) {
        if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), "pseudo")) {
            return false;
        }
    }

    if (!ignore_exceptions && feat.CanGetExcept() && feat.GetExcept() &&
        feat.CanGetExcept_text()) {
        const string& except_text = feat.GetExcept_text();
        if (NStr::Find(except_text, kUnclassifiedTranslationDiscrepancy) == string::npos
            && !ReportTranslationErrors(feat.GetExcept_text())) {
            return false;
        }
    }

    bool alt_start = false;
    string transl_prot;
    try {
        transl_prot = TranslateCodingRegionForValidation(feat, scope, alt_start);
    } catch (CException& ) {
        return false;
    }

    size_t internal_stop_codons = CountInternalStopCodons(transl_prot);
    if (internal_stop_codons > 0) {
        return true;
    } else {
        return false;
    }
}
//LCOV_EXCL_STOP


CRef<CSeqVector> MakeSeqVectorForResidueCounting(const CBioseq_Handle& bsh)
{
    CRef<CSeqVector> sv(new CSeqVector(bsh, CBioseq_Handle::eCoding_Iupac));
    CSeq_data::E_Choice seqtyp = bsh.GetInst().IsSetSeq_data() ?
        bsh.GetInst().GetSeq_data().Which() : CSeq_data::e_not_set;
    if (seqtyp == CSeq_data::e_Ncbieaa || seqtyp == CSeq_data::e_Ncbistdaa) {
        sv->SetCoding(CSeq_data::e_Ncbieaa);
    }
    return sv;
}


bool HasBadProteinStart(const CSeqVector& sv)
{
    if (sv.size() < 1) {
        return false;
    } else if (sv.IsInGap(0) || sv[0] == '-') {
        return true;
    } else {
        return false;
    }
}


//LCOV_EXCL_START
//not used by asnvalidate but used by other applications
bool HasBadProteinStart(const CSeq_feat& feat, CScope& scope)
{
    if (!feat.IsSetData() || !feat.GetData().IsCdregion() ||
        !feat.IsSetProduct()) {
        return false;
    }
    // use try catch for those weird situations where the product is
    // not specified as a single product sequence (in which case we
    // should just skip this test)
    try {
        CBioseq_Handle bsh = scope.GetBioseqHandle(feat.GetProduct());
        if (!bsh.IsAa()) {
            return false;
        }
        CConstRef<CSeqVector> sv = MakeSeqVectorForResidueCounting(bsh);
        return HasBadProteinStart(*sv);
    } catch (CException& ) {
        return false;
    }
}
//LCOV_STOP


size_t CountProteinStops(const CSeqVector& sv)
{
    size_t terminations = 0;

    for (CSeqVector_CI sv_iter(sv); (sv_iter); ++sv_iter) {
        if (*sv_iter == '*') {
            terminations++;
        }
    }
    return terminations;
}


//LCOV_EXCL_START
//not used by asnvalidate but used by other applications
bool HasStopInProtein(const CSeq_feat& feat, CScope& scope)
{
    if (!feat.IsSetData() || !feat.GetData().IsCdregion() ||
        !feat.IsSetProduct()) {
        return false;
    }
    // use try catch for those weird situations where the product is
    // not specified as a single product sequence (in which case we
    // should just skip this test)
    try {
        CBioseq_Handle bsh = scope.GetBioseqHandle(feat.GetProduct());
        if (!bsh.IsAa()) {
            return false;
        }
        CConstRef<CSeqVector> sv = MakeSeqVectorForResidueCounting(bsh);
        if (CountProteinStops(*sv) > 0) {
            return true;
        } else {
            return false;
        }
    } catch (CException& ) {
        return false;
    }
}
//LCOV_EXCL_STOP


void FeatureHasEnds(const CSeq_feat& feat, CScope* scope, bool& no_beg, bool& no_end)
{
    unsigned int part_loc = sequence::SeqLocPartialCheck(feat.GetLocation(), scope);
    no_beg = false;
    no_end = false;

    if (part_loc & sequence::eSeqlocPartial_Start) {
        no_beg = true;
    }
    if (part_loc & sequence::eSeqlocPartial_Stop) {
        no_end = true;
    }


    if ((!no_beg || !no_end) && feat.IsSetProduct()) {
        unsigned int part_prod = sequence::SeqLocPartialCheck(feat.GetProduct(), scope);
        if (part_prod & sequence::eSeqlocPartial_Start) {
            no_beg = true;
        }
        if (part_prod & sequence::eSeqlocPartial_Stop) {
            no_end = true;
        }
    }
}


//LCOV_EXCL_START
// not used by asnvalidate but needed for other applications
CBioseq_Handle GetCDSProductSequence(const CSeq_feat& feat, CScope* scope, const CTSE_Handle & tse, bool far_fetch, bool& is_far)
{
    CBioseq_Handle prot_handle;
    is_far = false;
    if (!feat.IsSetProduct()) {
        return prot_handle;
    }
    const CSeq_id* protid = NULL;
    try {
        protid = &sequence::GetId(feat.GetProduct(), scope);
    } catch (CException&) {}
    if (protid != NULL) {
        prot_handle = scope->GetBioseqHandleFromTSE(*protid, tse);
        if (!prot_handle  &&  far_fetch) {
            prot_handle = scope->GetBioseqHandle(*protid);
            is_far = true;
        }
    }
    return prot_handle;
}
//LCOV_EXCL_STOP


void CalculateEffectiveTranslationLengths(const string& transl_prot, const CSeqVector& prot_vec, size_t &len, size_t& prot_len)
{
    len = transl_prot.length();
    prot_len = prot_vec.size();

    if (NStr::EndsWith(transl_prot, "*") && (len == prot_len + 1)) { // ok, got stop
        --len;
    }
    while (len > 0) {
        if (transl_prot[len - 1] == 'X') {  //remove terminal X
            --len;
        } else {
            break;
        }
    }

    // ignore terminal 'X' from partial last codon if present
    while (prot_len > 0) {
        if (prot_vec[prot_len - 1] == 'X') {  //remove terminal X
            --prot_len;
        } else {
            break;
        }
    }
}


//LCOV_EXCL_START
// not used by asnvalidate but needed for other applications
vector<TSeqPos> GetMismatches(const CSeq_feat& feat, const CSeqVector& prot_vec, const string& transl_prot)
{
    vector<TSeqPos> mismatches;
    size_t prot_len;
    size_t len;

    CalculateEffectiveTranslationLengths(transl_prot, prot_vec, len, prot_len);

    if (len == prot_len)  {                // could be identical
        for (TSeqPos i = 0; i < len; ++i) {
            CSeqVectorTypes::TResidue p_res = prot_vec[i];
            CSeqVectorTypes::TResidue t_res = transl_prot[i];

            if (t_res != p_res) {
                if (i == 0) {
                    bool no_beg, no_end;
                    FeatureHasEnds(feat, &(prot_vec.GetScope()), no_beg, no_end);
                    if (feat.IsSetPartial() && feat.GetPartial() && (!no_beg) && (!no_end)) {
                    } else if (t_res == '-') {
                    } else {
                        mismatches.push_back(i);
                    }
                } else {
                    mismatches.push_back(i);
                }
            }
        }
    }
    return mismatches;
}


vector<TSeqPos> GetMismatches(const CSeq_feat& feat, const CBioseq_Handle& prot_handle, const string& transl_prot)
{
    vector<TSeqPos> mismatches;
    // can't check for mismatches unless there is a product
    if (!prot_handle || !prot_handle.IsAa()) {
        return mismatches;
    }

    CSeqVector prot_vec = prot_handle.GetSeqVector();
    prot_vec.SetCoding(CSeq_data::e_Ncbieaa);

    return GetMismatches(feat, prot_vec, transl_prot);
}


bool HasNoStop(const CSeq_feat& feat, CScope* scope)
{
    bool no_beg, no_end;
    FeatureHasEnds(feat, scope, no_beg, no_end);
    if (no_end) {
        return false;
    }

    string transl_prot;
    bool alt_start;
    try {
        transl_prot = TranslateCodingRegionForValidation(feat, *scope, alt_start);
    } catch (CException& ) {
    }
    if (NStr::EndsWith(transl_prot, "*")) {
        return false;
    }

    bool show_stop = true;
    if (!no_beg && feat.IsSetPartial() && feat.GetPartial()) {
        CBioseq_Handle prot_handle;
        try {
            CBioseq_Handle bsh = scope->GetBioseqHandle(feat.GetLocation());
            const CTSE_Handle tse = bsh.GetTSE_Handle();
            bool is_far = false;
            prot_handle = GetCDSProductSequence(feat, scope, tse, true, is_far);
            if (prot_handle) {
                vector<TSeqPos> mismatches = GetMismatches(feat, prot_handle, transl_prot);
                if (mismatches.size() == 0) {
                    show_stop = false;
                }
            }
        } catch (CException& ) {
        }
    }

    return show_stop;
}
//LCOV_EXCL_STOP


bool IsSequenceFetchable(const CSeq_id& id)
{
    bool fetchable = false;
    try {
        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
        CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
        scope->AddDefaults();
        CBioseq_Handle bsh = scope->GetBioseqHandle(idh);
        if (bsh) {
            fetchable = true;
        }
    } catch (CException& ) {
    } catch (std::exception &) {
    }
    return fetchable;
}


bool IsSequenceFetchable(const string& seq_id)
{
    bool fetchable = false;
    try {
        CRef<CSeq_id> id(new CSeq_id(seq_id));
        if (id) {
            fetchable = IsSequenceFetchable(*id);
        }
    } catch (CException& ) {
    } catch (std::exception &) {
    }
    return fetchable;
}


bool IsNTNCNWACAccession(const string& acc)
{
    if (NStr::StartsWith(acc, "NT_") || NStr::StartsWith(acc, "NC_") ||
        NStr::StartsWith(acc, "AC_") || NStr::StartsWith(acc, "NW_")) {
        return true;
    } else {
        return false;
    }
}


bool IsNTNCNWACAccession(const CSeq_id& id)
{
    if (id.IsOther() && id.GetOther().IsSetAccession() &&
        IsNTNCNWACAccession(id.GetOther().GetAccession())) {
        return true;
    } else {
        return false;
    }
}


bool IsNTNCNWACAccession(const CBioseq& seq)
{
    bool is_it = false;
    FOR_EACH_SEQID_ON_BIOSEQ(id_it, seq) {
        if (IsNTNCNWACAccession(**id_it)) {
            is_it = true;
            break;
        }
    }
    return is_it;
}


bool IsNG(const CSeq_id& id)
{
    if (id.IsOther() && id.GetOther().IsSetAccession() &&
        NStr::StartsWith(id.GetOther().GetAccession(), "NG_")) {
        return true;
    } else {
        return false;
    }
}


bool IsNG(const CBioseq& seq)
{
    bool is_it = false;
    FOR_EACH_SEQID_ON_BIOSEQ(id_it, seq) {
        if (IsNG(**id_it)) {
            is_it = true;
            break;
        }
    }
    return is_it;
}


// See VR-728. These Seq-ids are temporary and will be stripped
// by the ID Load process, so they should not be the only Seq-id
// on a Bioseq, and feature locations should not use these.
bool IsTemporary(const CSeq_id& id)
{
    if (id.IsGeneral() && id.GetGeneral().IsSetDb()) {
        const string& db = id.GetGeneral().GetDb();
        if (NStr::EqualNocase(db, "TMSMART") ||
            NStr::EqualNocase(db, "NCBIFILE") ||
            NStr::EqualNocase(db, "BankIt")) {
            return true;
        }
    }
    return false;
}


bool IsOrganelle(int genome)
{
    bool rval = false;
    switch (genome) {
    case CBioSource::eGenome_chloroplast:
    case CBioSource::eGenome_chromoplast:
    case CBioSource::eGenome_kinetoplast:
    case CBioSource::eGenome_mitochondrion:
    case CBioSource::eGenome_cyanelle:
    case CBioSource::eGenome_nucleomorph:
    case CBioSource::eGenome_apicoplast:
    case CBioSource::eGenome_leucoplast:
    case CBioSource::eGenome_proplastid:
    case CBioSource::eGenome_hydrogenosome:
    case CBioSource::eGenome_chromatophore:
    case CBioSource::eGenome_plastid:
        rval = true;
        break;
    default:
        rval = false;
        break;
    }
    return rval;
}


bool IsOrganelle(const CBioseq_Handle& seq)
{
    if (!seq) {
        return false;
    }
    bool rval = false;
    CSeqdesc_CI sd(seq, CSeqdesc::e_Source);
    if (sd && sd->GetSource().IsSetGenome() && IsOrganelle(sd->GetSource().GetGenome())) {
        rval = true;
    }
    return rval;
}


bool ConsistentWithA(Char ch)

{
    return (bool)(strchr("ANRMWHVD", ch) != NULL);
}

bool ConsistentWithC(Char ch)

{
    return (bool)(strchr("CNYMSHBV", ch) != NULL);
}

bool ConsistentWithG(Char ch)

{
    return (bool)(strchr("GNRKSBVD", ch) != NULL);
}

bool ConsistentWithT(Char ch)

{
    return (bool)(strchr("TNYKWHBD", ch) != NULL);
}


//LCOV_EXCL_START
//not used by validator, but used by Genome Workbench menu item for
//removing unneccessary exceptions
bool DoesCodingRegionHaveUnnecessaryException(const CSeq_feat& feat, const CBioseq_Handle& loc_handle, CScope& scope)
{
    CCDSTranslationProblems problems;
    CBioseq_Handle prot_handle;
    if (feat.IsSetProduct()) {
        prot_handle = scope.GetBioseqHandle(feat.GetProduct());
    }

    problems.CalculateTranslationProblems(feat,
        loc_handle,
        prot_handle,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        &scope);

    return (problems.GetTranslationProblemFlags() & CCDSTranslationProblems::eCDSTranslationProblem_UnnecessaryException);
}


bool DoesmRNAHaveUnnecessaryException(const CSeq_feat& feat, const CBioseq_Handle& nuc, CScope& scope)
{
    size_t mismatches = 0;
    CBioseq_Handle rna;
    if (feat.IsSetProduct()) {
        rna = scope.GetBioseqHandle(feat.GetProduct());
    }

    size_t problems = GetMRNATranslationProblems
        (feat, mismatches, false,
        nuc, rna, false, false, false, &scope);

    return (problems & eMRNAProblem_UnnecessaryException);
}


bool DoesFeatureHaveUnnecessaryException(const CSeq_feat& feat, CScope& scope)
{
    if (!feat.IsSetExcept_text()) {
        return false;
    }
    if (!feat.IsSetData()) {
        return false;
    }
    if (!feat.IsSetLocation()) {
        return false;
    }
    try {
        CBioseq_Handle bsh = scope.GetBioseqHandle(feat.GetLocation());
        if (!bsh) {
            return false;
        }
        CSpliceProblems splice_problems;
        splice_problems.CalculateSpliceProblems(feat, true, sequence::IsPseudo(feat, scope), bsh);
        if (splice_problems.IsExceptionUnnecessary()) {
            return true;
        }
        if (feat.GetData().IsCdregion()) {
            return DoesCodingRegionHaveUnnecessaryException(feat, bsh, scope);
        } else if (feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA) {
            return DoesmRNAHaveUnnecessaryException(feat, bsh, scope);
        } else {
            return false;
        }
    } catch (CException&) {
    }
    return false;
}
//LCOV_EXCL_STOP


END_SCOPE(validator)
END_SCOPE(objects)
END_NCBI_SCOPE
