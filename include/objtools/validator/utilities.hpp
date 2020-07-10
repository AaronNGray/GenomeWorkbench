/*  $Id: utilities.hpp 591867 2019-08-21 18:16:22Z asztalos $
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
 *      Definition for utility classes and functions.
 */

#ifndef VALIDATOR___UTILITIES__HPP
#define VALIDATOR___UTILITIES__HPP

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistr.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Org_ref.hpp>
//#include <objects/taxon3/T3Data.hpp>
//#include <objects/taxon3/Taxon3_reply.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/biblio/Id_pat.hpp> 
#include <objects/biblio/Auth_list.hpp>
#include <objmgr/seq_vector.hpp>

#include <serial/iterator.hpp>

#include <vector>
#include <list>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

class CGb_qual;
class CScope;
class CSeq_entry;
class CT3Data;
class CT3Reply;

BEGIN_SCOPE(validator)


// =============================================================================
//                                 Functions
// =============================================================================

bool IsClassInEntry(const CSeq_entry& se, CBioseq_set::EClass clss);
bool IsDeltaOrFarSeg(const CSeq_loc& loc, CScope* scope);
bool IsBlankStringList(const list< string >& str_list);
TGi GetGIForSeqId(const CSeq_id& id);
CScope::TIds GetSeqIdsForGI(TGi gi);
bool NCBI_VALIDATOR_EXPORT IsFarLocation(const CSeq_loc& loc, const CSeq_entry_Handle& seh);

CSeqVector GetSequenceFromLoc(const CSeq_loc& loc, CScope& scope,
    CBioseq_Handle::EVectorCoding coding = CBioseq_Handle::eCoding_Iupac);

CSeqVector GetSequenceFromFeature(const CSeq_feat& feat, CScope& scope,
    CBioseq_Handle::EVectorCoding coding = CBioseq_Handle::eCoding_Iupac,
    bool product = false);

string GetSequenceStringFromLoc(const CSeq_loc& loc,  CScope& scope);


inline
bool IsResidue(unsigned char residue) { return residue <= 250; }
CConstRef<CSeq_id> GetReportableSeqIdForAlignment(const CSeq_align& align, CScope& scope);
string GetAccessionFromObjects(const CSerialObject* obj, const CSeq_entry* ctx, CScope& scope, int* version);

CBioseq_set_Handle GetSetParent (const CBioseq_set_Handle& set, CBioseq_set::TClass set_class);
CBioseq_set_Handle GetSetParent (const CBioseq_Handle& bioseq, CBioseq_set::TClass set_class);
CBioseq_set_Handle GetGenProdSetParent (const CBioseq_set_Handle& set);
CBioseq_set_Handle GetGenProdSetParent (const CBioseq_Handle& set);
CBioseq_set_Handle GetNucProtSetParent (const CBioseq_Handle& bioseq);

CBioseq_Handle GetNucBioseq (const CBioseq_set_Handle& bioseq_set);
CBioseq_Handle GetNucBioseq (const CBioseq_Handle& bioseq);

typedef enum {
  eAccessionFormat_valid = 0,
  eAccessionFormat_no_start_letters,
  eAccessionFormat_wrong_number_of_digits,
  eAccessionFormat_null,
  eAccessionFormat_too_long,
  eAccessionFormat_missing_version,
  eAccessionFormat_bad_version } EAccessionFormatError;

EAccessionFormatError ValidateAccessionString (const string& accession, bool require_version);

bool s_IdXrefsAreReciprocal(const CSeq_feat &cds, const CSeq_feat &mrna);
bool s_FeatureIdsMatch (const CFeat_id& f1, const CFeat_id& f2);
bool s_StringHasPMID (const string& str);
bool HasBadCharacter (const string& str);
bool EndsWithBadCharacter (const string& str);

typedef enum {
  eDateValid_valid = 0x0,
  eDateValid_bad_str = 0x01,
  eDateValid_bad_year = 0x02,
  eDateValid_bad_month = 0x04,
  eDateValid_bad_day = 0x08,
  eDateValid_bad_season = 0x10,
  eDateValid_bad_other = 0x20 ,
  eDateValid_empty_date = 0x40 } EDateValid;

int CheckDate (const CDate& date, bool require_full_date = false);
bool NCBI_VALIDATOR_EXPORT IsDateInPast(const CDate& date);
string GetDateErrorDescription (int flags);

bool IsBioseqTSA (const CBioseq& seq, CScope* scope);

#if 0
// disabled for now
bool IsNCBIFILESeqId (const CSeq_id& id);
#endif

string GetValidatorLocationLabel (const CSeq_loc& loc, CScope& scope);
void AppendBioseqLabel(string& str, const CBioseq& sq, bool supress_context);
string GetBioseqIdLabel(const CBioseq& sq, bool limited = false);

bool NCBI_VALIDATOR_EXPORT HasECnumberPattern (const string& str);

bool SeqIsPatent (const CBioseq& seq);
bool SeqIsPatent (const CBioseq_Handle& seq);

bool s_PartialAtGapOrNs(CScope* scope, const CSeq_loc& loc, unsigned int tag, bool only_gap = false);

CBioseq_Handle BioseqHandleFromLocation (CScope* m_Scope, const CSeq_loc& loc);

typedef enum {
    eBioseqEndIsType_None = 0,
    eBioseqEndIsType_Last,
    eBioseqEndIsType_All
} EBioseqEndIsType;


void NCBI_VALIDATOR_EXPORT CheckBioseqEndsForNAndGap 
(const CBioseq_Handle& bsh,
 EBioseqEndIsType& begin_n,
 EBioseqEndIsType& begin_gap,
 EBioseqEndIsType& end_n,
 EBioseqEndIsType& end_gap,
 bool &begin_ambig,
 bool &end_ambig);

bool ShouldCheckForNsAndGap(const CBioseq_Handle& bsh);

void CheckBioseqEndsForNAndGap
(const CSeqVector& vec,
EBioseqEndIsType& begin_n,
EBioseqEndIsType& begin_gap,
EBioseqEndIsType& end_n,
EBioseqEndIsType& end_gap,
bool& begin_ambig,
bool& end_ambig);



/// Indicates whether feature is a dicistronic gene 
/// @param f Seq-feat-Handle [in]
/// @return Boolean
bool NCBI_VALIDATOR_EXPORT IsDicistronicGene (const CSeq_feat_Handle& f);
bool NCBI_VALIDATOR_EXPORT IsDicistronic(const CSeq_feat_Handle& f);

typedef enum {
    eDuplicate_Not = 0,
    eDuplicate_Duplicate,
    eDuplicate_SameIntervalDifferentLabel,
    eDuplicate_DuplicateDifferentTable,
    eDuplicate_SameIntervalDifferentLabelDifferentTable
} EDuplicateFeatureType;

typedef const CSeq_feat::TDbxref TDbtags;

/// Reports how two features duplicate each other
/// @param f1 Seq-feat-Handle [in]
/// @param f2 Seq-feat-Handle [in]
/// @return EDuplicateFeatureType return value indicates how features are duplicates
EDuplicateFeatureType NCBI_VALIDATOR_EXPORT IsDuplicate 
    (const CSeq_feat_Handle& f1,
     const CSeq_feat_Handle& f2,
     bool check_partials = false,
     bool case_sensitive = false);

bool IsLocFullLength (const CSeq_loc& loc, const CBioseq_Handle& bsh);
bool PartialsSame (const CSeq_loc& loc1, const CSeq_loc& loc2);

// specific-host functions
/// returns true and error_msg will be empty, if specific host is valid
/// returns true and error_msg will be "Host is empty", if specific host is empty
/// returns false if specific host is invalid
bool NCBI_VALIDATOR_EXPORT IsSpecificHostValid(const string& host, string& error_msg);
/// returns the corrected specific host, if the specific host is invalid and can be corrected
/// returns an empty string, if the specific host is invalid and cannot be corrected
/// returns the original value except the preceding/trailing spaces, if the specific host is valid
string NCBI_VALIDATOR_EXPORT FixSpecificHost(const string& host);

bool NCBI_VALIDATOR_EXPORT IsCommonName (const CT3Data& data);
bool NCBI_VALIDATOR_EXPORT HasMisSpellFlag (const CT3Data& data);
bool NCBI_VALIDATOR_EXPORT FindMatchInOrgRef (const string& str, const COrg_ref& org);
string NCBI_VALIDATOR_EXPORT SpecificHostValueToCheck(const string& val);
bool NCBI_VALIDATOR_EXPORT IsLikelyTaxname(const string& val);
string InterpretSpecificHostResult(const string& host, const CT3Reply& reply, const string& orig_host = kEmptyStr);
void NCBI_VALIDATOR_EXPORT AdjustSpecificHostForTaxServer (string& spec_host);

// function is used to convert a pub title into a 'term' parameter of CEutilsClient::Search method
void NCBI_VALIDATOR_EXPORT ConvertToEntrezTerm(string& title);

string NCBI_VALIDATOR_EXPORT TranslateCodingRegionForValidation(const CSeq_feat& feat, CScope &scope, bool& alt_start);

// if special text is found in a feature exception, translation errors will not be reported
bool NCBI_VALIDATOR_EXPORT ReportTranslationErrors(const string& except_text);

// checks to see if this feature would be reported as having a bad start codon
bool NCBI_VALIDATOR_EXPORT HasBadStartCodon(const CSeq_feat& feat, CScope &scope, bool ignore_exceptions);

// checks to see if this location and translation has a bad start codon
// note that this might not be reported if the feature is pseudo, or has an appropriate exception
bool NCBI_VALIDATOR_EXPORT HasBadStartCodon(const CSeq_loc& loc, const string& transl_prot);

size_t CountInternalStopCodons(const string& transl_prot);
bool NCBI_VALIDATOR_EXPORT HasInternalStop(const CSeq_feat& feat, CScope& scope, bool ignore_exceptions);

CRef<CSeqVector> MakeSeqVectorForResidueCounting(const CBioseq_Handle& bsh);
bool HasBadProteinStart(const CSeqVector& sv);
bool NCBI_VALIDATOR_EXPORT HasBadProteinStart(const CSeq_feat& cds, CScope& scope);

size_t CountProteinStops(const CSeqVector& sv);
bool NCBI_VALIDATOR_EXPORT HasStopInProtein(const CSeq_feat& feat, CScope& scope);

void FeatureHasEnds(const CSeq_feat& feat, CScope* scope, bool& no_beg, bool& no_end);
CBioseq_Handle GetCDSProductSequence(const CSeq_feat& feat, CScope* scope, const CTSE_Handle & tse, bool far_fetch, bool& is_far);
vector<TSeqPos> GetMismatches(const CSeq_feat& feat, const CBioseq_Handle& prot_handle, const string& transl_prot);
vector<TSeqPos> GetMismatches(const CSeq_feat& feat, const CSeqVector& prot_vec, const string& transl_prot);
void CalculateEffectiveTranslationLengths(const string& transl_prot, const CSeqVector& prot_vec, size_t &len, size_t& prot_len);
bool NCBI_VALIDATOR_EXPORT HasNoStop(const CSeq_feat& feat, CScope* scope);

bool NCBI_VALIDATOR_EXPORT IsSequenceFetchable(const CSeq_id& id);
bool NCBI_VALIDATOR_EXPORT IsSequenceFetchable(const string& seq_id);

bool NCBI_VALIDATOR_EXPORT DoesFeatureHaveUnnecessaryException(const CSeq_feat& feat, CScope& scope);

bool IsNTNCNWACAccession(const string& acc);
bool IsNTNCNWACAccession(const CSeq_id& id);
bool IsNTNCNWACAccession(const CBioseq& seq);
bool IsNG(const CSeq_id& id);
bool IsNG(const CBioseq& seq);

bool IsTemporary(const CSeq_id& id);

bool IsOrganelle(int genome);
bool NCBI_VALIDATOR_EXPORT IsOrganelle(const CBioseq_Handle& seq);

bool ConsistentWithA(Char ch);
bool ConsistentWithC(Char ch);
bool ConsistentWithG(Char ch);
bool ConsistentWithT(Char ch);

END_SCOPE(validator)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif  /* VALIDATOR___UTILITIES__HPP */
