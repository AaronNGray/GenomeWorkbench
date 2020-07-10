#ifndef GUI_WIDGETS_EDIT___MACRO_EDITOR_CONTEXT__HPP
#define GUI_WIDGETS_EDIT___MACRO_EDITOR_CONTEXT__HPP
/*  $Id: macro_editor_context.hpp 44628 2020-02-10 18:26:27Z asztalos $
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
#include <objects/seq/Seqdesc.hpp>
#include <gui/objutils/macro_engine.hpp>

BEGIN_NCBI_SCOPE

enum class EMacroFieldType {
    eBiosourceLocation,
    eBiosourceOrigin,
    eBiosourceTax,
    eBiosourceText,
    eBiosourceTextAll,
    eBiosourceAutodef,
    eBiosourceAll,
    eCds,
    eCdsGeneProt,
    eDBLink,
    eDescriptors,
    eFeatQualifiers,
    eGene,
    eMiscDescriptors,
    eMiscDescriptorsToRmv,
    eMolinfo,
    eMolinfoClass,
    eMolinfoCompleteness,
    eMolinfoMolecule,
    eMolinfoStrand,
    eMolinfoTech,
    eMolinfoTopology,
    eOther,
    eProtein,
    ePubAffil,
    ePubAffilFields,
    ePubAffilCountry,
    ePubAffilExcept,
    ePubAuthors,
    ePubTitle,
    ePubdesc,
    eRNA,
    eSetFrom,
    eSetTo,
    eStructComment,
    eNotSet
};

// CMacroEditorContext - a singleton
class CMacroEditorContext
{
public:
    static CMacroEditorContext& GetInstance() {
        static CMacroEditorContext instance;
        return instance;
    }

    vector<string> GetFieldNames(EMacroFieldType type) const;
    string GetAsnPathToFieldName(const string& field, EMacroFieldType type, const string& target = kEmptyStr);
    string GetAsnPathToFieldName(const string& feat, const string& qual);
    string GetAsnPathToFeature(const string& feat_name) const;
    
    string GetAsnPathToAuthorFieldName(const string& author_name);
    
    string GetAsnMolinfoValue(const string& molinfo_field, const string& choice);
    
    objects::CSeqdesc::E_Choice GetDescriptorType(const string& name) const;
    vector<string> GetFeatureTypes(bool for_removal = false, bool to_create = false) const;
    vector<string> GetFeatQualifiers() const;
    pair<int, int> GetFeatureType(const string& name) const;
    // returns the list of legal qualifiers for a feature type
    vector<string> GetLegalQualifiers(const string& feat_name) const;

    vector<string> GetRNATypes() const;
    vector<string> GetncRNAClassTypes(bool include_any = true) const;
    vector<string> GetRNAProductnames(const string& rna_type) const;
    vector<string> GetBondTypes() const;
    vector<string> GetSiteTypes() const;
    string GetConversionDescr(const string& from_feat, const string& to_feat);
    const vector<string>& GetBsrcKeywords() const { return m_SourceKeywords; }
    const vector<string>& GetParseToBsrcKeywords() const { return m_ParseToBsrcKeywords; }
    const vector<string>& GetParseToCdsGeneProtKeywords() const { return m_ParseTpCdsGeneProtKeywords; }

    // delete the copy constructor & assignment operator
    CMacroEditorContext(const CMacroEditorContext&) = delete;
    CMacroEditorContext& operator=(const CMacroEditorContext&) = delete;

    const vector<string>& GetKeywords(EMacroFieldType type) const;

    static wxString GetAbsolutePath(const wxString& localpath);
private:
    CMacroEditorContext();

    vector<string> x_GetProteinFieldnames() const;
    vector<string> x_GetBsrcTextFieldnames(bool complete = false) const;
    vector<string> x_GetBsrcTaxFieldnames() const;
    vector<string> x_GetAllBsrcFieldnames() const;
    vector<string> x_GetSetClassFieldnames(bool all = true) const;
    vector<string> x_GetDescriptorFieldnames() const;
    vector<string> x_GetStrCommFieldnames() const;

    // These functions check the content of m_Field
    string x_GetAsnPathToBsrcTax();
    string x_GetAsnPathToBsrcText();
    string x_GetAsnPathToAllBsrc();
    string x_GetAsnPathToMolinfo(const string& target);
    string x_GetAsnPathToPub();
    string x_GetAsnPathToAffilFields();
    string x_GetAsnPathToMiscDescr(const string& target);
    string x_GetAsnPathToDescriptors();
    string x_GetAsnPathToSet();
    string x_GetAsnPathToMiscFields();
    string x_GetAsnPathToDBLinkField(const string& target);

    string x_GetAsnPathtoGene();
    string x_GetAsnPathToCDS();
    string x_GetAsnPathToRna();
    string x_GetAsnPathToProtein();
    string x_GetAsnPathToCdsGeneProt();
    string x_GetAsnPathToFeatQuals();
    string x_GetAsnPathToFeatQuals(const string& feat, const string& qual);
    string x_GetAsnPathToRnaFieldName(const string& rna_type, const string& field);

    string m_Field;
    map<string, objects::CSeqdesc::E_Choice> m_DescriptorMap;
    map<string, pair<int, int>> m_FeatureMap;
    vector<string> m_Featuretypes;
    vector<string> m_SourceTextKeys{"strain", "host", "isolate", "isolation-source"};
    vector<string> m_SourceTaxKeys{ "taxname", "common", "lineage" };
    vector<string> m_SourceKeywords;
    vector<string> m_MolinfoKeywords;
    vector<string> m_PubdescKeywords;
    vector<string> m_DBLinkKeywords;
    vector<string> m_MiscKeywords{ "Definition line", "Comment descriptor", "Keyword" };
    vector<string> m_MiscKeywordsToRmv{ "Definition line", "Comment descriptor", "Keyword", "Genome Project ID"};
    vector<string> m_DescrKeywords{ "Biosource", "molinfo", "publication", "structured comment", "title" };
    vector<string> m_GeneKeywords{ "gene locus", "gene description", "gene comment", 
        "gene allele", "gene maploc", "gene locus tag", "gene synonym" };
    vector<string> m_RnaKeywords{ "rna product", "product name", "rna comment", "ncRNA class", "rna codons-recognized", 
                "rna tag-peptide", "rna anticodon", "gene locus"};
    vector<string> m_ProteinKeywords{ "product name", "protein name", "product description", "protein description", 
        "EC number", "protein activity", "product comment", "protein comment" };
    vector<string> m_FeatQualKeywords{ "cds product", "product name", "cds interference", "anticodon", "note", "EC", "rna", "misc-feature", "gene" };
    vector<string> m_CdsGeneProtKeywords;
    vector<string> m_ParseToBsrcKeywords{ "local id", "definition line", "defline", "taxname", "strain", "isolate"};
    vector<string> m_ParseTpCdsGeneProtKeywords{ "local id", "definition line", "cds comment", "gene locus", "protein name", "mRNA product", "mat_peptide" };
    vector<string> m_BsrcAutodefWords{ "clone", "cultivar", "culture-collection", "haplogroup", "isolate", "strain", "specimen-voucher" };
    vector<string> m_EmptyKeywords;
};

struct SFieldFromTable 
{
    string m_Column;   // field's column number in the table
    string m_MacroName;  // referencing the field in the macro
    string m_GUIName;   // name of the field as it appears in the dialog
    bool m_UseMacroName{ false };  // true, if the m_MacroName should be used in the macro 

    SFieldFromTable() {}
    SFieldFromTable(const string& col, const string& macro_name, const string& label, bool use_macro_name)
        : m_Column(col), m_MacroName(macro_name), m_GUIName(label), m_UseMacroName(use_macro_name) {}
    const string& GetFieldName() const
    {
        return (m_UseMacroName) ? m_MacroName : m_GUIName;
    }
    void Reset()
    {
        m_Column = m_MacroName = m_GUIName = kEmptyStr;
        m_UseMacroName = false;
    }
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_EDITOR_CONTEXT__HPP
