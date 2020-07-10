/*  $Id: assembly_info.cpp 42492 2019-03-12 18:49:27Z shkeda $
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
 * Author:  Liangshou Wu, Andrei Shkeda
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <objmgr/util/sequence.hpp>
#include <misc/xmlwrapp/xmlwrapp.hpp>
#include <util/xregexp/regexp.hpp>

#include <gui/objutils/gui_eutils_client.hpp>
#include <gui/objutils/gencoll_svc.hpp>
#include <gui/objutils/entrez_conversion.hpp>

#include <gui/objects/DL_Assembly.hpp>
#include <gui/objects/DL_TaxId.hpp>

#include <util/icanceled.hpp>
#include <gui/objects/assembly_info.hpp>

 #include <objects/genomecoll/GCClient_EquivalentAssembl.hpp>
 #include <objects/genomecoll/GCClient_GetEquivalentAsse.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


static const string kTaxDb = "taxonomy";
static const string kNucDb = "nucleotide";
static const string kAssmDb = "assembly";
static const int kRetMax = 5000;

CRef<CDL_AssmInfo> CAssemblyInfo::GetAssms_Gi(TGi gi,
                                              const string& release_type)
{
    CSeqUtils::TGis uids_from;
    uids_from.push_back(gi);
    CSeqUtils::TUids uids_to;

    try {
        CSeqUtils::ELinkQuery(kNucDb, kAssmDb, uids_from, uids_to);
    }
    catch (CException& e) {
        LOG_POST(Error << "Failed to get assembly entrez ids for gi: " << gi << ". Error: " << e.GetMsg());
        throw;
    }

    if (uids_to.empty()) {
        NCBI_THROW(CException, eUnknown, "Can't find the associated assembly for the given seq-id!");
    }

    // get all assembly info for the list of GC links
    CDL_AssmInfo::TAssemblies assms;
    x_GetAssmInfo(uids_to, assms);

    // extract the most recent version of assembly with the given release type
    x_FilterAssemblies(assms, release_type);

    return x_CreateAssmInfo(assms);
}


CRef<CDL_AssmInfo> CAssemblyInfo::GetAssms_TaxId(const string& tax_id,
                                               const string& release_type)
{
    if ( !x_ValidateTaxId(tax_id) ) {
        string msg = "Invalid tax-id: " + tax_id;
        NCBI_THROW(CException, eUnknown, msg);
    }

    // search all GC links associated with the tax-id filtered
    // by assembly release type
    string term = tax_id + "[taxid]";
    CSeqUtils::TUids uids;
    x_SearchIds(term, kAssmDb, uids);
    if (uids.empty()) {
        string msg = "Can't find the associated assembly for tax-id: " + tax_id;
        NCBI_THROW(CException, eUnknown, msg);
    }

    // get all assembly info for the list of GC links
    CDL_AssmInfo::TAssemblies assms;
    x_GetAssmInfo(uids, assms);
    x_FilterAssemblies(assms, release_type);

    return x_CreateAssmInfo(assms);
}


CRef<CDL_AssmInfo> CAssemblyInfo::GetAssms_Term(const string& term,
                                               const string& release_type,
                                               ICanceled* cancel)
{
    if (NStr::StringToInt(term, NStr::fConvErr_NoThrow) > 0) {
        // maybe, the term stores a tax-id
        return GetAssms_TaxId(term, release_type);
    }

    CRef<CDL_AssmInfo> assm_info;

    // Search the tax-id using the search term
    CSeqUtils::TUids uids;
    x_SearchIds(term, kTaxDb, uids);
    if (uids.empty()) {
        // Let try esearch in gencoll db directly.
        // This will work for 'term=' something like hg18 or grch37
        x_SearchIds(term, kAssmDb, uids);
        if ( !uids.empty() ) {
            // get all assembly info for the list of GC links
            CDL_AssmInfo::TAssemblies assms;
            x_GetAssmInfo(uids, assms);
            x_FilterAssemblies(assms, release_type);
            assm_info = x_CreateAssmInfo(assms);
            return assm_info;
        }
    }
    else {
        x_FilterTaxIds(uids);
    }

    if (uids.empty() ) {
        // modify the search term a bit to give it a second try
        x_SearchIds(term + "[orgn:exp]", kTaxDb, uids);
        if (!uids.empty()) {
            x_FilterTaxIds(uids);
        }
    }

    CDL_AssmInfo::TAssemblies assms;
    x_GetAssmInfo(uids, assms);
    if (assms.empty())
    {
        string msg = "Can't find any assembly related to \"" + term + "\"";
        NCBI_THROW(CException, eUnknown, msg);
    }

    assm_info = x_CreateAssmInfo(assms);
    return assm_info;
}


bool CAssemblyInfo::x_ValidateTaxId(const string& tax_id)
{
    CGuiEutilsClient ecli;

    string term(tax_id);
    term += "[taxid]";

    Uint8 count(0);

    try {
        count = ecli.Count(kTaxDb,term);
    } catch (CException& e) {
        LOG_POST(Error << "Failed to search for tax id: " << tax_id << ". Error: " << e.GetMsg());
    }

    return (count ? true : false);
}

void CAssemblyInfo::x_FilterTaxIds(CSeqUtils::TUids &uids)
{
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);

    string term(x_CreateIdStr(uids));
    term += "[taxid]";
    uids.clear();
    try {
        ecli.Search(kAssmDb, term, uids);
    }
    catch (CException& e) {
        LOG_POST(Error << "Get error when trying to search assembly for tax ids: " << x_CreateIdStr(uids) << ". Error: " << e.GetMsg());
    }
}

void CAssemblyInfo::x_SearchIds(const string& term,
                                const string& db,
                                CSeqUtils::TUids &uids)
{
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);

    try {
        ecli.Search(db, term, uids);
    } catch (CException& e) {
        LOG_POST(Error << "Get error when trying to search records on db: "
                 << db << " with search term: "
                 << term << ". Error: " << e.GetMsg());
    }
}

void CAssemblyInfo::x_GetAssmInfo(const CSeqUtils::TUids &uids, CDL_AssmInfo::TAssemblies &assms)
{
    xml::document docsums;
    CGuiEutilsClient ecli;
    ecli.SetMaxReturn(kRetMax);

    try {
        ecli.Summary(kAssmDb, uids, docsums);
        xml::node_set doc_sums ( docsums.get_root_node().run_xpath_query("//DocumentSummary[AssemblyAccession/text()][AssemblyName/text()]") );
        xml::node_set::const_iterator itDocSum;
        for (itDocSum = doc_sums.begin(); itDocSum != doc_sums.end(); ++itDocSum) {
            CRef<CDL_Assembly> assm(new CDL_Assembly);
            xml::node::const_iterator a_acc = itDocSum->find("AssemblyAccession");
            xml::node::const_iterator a_name = itDocSum->find("AssemblyName");
            assm->SetName(a_name->get_content());
            assm->SetAccession(a_acc->get_content());
            if (NStr::StartsWith(a_acc->get_content(), "GCF_", NStr::eNocase)) {
                assm->SetRelease_type("refseq");
            } else {
                assm->SetRelease_type("genbank");
            }
            ITERATE (xml::node, sub_iter, *itDocSum) {
                string n_name = sub_iter->get_name();
                string n_value = sub_iter->get_content();
                if (n_value.empty()) continue;

                if (NStr::EqualNocase(n_name, "Organism")) {
                    assm->SetOrganism(n_value);
                } else if (NStr::EqualNocase(n_name, "AssemblyClass")) {
                    assm->SetClass(n_value);
                } else if (NStr::EqualNocase(n_name, "AssemblyDescription")) {
                    assm->SetDescr(n_value);
                } else if (NStr::EqualNocase(n_name, "NCBIReleaseDate")) {
                    assm->SetRelease_date(n_value);
                } else if (NStr::EqualNocase(n_name, "UCSCName")) {
                    assm->SetOther_names().push_back("UCSC name: " + n_value);
                } else if (NStr::EqualNocase(n_name, "EnsemblName")) {
                    assm->SetOther_names().push_back("Ensembl name: " + n_value);
                }
            }
            assms.push_back(assm);

            // Using "gcassembly" Entrez, you will not get the the GB assemblies
            // for some cases (those paired to RS assemblies) as separate entries
            // from an Entrez search. But if you are looking at the Entrez Docsums,
            // there is a field "GbUid" that tells you the release id of the
            // corresponding GB, and fields <Synonym>/<Genbank> and <Synonym>/<RefSeq>
            // will tell you the accessions of both of them. (JIRA: GCOL-1493)
            if (assm->GetRelease_type() == "genbank") {
                // skip further searching if it is already a GenBank assembly.
                continue;
            }

            xml::node::const_iterator gbid_node = itDocSum->find("GbUid");
            if (gbid_node != itDocSum->end()  &&  !string(gbid_node->get_content()).empty()) {
                xml::node::const_iterator syn_node = itDocSum->find("Synonym");
                if (syn_node != itDocSum->end()) {
                    xml::node::const_iterator gb_assm_node = syn_node->find("Genbank", syn_node->begin());
                    if (gb_assm_node != syn_node->end()) {
                        CRef<CDL_Assembly> gb_assm(new CDL_Assembly);
                        gb_assm->SetName(assm->GetName());
                        gb_assm->SetAccession(gb_assm_node->get_content());
                        gb_assm->SetRelease_type("genbank");
                        if (assm->IsSetOrganism()) {
                            gb_assm->SetOrganism(assm->GetOrganism());
                        }
                        if (assm->IsSetClass()) {
                            gb_assm->SetClass(assm->GetClass());
                        }
                        if (assm->IsSetDescr()) {
                            gb_assm->SetDescr(assm->GetDescr());
                        }
                        if (assm->IsSetRelease_date()) {
                            gb_assm->SetRelease_date(assm->GetRelease_date());
                        }
                        if (assm->IsSetOther_names()) {
                            gb_assm->SetOther_names() = assm->GetOther_names();
                        }

                        assms.push_back(gb_assm);
                    }
                }
            }
        }
    } catch (CException& e) {
        LOG_POST(Error << "Failed to get summary for the following assemblies: " << x_CreateIdStr(uids) << ". Error: " << e.GetMsg());
    }
}


void CAssemblyInfo::x_FilterAssemblies(CDL_AssmInfo::TAssemblies& assms,
                                     const string& release_type)
{
    // do filtering based on release type
    if ( !release_type.empty()) {
        CDL_AssmInfo::TAssemblies::iterator iter = assms.begin();
        while (iter != assms.end()) {
            if ((*iter)->CanGetRelease_type()  &&
                (*iter)->GetRelease_type() != release_type) {
                iter = assms.erase(iter);
            } else {
                ++iter;
            }
        }
    }
}


static bool s_AssmCompare(const CRef<CDL_Assembly>& assm1, const CRef<CDL_Assembly>& assm2)
{
    if (assm2->CanGetRelease_date() && assm1->CanGetRelease_date()) 
        return assm2->GetRelease_date() < assm1->GetRelease_date();
        
    if (assm2->CanGetRelease_date() && !assm1->CanGetRelease_date())
        return true;

    return false;
}


void CAssemblyInfo::x_SortAssms(CDL_AssmInfo::TAssemblies& assms)
{
    if (assms.size() < 2) return;

    // sort assembly according to the release date
    assms.sort(s_AssmCompare);

    // for certain known organisms, we promote a specific assembly to the top
    // human: Homo sapiens: GCF_0000001405
    // mouse: Mus musculus: GCF_000001635
    // cow:   Bos taurus:   GCF_000003205
    // rat:   Rattus norvegicus: GCF_000001895
    typedef map<string, string> TAssmMap;
    TAssmMap a_map;
    a_map["Homo sapiens"] = "GCF_000001405";
    a_map["Mus musculus"] = "GCF_000001635";
    a_map["Bos taurus"] = "GCF_000003205";
    a_map["Rattus norvegicus"] = "GCF_000001895";

    CDL_AssmInfo::TAssemblies::iterator iter = assms.begin();

    if ((*iter)->CanGetOrganism()) {
        TAssmMap::const_iterator a_iter = a_map.find((*iter)->GetOrganism());
        if (a_iter != a_map.end()) {
            while (iter != assms.end()  &&
                   (*iter)->GetAccession().find(a_iter->second) == string::npos) {
                ++iter;
            }

            if (iter != assms.end()  &&  iter != assms.begin()) {
                CRef<CDL_Assembly> assm = *iter;
                assms.erase(iter);
                assms.push_front(assm);
            }
        }
    }
}

string CAssemblyInfo::x_CreateIdStr(const CSeqUtils::TUids &uids)
{
    stringstream idstrm;
    size_t count = uids.size();
    if (count) {
        idstrm << uids[0];
        for (size_t i=1; i<count; ++i)
            idstrm << ',' << uids[i];
    }
    return idstrm.str();
}

CRef<CDL_AssmInfo> CAssemblyInfo::x_CreateAssmInfo(CDL_AssmInfo::TAssemblies& assms)
{
    CRef<CDL_AssmInfo> assm_info;
    if ( !assms.empty() ) {
        x_SortAssms(assms);
        assm_info.Reset(new CDL_AssmInfo());
        assm_info->SetAssemblies() = assms;
    }

    return assm_info;
}

static void s_GetAssmEquiv(CGenomicCollectionsService& gc, const string& acc, set<string>& equiv_accs) 
{
    equiv_accs.clear();
    CRef<CGCClient_EquivalentAssemblies> equiv_assemblies = gc.GetEquivalentAssemblies(acc, CGCClient_GetEquivalentAssembliesRequest::eEquivalency_all_types_same_coordinates);
    if (equiv_assemblies && equiv_assemblies->CanGetAssemblies()) {
        ITERATE(CGCClient_EquivalentAssemblies::TAssemblies, it, equiv_assemblies->GetAssemblies()) {
            equiv_accs.insert((*it)->GetAccession());
        }
    }
}

static bool s_IsAsmAccEquiv(CGenomicCollectionsService& gc, const string& acc1, const string &acc2)
 {
     if (NStr::EqualNocase(acc1, acc2))
         return true;
     try {
         set<string> equiv_accs;
         s_GetAssmEquiv(gc, acc1, equiv_accs);
        auto it = equiv_accs.find(acc2);
        return it != equiv_accs.end();
     } catch (exception& e) {
         LOG_POST(Error << e.what());
     }
     return false;
 }
 
bool CAssemblyInfo::IsSameAssembly(const CGC_Assembly& gc_assm, const string& term)
{
    _ASSERT(!term.empty());
    if (term.empty())
        return false;
    const string& assm_acc = gc_assm.GetAccession();
    if (assm_acc == term)
        return true;
    if (gc_assm.GetName() == term)
        return true;
    bool res = false;
    CRegexp regexGenAcc("GC(A|F)_\\d{9}(.\\d+)");
    // if term is GenColl accession just check the equivalent accessions
    if (regexGenAcc.IsMatch(term)) {
        CRef<CGenomicCollectionsService> gencoll_service(CGencollSvc::GetGenCollService());
        return s_IsAsmAccEquiv(*gencoll_service, term, assm_acc);
    }
    bool equiv_acc_fetched = false;
    set<string> equiv_accs;
    string s_term = term;
    NStr::ReplaceInPlace(s_term, "\"", "");
    NStr::ReplaceInPlace(s_term, "*", "");
    string query("\"");
    query += term;
    query += "\"[All+Names]";
    xml::document docsums;
    size_t total_uids = 0;
    CEntrezDB::Query(kAssmDb, query, total_uids, docsums, kRetMax);
    xml::node_set nodes ( docsums.get_root_node().run_xpath_query("//DocumentSummary") );
	NON_CONST_ITERATE(xml::node_set, it, nodes) {
        xml::node::const_iterator itAccession = it->find("AssemblyAccession");
        if (itAccession == it->end())
            continue;
        const string& acc = itAccession->get_content();
        if (assm_acc == acc) {
            res = true;
            break;
        }
        if (equiv_acc_fetched == false) {
            CRef<CGenomicCollectionsService> gencoll_service(CGencollSvc::GetGenCollService());
            s_GetAssmEquiv(*gencoll_service, assm_acc, equiv_accs); 
            equiv_acc_fetched = true;
        }
        if (equiv_accs.count(acc) > 0) {
            res = true;
            break;
        }
    }
    return res;
} 

END_NCBI_SCOPE
