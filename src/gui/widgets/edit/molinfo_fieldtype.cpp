/*  $Id: molinfo_fieldtype.cpp 42116 2018-12-21 18:26:17Z asztalos $
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
#include <util/static_set.hpp>
#include <util/static_map.hpp>
#include <objtools/edit/field_handler.hpp>
#include <gui/widgets/edit/molinfo_fieldtype.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

vector<string> CMolInfoFieldType::GetFieldNames()
{
    vector<string> rval;
    for (int i = e_MoleculeType; i < e_Repr; i++) {
        rval.push_back(GetFieldName((EMolInfoFieldType)i));
    }
    return rval;
}

vector<string> CMolInfoFieldType::GetMacroFieldNames()
{
    vector<string> rval;
    for (int i = e_MoleculeType; i < e_Repr; i++) {
        rval.push_back(GetMacroFieldName((EMolInfoFieldType)i));
    }
    return rval;
}


string CMolInfoFieldType::GetFieldName(EMolInfoFieldType field_type)
{
    string rval = "";
    switch (field_type) {
    case e_MoleculeType:
        rval = "molecule";
        break;
    case e_Technique:
        rval = "technique";
        break;
    case e_Completedness:
        rval = "completedness";
        break;
    case e_Class:
        rval = "class";
        break;
    case e_Topology:
        rval = "topology";
        break;
    case e_Strand:
        rval = "strand";
        break;
    case e_Repr:
        rval = "representation";
    default:
        break;
    }
    return rval;
}

string CMolInfoFieldType::GetMacroFieldName(EMolInfoFieldType field_type)
{
    string rval;
    switch (field_type) {
    case e_MoleculeType:
        rval = "\"descr..molinfo.biomol\"";
        break;
    case e_Technique:
        rval = "\"descr..molinfo.tech\"";
        break;
    case e_Completedness:
        rval = "\"descr..molinfo.completeness\"";
        break;
    case e_Class:
        rval = "\"inst.mol\"";
        break;
    case e_Topology:
        rval = "\"inst.topology\"";
        break;
    case e_Strand:
        rval = "\"inst.strand\"";
        break;
    case e_Repr:
        rval = "\"inst.repr\"";
        break;
    default:
        break;
    }
    return rval;
}

CMolInfoFieldType::EMolInfoFieldType CMolInfoFieldType::GetFieldType(const string& field_name)
{
    for (int i = e_MoleculeType; i < e_Unknown; i++) {
        string match = GetFieldName((EMolInfoFieldType)i);
        if (NStr::EqualNocase(field_name, match)) {
            return (EMolInfoFieldType)i;
        }
    }
    return e_Unknown;
}

// For MolInfo editing
static const SStaticPair<const char*, CMolInfo::TBiomol> s_map_biomol_names[] =
{
    { " ", CMolInfo::eBiomol_unknown },
    { "cRNA", CMolInfo::eBiomol_cRNA },
    { "genomic", CMolInfo::eBiomol_genomic },
    { "genomic mRNA", CMolInfo::eBiomol_genomic_mRNA },
    { "mRNA", CMolInfo::eBiomol_mRNA },
    { "ncRNA", CMolInfo::eBiomol_ncRNA },
    { "other", CMolInfo::eBiomol_other },
    { "other-genetic", CMolInfo::eBiomol_other_genetic },
    { "peptide", CMolInfo::eBiomol_peptide },
    { "precursor RNA", CMolInfo::eBiomol_pre_RNA },
    { "rRNA", CMolInfo::eBiomol_rRNA },
    { "scRNA", CMolInfo::eBiomol_scRNA },
    { "snRNA", CMolInfo::eBiomol_snRNA },
    { "snoRNA", CMolInfo::eBiomol_snoRNA },
    { "tRNA", CMolInfo::eBiomol_tRNA },
    { "tmRNA", CMolInfo::eBiomol_tmRNA },
    { "transcribed_RNA", CMolInfo::eBiomol_transcribed_RNA }
};

typedef CStaticPairArrayMap<const char*, CMolInfo::TBiomol, PCase_CStr> TCStringBiomolPairsMap;
DEFINE_STATIC_ARRAY_MAP(TCStringBiomolPairsMap, k_biomol_names, s_map_biomol_names);

string CMolInfoFieldType::GetBiomolLabel(CMolInfo::TBiomol biomol)
{
    string rval = "";
    ITERATE(TCStringBiomolPairsMap, it, k_biomol_names) {
        if (it->second == biomol) {
            rval = it->first;
            break;
        }
    }
    return rval;
}


CMolInfo::TBiomol CMolInfoFieldType::GetBiomolFromLabel(const string& val)
{
    CMolInfo::TBiomol biomol = CMolInfo::eBiomol_unknown;
    ITERATE(TCStringBiomolPairsMap, it, k_biomol_names) {
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(it->first, val)) {
            biomol = it->second;
            break;
        }
    }
    return biomol;
}


static const SStaticPair<const char*, CMolInfo::TTech> s_map_tech_names[] =
{
    { " ", CMolInfo::eTech_unknown },
    { "EST", CMolInfo::eTech_est },
    { "FLI_cDNA", CMolInfo::eTech_fli_cdna },
    { "HTC", CMolInfo::eTech_htc },
    { "HTGS 0", CMolInfo::eTech_htgs_0 },
    { "HTGS 1", CMolInfo::eTech_htgs_1 },
    { "HTGS 2", CMolInfo::eTech_htgs_2 },
    { "HTGS 3", CMolInfo::eTech_htgs_3 },
    { "STS", CMolInfo::eTech_sts },
    { "TSA", CMolInfo::eTech_tsa },
    { "WGS", CMolInfo::eTech_wgs },
    { "both", CMolInfo::eTech_both },
    { "composite-WGS-HTGS", CMolInfo::eTech_composite_wgs_htgs },
    { "concept-trans", CMolInfo::eTech_concept_trans },
    { "concept-trans-A", CMolInfo::eTech_concept_trans_a },
    { "derived", CMolInfo::eTech_derived },
    { "genetic map", CMolInfo::eTech_genemap },
    { "physical map", CMolInfo::eTech_physmap },
    { "seq-pept", CMolInfo::eTech_seq_pept },
    { "seq-pept-homol", CMolInfo::eTech_seq_pept_homol },
    { "seq-pept-overlap", CMolInfo::eTech_seq_pept_overlap },
    { "standard", CMolInfo::eTech_standard },
    { "survey", CMolInfo::eTech_survey }
};

typedef CStaticPairArrayMap<const char*, CMolInfo::TBiomol, PCase_CStr> TCStringTechPairsMap;
DEFINE_STATIC_ARRAY_MAP(TCStringTechPairsMap, k_tech_names, s_map_tech_names);


string CMolInfoFieldType::GetTechLabel(CMolInfo::TTech tech)
{
    string rval = "";
    ITERATE(TCStringTechPairsMap, it, k_tech_names) {
        if (it->second == tech) {
            rval = it->first;
            break;
        }
    }
    return rval;
}


CMolInfo::TTech CMolInfoFieldType::GetTechFromLabel(const string& val)
{
    CMolInfo::TTech tech = CMolInfo::eTech_unknown;
    ITERATE(TCStringTechPairsMap, it, k_tech_names) {
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(it->first, val)) {
            tech = it->second;
            break;
        }
    }
    if (tech == CMolInfo::eTech_unknown && !NStr::IsBlank(val)) {
        tech = CMolInfo::eTech_other;
    }
    return tech;
}


static const SStaticPair<const char*, CMolInfo::TCompleteness> s_map_completeness_names[] =
{
    { " ", CMolInfo::eCompleteness_unknown },
    { "complete", CMolInfo::eCompleteness_complete },
    { "has left", CMolInfo::eCompleteness_has_left },
    { "has right", CMolInfo::eCompleteness_has_right },
    { "no ends", CMolInfo::eCompleteness_no_ends },
    { "no left", CMolInfo::eCompleteness_no_left },
    { "no right", CMolInfo::eCompleteness_no_right },
    { "other", CMolInfo::eCompleteness_other },
    { "partial", CMolInfo::eCompleteness_partial }
};


typedef CStaticPairArrayMap<const char*, CMolInfo::TCompleteness, PCase_CStr> TCStringCompletenessPairsMap;
DEFINE_STATIC_ARRAY_MAP(TCStringCompletenessPairsMap, k_completeness_names, s_map_completeness_names);


string CMolInfoFieldType::GetCompletenessLabel(CMolInfo::TCompleteness val)
{
    string rval = "";
    ITERATE(TCStringCompletenessPairsMap, it, k_completeness_names) {
        if (it->second == val) {
            rval = it->first;
            break;
        }
    }
    return rval;
}


CMolInfo::TCompleteness CMolInfoFieldType::GetCompletenessFromLabel(const string& val)
{
    CMolInfo::TCompleteness rval = CMolInfo::eCompleteness_unknown;
    ITERATE(TCStringCompletenessPairsMap, it, k_completeness_names) {
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(it->first, val)) {
            rval = it->second;
            break;
        }
    }

    return rval;
}


static const SStaticPair<const char*, CSeq_inst::TMol> s_map_mol_names[] =
{
    { " ", CSeq_inst::eMol_not_set },
    { "DNA", CSeq_inst::eMol_dna },
    { "RNA", CSeq_inst::eMol_rna },
    { "nucleotide", CSeq_inst::eMol_na },
    { "other", CSeq_inst::eMol_other },
    { "protein", CSeq_inst::eMol_aa }
};


typedef CStaticPairArrayMap<const char*, CSeq_inst::TMol, PCase_CStr> TCStringMolPairsMap;
DEFINE_STATIC_ARRAY_MAP(TCStringMolPairsMap, k_mol_names, s_map_mol_names);


string CMolInfoFieldType::GetMolLabel(CSeq_inst::TMol val)
{
    string rval = "";
    ITERATE(TCStringMolPairsMap, it, k_mol_names) {
        if (it->second == val) {
            rval = it->first;
            break;
        }
    }
    return rval;
}


CSeq_inst::TMol CMolInfoFieldType::GetMolFromLabel(const string& val)
{
    CSeq_inst::TMol rval = CSeq_inst::eMol_not_set;
    ITERATE(TCStringMolPairsMap, it, k_mol_names) {
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(it->first, val)) {
            rval = it->second;
            break;
        }
    }

    return rval;
}


static const SStaticPair<const char*, CSeq_inst::TTopology> s_map_topology_names[] =
{
    { " ", CSeq_inst::eTopology_not_set },
    { "circular", CSeq_inst::eTopology_circular },
    { "linear", CSeq_inst::eTopology_linear },
    { "other", CSeq_inst::eTopology_other },
    { "tandem", CSeq_inst::eTopology_tandem }
};


typedef CStaticPairArrayMap<const char*, CSeq_inst::TTopology, PCase_CStr> TCStringTopologyPairsMap;
DEFINE_STATIC_ARRAY_MAP(TCStringTopologyPairsMap, k_topology_names, s_map_topology_names);


string CMolInfoFieldType::GetTopologyLabel(CSeq_inst::TTopology val)
{
    string rval = "";
    ITERATE(TCStringTopologyPairsMap, it, k_topology_names) {
        if (it->second == val) {
            rval = it->first;
            break;
        }
    }
    return rval;
}


CSeq_inst::TTopology CMolInfoFieldType::GetTopologyFromLabel(const string& val)
{
    CSeq_inst::TTopology rval = CSeq_inst::eTopology_not_set;
    ITERATE(TCStringTopologyPairsMap, it, k_topology_names) {
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(it->first, val)) {
            rval = it->second;
            break;
        }
    }

    return rval;
}


static const SStaticPair<const char*, CSeq_inst::TStrand> s_map_strand_names[] =
{
    { " ", CSeq_inst::eStrand_not_set },
    { "double", CSeq_inst::eStrand_ds },
    { "mixed", CSeq_inst::eStrand_mixed },
    { "other", CSeq_inst::eStrand_other },
    { "single", CSeq_inst::eStrand_ss }
};


typedef CStaticPairArrayMap<const char*, CSeq_inst::TStrand, PCase_CStr> TCStringStrandPairsMap;
DEFINE_STATIC_ARRAY_MAP(TCStringStrandPairsMap, k_strand_names, s_map_strand_names);


string CMolInfoFieldType::GetStrandLabel(CSeq_inst::TStrand val)
{
    string rval = "";
    ITERATE(TCStringStrandPairsMap, it, k_strand_names) {
        if (it->second == val) {
            rval = it->first;
            break;
        }
    }
    return rval;
}


CSeq_inst::TStrand CMolInfoFieldType::GetStrandFromLabel(const string& val)
{
    CSeq_inst::TStrand rval = CSeq_inst::eStrand_not_set;
    ITERATE(TCStringStrandPairsMap, it, k_strand_names) {
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(it->first, val)) {
            rval = it->second;
            break;
        }
    }

    return rval;
}

static const SStaticPair<const char*, CSeq_inst::TRepr> s_map_repr_names[] =
{
    { " ", CSeq_inst::eRepr_not_set },
    { "consensus", CSeq_inst::eRepr_consen },
    { "constructed", CSeq_inst::eRepr_const },
    { "delta", CSeq_inst::eRepr_delta },
    { "ordered map", CSeq_inst::eRepr_map },
    { "other", CSeq_inst::eRepr_other },
    { "raw", CSeq_inst::eRepr_raw },
    { "reference to another", CSeq_inst::eRepr_ref },
    { "segmented", CSeq_inst::eRepr_seg },
    { "virtual", CSeq_inst::eRepr_virtual }
};

typedef CStaticPairArrayMap<const char*, CSeq_inst::TRepr, PCase_CStr> TCStringReprPairsMap;
DEFINE_STATIC_ARRAY_MAP(TCStringReprPairsMap, k_repr_names, s_map_repr_names);

string CMolInfoFieldType::GetReprLabel(CSeq_inst::TRepr repr)
{
    string rval;
    for (auto& it: k_repr_names) {
        if (it.second == repr) {
            rval = it.first;
            break;
        }
    }
    return rval;
}


CSeq_inst::TRepr CMolInfoFieldType::GetReprFromLabel(const string& val)
{
    CSeq_inst::TRepr repr = CSeq_inst::eRepr_not_set;
    for (auto& it : k_repr_names) {
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(it.first, val)) {
            repr = it.second;
            break;
        }
    }
    return repr;
}


vector<string> CMolInfoFieldType::GetChoicesForField(EMolInfoFieldType field_type, bool& allow_other)
{
    vector<string> choices;

    switch (field_type) {
    case CMolInfoFieldType::e_MoleculeType:
        ITERATE(TCStringBiomolPairsMap, it, k_biomol_names) {
            choices.push_back(it->first);
        }
        allow_other = false;
        break;
    case CMolInfoFieldType::e_Technique:
        ITERATE(TCStringTechPairsMap, it, k_tech_names) {
            choices.push_back(it->first);
        }
        allow_other = true;
        break;
    case CMolInfoFieldType::e_Completedness:
        ITERATE(TCStringCompletenessPairsMap, it, k_completeness_names) {
            choices.push_back(it->first);
        }
        allow_other = false;
        break;
    case CMolInfoFieldType::e_Class:
        ITERATE(TCStringMolPairsMap, it, k_mol_names) {
            choices.push_back(it->first);
        }
        allow_other = false;
        break;
    case CMolInfoFieldType::e_Topology:
        ITERATE(TCStringTopologyPairsMap, it, k_topology_names) {
            choices.push_back(it->first);
        }
        break;
    case CMolInfoFieldType::e_Strand:
        ITERATE(TCStringStrandPairsMap, it, k_strand_names) {
            choices.push_back(it->first);
        }
        allow_other = false;
        break;
    case CMolInfoFieldType::e_Repr:
        for (auto& it : k_repr_names) {
            choices.push_back(it.first);
        }
        allow_other = false;
        break;
    default:
        break;
    }
    return choices;
}

vector<string> CMolInfoFieldType::GetValuesForField(EMolInfoFieldType field_type)
{
    vector<string> values;
    switch (field_type) {
    case CMolInfoFieldType::e_MoleculeType:
    {
    values.push_back("unknown");
    values.push_back("cRNA");
    values.push_back("genomic");
    values.push_back("genomic-mRNA");
    values.push_back("mRNA");
    values.push_back("ncRNA");
    values.push_back("other");
    values.push_back("other-genetic");
    values.push_back("peptide");
    values.push_back("pre-RNA");
    values.push_back("rRNA");
    values.push_back("scRNA");
    values.push_back("snRNA");
    values.push_back("snoRNA");
    values.push_back("tRNA");
    values.push_back("tmRNA");
    values.push_back("transcribed-RNA");
    }
        break;
    case CMolInfoFieldType::e_Technique:
    {
    values.push_back("unknown");
    values.push_back("est");
    values.push_back("fli-cdna");
    values.push_back("htc");
    values.push_back("htgs-0");
    values.push_back("htgs-1");
    values.push_back("htgs-2");
    values.push_back("htgs-3");
    values.push_back("sts");
    values.push_back("tsa");
    values.push_back("wgs");
    values.push_back("both");
    values.push_back("composite-wgs-htgs");
    values.push_back("concept-trans");
    values.push_back("concept-trans-a");
    values.push_back("derived");
    values.push_back("genemap");
    values.push_back("physmap");
    values.push_back("seq-pept");
    values.push_back("seq-pept-homol");
    values.push_back("seq-pept-overlap");
    values.push_back("standard");
    values.push_back("survey");
    }
        break;
    case CMolInfoFieldType::e_Completedness:
    {
    values.push_back("unknown");
    values.push_back("complete");
    values.push_back("has-left");
    values.push_back("has-right");
    values.push_back("no-ends");
    values.push_back("no-left");
    values.push_back("no-right");
    values.push_back("other");
    values.push_back("partial");
    }
        break;
    case CMolInfoFieldType::e_Class:
    {
    values.push_back("not-set");
    values.push_back("dna");
    values.push_back("rna");
    values.push_back("na");
    values.push_back("other");
    values.push_back("aa");
    }
        break;
    case CMolInfoFieldType::e_Topology:
    {
    values.push_back("not-set");
    values.push_back("circular");
    values.push_back("linear");
    values.push_back("other");
    values.push_back("tandem");
    }
        break;
    case CMolInfoFieldType::e_Strand:
    {
    values.push_back("not-set");
    values.push_back("ds");
    values.push_back("mixed");
    values.push_back("other");
    values.push_back("ss");
    }
        break;
    case CMolInfoFieldType::e_Repr:
    {
    values.push_back("not-set");
    values.push_back("virtual");
    values.push_back("raw");
    values.push_back("seg");
    values.push_back("const");
    values.push_back("ref");
    values.push_back("consen");
    values.push_back("map");
    values.push_back("delta");
    values.push_back("other");
    }
        break;
    default:
        break;
    }

    return values;
}

END_NCBI_SCOPE

