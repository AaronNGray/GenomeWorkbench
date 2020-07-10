#ifndef GUI_OBJUTILS___MACRO_UTIL__HPP
#define GUI_OBJUTILS___MACRO_UTIL__HPP
/*  $Id: macro_util.hpp 44753 2020-03-05 17:24:00Z asztalos $
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
 * Authors: Andrea Asztalos, Colleen Bollin
 *
 * File Description: Utility functions used by macro functions
 *
 */

/// @file macro_util.hpp

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <objtools/cleanup/capitalization_string.hpp>
#include <gui/objutils/macro_exec.hpp>
#include <gui/objutils/label.hpp>


/** @addtogroup GUI_MACRO_SCRIPTS_UTIL
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CName_std;
    class CAuth_list;
END_SCOPE(objects);
BEGIN_SCOPE(macro)

namespace NMacroUtil
{
    using TVecFeatList = vector<CConstRef<objects::CSeq_feat>>;

    objects::edit::EExistingText NCBI_GUIOBJUTILS_EXPORT ActionTypeToExistingTextOption(const string& action_type, const string& delimiter);

    objects::ECapChange NCBI_GUIOBJUTILS_EXPORT ConvertStringtoCapitalOption(const string& cap_name);

    objects::CSeq_entry_Handle NCBI_GUIOBJUTILS_EXPORT GetParentEntryForBioSource(const objects::CBioSource& bsrc, const objects::CSeq_entry_Handle& tse);

    CConstRef<objects::CBioseq> NCBI_GUIOBJUTILS_EXPORT GetBioseqForSeqdescObject(const CSerialObject* obj, const objects::CSeq_entry_Handle& tse);

    CObjectInfo GetPrimitiveObjInfo(const CObjectInfo& info);

    void GetPrimitiveObjectInfos(CMQueryNodeValue::TObs& objs, const CMQueryNodeValue::SResolvedField& info);

    void GetPrimitiveObjInfosWithContainers(CMQueryNodeValue::TObs& objs, const CMQueryNodeValue::SResolvedField& info);

    CMQueryNodeValue::EType GetPrimitiveFromRef(CMQueryNodeValue& node);

    void SwapGbQualWithValues(CMQueryNodeValue::TObs& objs);

    void RemoveGeneXref(objects::CSeq_feat& feat);

    void RemoveEmptyDescriptors(objects::CBioseq& bseq);

    /// converts ints and doubles into string, by changing the type of the value
    string GetStringValue(CRef<CMQueryNodeValue>& value);

    objects::CSeqFeatData::ESubtype NCBI_GUIOBJUTILS_EXPORT GetFeatSubtype(const string& feat_type);

    bool NCBI_GUIOBJUTILS_EXPORT StringsAreEquivalent(const string& name1, const string& name2);

    bool NCBI_GUIOBJUTILS_EXPORT GetLocusTagFromGene(const objects::CGene_ref& gene, string& locus_tag);

    bool NCBI_GUIOBJUTILS_EXPORT GetLocusTagFromProtRef(const objects::CSeq_feat& prot_feat, objects::CScope& scope, string& locus_tag);

    CRef<objects::CSeqdesc> NCBI_GUIOBJUTILS_EXPORT MakeNcbiAutofixUserObject();

    bool NCBI_GUIOBJUTILS_EXPORT FindNcbiAutofixUserObject(const objects::CSeq_entry_Handle& tse);

    objects::CRNA_ref::EType NCBI_GUIOBJUTILS_EXPORT GetRNAType(const string& rna_type);

    bool NCBI_GUIOBJUTILS_EXPORT IsBiosourceModifier(const string& field);

    bool IsTaxname(CMQueryNodeValue::SResolvedField& res);

    void CleanupForTaxnameChange(CObjectInfo oi);

    void CleanupForTaxnameChange(CMQueryNodeValue::SResolvedField& res, CObjectInfo oi);

    bool NCBI_GUIOBJUTILS_EXPORT ApplyFirstName(objects::CName_std& std_name, const string& newValue, objects::edit::EExistingText existing_text);

    bool NCBI_GUIOBJUTILS_EXPORT RemoveFirstName(objects::CName_std& std_name);

    bool NCBI_GUIOBJUTILS_EXPORT ApplyMiddleInitial(objects::CName_std& std_name, const string& newValue, objects::edit::EExistingText existing_text);

    bool NCBI_GUIOBJUTILS_EXPORT RemoveMiddleInitial(objects::CName_std& std_name);

    unsigned NCBI_GUIOBJUTILS_EXPORT ApplyAuthorNames(objects::CAuth_list& auth_list, const string& newValue);

    enum EStructVoucherPart { eSV_Coll, eSV_Inst, eSV_Specid, eSV_Error };

    EStructVoucherPart GetSVPartFromString(const string voucher_part);

    bool NCBI_GUIOBJUTILS_EXPORT IsStructVoucherPart(const string& field);
    /// replace any instance of 2 or more spaces with a single instance of 'delimiter'
    void NCBI_GUIOBJUTILS_EXPORT ConvertMultiSpaces(string& line, const string& delimiter);

    string NCBI_GUIOBJUTILS_EXPORT TransformForCSV(const string& str);

    CLabel::ELabelType NameToLabelType(const string& name);

    bool NCBI_GUIOBJUTILS_EXPORT IsSatelliteSubfield(const string& field);

    bool NCBI_GUIOBJUTILS_EXPORT IsMobileElementTSubfield(const string& field);
}

NCBI_GUIOBJUTILS_EXPORT extern const char* kInst_suffix;
NCBI_GUIOBJUTILS_EXPORT extern const char* kColl_suffix;
NCBI_GUIOBJUTILS_EXPORT extern const char* kSpecid_suffix;
NCBI_GUIOBJUTILS_EXPORT extern const char* kSatelliteType;
NCBI_GUIOBJUTILS_EXPORT extern const char* kSatelliteName;
NCBI_GUIOBJUTILS_EXPORT extern const char* kMobileElementTQual;
NCBI_GUIOBJUTILS_EXPORT extern const char* kMobileElementTType;
NCBI_GUIOBJUTILS_EXPORT extern const char* kMobileElementTName;


class NCBI_GUIOBJUTILS_EXPORT CRemoveTextOptions : public CObject
{
public:
    enum EMatchType {
        eNone = 0,
        eText,
        eDigits,
        eLetters
    };

    CRemoveTextOptions(EMatchType before_match, const string& before_text, bool remove_before_match,
        EMatchType after_match, const string& after_text, bool remove_after_match,
        bool case_insensitive = false, bool whole_word = false)
        : m_BeforeMatch(before_match), m_BeforeText(before_text),
        m_RemoveBeforeMatch(remove_before_match),
        m_AfterMatch(after_match), m_AfterText(after_text),
        m_RemoveAfterMatch(remove_after_match),
        m_CaseInsensitive(case_insensitive),
        m_WholeWord(whole_word)
    {};
    ~CRemoveTextOptions() {}
    bool EditText(string& str) const;

private:
    EMatchType m_BeforeMatch;
    string m_BeforeText;
    bool m_RemoveBeforeMatch;
    EMatchType m_AfterMatch;
    string m_AfterText;
    bool m_RemoveAfterMatch;
    bool m_CaseInsensitive;
    bool m_WholeWord;
};

END_SCOPE(macro)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_OBJUTILS___MACRO_UTIL__HPP
