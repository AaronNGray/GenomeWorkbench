/*  $Id: snp_gui.cpp 44708 2020-02-27 16:13:12Z rudnev $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbiapp.hpp>
#include <misc/xmlwrapp/xmlwrapp.hpp>
#include <misc/xmlwrapp/event_parser.hpp>
#include <objects/variation/Variation.hpp>
#include <objects/variation/VariantPlacement.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/seqfeat/VariantProperties.hpp>
#include <objtools/snputil/snp_utils.hpp>
#include <objtools/snputil/snp_bins.hpp>
#include <gui/objutils/snp_gui.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/utils/fetch_url.hpp>
#include <misc/jsonwrapp/jsonwrapp.hpp>

#include <cmath>
#include <sstream>

#include <objects/variation_libs/dbsnp/search_by_rsid/Placement.hpp>
#include <objects/variation_libs/dbsnp/search_by_rsid/SearchByRsIdReply.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


const string NSnpGui::pmURL = "/pubmed/";
const string NSnpGui::omimURL = "/omim/";
const string NSnpGui::geneSymURL = "/gene/";
const string NSnpGui::geneRevURL = "/sites/entrez?Db=books&DbFrom=gene&Cmd=Link&LinkName=gene_books&LinkReadableName=Books&IdsFromResult=";
const string NSnpGui::snpURL = "/snp/rs";
const string NSnpGui::phenotypeURL = "/sites/entrez?db=mesh&cmd=search&term=";
const string NSnpGui::GAPStudyURL = "/projects/gap/cgi-bin/study.cgi?study_id=";
const string NSnpGui::PHAURL = "/projects/SNP/GaPBrowser_prod/callGaPBrowser2.cgi?aid=";
const string NSnpGui::GenomeURL = "https://www.genome.gov/gwastudies/index.cfm?snp=rs";
const string NSnpGui::VarVuURL = "/variation/view/?q=rs";
const string NSnpGui::ClinVarURL = "/clinvar/?term=rs";
string NSNPWebServices::m_sSearchHost;
bool NSNPWebServices::m_isSearchByRsid{false};

const int NSnpGui::c_BinWidth = 14;
const int NSnpGui::c_BinHeight = 14;


// coloring for clinical significance
/*
CRgbaColor NSnpGui::ClinSigAsColor(NSnp::TClinSigID ClinSigID)
{
    switch(ClinSigID)
    {
		case CPhenotype::eClinical_significance_probable_pathogenic:
			return CRgbaColor(0.57f, 0.43f, 0.85f, 1.0f); // light purple
		case CPhenotype::eClinical_significance_pathogenic:
			return CRgbaColor(0.5f, 0.0f, 0.5f, 1.0f); // purple
    }

  return CRgbaColor(.75f, 1.0f, .75f);    // light green
}
*/

string NSnpGui::ClinSigAsColorKey(NSnp::TClinSigID ClinSigID)
{
    switch(ClinSigID)
    {
		case CPhenotype::eClinical_significance_probable_pathogenic:
			return "ProbablePathogenic";
		case CPhenotype::eClinical_significance_pathogenic:
			return "Pathogenic";
    }
    return "Default";
}

string NSnpGui::ClinSigAsImgURL(NSnp::TClinSigID ClinSigID, const string &colorTheme)
{
    const string sBaseURL("/projects/sviewer/images/");
    switch(ClinSigID)
    {
		case CPhenotype::eClinical_significance_probable_pathogenic:
			return sBaseURL + "snp-patho-prob-patho_" + colorTheme + ".png";
		case CPhenotype::eClinical_significance_pathogenic:
			return sBaseURL + "snp-patho-patho_" + colorTheme + ".png";
    }
	return sBaseURL + "snp-patho-def_" + colorTheme + ".png";
}


string NSnpGui::PValueAsColorKey(double pvalue)
{
    static string ColorKeys[] = {
        "PValueLevel1",   // < 2
        "PValueLevel2",   // [2-3)
        "PValueLevel3",   // [3-4)
        "PValueLevel4",   // [4-5)
        "PValueLevel5",	  // [5-6)
        "PValueLevel6",   // [6-7)
        "PValueLevel7",	  // >= 7
    };

    int v = (int)floor(pvalue);

    if( v < 2)
        return ColorKeys[0];
    else if (v < 7)
        return ColorKeys[v-1];
    else
        return ColorKeys[6];
}

void NSnpGui::GetBinTooltip(const NSnpBins::SBin& bin,
							  CIRef<ITooltipFormatter> pFormatter,
							  TGi gi,
                              const string &colorTheme)
{
    vector<string> tokens;

    switch(bin.type)
    {
        case NSnpBins::eCLIN:
        case NSnpBins::eIND:
        {
            ITERATE(NSnpBins::TBinEntryList, iter, bin.m_EntryList) {
                CConstRef<NSnpBins::SBinEntry> entry(iter->GetPointer());
                string rsid = NStr::UIntToString(entry->snpid);
                if(iter != bin.m_EntryList.begin()) {
                    pFormatter->AddDividerRow();
                }

                pFormatter->StartRow();
                if(bin.type == NSnpBins::eCLIN) {
                    pFormatter->AddTagCol("Variation ID:", ClinSigAsImgURL(entry->ClinSigID, colorTheme));
                }
                else {
                    pFormatter->AddTagCol("Variation ID:");
                }
				string sClinSig(NSnp::ClinSigAsString(entry->ClinSigID, NSnp::eLetterCase_ForceLower));
                // "with ... allele" statement temporarily restored according to SNP-5006 09/17/12 12:16
                pFormatter->AddValueCol(pFormatter->CreateNcbiLink("rs"+rsid, snpURL+rsid) +
                                         (sClinSig.empty() ?  string() : ", with " + sClinSig + " allele"));
                pFormatter->FinishRow();
                pFormatter->StartRow();
                pFormatter->AddTagCol("Location:");
                pFormatter->AddValueCol(NStr::IntToString(entry->pos+1, NStr::fWithCommas));
                pFormatter->FinishRow();

                // parse the trait, extracting OMIM and Gene information
                string sPackagedOMIM, sGene;
                vector<string> tokens;
                NStr::Split(entry->trait, "|", tokens);
                if(tokens.size() > 0) {
                    sPackagedOMIM = tokens[0];
                }
                if(tokens.size() > 1) {
                    sGene = tokens[1];
                }
                tokens.clear();

/* temporarily disabled according to SNP-5006 09/17/12 12:16
                if(!snp->sHGVS.empty() || !sPackagedOMIM.empty()) {
                    pFormatter->StartRow();
                    pFormatter->AddTagCol("Phenotype:");
                    pFormatter->FinishRow();
                    if(!snp->sHGVS.empty()) {
                        pFormatter->StartRow();
                        pFormatter->AddTagCol("Allele:");
                        pFormatter->AddValueCol(snp->sHGVS);
                        pFormatter->FinishRow();
                    }
                    if(!sPackagedOMIM.empty()) {
                        tokens.clear();
                        NStr::Split(sPackagedOMIM, "^", tokens);
                        string sPhenotypeName;

                        for(size_t i=0; i<tokens.size(); ++i) {
                            vector<string> OmimPair;
                            NStr::Split(tokens[i], ":", OmimPair);
                            NCBI_ASSERT(OmimPair.size() == 2, "Invalid OMIM pair record");
                            vector<string> OmimIDPair;
                            NStr::Split(OmimPair[0], ".", OmimIDPair);
                            string sOmimLink(OmimIDPair.size() == 2 ? pFormatter->CreateNcbiLink("OMIM", omimURL+OmimIDPair[0] + "#" + OmimIDPair[0] + "Variants" + OmimIDPair[1]) : "");
                            if(i>0)
                                sPhenotypeName += ", ";
                            sPhenotypeName += (OmimPair.size() == 2 ? (OmimPair[1] + (sOmimLink.empty() ? sOmimLink : (" (" + sOmimLink + ")"))) : "");
                        }
                        if(!sPhenotypeName.empty()) {
                            string sClinSig(NSnp::ClinSigAsString(snp->ClinSigID, , NSnp::eLetterCase_ForceLower));
                            pFormatter->StartRow();
                            pFormatter->AddTagCol("Name:");
                            pFormatter->AddValueCol(sPhenotypeName + (sClinSig.empty() ?  string() : ", with " + sClinSig + " allele"));
                            pFormatter->FinishRow();
                        }
                    }
                }
*/
                bool isGoToPresent(false);
                x_MakeVarVuLink(entry, pFormatter, isGoToPresent, gi);
                x_MakeClinVarLink(entry, pFormatter, isGoToPresent);
                pFormatter->AddPubmedLinksRow(entry->pmids, isGoToPresent, true);
            }
            break;
        }
        case NSnpBins::eCITED:
        {
            ITERATE(NSnpBins::TBinEntryList, iter, bin.m_EntryList) {
                CConstRef<NSnpBins::SBinEntry> entry(iter->GetPointer());
                string rsid = NStr::UIntToString(entry->snpid);
                if(iter != bin.m_EntryList.begin())
                    pFormatter->AddDividerRow();

                pFormatter->StartRow();
                pFormatter->AddTagCol("Variation ID:");
                pFormatter->AddValueCol(pFormatter->CreateNcbiLink("rs"+rsid, snpURL+rsid));
                pFormatter->FinishRow();
                pFormatter->StartRow();
                pFormatter->AddTagCol("Location:");
                pFormatter->AddValueCol(NStr::IntToString(entry->pos+1));
                pFormatter->FinishRow();

                bool isGoToPresent(false);
                pFormatter->AddPubmedLinksRow(entry->pmids, isGoToPresent, false);
            }
            break;
        }
        case NSnpBins::eGAP:
        case NSnpBins::eGCAT:
        {
			ITERATE(NSnpBins::TBinEntryList, iter, bin.m_EntryList) {
				if(iter != bin.m_EntryList.begin())
					pFormatter->AddDividerRow();
				CConstRef<NSnpBins::SBinEntry> entry(iter->GetPointer() );
				if(entry->snpid > 0) {
                    string rsid = NStr::UIntToString(entry->snpid);
                    pFormatter->StartRow();
                    pFormatter->AddTagCol("Variation ID:");
                    pFormatter->AddValueCol(pFormatter->CreateNcbiLink("rs"+rsid, snpURL+rsid));
                    pFormatter->FinishRow();
                }
				if(entry->geneId > 0) {
                    string geneId = NStr::UIntToString(entry->geneId);
                    pFormatter->StartRow();
                    pFormatter->AddTagCol("Gene ID:");
                    if(!entry->geneName.empty()) {
                        geneId += " (" + entry->geneName + ")";
                    }
                    pFormatter->AddValueCol(geneId);
                    pFormatter->FinishRow();
                }
				pFormatter->StartRow();
				pFormatter->AddTagCol("Location:");
				pFormatter->AddValueCol(NStr::IntToString(entry->pos+1) +
				                            (entry->pos_end == -1 ? string() : (".." + NStr::IntToString(entry->pos_end+1))));
				pFormatter->FinishRow();
				pFormatter->StartRow();
				pFormatter->AddTagCol("Phenotype:");
				pFormatter->AddValueCol(pFormatter->CreateNcbiLink(entry->trait, phenotypeURL + entry->trait + "[mesh]"));
				pFormatter->FinishRow();
				pFormatter->StartRow();
				pFormatter->AddTagCol("Data source:");
				pFormatter->AddValueCol(NSnpBins::SourceAsString(entry->source));
				pFormatter->FinishRow();
				if(entry->pvalue > 0) {
                    pFormatter->StartRow();
                    pFormatter->AddTagCol("P-Value (-log10):");
                    pFormatter->AddValueCol(NStr::DoubleToString(entry->pvalue, 1));
                    pFormatter->FinishRow();
                }
				if(!entry->context.empty()) {
					pFormatter->StartRow();
					pFormatter->AddTagCol("Context:");
					pFormatter->AddValueCol(entry->context);
					pFormatter->FinishRow();
				}
				x_ProcessDbgaptext(entry, pFormatter);
				if(!entry->population.empty()) {
                    pFormatter->StartRow();
                    pFormatter->AddTagCol("Population:");
                    pFormatter->AddValueCol(entry->population);
                    pFormatter->FinishRow();
                }
                bool isGoToPresent(false);
                pFormatter->AddPubmedLinksRow(entry->pmids, isGoToPresent, false);
			}
			break;
		}
    }
}

void NSnpGui::x_MakeVarVuLink(CConstRef<NSnpBins::SBinEntry> entry,
							  TLinkList& LinkList,
							  TGi gi)
{
    NSnpBins::CGeneMap GeneMap(entry->genes_reported);

    // get assemblies associated with this GI
    //!! skip this and wait for a better solution per SV-2020
    // (https://jira.ncbi.nlm.nih.gov/browse/SV-2020?focusedCommentId=3259819&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-3259819)
//    CSeqUtils::TAccs assm_accs;
//    CSeqUtils::GetAssmAccs_Gi(assm_accs, gi);

    ITERATE(NSnpBins::CGeneMap, iGeneMap, GeneMap) {
        if(!iGeneMap->first.empty()) {
//            ITERATE(CAssemblyInfo::TAccs, i_assm_accs, assm_accs) {
//                LinkList.push_back(TLinkList::value_type(iGeneMap->first,
//                                                            VarVuURL + NStr::IntToString(entry->snpid) +
//                                                            "&filters=source:dbsnp&assm=" + *i_assm_accs));
//            }
            LinkList.push_back(TLinkList::value_type(iGeneMap->first,
                VarVuURL + NStr::IntToString(entry->snpid) +
                "&filters=source:dbsnp"));
        }
    }
}

void NSnpGui::x_MakeVarVuLink(CConstRef<NSnpBins::SBinEntry> entry,
							  CIRef<ITooltipFormatter> pFormatter,
							  bool& isGoToPresent,
							  TGi gi)
{
    TLinkList LinkList;
    x_MakeVarVuLink(entry, LinkList, gi);
    NSnpBins::CGeneMap GeneMap(entry->genes_reported);
    bool isFirstRow(true);
    ITERATE(TLinkList, iLinkList, LinkList) {
        pFormatter->StartRow();
        pFormatter->AddTagCol(isFirstRow ? "Variation Viewer:" : "");
        pFormatter->AddValueCol(pFormatter->CreateNcbiLink(iLinkList->first, iLinkList->second));
        pFormatter->FinishRow();
        isFirstRow = false;
    }
}

void NSnpGui::x_MakeClinVarLink(CConstRef<NSnpBins::SBinEntry> entry,
							  TLinkList& LinkList)
{
    LinkList.push_back(TLinkList::value_type("rs"+NStr::IntToString(entry->snpid),
                ClinVarURL + NStr::IntToString(entry->snpid)));
}

void NSnpGui::x_MakeClinVarLink(CConstRef<NSnpBins::SBinEntry> entry,
							  CIRef<ITooltipFormatter> pFormatter,
							  bool& isGoToPresent)
{
    TLinkList LinkList;
    x_MakeClinVarLink(entry, LinkList);
    bool isFirstRow(true);
    ITERATE(TLinkList, iLinkList, LinkList) {
        pFormatter->StartRow();
        pFormatter->AddTagCol(isFirstRow ? "ClinVar:" : "");
        pFormatter->AddValueCol(pFormatter->CreateNcbiLink(iLinkList->first, iLinkList->second));
        pFormatter->FinishRow();
        isFirstRow = false;
    }
}



void NSnpGui::x_ProcessDbgaptext(CConstRef<NSnpBins::SBinEntry> entry, ITooltipFormatter* pFormatter)
{
    if(entry->dbgaptext.empty())
        return;

    typedef map<string, string> TNameValueMap;

    TNameValueMap aNameValueMap;

    CStringPairs<TNameValueMap>::Parse(aNameValueMap,
                                       entry->dbgaptext,
                                       "&",
                                       "=",
                                       new CStringDecoder_Url());

    TNameValueMap::const_iterator iNameValue;

    if((iNameValue = aNameValueMap.find("data_src")) != aNameValueMap.end() &&
	   !iNameValue->second.empty()) {
        pFormatter->StartRow();
        pFormatter->AddTagCol("Data source:");
		pFormatter->AddValueCol(NSnpBins::SourceAsString(NStr::StringToInt(iNameValue->second)));
        pFormatter->FinishRow();
    }

    if((iNameValue = aNameValueMap.find("phs")) != aNameValueMap.end() && !iNameValue->second.empty() && iNameValue->second != "0") {
        // phs value must be padded with zeroes to achieve a length of 6 digits
        string phs("phs" + string(6 - iNameValue->second.length(), '0') + iNameValue->second);

        iNameValue = aNameValueMap.find("phs_desc");

        pFormatter->StartRow();
        pFormatter->AddTagCol(string((iNameValue != aNameValueMap.end() && !iNameValue->second.empty()) ? iNameValue->second : "PHS") + ":");
        pFormatter->AddValueCol(pFormatter->CreateNcbiLink(phs, GAPStudyURL + phs));
        pFormatter->FinishRow();
    }

    if((iNameValue = aNameValueMap.find("pha")) != aNameValueMap.end() && !iNameValue->second.empty() && iNameValue->second != "0") {
        string pha(iNameValue->second);
        string pha_padded("pha" + string(6 - iNameValue->second.length(), '0') + iNameValue->second);

        iNameValue = aNameValueMap.find("pha_desc");

        pFormatter->StartRow();
        pFormatter->AddTagCol(string((iNameValue != aNameValueMap.end() && !iNameValue->second.empty()) ? iNameValue->second : "PHA") + ":");
        pFormatter->AddValueCol(pFormatter->CreateNcbiLink(pha_padded, PHAURL + pha + "&snp=" + NStr::UIntToString(entry->snpid)));
        pFormatter->FinishRow();
    }
}


void NSnpGui::GetBinLinks(const NSnpBins::SBin& bin,
							 TLinkList& LinkList)
{
    LinkList.clear();

    ITERATE(NSnpBins::TBinEntryList, iter, bin.m_EntryList) {
        switch(bin.type) {
            case NSnpBins::eCLIN: //clinical
            case NSnpBins::eIND:  //individual
            {
                string snpid(NStr::UIntToString((*iter)->snpid));

                LinkList.push_back(TLinkList::value_type("RefSNP: rs" + snpid, snpURL + snpid));
                // similar to GetBinTooltip() above, parsing of OMIM/HGVS info is
                // temporarily disabled according to SNP-5006 09/17/12 12:16
                // if needed to be further worked on, the code may be taken from the old version of link.cpp
                //!! temporary GI == 0 to allow compilation for SV-2020
                x_MakeVarVuLink(*iter, LinkList, ZERO_GI);

                break;
            }
            case NSnpBins::eGAP:
            case NSnpBins::eGCAT:
            {
                string snpid(NStr::UIntToString((*iter)->snpid));

                LinkList.push_back(TLinkList::value_type("RefSNP: rs" + snpid, snpURL + snpid));
                break;
            }
        }
    }

    // Links for genome.gov
    if((bin.type == NSnpBins::eGAP || bin.type == NSnpBins::eGCAT) &&
        NStr::FindNoCase(bin.title, "nhgri") != NPOS) {
        ITERATE(NSnpBins::TBinEntryList, iter, bin.m_EntryList) {
            string snpid(NStr::UIntToString((*iter)->snpid));

            LinkList.push_back(TLinkList::value_type("NHGRI GWAS Catalog: rs" + snpid, GenomeURL + snpid + "#result_table"));
        }
    }
}

bool NSnpGui::isFromVcf(const CSeq_feat& ref)
{
    return !ref.FindExt("VcfAttributes").IsNull();
}

CVariation_inst::EType NSnpGui::GetVcfType(const CSeq_feat& ref)
{
    if(!isFromVcf(ref) || !ref.GetData().IsVariation()) {
        return CVariation_inst::eType_unknown;
    }
    const CVariation_ref& var = ref.GetData().GetVariation();
    CVariation_inst::EType type(CVariation_inst::eType_unknown);

    if (var.IsSetData() && var.GetData().IsSet() && var.GetData().GetSet().IsSetVariations()) {
        vector<string> alt_alleles;
        string ref_allele;
        auto var_list = var.GetData().GetSet().GetVariations();
        for (auto iVariation: var_list) {
            if (iVariation->IsSetData() && iVariation->GetData().IsInstance()) {
                const CVariation_ref::TData::TInstance& VarInst(iVariation->GetData().GetInstance());
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
                                    sAllele = Seq_data.GetIupacna().Get().empty() ? "" : Seq_data.GetIupacna().Get();
                                if (Seq_data.IsIupacaa())
                                    sAllele = Seq_data.GetIupacaa().Get().empty() ? "" : Seq_data.GetIupacaa().Get();

                                if(isReference) {
                                    ref_allele = sAllele;
                                } else {
                                    alt_alleles.push_back(sAllele);
                                }
                            }
                            break;
                        }
                        case CDelta_item::C_Seq::e_This:
                        // this can be a deletion
                            if(VarInst.GetType() == CVariation_inst::eType_del) {
                                alt_alleles.push_back("");
                                break;
                            }
                        default:
                            // no specific processing for other deltas
                            break;
                        }
                    }
                }
            }
        }
        bool maybeAllSnv = (ref_allele.size() == 1);
        if (maybeAllSnv) {
            for (size_t u=0; u < alt_alleles.size(); ++u) {
                if (alt_alleles[u].size() != 1) {
                    maybeAllSnv = false;
                    break;
                }
            }
            if (maybeAllSnv) {
                return CVariation_inst::eType_snv;
            }
        }

        //test for all mnvs:
        bool maybeAllMnv = true;
        size_t refSize = ref_allele.size();
        for (size_t u=0; u < alt_alleles.size(); ++u) {
            if (alt_alleles[u].size() != refSize) {
                maybeAllMnv = false;
                break;
            }
        }
        if (maybeAllMnv) {
            return CVariation_inst::eType_mnp;
        }

        //test for all insertions:
        bool maybeAllIns = true;
        for (size_t u=0; u < alt_alleles.size(); ++u) {
            if (alt_alleles[u].size() <= ref_allele.size()) {
                maybeAllIns = false;
                break;
            }
        }
        if (maybeAllIns) {
            return CVariation_inst::eType_ins;
        }

        if (alt_alleles.size() == 1  && alt_alleles[0].empty()) {
            return CVariation_inst::eType_del;
        }
        return CVariation_inst::eType_delins;
 
    }
    return type;
}

const string NSNPWebServices::c_SNPVarExt_SearchClass = "SNPSearch";
const string NSNPWebServices::c_SNPVarExt_GeneSymbol = "gene_symbol";
const string NSNPWebServices::c_SNPVarExt_TopLevel = "top_level";
const string NSNPWebServices::c_SNPVarExt_QueryId = "query_id";
const string NSNPWebServices::c_SNPVarExt_Comment = "comment";

void NSNPWebServices::x_GetSearchHost()
{
    if(m_sSearchHost.empty()) {
        const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
        m_isSearchByRsid = reg.GetBool("snp_web_services", "use_search_by_rsid", true);
        m_sSearchHost = reg.GetString("snp_web_services", 
                                        "search_host", 
                                        m_isSearchByRsid 
                                        ?
                                            "https://www.ncbi.nlm.nih.gov/projects/variation/search-by-rsid/?format=asn1&rsid="
                                        :
                                            "https:///projects/SNP/beVarSearch.cgi?format=xml&report=varloc&id=");
    }
}


void NSNPWebServices::Search(const std::string& sTerms, const string& sAssemblyAccession, TSNPSearchCompoundResultList& ResultList)
{
    x_GetSearchHost();

    if(m_isSearchByRsid) {
        list<string> Terms;
        NStr::Split(sTerms, ", ", Terms, NStr::fSplit_Tokenize);

        // map of rsids to variation lists with the same rsid
        typedef map<string, list<CRef<CVariation>>> TRsIdVariationMap;
        TRsIdVariationMap RsIdVariationMap;

        for(auto Term: Terms) {
            NStr::TrimPrefixInPlace(Term, "rs", NStr::eNocase);
            string sSNPSearchURL(m_sSearchHost + Term);

            if(!sAssemblyAccession.empty()) {
                sSNPSearchURL += "&assm=" + sAssemblyAccession;
            }
            string sSearchResult;
            CFetchURL::Fetch(sSNPSearchURL, sSearchResult, kDefaultTimeout, true/*use-cache*/);
            TSNPSearchCompoundResult SNPSearchResult;

            CSearchByRsIdReply SearchByRsIdReply;
            // deserialize sSearchResult into SearchByRsIdReply 
            sSearchResult >> SearchByRsIdReply;

            if(SearchByRsIdReply.IsPlacements()) {
                for(auto iPlacements: SearchByRsIdReply.GetPlacements()) {
                    // get the placement rsid
                    string rsid(iPlacements->GetRsid());
                    if(!NStr::StartsWith(rsid, "rs", NStr::eNocase)) {
                        rsid = "rs" + rsid;
                    }

                    auto pVariation(Ref(new CVariation));
                    pVariation->SetData().SetNote("SNP search result");
                    pVariation->SetDescription("SNP search result");
                    pVariation->SetId().SetDb("dbSNP");
                    pVariation->SetId().SetTag().SetStr(rsid);
                    auto pId(Ref(new CSeq_id(iPlacements->GetAcc_ver())));

                    auto pLocation(Ref(new CSeq_loc(*pId,
                                                    iPlacements->GetFrom(),
                                                    iPlacements->GetFrom() + iPlacements->GetLength() - 1)));

                    auto pPlacement(Ref(new CVariantPlacement));
                    pPlacement->SetLoc(*pLocation);
                    //!! review this, try to obtain actual molecule type from pId
                    pPlacement->SetMol(CVariantPlacement::eMol_genomic);
                    if(iPlacements->CanGetAssembly_acc()) {
                        pPlacement->SetAssembly().SetDb("");
                        pPlacement->SetAssembly().SetTag().SetStr(iPlacements->GetAssembly_acc());
                    }
                    pVariation->SetPlacements().push_back(pPlacement);

                    // check whether we have already handled a placement with this rsid
                    auto iRsIdVariationMap(RsIdVariationMap.find(rsid));
                    if(iRsIdVariationMap == RsIdVariationMap.end()) {
                        auto InsertResult(RsIdVariationMap.insert(TRsIdVariationMap::value_type(rsid, list<CRef<CVariation>>())));
                        iRsIdVariationMap = InsertResult.first;
                    }
                    iRsIdVariationMap->second.push_back(pVariation);
                }
            }
        }

        for(auto iRsIdVariationMap: RsIdVariationMap) {
/*
            cerr << "Placements at point of creation: " << endl;
            std::stringstream ostr; 
            ostr << MSerial_Json << *iRsIdVariationMap.second;
            cerr << ostr.str() << endl;
*/
            ResultList.push_back(TSNPSearchCompoundResult(iRsIdVariationMap.first, iRsIdVariationMap.second));
        }
    } else {
        list<string> Terms;
        NStr::Split(sTerms, ", ", Terms, NStr::fSplit_Tokenize);

        ITERATE(list<string>, iTerms, Terms) {
            string sSNPSearchURL(m_sSearchHost + *iTerms);

            if(!sAssemblyAccession.empty()) {
                sSNPSearchURL += "&assm=" + sAssemblyAccession;
            }
            string sSearchResult;
            CFetchURL::Fetch(sSNPSearchURL, sSearchResult, kDefaultTimeout, true/*use-cache*/);
            TSNPSearchCompoundResult SNPSearchResult;

            xml::document ResultXml(sSearchResult.c_str(), sSearchResult.length(), NULL);
            xml::node& ResultRoot(ResultXml.get_root_node());
            // get the SNP id common for all results
            xml::xpath_expression ParamIdExpr("/result_set/query/param_id");
            const xml::node_set ParamIdSet(ResultRoot.run_xpath_query(ParamIdExpr));

            // currently expect one and only one ParamId
            NCBI_ASSERT(ParamIdSet.size() == 1, "Unexpected number of <param_id> in SNP search result!");

            const xml::attributes& ParamIdAttrs(ParamIdSet.begin()->get_attributes());
            string sClin, sComment, sResultID, sQueryId;
            ITERATE(xml::attributes, iParamIdAttrs, ParamIdAttrs) {
                string sAttrName(iParamIdAttrs->get_name());
                if(sAttrName == "isClin")
                    sClin = iParamIdAttrs->get_value();
                if(sAttrName == "comment")
                    sComment = iParamIdAttrs->get_value();
                if(sAttrName == "query_id")
                    sQueryId = iParamIdAttrs->get_value();
                if(sAttrName == "result_id")
                    sResultID = iParamIdAttrs->get_value();
            }

            if(!sResultID.empty()) {
                SNPSearchResult.first = sResultID;
                // parse the individual results which are expected to have the following structure:
                // <result_set><query><placement_set><placement placement_type= to= from= assembly_name= accession= [top_level="yes"]/><placement .../> etc
                xml::xpath_expression PlacementExpr("/result_set/query/placement_set/placement");
                const xml::node_set PlacementSet(ResultRoot.run_xpath_query(PlacementExpr));

                ITERATE(xml::node_set, iPlacementSet, PlacementSet) {
                    const xml::attributes& PlacementAttrs(iPlacementSet->get_attributes());
                    string sAccession, sFrom, sTo, sAssemblyName, sGeneSymbol, sTopLevel;

                    ITERATE(xml::attributes, iPlacementAttrs, PlacementAttrs) {
                        string sAttrName(iPlacementAttrs->get_name());
                        if(sAttrName == "to")
                            sTo = iPlacementAttrs->get_value();
                        if(sAttrName == "from")
                            sFrom = iPlacementAttrs->get_value();
                        if(sAttrName == "assembly_name")
                            sAssemblyName = iPlacementAttrs->get_value();
                        if(sAttrName == "accession")
                            sAccession = iPlacementAttrs->get_value();
                        if(sAttrName == "gene_symbol")
                            sGeneSymbol = iPlacementAttrs->get_value();
                        if(sAttrName == "top_level")
                            sTopLevel = iPlacementAttrs->get_value();
                    }
                    CRef<CSeq_id>  pId(new CSeq_id(sAccession));
                    CRef<CSeq_loc> pLocation(new CSeq_loc(*pId,
                                                        NStr::StringToNumeric<TSeqPos>(sFrom),
                                                        NStr::StringToNumeric<TSeqPos>(sTo)));

                    CRef<CVariantPlacement> pPlacement(new CVariantPlacement);
                    pPlacement->SetLoc(*pLocation);
                    //!! review this, try to obtain actual molecule type from pId
                    pPlacement->SetMol(CVariantPlacement::eMol_genomic);
                    pPlacement->SetAssembly().SetDb("");
                    pPlacement->SetAssembly().SetTag().SetStr(sAssemblyName);
                    CRef<CVariation> pVariation(new CVariation);
                    pVariation->SetPlacements().push_back(pPlacement);
                    pVariation->SetData().SetNote("SNP search result");
                    pVariation->SetDescription("SNP search result");
                    pVariation->SetId().SetDb("dbSNP");
                    pVariation->SetId().SetTag().SetStr(sResultID);
                    if(sClin == "Y")
                        pVariation->SetVariant_prop().SetResource_link(CVariantProperties::eResource_link_clinical);
                    if(!sComment.empty())
                        pVariation->SetDescription(sComment);

                    // there is no direct place to put gene_symbol, top_level flag, comment, query_id
                    // so it goes to user object
                    if(!sGeneSymbol.empty() || !sTopLevel.empty() || !sComment.empty() || !sQueryId.empty()) {
                        CRef<CUser_object> pExt(new CUser_object());
                        if(!sGeneSymbol.empty()) {
                            pExt->AddField(c_SNPVarExt_GeneSymbol, sGeneSymbol);
                        }
                        if(!sQueryId.empty()) {
                            pExt->AddField(c_SNPVarExt_QueryId, sQueryId);
                        }
                        if(!sComment.empty()) {
                            pExt->AddField(c_SNPVarExt_Comment, sComment);
                        }
                        if(sTopLevel == "yes") {
                            pExt->AddField(c_SNPVarExt_TopLevel, "yes");
                        }
                        pExt->SetClass(c_SNPVarExt_SearchClass);
                        pExt->SetType().SetStr(c_SNPVarExt_SearchClass);
                        pVariation->SetExt().push_back(pExt);
                    }
                    SNPSearchResult.second.push_back(pVariation);
                }
                ResultList.push_back(SNPSearchResult);
            }
        }
    }
}

void NSNPWebServices::Search(const std::string& sTerms, TSNPSearchResultList& ResultList)
{
    TSNPSearchCompoundResultList CompoundResultList;
    Search(sTerms, "", CompoundResultList);
    ITERATE(TSNPSearchCompoundResultList, iCompoundResultList, CompoundResultList) {
        ResultList.insert(ResultList.end(), iCompoundResultList->second.begin(), iCompoundResultList->second.end());
    }
}


static const char* sSearchByVarIdHost = "https://www.ncbi.nlm.nih.gov/projects/variation/viewer/search_by_varid/?format=json&varid=";

void NSNPWebServices::SearchByVarId(const std::string& sTerms, const string& sAssemblyAccession, TSNPSearchCompoundResultList& ResultList)
{
    list<string> Terms;
    NStr::Split(sTerms, ", ", Terms, NStr::fSplit_Tokenize);

    // map of rsids to variation lists with the same rsid
    typedef map<string, list<CRef<CVariation>>> TRsIdVariationMap;
    TRsIdVariationMap RsIdVariationMap;

    for(auto Term: Terms) {
        string sSNPSearchURL(sSearchByVarIdHost);
        sSNPSearchURL += Term;

        if(!sAssemblyAccession.empty()) {
            sSNPSearchURL += "&assm=" + sAssemblyAccession;
        }

        string sSearchResult;
        CFetchURL::Fetch(sSNPSearchURL, sSearchResult, kDefaultTimeout, true/*use-cache*/);
        if (sSearchResult.empty())
            continue;

        CJson_Document doc;
        doc.ParseString(sSearchResult);
        if (!doc.ReadSucceeded() || !doc.IsObject())
            continue;

        CJson_ConstObject obj = doc.GetObject();
        CJson_ConstArray arr = obj["SearchByVarIdReply"].GetObject()["placements"].GetArray();

        for ( CJson_ConstArray::const_iterator it = arr.begin(); it != arr.end(); ++it)
        {
            CJson_ConstObject obj2 = it->GetObject();
            string varid = obj2["varid"].GetValue().GetString();
            string acc_ver = obj2["acc_ver"].GetValue().GetString();
            TSeqPos from = obj2["from"].GetValue().GetUint8();
            TSeqPos length = obj2["length"].GetValue().GetUint8();
            string assembly_acc = obj2["assembly_acc"].GetValue().GetString();

            auto pVariation(Ref(new CVariation));
            pVariation->SetData().SetNote("SNP search result");
            pVariation->SetDescription("SNP search result");
            pVariation->SetId().SetDb("dbSNP");
            pVariation->SetId().SetTag().SetStr(varid);
            auto pId(Ref(new CSeq_id(acc_ver)));

            auto pLocation(Ref(new CSeq_loc(*pId, from, from + length - 1)));

            auto pPlacement(Ref(new CVariantPlacement));
            pPlacement->SetLoc(*pLocation);
            //!! review this, try to obtain actual molecule type from pId
            pPlacement->SetMol(CVariantPlacement::eMol_genomic);
            pPlacement->SetAssembly().SetDb("");
            pPlacement->SetAssembly().SetTag().SetStr(assembly_acc);

            pVariation->SetPlacements().push_back(pPlacement);

            // check whether we have already handled a placement with this rsid
            auto iRsIdVariationMap(RsIdVariationMap.find(varid));
            if(iRsIdVariationMap == RsIdVariationMap.end()) {
                auto InsertResult(RsIdVariationMap.insert(TRsIdVariationMap::value_type(varid, list<CRef<CVariation>>())));
                iRsIdVariationMap = InsertResult.first;
            }
            iRsIdVariationMap->second.push_back(pVariation);
        }
    }

    for(auto iRsIdVariationMap: RsIdVariationMap) {
        ResultList.push_back(TSNPSearchCompoundResult(iRsIdVariationMap.first, iRsIdVariationMap.second));
    }    
}

END_NCBI_SCOPE
