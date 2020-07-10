#ifndef GUI_OBJUTILS___CONVERT_FEAT__HPP
#define GUI_OBJUTILS___CONVERT_FEAT__HPP

/*  $Id: convert_feat.hpp 43474 2019-07-11 19:20:45Z filippov $
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
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

#include <objmgr/scope.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <gui/objutils/cmd_composite.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIOBJUTILS_EXPORT CConversionOptionBool : public CObject
{
public:
    CConversionOptionBool(bool val = false, bool default_val = false) 
        : m_Val(val), m_Default (default_val) {};
    ~CConversionOptionBool() {};

    void SetVal(bool val) {m_Val = val; };
    bool GetVal() const {return m_Val; };
    void SetDefaultVal(bool val) { m_Default = val; };
    bool GetDefaultVal() const { return m_Default; };

protected:
    bool m_Val;
    bool m_Default;
};


class NCBI_GUIOBJUTILS_EXPORT CConversionOptionString : public CObject
{
public:
    CConversionOptionString (const string& val, const string& default_val)
        : m_Val (val), m_Default(default_val)
        { m_Suggested.clear(); m_OnlySuggested = false; }
    CConversionOptionString (const string& val, const string& default_val, vector<string> suggested, bool only_suggested)
        : m_Val (val), m_Default(default_val), m_Suggested(suggested), m_OnlySuggested(only_suggested)
    {};
    ~CConversionOptionString() {};
    void SetVal(const string& val) { m_Val = val; };
    const string& GetVal() const { return m_Val; };
    void SetDefaultVal(const string& val) { m_Default = val; };
    const string& GetDefaultVal() const { return m_Default; };
    void ClearSuggestedValues() { m_Suggested.clear(); };
    void AddSuggestedValue (const string& val) { m_Suggested.push_back(val); };
    const vector<string>& GetSuggestedValues () const { return m_Suggested; };
    void SetOnlySuggested(bool val) { m_OnlySuggested = val; };
    bool GetOnlySuggested() const { return m_OnlySuggested; };

protected:
    string m_Val;
    string m_Default;
    vector<string> m_Suggested;
    bool m_OnlySuggested;
};


class NCBI_GUIOBJUTILS_EXPORT CConversionOption : public CObject
{
public:
    CConversionOption(const string& label) : m_Label(label), m_Choice(e_NotSet) {};
    ~CConversionOption() {};

    const string& GetLabel() const { return m_Label; };

    enum E_Choice {
        e_NotSet = 0,
        e_Bool,
        e_String
    };

    CRef<CConversionOptionBool> SetBool();
    CRef<CConversionOptionString> SetString();
    CConstRef<CConversionOptionBool> GetBool() const ;
    CConstRef<CConversionOptionString> GetString() const;
    E_Choice Which() { return m_Choice; }
    bool IsBool() const { return m_Choice == e_Bool; };
    bool IsString() const { return m_Choice == e_String; };

protected:
    string m_Label;
    E_Choice m_Choice;
    CRef<CObject> m_Object;
};


static const string kConversionNotSupported = "Conversion not supported.";

class NCBI_GUIOBJUTILS_EXPORT CConvertFeatureBase : public CObject
{
public:
    CConvertFeatureBase(objects::CSeqFeatData::ESubtype feat_to = objects::CSeqFeatData::eSubtype_bad, 
        objects::CSeqFeatData::ESubtype feat_from = objects::CSeqFeatData::eSubtype_bad); 
    ~CConvertFeatureBase() {};

    typedef vector<CRef<CConversionOption> > TOptions;
    virtual string GetDescription() { return kConversionNotSupported; }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype subtype);
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype subtype);
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
    TOptions& SetOptions() { return m_Options; }
    const TOptions& GetOptions() const { return m_Options; };
    CRef<CConversionOption> FindRemoveTranscriptId();
    string x_GetCDSProduct(const objects::CSeq_feat& orig, objects::CScope& scope);

    static const string s_RemovemRNA;
    static const string s_RemoveGene;
    static const string s_RemoveTranscriptID;
    static const string s_NcrnaClass;
    static const string s_SiteType;
    static const string s_BondType;
    static const string s_PlaceOnProt;
    static const string s_IsPseudo;

    void SetCreateGeneralOnly(bool flag) {m_create_general_only = flag;}
    void SetOffset(int *offset) {m_offset = offset;}
protected:
    CRef<CCmdComposite> x_SimpleConversion(const objects::CSeq_feat& orig, CRef<objects::CSeq_feat> new_feat, bool keep_orig, objects::CScope& scope);
    CRef<CCmdComposite> x_ConvertToCDS(const objects::CSeq_feat& orig, CRef<objects::CSeq_feat> new_feat, bool keep_orig, objects::CScope& scope, vector<string> product);
    bool x_MoveFeatureToProtein(const objects::CSeq_feat& orig, CRef<objects::CSeq_feat> new_feat, bool keep_orig, objects::CScope& scope);
    vector<string> x_ExtractQuals(CRef<objects::CSeq_feat> new_feat, string qual_name);
    vector<string> x_ExtractProductQuals(CRef<objects::CSeq_feat> new_feat);
    void x_AddToComment(string comment, CRef<objects::CSeq_feat> new_feat, objects::edit::EExistingText existing_text = objects::edit::eExistingText_prefix_semi);
    void x_ClearGenericFields(CRef<objects::CSeq_feat> new_feat);
    
    objects::CSeqFeatData::ESubtype m_To;
    objects::CSeqFeatData::ESubtype m_From;
    TOptions m_Options;
    
    void x_AddCDSSourceConversionOptions();
    void x_AddCDSDestinationConversionOptions();
    void x_AddRnaDestinationConversionOptions();
    void x_AddSiteDestinationConversionOptions();
    void x_AddBondDestinationConversionOptions();
    void x_AddRegionDestionationConversionOptions();
    CRef<CConversionOption> x_FindOption(const string& label);
    CRef<CConversionOption> x_FindRemoveGene();
    CRef<CConversionOption> x_FindRemovemRNA();
    CRef<CConversionOption> x_FindNcrnaClass();
    CRef<CConversionOption> x_FindSiteType();
    CRef<CConversionOption> x_FindBondType();
    CRef<CConversionOption> x_FindPlaceOnProt();
    CRef<CConversionOption> x_FindPseudo();
    bool x_IsImportSubtype(objects::CSeqFeatData::ESubtype subtype);
    bool x_IsProtSubtype(objects::CSeqFeatData::ESubtype subtype);
    bool x_IsRnaSubtype(objects::CSeqFeatData::ESubtype subtype);
    string GetSiteName(objects::CSeqFeatData::TSite site_type);

    objects::CSiteList m_SiteList;
    objects::CBondList m_BondList;
    bool m_create_general_only = false;
    int *m_offset = nullptr;
};


class NCBI_GUIOBJUTILS_EXPORT CConvertFeatureBaseFactory
{
public:
    static CRef<CConvertFeatureBase> Create(objects::CSeqFeatData::ESubtype subtype_from, objects::CSeqFeatData::ESubtype subtype_to);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertMiscFeatToGene : public CConvertFeatureBase
{
public:
    CConvertMiscFeatToGene() : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_gene, objects::CSeqFeatData::eSubtype_misc_feature) {};
    virtual string GetDescription() { return "Creates gene with locus value from misc_feature comment."; }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertImpToImp : public CConvertFeatureBase
{
public:
    CConvertImpToImp(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(feat_to, feat_from) {}
    virtual string GetDescription() { return "Changes type of import feature."; }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype subtype) { return x_IsImportSubtype(subtype); }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype subtype) { return x_IsImportSubtype(subtype); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertCDSToRNA : public CConvertFeatureBase
{
public:
    CConvertCDSToRNA(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_cdregion) {}
    virtual string GetDescription() { return "Delete protein product sequence.\nClear product field if transcript ID removal was requested.\nIf converting to tRNA and anticodon value can be parsed from label, set aa value, and add any text that could not be parsed into an anticodon value to the feature note.\nIf converting to other RNA, put label in RNA product."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype subtype) { return x_IsRnaSubtype(subtype); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertGeneToRNA : public CConvertFeatureBase
{
public:
    CConvertGeneToRNA(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_gene) {}
    virtual string GetDescription() { return "If converting to tRNA and anticodon value can be parsed from label, set aa value, and add any text that could not be parsed into an anticodon value to the feature note.  If converting to other RNA, put label in RNA product.  Also append gene locus, allele, description, map location, and locus tag to comment (as long as these values are not already in the label and therefore in the RNA product)."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype subtype) { return x_IsRnaSubtype(subtype); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
protected:
    void x_AddGeneFieldsToComment(const objects::CGene_ref& orig, CRef<objects::CSeq_feat> new_feat);
    void x_AddOneGeneField(string& product, string& comment, const string& field);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertBioSrcToRptRegion : public CConvertFeatureBase
{
public:
    CConvertBioSrcToRptRegion() : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_repeat_region, objects::CSeqFeatData::eSubtype_biosrc) {};
    virtual string GetDescription() { return "Creates a repeat_region with mobile_element qualifiers for the transposon and/or insertion sequence qualifiers on the BioSource.  All other BioSource information is discarded."; }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);

};


class NCBI_GUIOBJUTILS_EXPORT CConvertCDSToMiscFeat : public CConvertFeatureBase
{
public:
    CConvertCDSToMiscFeat() : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_misc_feature, objects::CSeqFeatData::eSubtype_cdregion), m_Similar(true), m_Nonfunctional(false) {}
    virtual string GetDescription() { return "Copy comment from coding region to new misc_feature and remove product field.  If not pseudo coding region, add product name from protein feature to new misc_feature comment and delete product sequence."; }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
    void SetSimilar(bool f) {m_Similar = f;}
    void SetNonfunctional(bool f) {m_Nonfunctional = f;}
private:
    string x_GetProteinDesc(const objects::CSeq_feat& cds, objects::CScope& scope);
    bool m_Similar;
    bool m_Nonfunctional;

};


class NCBI_GUIOBJUTILS_EXPORT CConvertImpToProt : public CConvertFeatureBase
{
public:
    CConvertImpToProt(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(feat_to, feat_from) {}
    virtual string GetDescription() { return "Original feature must be on nucleotide sequence and be contained in coding region location.  Coding region must have product protein sequence.  New feature is created on product protein sequence so that the translated location will be as close as possible to the original nucleotide location (may not be exact because of codon boundaries)."; }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype feat_from) { return x_IsImportSubtype(feat_from); }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsProtSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);

};


class NCBI_GUIOBJUTILS_EXPORT CConvertProtToImp : public CConvertFeatureBase
{
public:
    CConvertProtToImp(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(feat_to, feat_from) {}
    virtual string GetDescription() { return
    "Original feature must be on a protein sequence that is a product of a coding region.\nNew feature will be created on same sequence as coding region.\n"
    "If protein feature has name, this will be saved as /product qualifier on new feature.\nIf protein feature does not have name but does have description, this will be saved as /product qualifier on new feature.\n"
    "EC_number values from the protein feature will be saved as /EC_number qualifiers on the new feature.\nActivity values will be saved as /function qualifiers on the new feature.\n"
    "Db_xref values from the protein feature will be saved as /db_xref qualifers on the new feature."; }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype feat_from) { return x_IsProtSubtype(feat_from); }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsImportSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);

};


class NCBI_GUIOBJUTILS_EXPORT CConvertImpToRNA : public CConvertFeatureBase
{
public:
    CConvertImpToRNA(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(feat_to, feat_from) {}
    virtual string GetDescription() { return "Creates an RNA feature of the specified subtype.  Import feature key is discarded."; }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype feat_from) { return x_IsImportSubtype(feat_from); }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsRnaSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);

};


class NCBI_GUIOBJUTILS_EXPORT CConvertImpToCDS : public CConvertFeatureBase
{
public:
    CConvertImpToCDS(objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_cdregion, feat_from) {}
    virtual string GetDescription() { return "Use /product qual or comment for coding region product name."; }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype feat_from) { return x_IsImportSubtype(feat_from); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertImpToGene : public CConvertFeatureBase
{
public:
    CConvertImpToGene(objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_gene, feat_from) {}
    virtual string GetDescription() { return "Use /gene qual or comment for gene locus."; }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype feat_from) { return x_IsImportSubtype(feat_from); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertRegionToImp : public CConvertFeatureBase
{
public:
    CConvertRegionToImp(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_region) {}
    virtual string GetDescription() { return "Use region name for new feature comment."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsImportSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertRegionToRna : public CConvertFeatureBase
{
public:
    CConvertRegionToRna(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_region) {}
    virtual string GetDescription() { return "Use region name for RNA product."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsRnaSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertCommentToImp : public CConvertFeatureBase
{
public:
    CConvertCommentToImp(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_comment) {}
    virtual string GetDescription() { return "Create an import feature with the same note as the comment."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsImportSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertGeneToImp : public CConvertFeatureBase
{
public:
    CConvertGeneToImp(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_gene) {}
    virtual string GetDescription() { return "Create an import feature with the gene description and locus prepended to the original comment, separated by semicolons.."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsImportSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertRnaToImp : public CConvertFeatureBase
{
public:
    CConvertRnaToImp(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(feat_to, feat_from) {}
    virtual string GetDescription() { return "Create an import feature with the RNA product name appended to the original comment, separated by semicolons."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsImportSubtype(feat_to); }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype feat_from) { return x_IsRnaSubtype(feat_from); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertmRNAToCDS : public CConvertFeatureBase
{
public:
    CConvertmRNAToCDS(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_cdregion, objects::CSeqFeatData::eSubtype_mRNA) {}
    virtual string GetDescription() { return "Create a coding region feature with the mRNA product name as the protein."; }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConverttRNAToGene : public CConvertFeatureBase
{
public:
    CConverttRNAToGene(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_gene, objects::CSeqFeatData::eSubtype_tRNA) {}
    virtual string GetDescription() { return "Convert tRNA to gene, use tRNA product for gene description."; }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertSiteToImp : public CConvertFeatureBase
{
public:
    CConvertSiteToImp(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_site) {}
    virtual string GetDescription() { return "Creates an import feature of the specified subtype with the site type name as a /note qualifier."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsImportSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertRegionToProt : public CConvertFeatureBase
{
public:
    CConvertRegionToProt(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_region) {}
    virtual string GetDescription() { return 
        "If feature is on nucleotide sequence, will create feature on protein product sequence for overlapping coding region.  Protein name will be region name."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype feat_to) { return x_IsProtSubtype(feat_to); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertRNAToRNA : public CConvertFeatureBase
{
public:
    CConvertRNAToRNA(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(feat_to, feat_from) {};
    virtual string GetDescription() { return "Changes type of RNA feature."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype subtype) { return x_IsRnaSubtype(subtype); }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype subtype) { return x_IsRnaSubtype(subtype); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertToBond : public CConvertFeatureBase
{
public:
    CConvertToBond(objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_bond, feat_from) {};
    virtual string GetDescription() { return "Create Bond feature with specified site type.  Location is a SeqLocBond with a point at the start of the original location and a point at the end of the original location. All feature ID, partialness, except, comment, product, location, genbank qualifiers, title, citation, experimental evidence, gene xrefs, db xrefs, and pseudo-ness information is discarded."; }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertToSite : public CConvertFeatureBase
{
public:
    CConvertToSite(objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_site, feat_from) {};
    virtual string GetDescription() { return "Create Site feature with specified site type.  All feature ID, partialness, except, product, location, genbank qualifiers, title, citation, experimental evidence, gene xrefs, db xrefs, and pseudo-ness information is discarded."; }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertToRegion : public CConvertFeatureBase
{
public:
    CConvertToRegion(objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_region, feat_from) {};
    virtual string GetDescription();
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertProtToProt : public CConvertFeatureBase
{
public:
    CConvertProtToProt(objects::CSeqFeatData::ESubtype feat_to, objects::CSeqFeatData::ESubtype feat_from) : CConvertFeatureBase(feat_to, feat_from) {};
    virtual string GetDescription() { return "Changes type of protein feature."; }
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype subtype) { return x_IsProtSubtype(subtype); }
    virtual bool CanConvertFrom(objects::CSeqFeatData::ESubtype subtype) { return x_IsProtSubtype(subtype); }
};


class NCBI_GUIOBJUTILS_EXPORT CConvertCDSToProt : public CConvertFeatureBase
{
public:
    CConvertCDSToProt(objects::CSeqFeatData::ESubtype feat_to) : CConvertFeatureBase(feat_to, objects::CSeqFeatData::eSubtype_cdregion) {};
    virtual string GetDescription(); 
    virtual bool CanConvertTo(objects::CSeqFeatData::ESubtype subtype) { return x_IsProtSubtype(subtype); }
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};


class NCBI_GUIOBJUTILS_EXPORT CConvertCDSToGene : public CConvertFeatureBase
{
public:
    CConvertCDSToGene() : CConvertFeatureBase(objects::CSeqFeatData::eSubtype_gene, objects::CSeqFeatData::eSubtype_cdregion) {};
    virtual string GetDescription() { return "Create gene feature, copy all protein values to gene note."; } 
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
};

class NCBI_GUIOBJUTILS_EXPORT CConvertCDSToPseudoGene : public CConvertFeatureBase
{
public:
    CConvertCDSToPseudoGene();
    virtual string GetDescription() { return "Create pseudo gene feature, copy all protein values to gene note."; } 
    virtual CRef<CCmdComposite> Convert(const objects::CSeq_feat& orig, bool keep_orig, objects::CScope& scope);
private:
    void x_AddToGeneLocus(string text, CRef<objects::CGene_ref> gene, objects::edit::EExistingText existing_text);
    void x_AddToGeneDescr(string text, CRef<objects::CGene_ref> gene, objects::edit::EExistingText existing_text);
};

END_NCBI_SCOPE

#endif
    // GUI_OBJUTILS___CONVERT_FEAT__HPP
