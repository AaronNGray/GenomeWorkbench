/*  $Id: link_utils.cpp 42299 2019-01-29 19:03:16Z evgeniev $
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
 * Author:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/link_utils.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/seqfeat/Variation_ref.hpp>

#include <objmgr/annot_selector.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>

#include <objtools/snputil/snp_bitfield.hpp>
#include <objtools/snputil/snp_utils.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/gui_object_info_seq_align.hpp>


BEGIN_NCBI_SCOPE

USING_SCOPE(objects);
USING_SCOPE(sequence);


/////////////////////////////////////////////////////////////////////////////
//  CLinkUtils::
//

static const string kCommonUrl = "https://www.ncbi.nlm.nih.gov";


void CLinkUtils::AddSequenceLinks(const CSeq_id_Handle& idh,
                                  const string& tax_id_file,
                                  CScope& scope,
                                  TLinksMap& links,
                                  TSeqPos from,
                                  TSeqPos to,
                                  bool relative)
{
    CBioseq_Handle bsh = scope.GetBioseqHandle(idh);
    if (!bsh)
       return;
    bool is_na = bsh.IsNa();
    CSeq_id_Handle source_idh = sequence::GetId(idh, scope,
                                                sequence::eGetId_Best);

    ///
    /// check to see if we add a genome-specific search link
    ///
    int tax_id = sequence::GetTaxId(bsh);
    bool genome_specific_search = false;
    if ( !tax_id_file.empty() ) {
        CNcbiIfstream istr(tax_id_file.c_str(), IOS_BASE::in);
        if (istr.good()) {
            string line;
            while (NcbiGetlineEOL(istr, line)) {
                try {
                    if (tax_id == (NStr::StringToInt(line))) {
                        genome_specific_search = true;
                        break;
                    }
                } catch (...) {
                    break;
                }
            }
        }
    }
    string source_id_str;
    CLabel::GetLabel(*source_idh.GetSeqId(), &source_id_str,
                     CLabel::eDefault, &scope);

    ///
    /// add a link for sequence retrieval
    ///
    string entrezdb_tag;
    if (is_na) {
        CSeq_id::EAccessionInfo acc_info =
            source_idh.GetSeqId()->IdentifyAccession();
        if ((acc_info & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_est) {
            entrezdb_tag = "/nucest/";
        } else if ((acc_info & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_gss) {
            entrezdb_tag = "/nucgss/";
        } else {
            entrezdb_tag = "/nuccore/";
        }
    } else {
        entrezdb_tag = "/protein/";
    }

    ///
    /// add links to self - that is, a link to view the current sequence
    ///
    typedef pair<string, string> TPair;
    const TPair sc_Pairs[] = {
        TPair("GenBank View", "report=genbank"),
        TPair("FASTA View", "report=fasta"),
    };

    string type = "Extra";
    string name = "";
    string label = "";
    string url = "";
    const size_t size = sizeof(sc_Pairs) / sizeof(TPair);
    for (size_t i = 0;  i < size;  ++i) {
        name = sc_Pairs[i].first;
        label = source_id_str;

        string tag = sc_Pairs[i].second;
        if ( !is_na ) {
            tag = NStr::Replace(tag, "genbank", "genpept");
        }
        url = entrezdb_tag + source_id_str + "?" + tag;
        x_AddLink(links, type, name, label, url, relative);
    }

    ///
    /// add a link to BLAST the genomic location
    ///
    {{
         name = "BLAST ";
         label = source_id_str;
         string params("QUERY=" + source_id_str);
         if (from || to) {
             params += "&QUERY_FROM=";
             params += NStr::IntToString(from+1);
             params += "&QUERY_TO=";
             params += NStr::IntToString(to+1);
         }

         string extra_params("&");
         if (is_na) {
             name += "Genomic";
             extra_params +=
                 "PAGE=Nucleotides&"
                 "PROGRAM=blastn&"
                 "MEGABLAST=on&"
                 "BLAST_PROGRAMS=megaBlast&"
                 "PAGE_TYPE=BlastSearch&"
                 "SHOW_DEFAULTS=on";

         } else {
             name += "protein";
             extra_params +=
                 "PAGE=Proteins&"
                 "PROGRAM=blastp&"
                 "BLAST_PROGRAMS=blastp&"
                 "PAGE_TYPE=BlastSearch&"
                 "SHOW_DEFAULTS=on";
         }
         url = "/blast/Blast.cgi?" + params + extra_params;
         x_AddLink(links, type, name, label, url, relative);

         // add a link for genome-specific BLAST
         if (genome_specific_search) {
             if (!is_na) {
                 params += "&PROGRAM=tblastn";
             }
             name = "BLAST Genome-specific";
             url = "/genome/seq/BlastGen/BlastGen.cgi?taxid=";
             url += NStr::IntToString(tax_id) + "&" + params;
             x_AddLink(links, type, name, label, url, relative);
         }
     }}
}



void CLinkUtils::AddFeatureLinks(const CSeq_feat& feat,
                                 const CBioseq_Handle& handle,
                                 const string& tax_id_file,
                                 TLinksMap& links,
                                 bool relative)
{
    const CSeq_feat_Base::TLocation& loc_obj = feat.GetLocation();
    CConstRef<CSeq_loc> loc;

    /// guard against the cases where the feature contains location
    /// with multiple seq-ids
    if ( !loc_obj.GetId() ) {
        loc = CSeqUtils::MixLocToLoc(loc_obj, handle);
    }

    if ( !loc ) {
        loc.Reset(&loc_obj);
    }

    AddFeatureLinks(feat, *loc, tax_id_file, handle.GetScope(), links, relative);
}


void CLinkUtils::AddFeatureLinks(const CSeq_feat& feat,
                                 const CSeq_loc& loc,
                                 const string& tax_id_file,
                                 CScope& scope,
                                 TLinksMap& links,
                                 bool relative)
{
    /// grab the source location identifier and range
    CBioseq_Handle bsh = scope.GetBioseqHandle(*loc.GetId());
    bool is_na = bsh.IsNa();
    CSeq_id_Handle source_idh =
        sequence::GetIdHandle(loc, &scope);
    source_idh = sequence::GetId(source_idh, scope,
                                 sequence::eGetId_Best);
    if (!source_idh  ||  source_idh.GetSeqId()->IsLocal()) {
        return;
    }

    int tax_id = sequence::GetTaxId(bsh);
    bool genome_specific_search = false;
    if ( !tax_id_file.empty() ) {
        CNcbiIfstream istr(tax_id_file.c_str(), IOS_BASE::in);
        if (istr.good()) {
            string line;
            while (NcbiGetlineEOL(istr, line)) {
                try {
                    if (tax_id == (NStr::StringToInt(line))) {
                        genome_specific_search = true;
                        break;
                    }
                } catch (...) {
                    break;
                }
            }
        }
    }
    string source_id_str;
    CLabel::GetLabel(*source_idh.GetSeqId(), &source_id_str,
                     CLabel::eDefault, &scope);
    string range_str(kEmptyStr);
    if ( !loc.IsWhole() ) {
        range_str = " (";
        range_str += NStr::IntToString(loc.GetTotalRange().GetFrom() + 1,
                                       NStr::fWithCommas);
        range_str += "..";
        range_str += NStr::IntToString(loc.GetTotalRange().GetTo() + 1,
                                       NStr::fWithCommas);
        range_str += ")";
    }

    string entrezdb_tag;
    if (is_na) {
        CSeq_id::EAccessionInfo acc_info =
            source_idh.GetSeqId()->IdentifyAccession();
        if ((acc_info & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_est) {
            entrezdb_tag = "/nucest/";
        } else if ((acc_info & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_gss) {
            entrezdb_tag = "/nucgss/";
        } else {
            entrezdb_tag = "/nuccore/";
        }
    } else {
        entrezdb_tag = "/protein/";
    }

    TSeqRange range = loc.GetTotalRange();
    if (range == TSeqRange::GetWhole()) {
        range.SetFrom(0);
        range.SetTo(bsh.GetBioseqLength());
    }

    /// grab the product location identifier
    CSeq_id_Handle product_idh;
    string product_id_str;
    bool is_product_na = false;
    if (feat.IsSetProduct()) {
        product_idh =
            sequence::GetIdHandle(feat.GetProduct(), &scope);
        CBioseq_Handle bsh = scope.GetBioseqHandle(product_idh);
        if (bsh) {
            is_product_na = bsh.IsNa();

            product_idh = sequence::GetId(product_idh, scope,
                                      sequence::eGetId_Best);
            CLabel::GetLabel(*product_idh.GetSeqId(), &product_id_str,
                         CLabel::eDefault, &scope);
        }
    }

    ///
    /// add links to self - that is, a link to view the current sequence
    ///
    string type = "Extra";
    string name = "";
    string label = "";
    string url = "";
    if (range.GetLength() > 5) {
        typedef pair<string, string> TPair;
        const TPair sc_Pairs[] = {
            TPair("GenBank View", "report=genbank"),
            TPair("FASTA View", "report=fasta"),
        };

        const size_t size = sizeof(sc_Pairs) / sizeof(TPair);

        for (size_t i = 0;  i < size;  ++i) {
            name = sc_Pairs[i].first;
            label = source_id_str + range_str;

            string tag = sc_Pairs[i].second;
            if ( !is_na ) {
                tag = NStr::Replace(tag, "bank", "pept");
            }

            url = entrezdb_tag + source_id_str + "?" + tag +
                "&from=" + NStr::IntToString(range.GetFrom() + 1) +
                "&to=" + NStr::IntToString(range.GetTo() + 1);
            x_AddLink(links, type, name, label, url, relative);
        }

        ///
        /// add a link to BLAST the genomic location
        ///
        {{
             name = "BLAST ";
             label = source_id_str + range_str;
             url = "/blast/Blast.cgi?";
             string params("QUERY=" + source_id_str);
             string extra_params("&");
             if ( !loc.IsWhole() ) {
                 params += "&QUERY_FROM=";
                 params += NStr::IntToString(range.GetFrom() + 1);
                 params += "&QUERY_TO=";
                 params += NStr::IntToString(range.GetTo() + 1);
             }
             if (is_na) {
                 name += "Genomic";
                 extra_params +=
                     "PAGE=Nucleotides&"
                     "PROGRAM=blastn&"
                     "MEGABLAST=on&"
                     "BLAST_PROGRAMS=megaBlast&"
                     "PAGE_TYPE=BlastSearch&"
                     "SHOW_DEFAULTS=on";

             } else {
                 name += "protein";
                 extra_params +=
                     "PAGE=Proteins&"
                     "PROGRAM=blastp&"
                     "BLAST_PROGRAMS=blastp&"
                     "PAGE_TYPE=BlastSearch&"
                     "SHOW_DEFAULTS=on";
             }
             url = url + params + extra_params;
             x_AddLink(links, type, name, label, url, relative);

             // add a link for genome-specific BLAST
             if (genome_specific_search) {
                 if (!is_na) {
                     params += "&PROGRAM=tblastn";
                 }
                 name = "BLAST Genome-specific";
                 label = source_id_str + range_str;
                 url = "/genome/seq/BlastGen/BlastGen.cgi?taxid=";
                 url += NStr::IntToString(tax_id) + "&" + params;
                 x_AddLink(links, type, name, label, url, relative);
             }
         }}
    }


    if (feat.IsSetProduct() && product_idh.Which() != CSeq_id::e_Local) {
        string product_db;
        if (is_product_na) {
            product_db = "/nuccore/";
        } else {
            product_db = "/protein/";
        }

        ///
        /// add links to self - that is, a link to view the product sequence
        ///
        typedef pair<string, string> TPair;
        const TPair sc_Pairs[] = {
            TPair("GenBank View", "report=genbank"),
            TPair("FASTA View", "report=fasta"),
            TPair("Graphical View", "report=graph")
        };

        const size_t size = sizeof(sc_Pairs) / sizeof(TPair);

        for (size_t i = 0;  i < size;  ++i) {
            string tag = sc_Pairs[i].second;
            if ( !is_product_na ) {
                tag = NStr::Replace(tag, "bank", "pept");
            }

            url = product_db + product_id_str + "?" + tag;
            x_AddLink(links, type, sc_Pairs[i].first, product_id_str, url, relative);
        }

        ///
        /// add a link to BLAST the product
        ///
        {{
            name = "BLAST ";
            string params("QUERY=" + product_id_str);
            string extra_params("&");
            if (is_product_na) {
                name += "mRNA";
                extra_params += "PAGE=Nucleotides&"
                    "PROGRAM=blastn&"
                    "MEGABLAST=on&"
                    "BLAST_PROGRAMS=megaBlast&"
                    "PAGE_TYPE=BlastSearch&"
                    "SHOW_DEFAULTS=on";
            } else {
                name += "Protein";
                extra_params += "PAGE=Proteins&"
                    "PROGRAM=blastp&"
                    "BLAST_PROGRAMS=blastp&"
                    "PAGE_TYPE=BlastSearch&"
                    "SHOW_DEFAULTS=on";
            }

            url = "/blast/Blast.cgi?" + params + extra_params;
            x_AddLink(links, type, name, product_id_str, url, relative);

            // add a link for genome-specific BLAST
            if (genome_specific_search) {
                if (!is_product_na) {
                    params += "&PROGRAM=tblastn";
                }
                name = "BLAST Genome-specific";
                url = "/genome/seq/BlastGen/BlastGen.cgi?taxid=";
                url += NStr::IntToString(tax_id) + "&" + params;
                x_AddLink(links, type, name, product_id_str, url, relative);
            }
        }}
    }

    ///
    /// type-specific links:
    ///


    ///
    /// process dbxrefs
    ///
    x_AddDbxrefFeatureLinks(feat, loc, scope, bsh, tax_id, links, true, relative);

    // extract links stored in seq-feat::exts
    // example ASN seq-feat that contains links:
    // seq-feat ::= {
    //   ...
    //   exts {
    //      {
    //        type str "links",
    //        data {
    //          {
    //           label str "GRC: HG-980",
    //           data str "projects/genome/assembly/grc/issue_detail.cgi?ID=HG-980"
    //          }
    //        }
    //      }
    //   }
    if (feat.IsSetExts()) {
        ITERATE (CSeq_feat::TExts, iter, feat.GetExts()) {
            if ( (*iter)->GetType().IsStr()  &&
                NStr::EqualNocase((*iter)->GetType().GetStr(), "links") ) {
                ITERATE (CUser_object::TData, link_iter, (*iter)->GetData()) {
                    if ((*link_iter)->GetData().IsStr()) {
                        url = (*link_iter)->GetData().GetStr();
                        if ( !url.empty()  &&  (*link_iter)->GetLabel().IsStr()) {
                            label = (*link_iter)->GetLabel().GetStr();
                            name = kEmptyStr;
                            size_t pos = label.find_first_of(":");
                            if (pos != string::npos) {
                                name = NStr::TruncateSpaces(label.substr(0, pos));
                                label = NStr::TruncateSpaces(label.substr(pos + 1));
                            }
                            if ( !NStr::StartsWith(url, "http://")   &&  url[0] != '/') {
                                url = "/" + url;
                            }

                            x_AddLink(links, type, name, label, url, relative);
                        }
                    }
                }
                break;
            }
        }
    }

    if (feat.GetData().Which() == CSeqFeatData::e_Variation  &&
        feat.GetData().GetVariation().CanGetId()  &&
        feat.GetData().GetVariation().GetId().GetDb() == "dbVar") {
        const CVariation_ref& var = feat.GetData().GetVariation();
        label = "";
        if (var.CanGetId()  &&  var.GetId().GetTag().IsStr()  &&
            var.GetId().GetTag().GetStr().find("sv") == 1) {
            CLabel::GetLabel(feat, &label, CLabel::eContent, &scope);
        } else if (var.CanGetParent_id()) {
            if (var.GetParent_id().GetTag().IsId()) {
                label = NStr::IntToString(var.GetParent_id().GetTag().GetId());
            } else {
                label = var.GetParent_id().GetTag().GetStr();
            }
        }

        if ( !label.empty() ) {
            name = "dbVar";
            url = "/dbvar/variants/" + label;
            x_AddLink(links, type, name, label, url, relative);
        }

        if (feat.IsSetExts()) {
            ITERATE (CSeq_feat::TExts, iter, feat.GetExts()) {

                if ((*iter)->GetType().IsStr()  &&
                    !(*iter)->GetData().empty()) {
                    name = "Other Variant Calls from this Sample";
                    if ((*iter)->GetType().GetStr() == "related calls") {
                        const CUser_object::TData& fields = (*iter)->GetData();
                        if (fields.size() < 5) {
                            ITERATE (CUser_object::TData, f_iter, fields) {
                                label = (*f_iter)->GetLabel().GetStr();
                                url = "/dbvar/variants/" + (*f_iter)->GetData().GetStr();
                                x_AddLink(links, type, name, label, url, relative);
                            }
                        } else if (var.CanGetSample_id()) {
                            string term;
                            if (var.GetSample_id().IsStr()) {
                                term = var.GetSample_id().GetStr();
                            } else {
                                term = NStr::NumericToString(var.GetSample_id().GetId());
                            }
                            CConstRef<CDbtag> study_id = feat.GetNamedDbxref("study_accession");
                            if (study_id) {
                                term += " and " + study_id->GetTag().GetStr();
                            }

                            label = NStr::NumericToString(fields.size());
                            url = "/dbvar/?term=" + term;
                            x_AddLink(links, type, name, label, url, relative);
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
                            CConstRef<CDbtag> study_id = feat.GetNamedDbxref("study_accession");
                            if (study_id) {
                                term += " and " + study_id->GetTag().GetStr();
                            }
                            url = "/dbvar/?term=" + term;
                            x_AddLink(links, type, name, label, url, relative);
                        }

                        break;
                    }
                }
            }
        }
    }
}

void CLinkUtils::AddBlastProductLink(const CSeq_feat& feat,
    CScope& scope,
    TLinksMap& links,
    bool relative)
{
    if (!feat.IsSetProduct()) {
        return;
    }

    /// grab the product location identifier
    string product_id_str;
    bool is_product_na = false;
    CSeq_id_Handle product_idh = sequence::GetIdHandle(feat.GetProduct(), &scope);
    CBioseq_Handle bsh = scope.GetBioseqHandle(product_idh);
    if (bsh) {
        is_product_na = bsh.IsNa();

        product_idh = sequence::GetId(product_idh, scope, sequence::eGetId_Best);
        CLabel::GetLabel(*product_idh.GetSeqId(), &product_id_str, CLabel::eDefault, &scope);
    }

    if (!product_idh && product_idh.Which() == CSeq_id::e_Local)
        return;

    string type = "Extra";
    string name = "BLAST ";
    string label = "";
    string url = "";
    
    string params("QUERY=" + product_id_str);
    string extra_params("&");
    if (is_product_na) {
        name += "mRNA";
        extra_params += "PAGE=Nucleotides&"
            "PROGRAM=blastn&"
            "MEGABLAST=on&"
            "BLAST_PROGRAMS=megaBlast&"
            "PAGE_TYPE=BlastSearch&"
            "SHOW_DEFAULTS=on";
    }
    else {
        name += "Protein";
        extra_params += "PAGE=Proteins&"
            "PROGRAM=blastp&"
            "BLAST_PROGRAMS=blastp&"
            "PAGE_TYPE=BlastSearch&"
            "SHOW_DEFAULTS=on";
    }

    url = "/blast/Blast.cgi?" + params + extra_params;
    x_AddLink(links, type, name, product_id_str, url, relative);
}

void CLinkUtils::AddDbxrefFeatureLinks(const CSeq_feat& feat,
    const CBioseq_Handle& handle,
    TLinksMap& links,
    bool add_parent_gene_dbxrefs,
    bool relative)
{
    const CSeq_feat_Base::TLocation& loc_obj = feat.GetLocation();
    CConstRef<CSeq_loc> loc;

    /// guard against the cases where the feature contains location
    /// with multiple seq-ids
    if (!loc_obj.GetId()) {
        loc = CSeqUtils::MixLocToLoc(loc_obj, handle);
    }

    if (!loc) {
        loc.Reset(&loc_obj);
    }

    AddDbxrefFeatureLinks(feat, *loc, handle.GetScope(), links, add_parent_gene_dbxrefs, relative);
}

void CLinkUtils::AddDbxrefFeatureLinks(const CSeq_feat& feat,
    const CSeq_loc& loc,
    CScope& scope,
    TLinksMap& links,
    bool add_parent_gene_dbxrefs,
    bool relative)
{
    /// grab the source location identifier and range
    CBioseq_Handle bsh = scope.GetBioseqHandle(*loc.GetId());
    CSeq_id_Handle source_idh =
        sequence::GetIdHandle(loc, &scope);
    source_idh = sequence::GetId(source_idh, scope,
        sequence::eGetId_Best);
    if (!source_idh || source_idh.GetSeqId()->IsLocal()) {
        return;
    }

    int tax_id = sequence::GetTaxId(bsh);
    x_AddDbxrefFeatureLinks(feat, loc, scope, bsh, tax_id, links, add_parent_gene_dbxrefs, relative);
}

void CLinkUtils::AddDbxrefLinksForSNP(const CSeq_feat& feat,
                                      const CRef<CDbtag>& dbt,
                                      CScope& scope,
                                      TLinksMap& links,
                                      bool relative)
{
    if (dbt) {
        string rsid{NStr::NumericToString(NSnp::GetRsid(*dbt))};

        ///
        /// link for SNP summary
        ///
        string type = "Basic";
        string name = "SNP summary";
        string label = "rs" + rsid;
        string url = "/snp/rs" + rsid;
        x_AddLink(links, type, name, label, url, relative);

        CSnpBitfield bf(feat);
        if(bf.isGood()) {
            type = "Extra";

            ///
            /// link for Genotype information
            ///
            if (bf.IsTrue(CSnpBitfield::eHasGenotype)) {
                name = "Genotype information";
                url = "/snp/rs" + rsid + "#frequency_tab";
                x_AddLink(links, type, name, label, url, relative);
            }

            ///
            /// link for SNP3D web page
            ///
            if (bf.IsTrue(CSnpBitfield::eHasSnp3D)) {
                name = "SNP3D Page";
                url = "/projects/SNP/snp3D.cgi?rsnum=" + rsid;
                x_AddLink(links, type, name, label, url, relative);
            }

            ///
            /// link for OMIM page for this SNP
            ///
            if (bf.IsTrue(CSnpBitfield::eHasOMIM_OMIA)) {
                name = "OMIM information";
                url = "/projects/SNP/snp_redirect.cgi?snp_id=" + rsid;
                x_AddLink(links, type, name, label, url, relative);
            }

            // VarVu link for clinical SNPs
            if(bf.IsTrue(CSnpBitfield::eIsClinical)) {
                // try to match genes to the current position
                SAnnotSelector sel_gene(CSeqUtils::GetAnnotSelector());
                sel_gene.IncludeFeatSubtype(CSeqFeatData::eSubtype_gene);

                if(feat.CanGetLocation()) {
                    CSeq_loc location;

                    location.Assign(feat.GetLocation());
                    // allow matching on any strand (SV-3025)
                    location.SetStrand(eNa_strand_both);
                    CFeat_CI feat_iter(scope, location, sel_gene);

                    CConstRef<CSeq_feat> mapped_feat;
                    for ( ;  feat_iter;  ++feat_iter) {
                        mapped_feat.Reset(&feat_iter->GetMappedFeature());
                        string sGeneLocus;
                        if(mapped_feat->GetData().Which() == CSeqFeatData::e_Gene  &&
                            mapped_feat->GetData().GetGene().IsSetLocus()) {
                        sGeneLocus = mapped_feat->GetData().GetGene().GetLocus();
                            }
                            if( !sGeneLocus.empty() ) {
                                name = "Variation viewer";
                                label += " (" + sGeneLocus + ")";
                                url = "/sites/varvu?rs=" + rsid + "&gene=" + sGeneLocus;
                                x_AddLink(links, type, name, label, url, relative);
                            }
                    }
                }
            }
        }
    }
}


void CLinkUtils::x_AddLink(TLinksMap& links,
                           const string& type, const string& name,
                           const string& label, const string& url,
                           bool relative)
{
    string edited_url(url);
    if (!relative) {
        if (!NStr::StartsWith(url, "http://") && !NStr::StartsWith(url, "https://")) {
            edited_url = kCommonUrl + url;
        }
    }
    else {
        if (NStr::StartsWith(url, kCommonUrl)) {
            edited_url = NStr::Replace(url, kCommonUrl, "");
        }
    }

    ((links[type])[name]).push_back(std::pair<string, string>(label, edited_url));
}

void CLinkUtils::x_AddDbxrefFeatureLinks(const CSeq_feat& feat,
    const CSeq_loc& loc,
    CScope& scope,
    CBioseq_Handle& bsh,
    int tax_id,
    TLinksMap& links,
    bool add_parent_gene_dbxrefs,
    bool relative)
{
    string type = "Basic";
    string name = "";
    string label = "";
    string url = "";
    if (feat.IsSetDbxref()) {
        const CSeq_feat::TDbxref& refs = feat.GetDbxref();
        ITERATE(CSeq_feat::TDbxref, iter, refs) {
            switch ((*iter)->GetType()) {
            case CDbtag::eDbtagType_dbSNP:
                AddDbxrefLinksForSNP(feat, *iter, scope, links, relative);
                break;
            default:
                name = "View";
                string label_str;
                (*iter)->GetLabel(&label_str);
                size_t pos = label_str.find(":");
                if (pos == NPOS) {
                    pos = label_str.rfind(" ");
                }

                if (pos != NPOS) {
                    name += " " + label_str.substr(0, pos);
                    label = label_str.substr(pos + 1);
                }
                else {
                    label = label_str;
                }
                url = tax_id ? (*iter)->GetUrl(tax_id) : (*iter)->GetUrl();
                NStr::TruncateSpacesInPlace(label);
                if (label.empty() || url.empty()) {
                    break;
                }

                if (relative && NStr::StartsWith(url, kCommonUrl)) {
                    NStr::ReplaceInPlace(url, kCommonUrl, "");
                }

                if ((*iter)->GetType() == CDbtag::eDbtagType_GeneID) {
                    // Here we get Gene ID for a feature, so we can add Gene
                    // Symbol. Unfortunately, Gene Symbol is valid only for
                    // a feature of subtype eSubtype_gene, so we get best
                    // gene here first.
                    CConstRef<CSeq_feat> gene_symbol_feat(&feat);
                    // First attempt - through GetBestGeneFor*
                    switch (feat.GetData().GetSubtype()) {
                    case CSeqFeatData::eSubtype_gene:
                        break;
                    case CSeqFeatData::eSubtype_cdregion:
                        gene_symbol_feat =
                            GetBestGeneForCds(feat, scope);
                        break;
                    case CSeqFeatData::eSubtype_mRNA:
                        gene_symbol_feat =
                            GetBestGeneForMrna(feat, scope);
                        break;
                    default:
                        gene_symbol_feat = NULL;
                        break;
                    }
                    if (gene_symbol_feat.Empty()) {
                        // Second attempt, if GetBestGeneFor*
                        // did not find anything, or feature is not CDS or
                        // mRNA and not a gene - get gene with this GeneID
                        int gene_id = (*iter)->GetTag().GetId();
                        SAnnotSelector sel;
                        sel.SetResolveAll()
                            .IncludeFeatSubtype(CSeqFeatData::eSubtype_gene);
                        for (CFeat_CI feat_iter(bsh, loc.GetTotalRange(), sel);
                            feat_iter;
                            ++feat_iter)
                        {
                            CConstRef<CDbtag> tag =
                                feat_iter->GetNamedDbxref("GeneID");
                            if (tag  &&  tag->GetTag().GetId() == gene_id) {
                                gene_symbol_feat = feat_iter->GetSeq_feat();
                                break;
                            }
                        }
                    }
                    if (gene_symbol_feat.NotEmpty()) {
                        string gene_symbol;
                        feature::GetLabel(gene_symbol_feat.GetObject(),
                            &gene_symbol, feature::fFGL_Content);
                        if (!gene_symbol.empty()) {
                            label += " (" + gene_symbol + ")";
                        }
                        if (add_parent_gene_dbxrefs && (CSeqFeatData::eSubtype_gene != feat.GetData().GetSubtype())) {
                            // Add Dbxrefs from the parent gene
                            AddDbxrefFeatureLinks(gene_symbol_feat.GetObject(), loc, scope, links, add_parent_gene_dbxrefs, relative);
                        }
                    }
                }

                x_AddLink(links, type, name, label, url, relative);
                break;
            }
        }
    }
}

END_NCBI_SCOPE
