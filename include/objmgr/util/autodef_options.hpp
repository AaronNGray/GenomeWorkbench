#ifndef OBJMGR_UTIL___AUTODEF_OPTIONS__HPP
#define OBJMGR_UTIL___AUTODEF_OPTIONS__HPP

/*  $Id: autodef_options.hpp 530276 2017-03-13 18:20:08Z bollin $
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
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/util/autodef_available_modifier.hpp>
#include <objmgr/util/autodef_source_desc.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)    

class NCBI_XOBJEDIT_EXPORT CAutoDefOptions
{
public:
    CAutoDefOptions();
    ~CAutoDefOptions() {}

    CRef<CUser_object> MakeUserObject() const;
    void InitFromUserObject(const CUser_object& obj);

    enum EOptionFieldType {
        eOptionFieldType_Unknown = 0,
        eOptionFieldType_MaxMods,
        eOptionFieldType_UseLabels,
        eOptionFieldType_AllowModAtEndOfTaxname,
        eOptionFieldType_LeaveParenthetical,
        eOptionFieldType_DoNotApplyToSp,
        eOptionFieldType_DoNotApplyToNr,
        eOptionFieldType_DoNotApplyToCf,
        eOptionFieldType_DoNotApplyToAff,
        eOptionFieldType_IncludeCountryText,
        eOptionFieldType_KeepAfterSemicolon,
        eOptionFieldType_HIVRule,
        eOptionFieldType_FeatureListType,
        eOptionFieldType_MiscFeatRule,
        eOptionFieldType_ProductFlag,
        eOptionFieldType_NuclearCopyFlag,
        eOptionFieldType_SpecifyNuclearProduct,
        eOptionFieldType_AltSpliceFlag,
        eOptionFieldType_SuppressLocusTags,
        eOptionFieldType_SuppressAlleles,
        eOptionFieldType_GeneClusterOppStrand,
        eOptionFieldType_SuppressFeatureAltSplice,
        eOptionFieldType_SuppressMobileElementSubfeatures,
        eOptionFieldType_KeepExons,
        eOptionFieldType_KeepIntrons,
        eOptionFieldType_UseFakePromoters,
        eOptionFieldType_KeepRegulatoryFeatures,
        eOptionFieldType_KeepLTRs,
        eOptionFieldType_Keep3UTRs,
        eOptionFieldType_Keep5UTRs,
        eOptionFieldType_KeepuORFs,
        eOptionFieldType_KeepMobileElements,
        eOptionFieldType_KeepPrecursorRNA,
        eOptionFieldType_KeepRepeatRegion,
        eOptionFieldType_KeepMiscRecomb,
        eOptionFieldType_UseNcRNAComment,
        eOptionFieldType_SuppressedFeatures,
        eOptionFieldType_ModifierList,
        eOptionFieldType_TargetedLocusName,
        eOptionFieldType_SuppressMiscFeatureSubfeatures,
        eOptionFieldType_CustomFeatureClause,
        eOptionFieldMax
    };

    enum EFeatureListType {
        eListAllFeatures = 0,
        eCompleteSequence,
        eCompleteGenome,
        ePartialSequence,
        ePartialGenome,
        eSequence
    };

    typedef unsigned int TFeatureListType;
    TFeatureListType GetFeatureListType() const { return m_FeatureListType; }
    void SetFeatureListType(EFeatureListType list_type) { m_FeatureListType = list_type; }

    enum EMiscFeatRule {
        eDelete = 0,
        eNoncodingProductFeat,
        eCommentFeat
    };

    typedef unsigned int TMiscFeatRule;
    TMiscFeatRule GetMiscFeatRule() const { return m_MiscFeatRule; }
    void SetMiscFeatRule(TMiscFeatRule rule) { m_MiscFeatRule = rule; }

    enum EHIVCloneIsolateRule {
        ePreferClone = 0,
        ePreferIsolate,
        eWantBoth
    };
    typedef unsigned int THIVRule;
    THIVRule GetHIVRule() const { return m_HIVRule; }
    void SetHIVRule(EHIVCloneIsolateRule rule) { m_HIVRule = rule; }

    bool GetUseFakePromoters() const { return m_BooleanFlags[eOptionFieldType_UseFakePromoters]; }
    void SetUseFakePromoters(bool val = true) {
        m_BooleanFlags[eOptionFieldType_UseFakePromoters] = val;
    }

    CBioSource::TGenome GetProductFlag() const { return m_ProductFlag; };
    void SetProductFlag(CBioSource::EGenome val) { 
        m_ProductFlag = val; 
        m_BooleanFlags[eOptionFieldType_SpecifyNuclearProduct] = false; 
        m_NuclearCopyFlag = CBioSource::eGenome_unknown;
    }

    CBioSource::TGenome GetNuclearCopyFlag() const { return m_NuclearCopyFlag; };
    void SetNuclearCopyFlag(CBioSource::EGenome val) {
        m_NuclearCopyFlag = val;
        m_BooleanFlags[eOptionFieldType_SpecifyNuclearProduct] = false;
        m_ProductFlag = CBioSource::eGenome_unknown;
    }

    bool GetSpecifyNuclearProduct() const { return m_BooleanFlags[eOptionFieldType_SpecifyNuclearProduct]; }
    void SetSpecifyNuclearProduct(bool val) {
        m_BooleanFlags[eOptionFieldType_SpecifyNuclearProduct] = val;
        if (val) {
            m_ProductFlag = CBioSource::eGenome_unknown;
            m_NuclearCopyFlag = CBioSource::eGenome_unknown;
        }
    }

    int GetMaxMods() const { return m_MaxMods; }
    void SetMaxMods(int val) { m_MaxMods = val; }

#define AUTODEFBOOLFIELD(Fieldname) \
    bool Get##Fieldname() const { return m_BooleanFlags[eOptionFieldType_##Fieldname]; }; \
    void Set##Fieldname(bool val = true) { m_BooleanFlags[eOptionFieldType_##Fieldname] = val; }

    AUTODEFBOOLFIELD(UseLabels)
    AUTODEFBOOLFIELD(LeaveParenthetical)
    AUTODEFBOOLFIELD(AllowModAtEndOfTaxname)
    AUTODEFBOOLFIELD(DoNotApplyToSp)
    AUTODEFBOOLFIELD(DoNotApplyToNr)
    AUTODEFBOOLFIELD(DoNotApplyToCf)
    AUTODEFBOOLFIELD(DoNotApplyToAff)
    AUTODEFBOOLFIELD(IncludeCountryText)
    AUTODEFBOOLFIELD(KeepAfterSemicolon)
    AUTODEFBOOLFIELD(AltSpliceFlag)
    AUTODEFBOOLFIELD(SuppressLocusTags)
    AUTODEFBOOLFIELD(SuppressAlleles)
    AUTODEFBOOLFIELD(GeneClusterOppStrand)
    AUTODEFBOOLFIELD(SuppressFeatureAltSplice)
    AUTODEFBOOLFIELD(SuppressMobileElementSubfeatures)
    AUTODEFBOOLFIELD(KeepExons)
    AUTODEFBOOLFIELD(KeepIntrons)
    AUTODEFBOOLFIELD(KeepRegulatoryFeatures)
    AUTODEFBOOLFIELD(KeepLTRs)
    AUTODEFBOOLFIELD(Keep3UTRs)
    AUTODEFBOOLFIELD(Keep5UTRs)
    AUTODEFBOOLFIELD(KeepuORFs)
    AUTODEFBOOLFIELD(KeepMobileElements)
    AUTODEFBOOLFIELD(KeepPrecursorRNA)
    AUTODEFBOOLFIELD(KeepRepeatRegion)
    AUTODEFBOOLFIELD(KeepMiscRecomb)
    AUTODEFBOOLFIELD(UseNcRNAComment)
    AUTODEFBOOLFIELD(SuppressMiscFeatureSubfeatures)

    bool IsFeatureSuppressed(CSeqFeatData::ESubtype subtype) const;
    bool AreAnyFeaturesSuppressed() const { return !m_SuppressedFeatureSubtypes.empty(); }
    void SuppressFeature(CSeqFeatData::ESubtype subtype);
    void SuppressAllFeatures();
    void ClearSuppressedFeatures();

    void AddSubSource(CSubSource::TSubtype subtype);
    void AddOrgMod(COrgMod::TSubtype subtype);
    typedef vector<COrgMod::TSubtype> TOrgMods;
    const TOrgMods& GetOrgMods() const { return m_OrgMods; }
    typedef vector<CSubSource::TSubtype> TSubSources;
    const TSubSources& GetSubSources() const { return m_SubSources; }
    void ClearModifierList();

    string GetFeatureListType(TFeatureListType list_type) const;
    TFeatureListType GetFeatureListType(const string& list_type) const;

    string GetMiscFeatRule(TMiscFeatRule list_type) const;
    TMiscFeatRule GetMiscFeatRule(const string& list_type) const;

    string GetHIVRule(TMiscFeatRule list_type) const;
    TMiscFeatRule GetHIVRule(const string& list_type) const;

    string GetProductFlag(CBioSource::TGenome value) const;
    CBioSource::TGenome GetProductFlag(const string& value) const;

    string GetNuclearCopyFlag(CBioSource::TGenome value) const;
    CBioSource::TGenome GetNuclearCopyFlag(const string& value) const;

    string GetTargetedLocusName() const { return m_TargetedLocusName;  }
    void SetTargetedLocusName(const string& tls) { m_TargetedLocusName = tls; }

    string GetCustomFeatureClause() const { return m_CustomFeatureClause; }
    void SetCustomFeatureClause(const string& val) { m_CustomFeatureClause = val; }

private:

    bool m_BooleanFlags[eOptionFieldMax];
    int m_MaxMods;
    THIVRule m_HIVRule;
    TFeatureListType m_FeatureListType;
    TMiscFeatRule m_MiscFeatRule;
    CBioSource::TGenome m_ProductFlag;
    CBioSource::TGenome m_NuclearCopyFlag;
    typedef vector<CSeqFeatData::ESubtype> TSuppressedFeatureSubtypes;
    TSuppressedFeatureSubtypes m_SuppressedFeatureSubtypes;
    string m_TargetedLocusName;
    string m_CustomFeatureClause;

    TOrgMods m_OrgMods;
    TSubSources m_SubSources;
    
    typedef unsigned int TFieldType;
    string GetFieldType(TFieldType field_type) const;
    TFieldType GetFieldType(const string& field_name) const;



    bool x_IsBoolean(TFieldType field_type) const;
    CRef<CUser_field> x_MakeBooleanField(TFieldType field_type) const;

    void x_MakeSuppressedFeatures(CUser_object& user) const;
    void x_SetSuppressedFeatures(const CUser_field& field);

    void x_MakeModifierList(CUser_object& user) const;
    void x_SetModifierList(const CUser_field& field);


    CRef<CUser_field> x_MakeMaxMods() const;
    CRef<CUser_field> x_MakeTargetedLocusName() const;
    CRef<CUser_field> x_MakeCustomFeatureClause() const;

#define AUTODEFENUMFIELD(Fieldname) \
    CRef<CUser_field> x_Make##Fieldname() const { \
        CRef<CUser_field> field(new CUser_field()); \
        field->SetLabel().SetStr(GetFieldType(eOptionFieldType_##Fieldname)); \
        field->SetData().SetStr(Get##Fieldname(m_##Fieldname)); \
        return field; \
    } 

    AUTODEFENUMFIELD(FeatureListType)
    AUTODEFENUMFIELD(MiscFeatRule)
    AUTODEFENUMFIELD(HIVRule)
    AUTODEFENUMFIELD(ProductFlag)
    AUTODEFENUMFIELD(NuclearCopyFlag)

    void x_Reset();
};



END_SCOPE(objects)
END_NCBI_SCOPE

#endif //OBJMGR_UTIL___AUTODEF_OPTIONS__HPP
