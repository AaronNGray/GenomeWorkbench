/*  $Id: gui_object_info_variant.cpp 44721 2020-02-28 17:28:24Z rudnev $
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
* Authors: Liangshou Wu
*
* File Description:
*     Gui Object Info class specifically for structural variations from dbVar.
*
*/

#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Variation_inst.hpp>
#include <objects/seqfeat/Delta_item.hpp>
#include <objects/seqfeat/VariantProperties.hpp>
#include <objects/seqfeat/Phenotype.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objtools/snputil/snp_utils.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/snp_gui.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CGuiObjectInfoVariant : public CGuiObjectInfoSeq_feat
{
public:
    virtual string GetSubtype() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const;
};


string CGuiObjectInfoVariant::GetSubtype() const
{
    return "Structural variant";
}


static int s_GetChildNum(const objects::CSeq_feat& feat)
{
    string num_str = feat.GetNamedQual("Child Count");
    if ( !num_str.empty() ) {
        try {
            return NStr::StringToInt(num_str);
        } catch (CException&) {
            // ingore it
        }
    }
    return -1;
}


void CGuiObjectInfoVariant::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }

    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());

    const CVariation_ref& var = m_Feat->GetData().GetVariation();
    bool is_sv = !var.IsSetId()  ||  (var.GetId().GetTag().IsStr()  &&
            var.GetId().GetTag().GetStr().find("sv") == 1);

    // variations that came from CVcfReader do not follow some common conventions, so sometimes they need to be 
    // handled in a special way
    bool is_vcf(NSnpGui::isFromVcf(*m_Feat));
    string validation = is_vcf ? "" : "Not Tested";
    string additional_info = kEmptyStr;

    if (var.IsSetId()) {
        string id_text;
        // avoid "local" dbtag when data came from CVcfReader
        if(is_vcf && var.GetId().GetDb() == "local") {
            id_text = var.GetId().GetTag().GetStr();
        } else {
            var.GetId().GetLabel(&id_text);
        }
        tooltip.AddRow("Variation ID:", id_text);
    }

    if (var.IsSetData() && var.GetData().IsSet() && var.GetData().GetSet().IsSetVariations()) {
        list<string> alt_alleles;
        string ref_allele;
        int allele_len = -1;
        bool all_alt_alleles_same_len(true);
        auto var_list = var.GetData().GetSet().GetVariations();
        for (auto iVariation = var_list.begin(); iVariation != var_list.end(); ++iVariation) {
            if ((*iVariation)->IsSetData() && (*iVariation)->GetData().IsInstance()) {
                const CVariation_ref::TData::TInstance& VarInst((*iVariation)->GetData().GetInstance());
                if (!VarInst.CanGetDelta())
                    continue;

                bool isReference(false);
                if(VarInst.CanGetObservation() && (VarInst.GetObservation() & CVariation_inst::eObservation_reference)) {
                    isReference = true;
                }

                ITERATE(CVariation_ref::TData::TInstance::TDelta, iDelta, VarInst.GetDelta()) {
                    if ((*iDelta)->CanGetSeq()) {
                        const CDelta_item::C_Seq& DeltaSeq((*iDelta)->GetSeq());
                        switch (DeltaSeq.Which()) {
                        case CDelta_item::C_Seq::e_Literal:
                        {
                            if (DeltaSeq.GetLiteral().CanGetSeq_data()) {
                                const CSeq_data& Seq_data(DeltaSeq.GetLiteral().GetSeq_data());
                                // variations normally use Iupacna/Iupacaa
                                string sAllele;
                                if (Seq_data.IsIupacna())
                                    sAllele = Seq_data.GetIupacna().Get().empty() ? "-" : Seq_data.GetIupacna().Get();
                                if (Seq_data.IsIupacaa())
                                    sAllele = Seq_data.GetIupacaa().Get().empty() ? "-" : Seq_data.GetIupacaa().Get();

                                if (!sAllele.empty()) {
                                    if(isReference) {
                                        ref_allele = sAllele;
                                    } else {
                                        alt_alleles.push_back(sAllele);
                                        int new_allele_len(max(allele_len, (int)DeltaSeq.GetLiteral().GetLength()));
                                        if(allele_len != -1 && new_allele_len != allele_len) {
                                            all_alt_alleles_same_len = false;
                                        }
                                        allele_len = new_allele_len;
                                    }
                                }
                            }
                            break;
                        }
                        case CDelta_item::C_Seq::e_This:
                        // this can be a deletion
                        if(VarInst.GetType() == CVariation_inst::eType_del) {
                            if(!isReference) {
                                alt_alleles.push_back("-");
                            }
                        }
                        default:
                            // no specific processing for other deltas
                            break;
                        }
                    }
                }
            }
        }
        tooltip.AddRow("Alleles" + string(alt_alleles.size() > 1 ? "s" : "") + ":", string(ref_allele.empty() ? "-" : ref_allele) + "/" + NStr::Join(alt_alleles, "/"));
        if (allele_len > 0) {
            tooltip.AddRow(string(all_alt_alleles_same_len ? "A" : "Longest a") + "llele length:", NStr::NumericToString(allele_len, NStr::fWithCommas));
        }
    }

    if (var.IsSetValidated()) {
        if (var.GetValidated()) {
            validation = "Yes";
        } else {
            validation = "Fail";
        }
    } else if (var.IsSetVariant_prop()  &&  var.GetVariant_prop().IsSetOther_validation()) {
        if (var.GetVariant_prop().GetOther_validation()) {
            validation = "Yes";
        } else {
            validation = "Fail";
        }
    } else if (m_Feat->IsSetExts()) {
        const CSeq_feat::TExts& exts = m_Feat->GetExts();
        ITERATE (CSeq_feat::TExts, iter, exts) {
            if ( (*iter)->GetType().IsStr()  &&
                NStr::EqualNocase((*iter)->GetType().GetStr(), "Validation")  &&
                 (*iter)->GetFieldRef("Status")  &&
                 (*iter)->GetFieldRef("Status")->GetData().IsStr()) {
                     validation = (*iter)->GetFieldRef("Status")->GetData().GetStr();
                     CConstRef<CUser_field> a_field = (*iter)->GetFieldRef("Addition");
                     if (a_field) {
                        additional_info = a_field->GetData().GetStr();
                     }
                     break;
            }
        }
    }

    // phenotype and clinical-significance
    string phenotype = "";
    string clinical_sig = "";
    if (var.CanGetPhenotype()) {
        ITERATE (CVariation_ref::TPhenotype, pnt_iter, var.GetPhenotype()) {
            if (clinical_sig.empty()  &&  (*pnt_iter)->CanGetClinical_significance()) {
                clinical_sig = NSnp::ClinSigAsString((*pnt_iter)->GetClinical_significance());
            }

            if (phenotype.empty()  &&  (*pnt_iter)->CanGetTerm()) {
                phenotype = (*pnt_iter)->GetTerm();
            }
        }
    }

    string sample_sex = "";
    if (var.CanGetSomatic_origin()) {
        ITERATE (CVariation_ref::TSomatic_origin, so_iter, var.GetSomatic_origin()) {
            if ((*so_iter)->CanGetSource()  &&
                (*so_iter)->GetSource().GetSubtype() == CSubSource::eSubtype_sex) {
                    sample_sex = (*so_iter)->GetSource().GetName();
                    break;
            }
        }
    }

    // allele type
    string variant_type("Unknown");
    int multiplier = -1;
    if(!is_vcf) {
        if (var.IsComplex()) {
            variant_type = "Complex";
        } else if (var.IsInsertion()) {
            variant_type = "Insertion";
        } else if (var.IsInversion()) {
            variant_type = "Inversion";
        } else if (var.IsEversion()) {
            variant_type = "Tandem duplication";
        } else if (var.IsTranslocation()) {
            variant_type = "Translocation";
        } else if (var.IsDeletion()) {
            variant_type = "Deletion";
        } else if (var.IsDeletionInsertion()) {
            variant_type = "Indel";
        } else if (var.IsSNV()) {
            variant_type = "Single nucleotide variant";
        } else if (var.IsCNV()) {
            if (var.GetData().GetInstance().IsSetDelta()) {
                const CVariation_inst::TDelta& delta = var.GetData().GetInstance().GetDelta();
                ITERATE (CVariation_inst::TDelta, iter, delta) {
                    if ((*iter)->IsSetMultiplier()) {
                        multiplier = (*iter)->GetMultiplier();
                        break;
                    }
                }
            }
            if (is_sv) {
                variant_type = "Copy number variation";
            } else {
                if (var.IsGain()) {
                    variant_type = "Copy number gain";
                } else if (var.IsLoss()) {
                    variant_type = "Copy number loss";
                } else {
                    variant_type = "Copy number variation";
                }
            }
        }
    } else {
        switch(NSnpGui::GetVcfType(*m_Feat)) {
            case CVariation_inst::eType_snv:
                variant_type = "SNV";
                break;
            case CVariation_inst::eType_mnp:
                variant_type = "MNP";
                break;
            case CVariation_inst::eType_ins:
                variant_type = "Insertion";
                break;
            case CVariation_inst::eType_del:
                variant_type = "Deletion";
                break;
            case CVariation_inst::eType_delins:
                variant_type = "Delins";
                break;
        }
    }
    if (is_sv) {
        int child_num = s_GetChildNum(*m_Feat);
        if (child_num == 0) {
            // sv, but treated as ssv
            is_sv = false;
        } else {
            if (child_num > 0) {
                tooltip.AddRow("Supporting Variant Calls:", NStr::IntToString(child_num));
            }
            tooltip.AddRow("Variant Region Type:", variant_type);
            if (multiplier > -1) {
                tooltip.AddRow("Copy Number:", NStr::NumericToString(multiplier));
            }
            if ( !phenotype.empty() ) {
                tooltip.AddRow("Phenotype:", phenotype);
            }
            if ( !clinical_sig.empty() ) {
                tooltip.AddRow("Clinical Interpretation:", clinical_sig);
            }
            if ( !sample_sex.empty() ) {
                tooltip.AddRow("Gender:", sample_sex);
            }
            tooltip.AddRow("Validation Status:", validation);
            if ( !additional_info.empty() ) {
                tooltip.AddRow("Additional Info:", additional_info);
            }
            if (m_Feat->IsSetComment()) {
                tooltip.AddRow("Comment:", m_Feat->GetComment());
            }

            if (m_Location) {
                tooltip.AddRow("Total Length:", NStr::IntToString(m_Location->GetTotalRange().GetLength(), NStr::fWithCommas));
            }
        }
    }

    if ( !is_sv ) {
        if (var.CanGetParent_id()) {
            string parent;
            if (var.GetParent_id().GetTag().IsId()) {
                parent = NStr::IntToString(var.GetParent_id().GetTag().GetId());
            } else {
                parent = var.GetParent_id().GetTag().GetStr();
            }
            tooltip.AddRow("Parent Variant Region:", parent);
        }

        tooltip.AddRow("Variant Call Type:", variant_type);
        if (multiplier > -1) {
            tooltip.AddRow("Copy Number:", NStr::NumericToString(multiplier));
        }
        if ( !phenotype.empty() ) {
            tooltip.AddRow("Phenotype:", phenotype);
        }
        if ( !clinical_sig.empty() ) {
            tooltip.AddRow("Clinical Interpretation:", clinical_sig);
        }
        if ( !sample_sex.empty() ) {
            tooltip.AddRow("Gender:", sample_sex);
        }

        // inheritance
        CVariation_ref::TAllele_origin org = -1;
        if (var.IsSetAllele_origin()) {
            org = var.GetAllele_origin();
        } else if (var.IsSetVariant_prop()  &&  var.GetVariant_prop().IsSetAllele_origin()) {
            org = var.GetVariant_prop().GetAllele_origin();
        }

        if (org > 0) {
            string inh;
            if (org & CVariation_ref::eAllele_origin_somatic) {
                inh += "somatic ";
            }
            if (org & CVariation_ref::eAllele_origin_inherited) {
                inh += "inherited ";
            }
            if (org & CVariation_ref::eAllele_origin_paternal) {
                inh += "paternal ";
            }
            if (org & CVariation_ref::eAllele_origin_maternal) {
                inh += "maternal ";
            }
            if (org & CVariation_ref::eAllele_origin_de_novo) {
                inh += "de_novo ";
            }
            if (org & CVariation_ref::eAllele_origin_biparental) {
                inh += "biparental ";
            }
            if (org & CVariation_ref::eAllele_origin_uniparental) {
                inh += "uniparental ";
            }
            if (org & CVariation_ref::eAllele_origin_not_tested) {
                inh += "not-tested ";
            }
            if (org & CVariation_ref::eAllele_origin_tested_inconclusive) {
                inh += "tested-inconclusive ";
            }
            if (org & CVariation_ref::eAllele_origin_other) {
                inh += "other";
            }

            if (org == CVariation_ref::eAllele_origin_unknown) {
                inh = "unknown";
            }

            tooltip.AddRow("Inheritance:", inh);
        }

        // validation status
        // do not report empty validation status for variations obtained from CVcfReader
        if(!validation.empty()) {
            tooltip.AddRow("Validation Status:", validation);
        }
        if ( !additional_info.empty() ) {
            tooltip.AddRow("Additional Info:", additional_info);
        }

        if (m_Feat->CanGetComment()) {
            tooltip.AddRow("Comment:", m_Feat->GetComment());
        }
    }

    int allele_len = -1;
    if (var.GetData().IsInstance()  &&  var.GetData().GetInstance().IsSetDelta()) {
        const CVariation_inst::TDelta& delta = var.GetData().GetInstance().GetDelta();
        ITERATE (CVariation_inst::TDelta, iter, delta) {
            if ((*iter)->IsSetSeq()  &&  (*iter)->GetSeq().IsLiteral()) {
                allele_len = (*iter)->GetSeq().GetLiteral().GetLength();
                break;
            }
        }
    }
    if (allele_len > -1) {
        tooltip.AddRow("Allele Length:", NStr::NumericToString(allele_len, NStr::fWithCommas));
    }
    if (var.CanGetSample_id()) {
        string sample_id;
        const CVariation_ref::TSample_id& sid = var.GetSample_id();
        if (sid.IsStr()) {
            sample_id = sid.GetStr();
        } else {
            sample_id = NStr::NumericToString(sid.GetId());
        }
        if ( !sample_id.empty() ) {
            tooltip.AddRow("Sample id:", sample_id);
        }
    }
}

static const string kBaseUrl = "http://ncbi.nlm.nih.gov/dbvar/";

void CGuiObjectInfoVariant::GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const
{
    // maps link names to link urls and labels (allows links with same name to be consolidated)
    // (I don't see for dbvar that links generally require this, but it was an issue in
    // CGuiObjectInfoSeq_feat
    map<string, vector<pair<string, string> > > link_map;
    CIRef<ITooltipFormatter> tmp_links = links.CreateInstance();

    const CVariation_ref& var = m_Feat->GetData().GetVariation();
    string label, name, url;
    if (var.CanGetId()  &&  var.GetId().GetTag().IsStr()  &&
        var.GetId().GetTag().GetStr().find("sv") == 1) {
            CLabel::GetLabel(*m_Feat, &label, CLabel::eContent, m_Scope.GetPointer());
    } else if (var.CanGetParent_id()) {
        if (var.GetParent_id().GetTag().IsId()) {
            label = NStr::IntToString(var.GetParent_id().GetTag().GetId());
        } else {
            label = var.GetParent_id().GetTag().GetStr();
        }
    }

    if ( !label.empty() ) {
        name = "dbVar";
        url = kBaseUrl + "variants/" + label;
        tmp_links->AddLinkRow("dbVar", label, url);
    }

    if (m_Feat->IsSetExts()) {
        ITERATE (CSeq_feat::TExts, iter, m_Feat->GetExts()) {
            if ((*iter)->GetType().IsStr()  &&
                !(*iter)->GetData().empty()) {
                    name = "Other Variant Calls from this Sample";
                    if ((*iter)->GetType().GetStr() == "related calls") {
                        const CUser_object::TData& fields = (*iter)->GetData();
                        if (fields.size() < 5) {
                            ITERATE (CUser_object::TData, f_iter, fields) {
                                label = (*f_iter)->GetLabel().GetStr();
                                url = kBaseUrl + "variants/" + (*f_iter)->GetData().GetStr();
                                //table_rows += CSeqUtils::CreateLinkRow(name, label, url);
                                link_map[name].push_back(std::pair<string, string>(label, url));
                            }
                        } else if (var.CanGetSample_id()) {
                            string term;
                            if (var.GetSample_id().IsStr()) {
                                term = var.GetSample_id().GetStr();
                            } else {
                                term = NStr::NumericToString(var.GetSample_id().GetId());
                            }
                            CConstRef<CDbtag> study_id = m_Feat->GetNamedDbxref("study_accession");
                            if (study_id) {
                                term += " and " + study_id->GetTag().GetStr();
                            }

                            label = NStr::NumericToString(fields.size());
                            url = kBaseUrl + "?term=" + term;
                            //table_rows += CSeqUtils::CreateLinkRow(name, label, url);
                            link_map[name].push_back(std::pair<string, string>(label, url));
                        }
                        break;
                    } else if ((*iter)->GetType().GetStr() == "related call count") {
                        string term;
                        ITERATE (CUser_object::TData, f_iter, (*iter)->GetData()) {
                            if ((*f_iter)->GetLabel().IsStr()  &&  (*f_iter)->GetData().IsStr()) {
                                label = (*f_iter)->GetLabel().GetStr();
                                term = (*f_iter)->GetData().GetStr();
                                break;
                            }
                        }

                        if ( !term.empty() ) {
                            CConstRef<CDbtag> study_id = m_Feat->GetNamedDbxref("study_accession");
                            if (study_id) {
                                term += " and " + study_id->GetTag().GetStr();
                            }
                            url = kBaseUrl + "?term=" + term;
                            //table_rows += CSeqUtils::CreateLinkRow(name, label, url);
                            link_map[name].push_back(std::pair<string, string>(label, url));
                        }
                        break;
                    }
            }
        }
    }

    // Create links, consolidating links with same name into a single row
    map<string, vector<pair<string, string> > >::iterator iter;
    for (iter=link_map.begin(); iter != link_map.end(); ++iter) {
        string table_rows;

        for (size_t i=0; i<(*iter).second.size(); ++i) {
            // This gives a separate row to each link:
            //tmp_links += CSeqUtils::CreateLinkRow((*iter).first, (*uiter).first, (*uiter).second);
            if (i>0) table_rows += ",&nbsp;";
            table_rows += tmp_links->CreateLink((*iter).second[i].first, (*iter).second[i].second);
        }
        tmp_links->AddLinkRow((*iter).first, table_rows, 250);
    }

    if ( !tmp_links->IsEmpty()) {
        links.AddLinksTitle("Go To");
        links.Append(*tmp_links);
    }
}


END_NCBI_SCOPE
