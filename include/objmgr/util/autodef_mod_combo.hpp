#ifndef OBJMGR_UTIL___AUTODEF_MOD_COMBO__HPP
#define OBJMGR_UTIL___AUTODEF_MOD_COMBO__HPP

/*  $Id: autodef_mod_combo.hpp 575076 2018-11-23 14:45:42Z bollin $
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
* Author:  Colleen Bollin
*
* File Description:
*   Creates unique definition lines for sequences in a set using organism
*   descriptions and feature clauses.
*/

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seq/MolInfo.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <objmgr/util/autodef_available_modifier.hpp>
#include <objmgr/util/autodef_source_desc.hpp>
#include <objmgr/util/autodef_source_group.hpp>
#include <objmgr/util/autodef_options.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)


class NCBI_XOBJEDIT_EXPORT CAutoDefModifierCombo : public CObject, 
                                                   public IAutoDefCombo
{
public:

    CAutoDefModifierCombo();
    CAutoDefModifierCombo(CAutoDefModifierCombo *orig);
    ~CAutoDefModifierCombo();

    void InitFromOptions(const CAutoDefOptions& options);
    void SetOptions(const CAutoDefOptions& options) { InitFromOptions(options); }
    void InitOptions(CAutoDefOptions& options) const;
    
    unsigned int GetNumGroups();
    
    unsigned int GetNumSubSources();
    CSubSource::ESubtype GetSubSource(unsigned int index);
    unsigned int GetNumOrgMods();
    COrgMod::ESubtype GetOrgMod(unsigned int index);
    
    bool HasSubSource(CSubSource::ESubtype st);
    bool HasOrgMod(COrgMod::ESubtype st);
    
    void AddSource(const CBioSource& bs, const string& feature_clauses = "");
    
    void AddSubsource(CSubSource::ESubtype st, bool even_if_not_uniquifying = false);
    void AddOrgMod(COrgMod::ESubtype st, bool even_if_not_uniquifying = false);
    void GetAvailableModifiers (CAutoDefSourceDescription::TAvailableModifierVector &modifier_list);
    bool HasTrickyHIV();
    bool GetDefaultExcludeSp();
    
    void SetUseModifierLabels(bool use);
    bool GetUseModifierLabels();
    void SetMaxModifiers(int max_mods);
    int GetMaxModifiers();
    void SetAllowModAtEndOfTaxname(bool keep);
    bool GetAllowModAtEndOfTaxname();
    void SetKeepCountryText(bool keep);
    bool GetKeepCountryText();
    void SetExcludeSpOrgs(bool exclude);
    bool GetExcludeSpOrgs ();
    void SetExcludeCfOrgs(bool exclude);
    bool GetExcludeCfOrgs ();
    void SetExcludeNrOrgs(bool exclude);
    bool GetExcludeNrOrgs ();
    void SetExcludeAffOrgs(bool exclude);
    bool GetExcludeAffOrgs ();
    void SetKeepParen(bool keep);
    bool GetKeepParen();
    void SetKeepAfterSemicolon(bool keep);
    bool GetKeepAfterSemicolon();
    void SetHIVCloneIsolateRule(CAutoDefOptions::EHIVCloneIsolateRule rule_num);
    CAutoDefOptions::EHIVCloneIsolateRule GetHIVCloneIsolateRule();

    string GetSourceDescriptionString(const CBioSource& bsrc);
    static bool IsUsableInDefline(CSubSource::ESubtype subtype);
    static bool IsUsableInDefline(COrgMod::ESubtype subtype);

    typedef vector<CSubSource::ESubtype> TSubSourceTypeVector;
    typedef vector<COrgMod::ESubtype> TOrgModTypeVector;
    typedef vector<CRef<CAutoDefSourceGroup> > TGroupListVector;

    const TGroupListVector& GetGroupList() const { return m_GroupList; }
    const CAutoDefSourceDescription::TModifierVector& GetModifiers() const { return m_Modifiers; }

    unsigned int GetNumUnique () const;
    unsigned int GetMaxInGroup () const;

    int Compare(const CAutoDefModifierCombo& other) const;
    bool operator>(const CAutoDefModifierCombo& src) const
    {
        return Compare (src) > 0;
    }

    bool operator<(const CAutoDefModifierCombo& src) const
    {
        return Compare (src) < 0;
    }

    bool AddQual (bool IsOrgMod, int subtype, bool even_if_not_uniquifying = false);
    bool RemoveQual (bool IsOrgMod, int subtype);

    vector<CRef<CAutoDefModifierCombo>> ExpandByAnyPresent();
    bool AreFeatureClausesUnique();

    static bool IsModifierRequiredByDefault(bool is_orgmod, int subtype);
    static bool IsModifierInString(const string& find_this, const string& find_in, bool ignore_at_end);

    typedef enum {
        eNotInfluenza = 0,
        eInfluenzaA,
        eInfluenzaB,
        eInfluenzaC,
        eInfluenzaD
    } EInfluenzaType;

    static EInfluenzaType GetInfluenzaType(const string& taxname);

private:
    TSubSourceTypeVector m_SubSources;
    TOrgModTypeVector    m_OrgMods;
    TGroupListVector     m_GroupList;
    CAutoDefSourceDescription::TModifierVector m_Modifiers;
   
    bool         m_UseModifierLabels;
    int          m_MaxModifiers;
    bool         m_AllowModAtEndOfTaxname;
    bool         m_KeepCountryText;
    bool         m_ExcludeSpOrgs;
    // NOTE - after the wxWidgets conversion is complete, need to add controls for the following three
    // values to the autodef configuration dialog 
    bool         m_ExcludeCfOrgs;
    bool         m_ExcludeNrOrgs;
    bool         m_ExcludeAffOrgs;

    bool         m_KeepParen;
    bool         m_KeepAfterSemicolon;
    CAutoDefOptions::EHIVCloneIsolateRule m_HIVCloneIsolateRule;
    
    string x_GetSubSourceLabel (CSubSource::ESubtype st);
    string x_GetOrgModLabel(COrgMod::ESubtype st);
    void x_CleanUpTaxName (string &tax_name);
    bool x_AddSubsourceString (string &source_description, const CBioSource& bsrc, CSubSource::ESubtype st);
    bool x_AddOrgModString (string &source_description, const CBioSource& bsrc, COrgMod::ESubtype st);

    static bool x_AddMinicircle(string& source_description, const CBioSource& bsrc);
    static bool x_AddMinicircle(string& source_description, const string& note_text);
    
    typedef pair<COrgMod::ESubtype, bool> TExtraOrgMod;
    typedef map<COrgMod::ESubtype, bool> TExtraOrgMods;
    typedef pair<CSubSource::ESubtype, bool> TExtraSubSrc;
    typedef map<CSubSource::ESubtype, bool> TExtraSubSrcs;
    static bool x_BioSourceHasSubSrc(const CBioSource& src, CSubSource::ESubtype subtype);
    static bool x_BioSourceHasOrgMod(const CBioSource& src, COrgMod::ESubtype subtype);

    void x_AddHIVModifiers(TExtraOrgMods& extra_orgmods, TExtraSubSrcs& extra_subsrcs, const CBioSource& bsrc);
    void x_AddInfluenzaModifiers(TExtraOrgMods& extra_orgmods, TExtraSubSrcs& extra_subsrcs, EInfluenzaType influenza_type);
    static bool x_SpecialHandlingForInfluenza(EInfluenzaType influenza_type, CSubSource::ESubtype subtype);
    static bool x_SpecialHandlingForInfluenza(EInfluenzaType influenza_type, COrgMod::ESubtype subtype);
    void x_AddRequiredSubSourceModifiers(TExtraOrgMods& extra_orgmods, TExtraSubSrcs& extra_subsrcs, const CBioSource& bsrc);
    void x_AddTypeStrainModifiers(TExtraOrgMods& extra_orgmods, TExtraSubSrcs& extra_subsrcs, const CBioSource& bsrc);
    static bool x_HasTypeStrainComment(const CBioSource& bsrc);
};


inline
void CAutoDefModifierCombo::SetUseModifierLabels(bool use)
{
    m_UseModifierLabels = use;
}


inline
bool CAutoDefModifierCombo::GetUseModifierLabels()
{
    return m_UseModifierLabels;
}


inline
void CAutoDefModifierCombo::SetMaxModifiers(int max_mods)
{
    m_MaxModifiers = max_mods;
}


inline
int CAutoDefModifierCombo::GetMaxModifiers()
{
    return m_MaxModifiers;
}


inline
void CAutoDefModifierCombo::SetAllowModAtEndOfTaxname(bool keep)
{
    m_AllowModAtEndOfTaxname = keep;
}


inline
bool CAutoDefModifierCombo::GetAllowModAtEndOfTaxname()
{
    return m_AllowModAtEndOfTaxname;
}


inline
void CAutoDefModifierCombo::SetKeepCountryText(bool keep)
{
    m_KeepCountryText = keep;
}


inline
bool CAutoDefModifierCombo::GetKeepCountryText()
{
    return m_KeepCountryText;
}


inline
void CAutoDefModifierCombo::SetExcludeSpOrgs(bool exclude)
{
    m_ExcludeSpOrgs = exclude;
}


inline
bool CAutoDefModifierCombo::GetExcludeSpOrgs()
{
    return m_ExcludeSpOrgs;
}


inline
void CAutoDefModifierCombo::SetExcludeCfOrgs(bool exclude)
{
    m_ExcludeCfOrgs = exclude;
}


inline
bool CAutoDefModifierCombo::GetExcludeCfOrgs()
{
    return m_ExcludeCfOrgs;
}


inline
void CAutoDefModifierCombo::SetExcludeNrOrgs(bool exclude)
{
    m_ExcludeNrOrgs = exclude;
}


inline
bool CAutoDefModifierCombo::GetExcludeNrOrgs()
{
    return m_ExcludeNrOrgs;
}


inline
void CAutoDefModifierCombo::SetExcludeAffOrgs(bool exclude)
{
    m_ExcludeAffOrgs = exclude;
}


inline
bool CAutoDefModifierCombo::GetExcludeAffOrgs()
{
    return m_ExcludeAffOrgs;
}


inline
void CAutoDefModifierCombo::SetKeepParen(bool keep)
{
    m_KeepParen = keep;
}


inline
bool CAutoDefModifierCombo::GetKeepParen()
{
    return m_KeepParen;
}


inline
void CAutoDefModifierCombo::SetKeepAfterSemicolon(bool keep)
{
    m_KeepAfterSemicolon = keep;
}


inline
bool CAutoDefModifierCombo::GetKeepAfterSemicolon()
{
    return m_KeepAfterSemicolon;
}


inline
void CAutoDefModifierCombo::SetHIVCloneIsolateRule(CAutoDefOptions::EHIVCloneIsolateRule rule_num)
{
    m_HIVCloneIsolateRule = rule_num;
}


inline
CAutoDefOptions::EHIVCloneIsolateRule CAutoDefModifierCombo::GetHIVCloneIsolateRule()
{
    return m_HIVCloneIsolateRule;
}


END_SCOPE(objects)
END_NCBI_SCOPE

#endif //OBJMGR_UTIL___AUTODEF_MOD_COMBO__HPP
