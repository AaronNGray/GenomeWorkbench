/*  $Id: gui_object_info_variation.cpp 42378 2019-02-12 15:24:32Z rudnev $
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
* Authors: Liangshou Wu, Dmitry Rudnev
*
* File Description:
*     Gui Object Info class specifically for Single-nucleotide polymorphism
*  commonly known as SNP.
*
*/
#include <sstream>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_point.hpp>
#include <objects/variation/Variation.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/snp_gui.hpp>
#include <gui/utils/fetch_url.hpp>
#include <objtools/snputil/snp_bitfield.hpp>
#include <objtools/snputil/snp_utils.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/variation/VariantPlacement.hpp>
#include <objects/seqfeat/VariantProperties.hpp>

#define __CAN_USE_TOOLTIP_SVC
#ifdef __CAN_USE_TOOLTIP_SVC
#include <objects/variation_libs/dbsnp/tooltip_service/dbsnp_tooltip_service__.hpp>
#endif

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


class CGuiObjectInfoVariation : public CGuiObjectInfoSeq_feat
{
public:
    virtual string GetSubtype() const;
    virtual void GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p = (TSeqPos)-1, bool* isGeneratedBySvc = NULL) const;
    virtual void GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const;

private:
#ifdef __CAN_USE_TOOLTIP_SVC
	static void x_ConvertTooltips(ITooltipFormatter& tooltip, const CDbsnpTooltipServiceReply& DbsnpTooltipServiceReply);
	static void x_GetTooltipService();
	static bool m_isUseTooltipService;
	static string m_sTooltipServiceHost;
#endif
};

#ifdef __CAN_USE_TOOLTIP_SVC
bool CGuiObjectInfoVariation::m_isUseTooltipService{false};
string CGuiObjectInfoVariation::m_sTooltipServiceHost;
#endif

string CGuiObjectInfoVariation::GetSubtype() const
{
    return "SNP";
}

static void	s_AddTopLevelGenomicLinks(bool& isFirstGenomicLocation,
									  bool isMultiple,
									  const string& sVariationID,
									  const NSNPWebServices::TSNPSearchResultList& SNPSearchResultList,
									  ITooltipFormatter* pFormatter)
{
    ITERATE(NSNPWebServices::TSNPSearchResultList, iSNPSearchResultList, SNPSearchResultList) {
		pFormatter->StartRow();
		if(isFirstGenomicLocation) {
			pFormatter->AddTagCol(isMultiple ?
									"Genomic locations:" : "Genomic location:");
			isFirstGenomicLocation = false;
		} else {
			pFormatter->AddTagCol("");
		}
		const CVariation::TPlacements& Placements((*iSNPSearchResultList)->GetPlacements());
		CRef<CVariantPlacement> pPlacement(Placements.front());
		string sAccession(pPlacement->GetLoc().GetId()->GetSeqIdString(true));
		string sAssembly(pPlacement->GetAssembly().GetTag().GetStr());
        string sGenomicPosFrom(NStr::NumericToString(pPlacement->GetLoc().GetStart(eExtreme_Positional)+1));
        string sGenomicPosTo(NStr::NumericToString(pPlacement->GetLoc().GetStop(eExtreme_Positional)+1));
        string sLabel(sAccession + " @ " + sGenomicPosFrom + " @" + sAssembly);
        string sUrl("/projects/sviewer/?id=" + sAccession +
            "&v=" + sGenomicPosFrom + ":" + sGenomicPosTo +
            "&mk=" + sGenomicPosFrom + '|' + sVariationID + "|008000");
		pFormatter->AddValueCol(pFormatter->CreateGenomicLink(sLabel, sUrl), true);
		pFormatter->FinishRow();
	}
}

#ifdef __CAN_USE_TOOLTIP_SVC
void CGuiObjectInfoVariation::x_GetTooltipService() 
{
    if(m_sTooltipServiceHost.empty()) {
		const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
		m_isUseTooltipService = reg.GetBool("snp_web_services", "use_tooltip_service", true);
        m_sTooltipServiceHost = m_isUseTooltipService 
                        ? 
                        reg.GetString("snp_web_services", "tooltip_host", "https://www.ncbi.nlm.nih.gov/projects/variation/tooltips")
                        :
                        string("dummy");
	}
}
#endif

void CGuiObjectInfoVariation::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos /*at_p*/, bool* isGeneratedBySvc) const
{
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    CSnpBitfield bitfield(NSnp::GetBitfield(*m_Feat));
    bool isSnp(NSnp::IsSnp(*m_Feat));
    NSnp::TRsid Rsid(NSnp::GetRsid(*m_Feat));

    string sVariationID;
    if (Rsid) {
        sVariationID = "rs" + NStr::NumericToString(Rsid);
    } else {
        feature::GetLabel(*m_Feat, &sVariationID,
            feature::fFGL_Content, m_Scope.GetPointer());
		if(isGeneratedBySvc) {
			*isGeneratedBySvc = false;
		}
    }
/*
	// commented out as requested in SV-1343 on 07/23/2013 in favor of
	// using SNP search for top-level SNP hits discovery below
	// this code is retained as it may prove useful for tooltips other than SNP
	CSeqUtils::TLocVec MappedUpLocs(CSeqUtils::GetLocPlacements(m_Feat->GetLocation()));
	bool isFirstGenomicLocation(true);
    ITERATE(CSeqUtils::TLocVec, iMappedUpLocs, MappedUpLocs) {
		const CSeq_id& seq_id((*iMappedUpLocs)->GetInt().GetId());
		string sChr(CSeqUtils::GetChrGI(seq_id.GetGi()));
		if(!sChr.empty()) {
			tooltip.StartRow();
			if(isFirstGenomicLocation) {
				tooltip.AddTagCol(MappedUpLocs.size() > 1 ?
										"Genomic locations:" : "Genomic location:");
				isFirstGenomicLocation = false;
			} else {
				tooltip.AddTagCol("");
			}
			string sAccession;
            CLabel::GetLabel(seq_id, &sAccession, CLabel::eContent, m_Scope.GetPointer());
            string sGenomicPosFrom(NStr::NumericToString((*iMappedUpLocs)->GetInt().GetFrom()+1));
            string sGenomicPosTo(NStr::NumericToString((*iMappedUpLocs)->GetInt().GetTo()+1));
            string sLabel(sAccession + " @ " + sGenomicPosFrom);
            string sUrl("/projects/sviewer/?id=" + NStr::NumericToString(seq_id.GetGi()) +
                "&v=" + sGenomicPosFrom + ":" + sGenomicPosTo +
                "&m=" + sGenomicPosFrom + "!&mn=" + sVariationID);
			tooltip.AddValueCol(tooltip.CreateNcbiLink(sLabel, sUrl));
			tooltip.FinishRow();
		}
	}
*/
    if(Rsid) {
#ifdef __CAN_USE_TOOLTIP_SVC
		x_GetTooltipService();
		if(m_isUseTooltipService) {
            string sSNPTooltipURL(m_sTooltipServiceHost + "/" + NStr::NumericToString(Rsid));
            string sTooltipResult;
            CFetchURL::Fetch(sSNPTooltipURL, sTooltipResult, kDefaultTimeout, true/*use-cache*/);
			CDbsnpTooltipServiceReply DbsnpTooltipServiceReply;
			if(isGeneratedBySvc) {
				*isGeneratedBySvc = true;
			}

			//!! check for empty string, deserialization exceptions, etc
			if(sTooltipResult.empty()) {
				tooltip.AddRow("Error:", "Empty response from tooltip service");
				LOG_POST(Error << "Empty response from tooltip service for id '" << sVariationID << "'");
				return;
			}

			std::stringstream TooltipResultStream(sTooltipResult);
			
			try {
				TooltipResultStream >> MSerial_Json >> DbsnpTooltipServiceReply;
			} catch(const std::exception& err) {
				tooltip.AddRow("Error:", "Invalid response from tooltip service");
				LOG_POST(Error << "Invalid response from tooltip service for id '" << sVariationID << "', error: " << err.what());
				return;
			}

			x_ConvertTooltips(tooltip, DbsnpTooltipServiceReply);
		} else {
#endif
			if(isGeneratedBySvc) {
				*isGeneratedBySvc = false;
			}
			NSNPWebServices::TSNPSearchResultList SNPSearchResultList;
			try {
				NSNPWebServices::Search(sVariationID, SNPSearchResultList);
			}
			catch (const std::exception& err) {
				LOG_POST(Warning << "NSNPWebServices::Search() failed for id '" << sVariationID << "':" << err.what());
			}

			NSNPWebServices::TSNPSearchResultList FirstPriorityList;
			NSNPWebServices::TSNPSearchResultList SecondPriorityList;
			NSNPWebServices::TSNPSearchResultList ThirdPriorityList;
			int nLocations(0);

			ITERATE(NSNPWebServices::TSNPSearchResultList, iSNPSearchResultList, SNPSearchResultList) {
				// look for top level information in ext
				if(!(*iSNPSearchResultList)->CanGetExt()) {
					continue;
				}
				const CVariation::TExt& Ext((*iSNPSearchResultList)->GetExt());
				ITERATE(CVariation::TExt, iExt, Ext) {
					if((*iExt)->HasField(NSNPWebServices::c_SNPVarExt_TopLevel)) {
						const CUser_field& UserField((*iExt)->GetField(NSNPWebServices::c_SNPVarExt_TopLevel));
						// must be a string or we cry
						NCBI_ASSERT(UserField.CanGetData() && UserField.GetData().IsStr(), "Wrong type of user field for gene_symbol");

						// only if the string is "yes", we have a top level
						if(UserField.GetData().GetStr() == "yes") {
							// CVariation used as a search result can have one and only one placement
							NCBI_ASSERT((*iSNPSearchResultList)->CanGetPlacements(), "Unexpected absence of placements in SNP Search Result!");
							const CVariation::TPlacements& Placements((*iSNPSearchResultList)->GetPlacements());
							NCBI_ASSERT(Placements.size(), "Unexpected number of placements in SNP Search Result!");
							CRef<CVariantPlacement> pPlacement(Placements.front());
							string sAssembly(pPlacement->GetAssembly().GetTag().GetStr());
							++nLocations;

							if(NStr::StartsWith(sAssembly, "GRCh", NStr::eNocase)) {
								FirstPriorityList.push_back(*iSNPSearchResultList);
								break;
							}
							if(NStr::StartsWith(sAssembly, "CHM", NStr::eNocase)) {
								SecondPriorityList.push_back(*iSNPSearchResultList);
								break;
							}
							ThirdPriorityList.push_back(*iSNPSearchResultList);
							break;
						}
					}
				}
			}
			if(nLocations) {
				bool isFirstGenomicLocation(true);
				s_AddTopLevelGenomicLinks(isFirstGenomicLocation, nLocations > 1, sVariationID, FirstPriorityList, &tooltip);
				s_AddTopLevelGenomicLinks(isFirstGenomicLocation, nLocations > 1, sVariationID, SecondPriorityList, &tooltip);
				s_AddTopLevelGenomicLinks(isFirstGenomicLocation, nLocations > 1, sVariationID, ThirdPriorityList, &tooltip);
			}

			string sVariantType(bitfield.GetVariationClassString());
			tooltip.AddRow("Variation ID:", sVariationID);

			string sLength(NStr::IntToString(m_Feat->GetLocation().GetTotalRange().GetLength(), NStr::fWithCommas));
			tooltip.AddRow("Variant Type:", sVariantType + ", length " + sLength);
#ifdef __CAN_USE_TOOLTIP_SVC
		}
#endif
    } else {
        tooltip.AddRow("Variation:", sVariationID);
    }
	if(!m_isUseTooltipService || !isSnp) {
		/// alleles come next
		NSnp::TAlleles Alleles;
		NSnp::GetAlleles(*m_Feat, Alleles);
		if ( !Alleles.empty() ) {
			tooltip.AddRow("Alleles:", NStr::Join(Alleles, "/"));
		}
		if(bitfield.IsTrue(CSnpBitfield::eIsSomatic)) {
			tooltip.AddRow("Allele origin:", "Somatic");
		}

		if (m_Feat->IsSetComment()) {
			tooltip.AddRow("Comment:", m_Feat->GetComment());
		}
	}
}


void CGuiObjectInfoVariation::GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const
{
    if (m_Feat->IsSetDbxref()) {
        string base_url = no_ncbi_base ? "" : CSeqUtils::GetNcbiBaseUrl();
        const CSeq_feat::TDbxref& refs = m_Feat->GetDbxref();
        links.AddLinksTitle("Go To");

		ITERATE (CSeq_feat::TDbxref, iter, refs) {
            const CRef<CDbtag>& dbt = *iter;
            if (NSnp::IsSnp(*dbt)) {
                string rsid{NStr::NumericToString(NSnp::GetRsid(*dbt))};
                ///
                /// link for SNP summary
                ///
				string name = "SNP summary:";
                string label = "rs";
                label += rsid;
                string link = base_url + "/snp/rs";
                link += rsid;

				links.AddLinkRow(name, label, link);

                CSnpBitfield bf(*m_Feat);
				if(bf.isGood()) {
				    ///
				    /// link for Genotype information
				    ///
				    if (bf.IsTrue(CSnpBitfield::eHasGenotype)) {
				        name = "Genotype information for";
				        label = "rs";
				        label += rsid;
				        link = base_url + "/snp/rs";
				        link += rsid + "#frequency_tab";
				        links.AddLinkRow(name, label, link);
				    }

				    ///
				    /// link for SNP3D web page
				    ///
				    if (bf.IsTrue(CSnpBitfield::eHasSnp3D)) {
				        name = "SNP3D Page for";
				        label = "rs" + rsid;
				        link = base_url + "/projects/SNP/snp3D.cgi?rsnum=";
				        link += rsid;
				        links.AddLinkRow(name, label, link);
				    }

				    ///
				    /// link for OMIM page for this SNP
				    ///
				    if (bf.IsTrue(CSnpBitfield::eHasOMIM_OMIA)) {
				        name = "OMIM information for";
				        label = "rs" + rsid;
				        link = base_url + "/projects/SNP/snp_redirect.cgi?snp_id=";
				        link += rsid;
				        links.AddLinkRow(name, label, link);
				    }

				    // VarVu link for clinical SNPs
				    if(bf.IsTrue(CSnpBitfield::eIsClinical)) {
				        // try to match genes to the current position
				        SAnnotSelector sel_gene(CSeqUtils::GetAnnotSelector());
				        sel_gene.IncludeFeatSubtype(CSeqFeatData::eSubtype_gene);

				        if(m_Feat->CanGetLocation()) {
				            CFeat_CI feat_iter(*m_Scope, m_Feat->GetLocation(), sel_gene);

				            CConstRef<CSeq_feat> mapped_feat;
				            for ( ;  feat_iter;  ++feat_iter) {
				                mapped_feat.Reset(&feat_iter->GetMappedFeature());
				                string sGeneLocus;
				                if(mapped_feat->GetData().Which() == CSeqFeatData::e_Gene  &&
				                    mapped_feat->GetData().GetGene().IsSetLocus()) {
				                sGeneLocus = mapped_feat->GetData().GetGene().GetLocus();
                                    }

                                    if( !sGeneLocus.empty() ) {
                                        name = "Variation viewer for";
                                        label = "rs" + rsid;
                                        label += " (" + sGeneLocus + ")",
                                        link = base_url + "/sites/varvu?rs=";
                                        link += rsid + "&gene=" + sGeneLocus;
                                        links.AddLinkRow(name, label, link);
                                    }
                            }
                        }
                    }
                }
            }
        }
    }
}

#ifdef __CAN_USE_TOOLTIP_SVC
void CGuiObjectInfoVariation::x_ConvertTooltips(ITooltipFormatter& tooltip, const CDbsnpTooltipServiceReply& DbsnpTooltipServiceReply)
{
	if(DbsnpTooltipServiceReply.IsTooltip_data()) {
		string sCurrentSectionName;
		const auto& tooltip_data(DbsnpTooltipServiceReply.GetTooltip_data());
		for(const auto& tooltip_section: tooltip_data.Get()) {
			if(tooltip_section->CanGetSection()) {
				sCurrentSectionName = tooltip_section->GetSection();
				if(!sCurrentSectionName.empty()) {
					tooltip.AddSectionRow(sCurrentSectionName);
				}
			}
			if(tooltip_section->CanGetValues()) {
				for(const auto& tooltip_row: tooltip_section->GetValues()) {
					string sURL(tooltip_row->CanGetUrl() ? tooltip_row->GetUrl() : "");
					string sValue(tooltip_row->GetValue());
					bool isLink(!sURL.empty());
					if(isLink) {
						// this is a hackish solution for the problem of special treatment for
						// genomic URLs in GBench
						if(sCurrentSectionName == "Genomic locations") {
							sValue = tooltip.CreateGenomicLink(sValue, sURL);
						} else {
							sValue = tooltip.CreateNcbiLink(sValue, sURL);
						}
					}
					tooltip.AddRow(tooltip_row->GetKey() + ":", sValue, 200, isLink);
				}
			}
		}
	}
	if(DbsnpTooltipServiceReply.IsError()) {
		tooltip.AddRow("Error:", DbsnpTooltipServiceReply.GetError().GetMessage());
	}
}
#endif

END_NCBI_SCOPE
