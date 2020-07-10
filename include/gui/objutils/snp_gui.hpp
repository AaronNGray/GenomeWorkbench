#ifndef SNP_OBJUTILS___SNP_GUI__HPP
#define SNP_OBJUTILS___SNP_GUI__HPP

/*  $Id: snp_gui.hpp 44708 2020-02-27 16:13:12Z rudnev $
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
 *   Some GUI attributes of SNP objects
 *
 */

#include <string>

#include <corelib/ncbistd.hpp>

#include <objtools/snputil/snp_utils.hpp>
#include <objtools/snputil/snp_bins.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/tooltip.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

/// a collection of functions helping to display the SNP bins
class NCBI_GUIOBJUTILS_EXPORT NSnpGui
{
public:
	/// preferred colorings for SNP clinical significances
	///
	/// @param ClinSigID
	///   clinical significance ID
	/// @return
	///   a registry key to color used for the given clinical significance
	static string ClinSigAsColorKey(NSnp::TClinSigID ClinSigID);

	/// gets a URL to an image representing the given clinical significance
	///
	/// @param[in] ClinSigID
	///   clinical significance ID
    /// @param[in] colorTheme
	///   Current color theme
	/// @return
	///   URL path to the image that is hosted on NCBI servers
	/// (currently within /projects/sviewer/images/)
	static string ClinSigAsImgURL(NSnp::TClinSigID ClinSigID,
                                  const string &colorTheme);

	/// preferred colorings for a given pvalue
	///
	/// @param pvalue
	///   p-value as often contained within SNP bins
	/// @return
	///		a registry key to color associated with the given p-value
	static string PValueAsColorKey(double pvalue);

	/// generate a tooltip for a given bin object and put it into the formatter
	///
	/// @param[in] bin
	///   SNP bin from which data for the tooltip will be taken
	/// @param[out] pFormatter
	///   The generated tooltip will be put into the formatter provided here
    /// @param[in] gi
	///   gi of sequence on which tooltip is located
    /// @param[in] colorTheme
	///   Current color theme
	static void GetBinTooltip(const NSnpBins::SBin& bin,
							  CIRef<ITooltipFormatter> pFormatter,
							  TGi gi,
                              const std::string &colorTheme);

	/// representation for a link have an URL and a human-readable label
	///
	/// in the pair, first is the label, second is the URL
	typedef pair<string, string> TLink;

	/// list of URL links
	typedef list<TLink> TLinkList;

	/// get a list of links associated with a given SNP bin
	///
	/// @param bin
	///   SNP bin for which the links will be generated
	/// @param LinkList
	///   The generated links will be put into the list provided here
	///   links to NCBI resources are URL paths, outside links are absolute
	static void GetBinLinks(const NSnpBins::SBin& bin,
							 TLinkList& LinkList);

	/// standard width and height of a bin
	static const int c_BinWidth;
	static const int c_BinHeight;

	// helpers for dealing with variation info that was read from a VCF file by CVcfReader and
	// and put into a Variation_ref in its idiosyncratic way
	// later this should better be moved into NSnp
	// checks whether this feature was created by CVcfReader by looking for "VcfAttibutes" signature in ext and 
	// returns true if it is present there
	static bool isFromVcf(const CSeq_feat& ref);

	// decontructs list of variations and tries to deduce the variation type from alleles created by CVcfReader
	// returns CVariation_inst::eType_unknown if ref was not obtained from CVcfReader
	static CVariation_inst::EType GetVcfType(const CSeq_feat& ref);


private:
	/// makes a varVu link suitable for putting into a tooltip
	static void x_MakeVarVuLink(CConstRef<NSnpBins::SBinEntry> BinEntry,
								CIRef<ITooltipFormatter> pFormatter,
								bool& isGoToPresent,
								TGi gi);
	/// makes a varVu link suitable for putting into a link list
	static void x_MakeVarVuLink(CConstRef<NSnpBins::SBinEntry> BinEntry,
								TLinkList& LinkList,
								TGi gi);

	/// makes a ClinVar link suitable for putting into a tooltip
	static void x_MakeClinVarLink(CConstRef<NSnpBins::SBinEntry> BinEntry,
								CIRef<ITooltipFormatter> pFormatter,
								bool& isGoToPresent);
	/// makes a ClinVar link suitable for putting into a link list
	static void x_MakeClinVarLink(CConstRef<NSnpBins::SBinEntry> BinEntry,
								TLinkList& LinkList);

	/// processes dbgaptext field and puts its contents into a tooltip
	///
	/// unpacks whatever may be in dbgaptext field, creates links and
	/// puts them into the supplied tooltip formatter
	/// dbgaptext is a set of URL encoded key/value pairs, see DG-1985 for details
	static void x_ProcessDbgaptext(CConstRef<NSnpBins::SBinEntry> BinEntry,
								   ITooltipFormatter* pFormatter);

	/// standard NCBI resourse URLs
	static const string pmURL;
	static const string omimURL;
	static const string geneSymURL;
	static const string geneRevURL;
	static const string snpURL;
	static const string phenotypeURL;
	static const string GAPStudyURL;
	static const string PHAURL;
	static const string GenomeURL;
	static const string VarVuURL;
	static const string ClinVarURL;
};

/// helpers for accessing SNP web services
class NCBI_GUIOBJUTILS_EXPORT NSNPWebServices
{
public:
    /// various placements of a variation
    typedef list<CRef<CVariation> > TSNPSearchResultList;
    /// results of a search for a given id (string)
    /// TSNPSearchResultList may be empty if the id has been found to be valid, but not satisfying other search criteria
    /// (currently: assembly accession)
    typedef pair<string, TSNPSearchResultList> TSNPSearchCompoundResult;

    /// results of a search for one or several ids
	typedef list<TSNPSearchCompoundResult> TSNPSearchCompoundResultList;

	/// search for given SNP ID(s) and get a list of results
	///
	/// uses projects/SNP/beVarSearch.cgi
	///
	/// @param sTerm
	///   search term, a comma-delimited list of ss or rs ids (e.g. ss222424,rs4334323)
    ///
    /// @param sAssemblyAccession
    ///   if not empty, limit the results only to a given assembly accession
	/// @return
    ///   true if the search term is valid
	///   list of results as list of found IDs and linked results per each ID
	///   in addition to standard CVariation fields, variations have some additional data in ext User-field
	///   the following labels can appear in ext:
	///      c_SNPVarExt_GeneSymbol ("gene_symbol")
	///      c_SNPVarExt_TopLevel ("top_level") "yes" if this particular placement is on top-level genomic seq
    ///      c_SNPVarExt_QueryId ("query_id") original query id
    ///      c_SNPVarExt_Comment ("comment")
	static void Search(const std::string& sTerms, const std::string& sAssemblyAccession, TSNPSearchCompoundResultList& ResultList);
	/// same thing without taking assembly into account
    static void Search(const std::string& sTerms, TSNPSearchResultList& ResultList);

    static void SearchByVarId(const std::string& sTerms, const string& sAssemblyAccession, TSNPSearchCompoundResultList& ResultList);

	static const string c_SNPVarExt_SearchClass;
	static const string c_SNPVarExt_GeneSymbol;
	static const string c_SNPVarExt_TopLevel;
	static const string c_SNPVarExt_QueryId;
	static const string c_SNPVarExt_Comment;

private:
	static void x_GetSearchHost();
	static string m_sSearchHost;
	static bool m_isSearchByRsid;
};


END_SCOPE(objects)
END_NCBI_SCOPE
/* @} */
#endif // SNP_OBJUTILS___SNP_GUI__HPP

