/*  $Id: macro_editor_context.cpp 44691 2020-02-21 19:19:39Z asztalos $
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


#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/macro/Feat_qual_legal.hpp>
#include <objtools/edit/field_handler.hpp>

#include <util/static_map.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>
#include <gui/widgets/edit/dblink_fieldtype.hpp>
#include <gui/widgets/edit/pub_fieldtype.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CMacroEditorContext::CMacroEditorContext()
{
    m_DescriptorMap.clear();
    m_DescriptorMap.emplace("Any", CSeqdesc::e_not_set);
    m_DescriptorMap.emplace("Name", CSeqdesc::e_Name);
    m_DescriptorMap.emplace("Title", CSeqdesc::e_Title);
    m_DescriptorMap.emplace("Comment", CSeqdesc::e_Comment);
    m_DescriptorMap.emplace("Numbering", CSeqdesc::e_Num);
    m_DescriptorMap.emplace("MapLoc", CSeqdesc::e_Maploc);
    m_DescriptorMap.emplace("PIR", CSeqdesc::e_Pir);
    m_DescriptorMap.emplace("GenBank", CSeqdesc::e_Genbank);
    m_DescriptorMap.emplace("Publication", CSeqdesc::e_Pub);
    m_DescriptorMap.emplace("Region", CSeqdesc::e_Region);
    m_DescriptorMap.emplace("User", CSeqdesc::e_User);
    m_DescriptorMap.emplace("SWISS-PROT", CSeqdesc::e_Sp);
    m_DescriptorMap.emplace("dbXREF", CSeqdesc::e_Dbxref);
    m_DescriptorMap.emplace("EMBL", CSeqdesc::e_Embl);
    m_DescriptorMap.emplace("Create Date", CSeqdesc::e_Create_date);
    m_DescriptorMap.emplace("Update Date", CSeqdesc::e_Update_date);
    m_DescriptorMap.emplace("PRF", CSeqdesc::e_Prf);
    m_DescriptorMap.emplace("PDB", CSeqdesc::e_Pdb);
    m_DescriptorMap.emplace("Heterogen", CSeqdesc::e_Het);
    m_DescriptorMap.emplace("BioSource", CSeqdesc::e_Source);
    m_DescriptorMap.emplace("MolInfo", CSeqdesc::e_Molinfo);
    m_DescriptorMap.emplace("StructuredComment", CSeqdesc::e_User);
    m_DescriptorMap.emplace("DBLink", CSeqdesc::e_User);

    CSeq_entry_Handle seh;
    set<string> existing;

    vector<const CFeatListItem *> featlist = GetSortedFeatList(seh);
    ITERATE(vector<const CFeatListItem *>, feat_it, featlist) {
        const CFeatListItem& item = **feat_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();
        int feat_subtype = item.GetSubtype();
        if (existing.find(desc) == existing.end()) {
            existing.insert(desc);
            m_Featuretypes.push_back(desc);
            m_FeatureMap.insert(make_pair(desc, make_pair(feat_type, feat_subtype)));
        }
    }

    copy(m_SourceTextKeys.begin(), m_SourceTextKeys.end(), back_inserter(m_SourceKeywords));
    copy(m_SourceTaxKeys.begin(), m_SourceTaxKeys.end(), back_inserter(m_SourceKeywords));
    m_DBLinkKeywords = CDBLinkFieldType::GetFieldNames();
    m_MolinfoKeywords = CMolInfoFieldType::GetFieldNames();
    m_PubdescKeywords = CPubFieldType::GetFieldNames(true);
    for (auto& it : m_PubdescKeywords) {
        it = "pub " + it;
    }
    for (auto& it : m_SourceTextKeys) {
        m_SourceKeywords.push_back(it);
    }
    m_CdsGeneProtKeywords = CCDSGeneProtFieldNamePanel::GetStrings();
}

vector<string> CMacroEditorContext::GetFieldNames(EMacroFieldType type) const
{
    bool allow_other = false;
    switch (type) {
    case EMacroFieldType::eBiosourceText:
        return x_GetBsrcTextFieldnames(false);
    case EMacroFieldType::eBiosourceTextAll:
        return x_GetBsrcTextFieldnames(true);
    case EMacroFieldType::eBiosourceTax:
        return x_GetBsrcTaxFieldnames();
    case EMacroFieldType::eBiosourceLocation:
        return CSourceFieldNamePanel::s_GetSourceLocationOptions();
    case EMacroFieldType::eBiosourceOrigin:
        return CSourceFieldNamePanel::s_GetSourceOriginOptions();
    case EMacroFieldType::eBiosourceAll:
        return x_GetAllBsrcFieldnames();
    case EMacroFieldType::eMolinfo:
        return m_MolinfoKeywords;
    case EMacroFieldType::eMolinfoClass:
        return CMolInfoFieldType::GetChoicesForField(CMolInfoFieldType::e_Class, allow_other);
    case EMacroFieldType::eMolinfoCompleteness:
        return CMolInfoFieldType::GetChoicesForField(CMolInfoFieldType::e_Completedness, allow_other);
    case EMacroFieldType::eMolinfoMolecule:
        return CMolInfoFieldType::GetChoicesForField(CMolInfoFieldType::e_MoleculeType, allow_other);
    case EMacroFieldType::eMolinfoStrand:
        return CMolInfoFieldType::GetChoicesForField(CMolInfoFieldType::e_Strand, allow_other);
    case EMacroFieldType::eMolinfoTech:
        return CMolInfoFieldType::GetChoicesForField(CMolInfoFieldType::e_Technique, allow_other);
    case EMacroFieldType::eMolinfoTopology:
        return CMolInfoFieldType::GetChoicesForField(CMolInfoFieldType::e_Topology, allow_other);
    case EMacroFieldType::eDBLink:
        return m_DBLinkKeywords;
    case EMacroFieldType::eMiscDescriptors:
        return m_MiscKeywords;
    case EMacroFieldType::eMiscDescriptorsToRmv:
        return m_MiscKeywordsToRmv;
    case EMacroFieldType::ePubdesc:
        return CPubFieldType::GetFieldNames(true);
    case EMacroFieldType::eDescriptors:
        return x_GetDescriptorFieldnames();
    case EMacroFieldType::eStructComment:
        return x_GetStrCommFieldnames();
    case EMacroFieldType::eGene:
        return m_GeneKeywords;
    case EMacroFieldType::eProtein:
        return x_GetProteinFieldnames();
    case EMacroFieldType::eRNA:
        return CRNAFieldNamePanel::GetRNAFields();
    case EMacroFieldType::eCdsGeneProt:
        return m_CdsGeneProtKeywords;
    case EMacroFieldType::eOther:
        return{ "Local id", "Definition line" };
    case EMacroFieldType::eBiosourceAutodef:
        return m_BsrcAutodefWords;
    case EMacroFieldType::eSetFrom:
        return x_GetSetClassFieldnames(true);
    case EMacroFieldType::eSetTo:
        return x_GetSetClassFieldnames(false);
    default:
        break;
    }
    return vector<string>();
}

vector<string> CMacroEditorContext::x_GetBsrcTextFieldnames(bool complete) const
{
    vector<string> fieldnames;
    // subsource modifiers
    for (size_t i = 0; i < CSubSource::eSubtype_other; i++) {
        try {
            string qual_name = CSubSource::GetSubtypeName(i);
            if (!NStr::IsBlank(qual_name) && !CSubSource::IsDiscouraged(i)) {
                fieldnames.push_back((qual_name));
            }

        }
        catch (const exception&) {
        }
    }
    fieldnames.push_back(kSubSourceNote);

    // orgmod modifiers
    for (size_t i = 0; i < COrgMod::eSubtype_other; i++) {
        try {
            string qual_name = COrgMod::GetSubtypeName(i);
            if (!NStr::IsBlank(qual_name)) {
                // special case for nat-host
                if (NStr::EqualNocase(qual_name, kNatHost)) {
                    fieldnames.push_back(kHost);
                }
                else {
                    fieldnames.push_back(qual_name);
                    if (complete &&
                        (i == COrgMod::eSubtype_bio_material ||
                        i == COrgMod::eSubtype_culture_collection ||
                        i == COrgMod::eSubtype_specimen_voucher)) {
                        fieldnames.push_back(qual_name + macro::kColl_suffix);
                        fieldnames.push_back(qual_name + macro::kInst_suffix);
                        fieldnames.push_back(qual_name + macro::kSpecid_suffix);
                    }
                }
            }

        }
        catch (const exception&) {
        }
    }
    fieldnames.push_back(kOrgModNote);

    // primers
    fieldnames.push_back(kFwdPrimerName);
    fieldnames.push_back(kFwdPrimerSeq);
    fieldnames.push_back(kRevPrimerName);
    fieldnames.push_back(kRevPrimerSeq);

    fieldnames.push_back(kDbXref);

    sort(fieldnames.begin(), fieldnames.end());
    return fieldnames;
}

vector<string> CMacroEditorContext::x_GetBsrcTaxFieldnames() const
{
    return { "taxname", "common name", "division", "lineage" };
}

vector<string> CMacroEditorContext::x_GetAllBsrcFieldnames() const
{
    vector<string> fieldnames = x_GetBsrcTaxFieldnames();
    vector<string> text_names = x_GetBsrcTextFieldnames(true);
    fieldnames.reserve(fieldnames.size() + text_names.size() + 2);
    move(text_names.begin(), text_names.end(), inserter(fieldnames, fieldnames.end()));
    fieldnames.push_back("location");
    fieldnames.push_back("origin");
    return fieldnames;
}

vector<string> CMacroEditorContext::x_GetStrCommFieldnames() const
{
    return { "Field", "Database name", "Field name" };
}

vector<string> CMacroEditorContext::x_GetDescriptorFieldnames() const
{
    vector<string> fieldnames;
    for (auto& it : m_DescriptorMap) {
        fieldnames.push_back(it.first);
    }
    sort(fieldnames.begin(), fieldnames.end());
    return fieldnames;
}

vector<string> CMacroEditorContext::x_GetProteinFieldnames() const
{
    return {
        "protein name",
        "protein description",
        "protein EC number",
        "protein activity",
        "protein comment",
    };
}

vector<string> CMacroEditorContext::x_GetSetClassFieldnames(bool all) const
{
    vector<string> fieldnames;
    if (all) {
        CEnumeratedTypeValues::TValues values = CBioseq_set::ENUM_METHOD_NAME(EClass)()->GetValues();
        for (auto& it : values) {
            if (it.first == "gi" || it.first == "gibb") {
                it.first += "-set";
            }
            toupper((unsigned char)it.first[0]);
            fieldnames.push_back(it.first);
        }
    }
    else {
        fieldnames.push_back(CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(CBioseq_set::eClass_genbank, true));
        fieldnames.push_back(CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(CBioseq_set::eClass_mut_set, true));
        fieldnames.push_back(CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(CBioseq_set::eClass_pop_set, true));
        fieldnames.push_back(CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(CBioseq_set::eClass_phy_set, true));
        fieldnames.push_back(CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(CBioseq_set::eClass_eco_set, true));
        fieldnames.push_back(CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(CBioseq_set::eClass_wgs_set, true));
        fieldnames.push_back(CBioseq_set::ENUM_METHOD_NAME(EClass)()->FindName(CBioseq_set::eClass_small_genome_set, true));
        for (auto&& it : fieldnames) {
            toupper((unsigned char)it[0]);
        }
    }
    return fieldnames;
}

vector<string> CMacroEditorContext::GetFeatureTypes(bool for_removal, bool to_create) const
{
    if (for_removal) {
        return m_Featuretypes;
    }

    vector<string> feat_list;
    if (to_create) {
        feat_list.push_back("Gene");
        feat_list.push_back("misc_feature");
    }

    for (auto& it : m_Featuretypes) {
        CSeqFeatData::ESubtype subtype = CSeqFeatData::SubtypeNameToValue(it);
        if (!CFeatureTypePanel::s_IsRarelyUsedOrDiscouragedFeatureType(subtype) && !CSeqFeatData::IsRegulatory(subtype)) {
            CSeqFeatData::E_Choice type = CSeqFeatData::GetTypeFromSubtype(subtype);
            if (!to_create) {
                feat_list.push_back(it);
            } else if (type == CSeqFeatData::e_Imp && find(feat_list.begin(), feat_list.end(), it) == feat_list.end()) {
                feat_list.push_back(it);
            }
        }
    }
    return feat_list;
}

pair<int, int> CMacroEditorContext::GetFeatureType(const string& name) const
{
    auto it = m_FeatureMap.find(name);
    return (it != m_FeatureMap.end()) ? it->second : pair<int, int>();
}

vector<string> CMacroEditorContext::GetLegalQualifiers(const string& feat_name) const
{
    CSeqFeatData::ESubtype subtype = macro::NMacroUtil::GetFeatSubtype(feat_name);
    auto quals = CSeqFeatData::GetLegalQualifiers(subtype);
    vector<string> qual_names;
    for (auto it : quals) {
        qual_names.push_back(CSeqFeatData::GetQualifierAsString(it));
    }
    return qual_names;
}

vector<string> CMacroEditorContext::GetFeatQualifiers() const
{
    wxArrayString qual_list = CFeatureFieldNamePanel::s_FillFeatQualList(CSeqFeatData::eSubtype_any);
    vector<string> names;
    FromArrayString(qual_list, names);
    return names;
}

vector<string> CMacroEditorContext::GetRNATypes() const
{
    return CRNAFieldNamePanel::GetRNATypes();
}

vector<string> CMacroEditorContext::GetncRNAClassTypes(bool include_any) const
{
    if (!include_any) {
        return CRNA_gen::GetncRNAClassList();
    }

    vector<string> types{ "any" };
    vector<string> class_types = CRNA_gen::GetncRNAClassList();
    types.reserve(class_types.size() + 1);
    copy(class_types.begin(), class_types.end(), back_inserter(types));
    return types;
}

vector<string> CMacroEditorContext::GetRNAProductnames(const string& rna_type) const
{
    if (NStr::EqualNocase(rna_type, "rRNA")) {
        return {
            "4.5S ribosomal RNA",
            "5S ribosomal RNA",
            "5.8S ribosomal RNA",
            "12S ribosomal RNA",
            "15S ribosomal RNA",
            "16S ribosomal RNA",
            "18S ribosomal RNA",
            "21S ribosomal RNA",
            "23S ribosomal RNA",
            "25S ribosomal RNA",
            "26S ribosomal RNA",
            "28S ribosomal RNA",
            "large subunit ribosomal RNA",
            "small subunit ribosomal RNA" };
    }
    else if (NStr::EqualNocase(rna_type, "tRNA")) {
        return{
            "A Alanine",
            "B Asp or Asn",
            "C Cysteine",
            "D Aspartic Acid",
            "E Glutamic Acid",
            "F Phenylalanine",
            "G Glycine",
            "H Histidine",
            "I Isoleucine",
            "J Leu or Ile",
            "K Lysine",
            "L Leucine",
            "M Methionine",
            "N Asparagine",
            "O Pyrrolysine",
            "P Proline",
            "Q Glutamine",
            "R Arginine",
            "S Serine",
            "T Threonine",
            "U Selenocysteine",
            "V Valine",
            "W Tryptophan",
            "X Undetermined",
            "Y Tyrosine",
            "Z Glu or Gln",
            "* Stop Codon" };
    }
    return {};
}

vector<string> CMacroEditorContext::GetBondTypes() const
{
    const CBondList* list = CSeqFeatData::GetBondList();
    vector<string> bond_list;
    for (auto it = list->begin(); it != list->end(); ++it) {
        bond_list.push_back(it->first);
    }
    return bond_list;
}

vector<string> CMacroEditorContext::GetSiteTypes() const
{
    const CSiteList* list = CSeqFeatData::GetSiteList();
    vector<string> site_list;
    for (auto it = list->begin(); it != list->end(); ++it) {
        site_list.push_back(it->first);
    }
    return site_list;
}

string CMacroEditorContext::GetConversionDescr(const string& from_feat, const string& to_feat)
{
    CSeqFeatData::ESubtype from_subtype = macro::NMacroUtil::GetFeatSubtype(from_feat);
    CSeqFeatData::ESubtype to_subtype = macro::NMacroUtil::GetFeatSubtype(to_feat);
    CRef<CConvertFeatureBase> converter = CConvertFeatureBaseFactory::Create(from_subtype, to_subtype);
    return converter->GetDescription();
}

const vector<string>& CMacroEditorContext::GetKeywords(EMacroFieldType type) const
{
    switch (type) {
    case EMacroFieldType::eBiosourceText:
        return m_SourceTextKeys;
    case EMacroFieldType::eBiosourceTax:
        return m_SourceTaxKeys;
    case EMacroFieldType::eMolinfo:
        return m_MolinfoKeywords;
    case EMacroFieldType::ePubdesc:
        return m_PubdescKeywords;
    case EMacroFieldType::eDBLink:
        return m_DBLinkKeywords;
    case EMacroFieldType::eMiscDescriptors:
        return m_MiscKeywords;
    case EMacroFieldType::eMiscDescriptorsToRmv:
        return m_MiscKeywordsToRmv;
    case EMacroFieldType::eDescriptors:
        return m_DescrKeywords;
    case EMacroFieldType::eGene:
        return m_GeneKeywords;
    case EMacroFieldType::eRNA:
        return m_RnaKeywords;
    case EMacroFieldType::eProtein:
        return m_ProteinKeywords;
    case EMacroFieldType::eFeatQualifiers:
        return m_FeatQualKeywords;
    case EMacroFieldType::eCdsGeneProt:
        return m_CdsGeneProtKeywords;
    case EMacroFieldType::eBiosourceAutodef:
        return m_BsrcAutodefWords;
    default:
        break;
    }
    return m_EmptyKeywords;
}



wxString CMacroEditorContext::GetAbsolutePath(const wxString& localpath)
{
#ifdef NCBI_OS_MSWIN
#include <Winnetwk.h>

    UNIVERSAL_NAME_INFO* unameinfo;
    DWORD bufferLength = 1024;

    const TCHAR* name = localpath.data();
    TCHAR buffer[1024];
    unameinfo = (UNIVERSAL_NAME_INFO *)&buffer;
    DWORD result = WNetGetUniversalName(localpath.data(), UNIVERSAL_NAME_INFO_LEVEL, (LPVOID)unameinfo, &bufferLength);

    if (result == NO_ERROR) {
        return unameinfo->lpUniversalName;
    }
    return localpath;
#else
    auto path = CDirEntry::NormalizePath(CDirEntry::CreateAbsolutePath(string(localpath.ToUTF8())), eFollowLinks);
    return wxString::FromUTF8(path.c_str());
#endif
}

string CMacroEditorContext::GetAsnPathToFieldName(const string& feat, const string& qual)
{
    return x_GetAsnPathToFeatQuals(feat, qual);
}

string CMacroEditorContext::x_GetAsnPathToFeatQuals(const string& feat, const string& qual)
{
    string qual_name(qual);
    NStr::ReplaceInPlace(qual_name, "-", "_");

    string path;
    if (NStr::EqualNocase(feat, "gene")) {
        if (qual_name == "description") {
            path = "data.gene.desc";
        } else if (qual_name == "locus") {
            path = "data.gene.locus";
        } else if (macro::NMacroUtil::StringsAreEquivalent(qual_name, "locus-tag")) {
            path = "data.gene.locus-tag";
        } else if (qual_name == "synonym") {
            path = "data.gene.syn";
        } else if (qual_name == "allele") {
            path = "data.gene.allele";
        }
    }
   
    if (NStr::EqualNocase(feat, "protein")) {
        if (qual_name == "product") {
            path = "data.prot.name";
        } else if (qual_name == "description") {
            path = "data.prot.desc";
        } else if (qual_name == "activity") {
            path = "data.prot.activity";
        } else if (macro::NMacroUtil::StringsAreEquivalent(qual_name, "EC-number")) {
            path = "data.prot.ec";
        }
    }

    if (qual_name == "product") {
        if (feat.find("RNA") != NPOS) {
            if (NStr::EqualNocase(feat, "mRNA") ||
                NStr::EqualNocase(feat, "rRNA") ||
                NStr::EqualNocase(feat, "preRNA")) {
                path = "data.rna.ext.name";
            }
            else if (NStr::EqualNocase(feat, "misc_RNA") ||
                NStr::EqualNocase(feat, "ncRNA") ||
                NStr::EqualNocase(feat, "tmRNA")) {
                path = "data.rna.ext.gen.product";
            }
        }
    }

    if (NStr::EqualNocase(feat, "cds")) {
        if (qual_name == "product") {
            path = "data.prot.name";
        } else if (qual_name == "activity") {
            path = "data.prot.activity";
        }
        else if (macro::NMacroUtil::StringsAreEquivalent(qual_name, "EC-number")) {
            path = "data.prot.ec";
        }
    }
     
    if (qual_name == "note" || qual_name == "comment") {
        path = "comment";
    }

    if (qual_name == "db_xref") {
        path = "dbxref";
    }

    if (path.empty()) {
        path = "qual," + qual_name;
    }

    return path;
}

string CMacroEditorContext::GetAsnPathToFeature(const string& feat_name) const
{
    CSeqFeatData::ESubtype subtype = macro::NMacroUtil::GetFeatSubtype(feat_name);
    string asn_path_to_feature;
    switch (subtype) {
    case CSeqFeatData::eSubtype_cdregion:
    case CSeqFeatData::eSubtype_gene:
    case CSeqFeatData::eSubtype_prot:
    case CSeqFeatData::eSubtype_preprotein:
    case CSeqFeatData::eSubtype_mat_peptide_aa:
    case CSeqFeatData::eSubtype_sig_peptide_aa:
    case CSeqFeatData::eSubtype_transit_peptide_aa:
    case CSeqFeatData::eSubtype_preRNA:
    case CSeqFeatData::eSubtype_mRNA:
    case CSeqFeatData::eSubtype_bond:
    case CSeqFeatData::eSubtype_site:
    case CSeqFeatData::eSubtype_psec_str:
    case CSeqFeatData::eSubtype_region:
    case CSeqFeatData::eSubtype_pub:
    case CSeqFeatData::eSubtype_comment:
    case CSeqFeatData::eSubtype_biosrc:
        break;
    default:
        asn_path_to_feature = "\"data.imp.key\", \"" + CSeqFeatData::SubtypeValueToName(subtype) + "\"";
        break;
    }
    return asn_path_to_feature;
}

string CMacroEditorContext::GetAsnPathToAuthorFieldName(const string& author_name)
{
    string val;
    if (NStr::FindNoCase(author_name, "last") != NPOS)
        val = "last";
    else if (NStr::FindNoCase(author_name, "first") != NPOS)
        val = "first";
    else if (NStr::FindNoCase(author_name, "suffix") != NPOS)
        val = "suffix";
    else if (NStr::FindNoCase(author_name, "initials") != NPOS)
        val = "initials";
    else if (NStr::FindNoCase(author_name, "consortium") != NPOS)
        val = "consortium";
    return val;
}

string CMacroEditorContext::GetAsnPathToFieldName(const string& field, EMacroFieldType type, const string& target)
{
    m_Field = field;
    // special case for sequence-id
    if (NStr::EqualNocase(m_Field, kFieldTypeSeqId)) {
        return macro::CMacroFunction_SeqID::GetFuncName() + "()";
    }

    switch (type) {
    case EMacroFieldType::eBiosourceText:
    case EMacroFieldType::eBiosourceTextAll:
        return x_GetAsnPathToBsrcText();
    case EMacroFieldType::eBiosourceTax:
        return x_GetAsnPathToBsrcTax();
    case EMacroFieldType::eBiosourceOrigin:
        return "origin";
    case EMacroFieldType::eBiosourceLocation:
        return "genome";
    case EMacroFieldType::eBiosourceAll:
        return x_GetAsnPathToAllBsrc();
    case EMacroFieldType::eMolinfo:
        return x_GetAsnPathToMolinfo(target);
    case EMacroFieldType::ePubdesc:
    case EMacroFieldType::ePubTitle:
    case EMacroFieldType::ePubAuthors:
    case EMacroFieldType::ePubAffilCountry:
        return x_GetAsnPathToPub();
    case EMacroFieldType::ePubAffilFields:
        return x_GetAsnPathToAffilFields();
    case EMacroFieldType::ePubAffil:
        return "PUB_AFFIL()";
    case EMacroFieldType::ePubAffilExcept:
        return "PUB_AFFIL()";
    case EMacroFieldType::eDBLink:
        return x_GetAsnPathToDBLinkField(target);
    case EMacroFieldType::eMiscDescriptors:
    case EMacroFieldType::eMiscDescriptorsToRmv:
        return x_GetAsnPathToMiscDescr(target);
    case EMacroFieldType::eDescriptors:
        return x_GetAsnPathToDescriptors();
    case EMacroFieldType::eGene:
        return x_GetAsnPathtoGene();
    case EMacroFieldType::eRNA:
        return x_GetAsnPathToRna();
    case EMacroFieldType::eCds:
        return x_GetAsnPathToCDS();
    case EMacroFieldType::eCdsGeneProt:
        return x_GetAsnPathToCdsGeneProt();
    case EMacroFieldType::eProtein:
        return x_GetAsnPathToProtein();
    case EMacroFieldType::eFeatQualifiers:
        return x_GetAsnPathToFeatQuals();
    case EMacroFieldType::eOther:
        return x_GetAsnPathToMiscFields();
    case EMacroFieldType::eSetFrom:
    case EMacroFieldType::eSetTo:
        return x_GetAsnPathToSet();
    default:
        break;
    }
    return kEmptyStr;
}

string CMacroEditorContext::x_GetAsnPathToBsrcTax()
{
    string path;
    if (NStr::EqualNocase(m_Field, "taxname")) {
        path = "org.taxname";
    }
    else if (NStr::EqualNocase(m_Field, "common name")) {
        path = "org.common";
    }
    else if (NStr::EqualNocase(m_Field, "division")) {
        path = "org.orgname.div";
    }
    else if (NStr::EqualNocase(m_Field, "lineage")) {
        path = "org.orgname.lineage";
    }
    return path;
}

string CMacroEditorContext::x_GetAsnPathToBsrcText()
{
    string path;
    if (edit::CFieldHandler::QualifierNamesAreEquivalent(m_Field, kHost) ||
        edit::CFieldHandler::QualifierNamesAreEquivalent(m_Field, kNatHost)) {
        path = "org.orgname.mod,nat-host";
    }
    else if (edit::CFieldHandler::QualifierNamesAreEquivalent(m_Field, kOrgModNote)) {
        path = "org.orgname.mod,other";
    }
    else if (edit::CFieldHandler::QualifierNamesAreEquivalent(m_Field, kSubSourceNote)) {
        path = "subtype,other";
    }
    else if (NStr::EqualNocase(m_Field, kFwdPrimerSeq)) {
        path = "pcr-primers..forward..seq";
    }
    else if (NStr::EqualNocase(m_Field, kRevPrimerSeq)) {
        path = "pcr-primers..reverse..seq";
    }
    else if (NStr::EqualNocase(m_Field, kFwdPrimerName)) {
        path = "pcr-primers..forward..name";
    }
    else if (NStr::EqualNocase(m_Field, kRevPrimerName)) {
        path = "pcr-primers..reverse..name";
    }
    else if (edit::CFieldHandler::QualifierNamesAreEquivalent(m_Field, kDbXref)) {
        path = "org.db";
    }
    else if (COrgMod::IsValidSubtypeName(m_Field, COrgMod::eVocabulary_insdc)) {
        path = "org.orgname.mod," + m_Field;
    }
    else if (CSubSource::IsValidSubtypeName(m_Field, CSubSource::eVocabulary_insdc)) {
        path = "subtype," + m_Field;
    }
    
    return path;
}

string CMacroEditorContext::x_GetAsnPathToAllBsrc()
{
    string path = x_GetAsnPathToBsrcTax();
    if (path.empty()) {
        if (NStr::EqualNocase(m_Field, "origin"))
            path = "origin";
        else if (NStr::EqualNocase(m_Field, "location"))
            path = "genome";
        else {
            path = x_GetAsnPathToBsrcText();
        }
    }
    return path;
}

string CMacroEditorContext::x_GetAsnPathToMolinfo(const string& target)
{
    string path;
    if (NStr::EqualNocase(m_Field, "molecule")) {
        path = "biomol";
    }
    else if (NStr::EqualNocase(m_Field, "technique")) {
        path = "tech";
    }
    else if (NStr::EqualNocase(m_Field, "completedness")) {
        path = "completeness";
    }


    if ((target == macro::CMacroBioData::sm_Seq ||
        target == macro::CMacroBioData::sm_SeqNa ||
        target == macro::CMacroBioData::sm_SeqAa) && !path.empty()) {
        path = "descr..molinfo." + path;
    }

    if (NStr::EqualNocase(m_Field, "class")) {
        path = "inst.mol";
    }
    else if (NStr::EqualNocase(m_Field, "topology")) {
        path = "inst.topology";
    }
    else if (NStr::EqualNocase(m_Field, "strand")) {
        path = "inst.strand";
    }
    return path;
}


string CMacroEditorContext::x_GetAsnPathToMiscDescr(const string& target)
{
    string path;
    if (NStr::EqualNocase(m_Field, kDefinitionLineLabel)) {
        path = "title";
    }
    else if (NStr::EqualNocase(m_Field, kCommentDescriptorLabel)) {
        path = "comment";
    }
    else if (NStr::EqualNocase(m_Field, kGenbankBlockKeyword)) {
        path = "genbank.keywords";
    }

    if ((target == macro::CMacroBioData::sm_Seq ||
        target == macro::CMacroBioData::sm_SeqNa ||
        target == macro::CMacroBioData::sm_SeqAa) && !path.empty()) {
        path = "descr.." + path;
    }
    return path;
}

string CMacroEditorContext::x_GetAsnPathToMiscFields()
{
    string path;
    if (macro::NMacroUtil::StringsAreEquivalent(m_Field, "localid")) {
        path = "localid";
    }
    else if (NStr::EqualNocase(m_Field, "definition line")) {
        path = "defline";
    }
    return path;
}

string CMacroEditorContext::x_GetAsnPathToDBLinkField(const string& target)
{
    if (target == macro::CMacroBioData::sm_SeqNa) {
        return macro::CMacroFunction_GetDBLink::GetFuncName() + "(\"" + m_Field + "\")";
    }
    return "data.data.strs";
}

string CMacroEditorContext::x_GetAsnPathToPub()
{
    CPubFieldType::EPubFieldType field_type = CPubFieldType::GetTypeForLabel(m_Field);
    return CPubFieldType::GetMacroLabelForType(field_type);
}

string CMacroEditorContext::x_GetAsnPathToAffilFields()
{
    CPubFieldType::EPubFieldType type = CPubFieldType::GetTypeForLabel(m_Field);
    switch (type) {
    case CPubFieldType::ePubFieldType_AffilAffil:
        return "affil";
    case CPubFieldType::ePubFieldType_AffilDept:
        return "div";
    case CPubFieldType::ePubFieldType_AffilState:
        return "sub";
    case CPubFieldType::ePubFieldType_AffilPostalCode:
        return "postal-code";
    default:
        break;
    }
    return m_Field;
}

string CMacroEditorContext::x_GetAsnPathToDescriptors()
{
    auto it = m_DescriptorMap.find(m_Field);
    if (it != m_DescriptorMap.end()) {
        string name = CSeqdesc::SelectionName(it->second);
        return name;
    }
    return kEmptyStr;
}


string CMacroEditorContext::x_GetAsnPathtoGene()
{
    string path;
    if (NStr::EqualNocase(m_Field, "gene locus")) {
        path = "data.gene.locus";
    }
    else if (NStr::EqualNocase(m_Field, "gene description")) {
        path = "data.gene.desc";
    }
    else if (NStr::EqualNocase(m_Field, "gene comment")) {
        path = "comment";
    }
    else if (NStr::EqualNocase(m_Field, "gene allele")) {
        path = "data.gene.allele";
    }
    else if (NStr::EqualNocase(m_Field, "gene maploc")) {
        path = "data.gene.maploc";
    }
    else if (NStr::EqualNocase(m_Field, "gene locus tag")) {
        path = "data.gene.locus-tag";
    }
    else if (NStr::EqualNocase(m_Field, "gene synonym")) {
        path = "data.gene.syn";
    }
    else if (NStr::StartsWith(m_Field, "gene ")) {
        SIZE_TYPE pos = m_Field.find(" ");
        path = "qual," + m_Field.substr(pos + 1);
    }
    return path;
}

string CMacroEditorContext::x_GetAsnPathToCDS()
{
    string path;
    if (NStr::EqualNocase(m_Field, "cds comment")) {
        path = "comment";
    }
    else if (NStr::EqualNocase(m_Field, "cds inference")) {
        path = "qual,inference";
    }
    else if (macro::NMacroUtil::StringsAreEquivalent(m_Field, "codon-start")) {
        path = "data.cdregion.frame";
    }
    return path;
}

string CMacroEditorContext::x_GetAsnPathToRna()
{
    if (NStr::StartsWith(m_Field, "gene")) {
        return x_GetAsnPathtoGene();
    }
    else {
        SIZE_TYPE pos = m_Field.find(" ");
        if (pos != NPOS) {
            return x_GetAsnPathToRnaFieldName(m_Field.substr(0, pos), m_Field.substr(pos + 1));
        }
    }
    return kEmptyStr;
}

string CMacroEditorContext::x_GetAsnPathToProtein()
{
    string path;
    if (NStr::EqualNocase(m_Field, "protein comment")) {
        path = "comment";
    }
    else if (NStr::EqualNocase(m_Field, "protein name")) {
        path = "data.prot.name";
    }
    else if (NStr::EqualNocase(m_Field, "protein description")) {
        path = "data.prot.desc";
    }
    else if (NStr::EqualNocase(m_Field, "protein EC number")) {
        path = "data.prot.ec";
    }
    else if (NStr::EqualNocase(m_Field, "protein activity")) {
        path = "data.prot.activity";
    }
    return path;
}

string CMacroEditorContext::x_GetAsnPathToCdsGeneProt()
{
    if (NStr::StartsWith(m_Field, "mat_peptide")) {
        NStr::ReplaceInPlace(m_Field, "mat_peptide", "protein");
    }

    if (NStr::StartsWith(m_Field, "protein")) {
        return x_GetAsnPathToProtein();
    }
    else if (NStr::StartsWith(m_Field, "CDS", NStr::eNocase) ||
        macro::NMacroUtil::StringsAreEquivalent(m_Field, "codon-start")) {
        return x_GetAsnPathToCDS();
    }
    else if (NStr::StartsWith(m_Field, "gene")) {
        return x_GetAsnPathtoGene();
    }
    else if (NStr::StartsWith(m_Field, "mRNA")) {
        SIZE_TYPE pos = m_Field.find(" ");
        return x_GetAsnPathToRnaFieldName("mRNA", m_Field.substr(pos + 1));
    }
    return kEmptyStr;
}

string CMacroEditorContext::x_GetAsnPathToFeatQuals()
{
    vector<string> tokens;
    NStr::Split(m_Field, " ", tokens);
    if (tokens.size() == 2) {
        return x_GetAsnPathToFeatQuals(tokens[0], tokens[1]);
    }
    return kEmptyStr;
}

string CMacroEditorContext::x_GetAsnPathToRnaFieldName(const string& rna_type, const string& field)
{
    string path;

    if (field == "comment" || field == "note") {
        path = "comment";
    }

    if (field == "product") {
        if (NStr::EqualNocase(rna_type, "mRNA") ||
            NStr::EqualNocase(rna_type, "rRNA") ||
            NStr::EqualNocase(rna_type, "preRNA")) {
            path = "data.rna.ext.name";
        }
        else if (NStr::EqualNocase(rna_type, "miscRNA") ||
            NStr::EqualNocase(rna_type, "ncRNA") ||
            NStr::EqualNocase(rna_type, "tmRNA")) {
            path = "data.rna.ext.gen.product";
        }
        else if (NStr::EqualNocase(rna_type, "tRNA") ||
            NStr::EqualNocase(rna_type, "any")) {
            path = rna_type + "::" + field;
        }
    }

    if (NStr::EqualNocase(field, "ncRNA class") && NStr::EqualNocase(rna_type, "ncRNA")) {
        path = "data.rna.ext.gen.class";
    }

    if (NStr::EqualNocase(field, "codons-recognized")) {
        if (NStr::EqualNocase(rna_type, "tRNA")) {
            // only to remove
            path = "data.rna.ext.tRNA.codon";
        }
        else {
            path = "qual,codons_recognized";
        }

    }

    if (NStr::EqualNocase(field, "tag-peptide")) {
        if (NStr::EqualNocase(rna_type, "tmRNA")) {
            path = "data.rna.ext.gen.quals,tag_peptide";
        }
        else {
            path = "qual,tag_peptide";
        }
    }

    if (NStr::EqualNocase(field, "anticodon")) {
        if (NStr::EqualNocase(rna_type, "tRNA")) {
            // only to remove
            path = "data.rna.ext.tRNA.anticodon";
        }
        else {
            path = "qual," + field;
        }
    }

    return path;
}

CSeqdesc::E_Choice CMacroEditorContext::GetDescriptorType(const string& name) const
{
    auto it = m_DescriptorMap.find(name);
    if (it != m_DescriptorMap.end()) {
        return it->second;
    }
    return CSeqdesc::e_not_set;
}

string CMacroEditorContext::GetAsnMolinfoValue(const string& molinfo_field, const string& choice)
{
    // handle outliers first:
    if (choice == "nucleotide")
        return "na";
    else if (choice == "protein")
        return "aa";
    else if (choice == "double")
        return "ds";
    else if (choice == "single")
        return "ss";
    else if (choice == "precursor RNA") {
        return "pre-RNA";
    }


    CMolInfoFieldType::EMolInfoFieldType field_type = CMolInfoFieldType::GetFieldType(molinfo_field);
    if (choice == " ") {
        switch (field_type) {
        case CMolInfoFieldType::e_Class:
        case CMolInfoFieldType::e_Strand:
        case CMolInfoFieldType::e_Topology:
        case CMolInfoFieldType::e_Repr:
            return "not-set";
        case CMolInfoFieldType::e_Completedness:
        case CMolInfoFieldType::e_MoleculeType:
        case CMolInfoFieldType::e_Technique:
            return "unknown";
        default:
            return kEmptyStr;
        }
    }

    if (field_type == CMolInfoFieldType::e_Repr) {
        if (choice == "consensus")
            return "consen";
        else if (choice == "constructed")
            return "const";
        else if (choice == "ordered map")
            return "map";
        else if (choice == "reference to another")
            return "ref";
        else if (choice == "segmented")
            return "seg";
    }


    CEnumeratedTypeValues::TValues values;
    switch (field_type) {
    case CMolInfoFieldType::e_Class:
        values = CSeq_inst::ENUM_METHOD_NAME(EMol)()->GetValues();
        break;
    case CMolInfoFieldType::e_Completedness:
        values = CMolInfo::ENUM_METHOD_NAME(ECompleteness)()->GetValues();
        break;
    case CMolInfoFieldType::e_MoleculeType:
        values = CMolInfo::ENUM_METHOD_NAME(EBiomol)()->GetValues();
        break;
    case CMolInfoFieldType::e_Strand:
        values = CSeq_inst::ENUM_METHOD_NAME(EStrand)()->GetValues();
        break;
    case CMolInfoFieldType::e_Technique:
        values = CMolInfo::ENUM_METHOD_NAME(ETech)()->GetValues();
        break;
    case CMolInfoFieldType::e_Topology:
        values = CSeq_inst::ENUM_METHOD_NAME(ETopology)()->GetValues();
        break;
    case CMolInfoFieldType::e_Repr:
        values = CSeq_inst::ENUM_METHOD_NAME(ERepr)()->GetValues();
    default:
        break;
    }

    auto it = find_if(values.begin(), values.end(), [&choice](const pair<string, TEnumValueType>& elem)
    { return edit::CFieldHandler::QualifierNamesAreEquivalent(elem.first, choice); });
    
    if (it != values.end()) {
        return it->first;
    }
    
    LOG_POST(Error << choice  << " could not be mapped onto an enum value");
    return kEmptyStr;
}

 
string CMacroEditorContext::x_GetAsnPathToSet()
{
    m_Field[0] = tolower((unsigned char)m_Field[0]);
    if (m_Field == "gi-set" || m_Field == "gibb-set") {
        m_Field = m_Field.substr(0, m_Field.find("-"));
    }
    return m_Field;
}




END_NCBI_SCOPE
