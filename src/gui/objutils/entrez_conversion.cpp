/*  $Id: entrez_conversion.cpp 42427 2019-02-21 18:16:11Z ivanov $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *     */

#include <ncbi_pch.hpp>

#include <gui/objutils/entrez_conversion.hpp>
#include <corelib/ncbitime.hpp>
#include <util/static_map.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/gui_eutils_client.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

typedef SStaticPair<const char*, const char*> TPair;
static const TPair sc_Dbs[] = {
	{ "assembly",	"Genome Assembly" },
	{ "gene",       "Gene" },
    { "nucleotide", "Nucleotide" },
    { "protein",    "Protein" },
};
typedef CStaticPairArrayMap<const char*, const char*, PNocase_CStr> TEntrezDbMap;
DEFINE_STATIC_ARRAY_MAP(TEntrezDbMap, sc_EntrezDbMap, sc_Dbs);

static const TPair sc_Pairs[] = {
    { "genomeprj_genome",          "Chromosomes" },
    { "genomeprj_nucleotide",      "Contigs" },
    { "genomeprj_nucleotide_mrna", "mRNAs" },
    { "genomeprj_nucleotide_wgs",  "Whole Genome Shotgun sequences" },
    { "genomeprj_organella",       "Organelles" },
    { "genomeprj_protein",         "Proteins" },
};

typedef CStaticPairArrayMap<const char*, const char*, PNocase_CStr> TGenomeLinkMap;
DEFINE_STATIC_ARRAY_MAP(TGenomeLinkMap, sc_GenomeLinkMap, sc_Pairs);

class CXmlNodeObjectLabelHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* str,
                  CLabel::ELabelType type, objects::CScope* scope) const
    {
        _ASSERT(str);
        _ASSERT(scope);
        const CXmlNodeObject* xmlNode = dynamic_cast<const CXmlNodeObject*>(&obj);
        if (xmlNode) {
            switch (type) {
            case CLabel::eUserType:
                *str += "Entrez DocumentSummary Record";
                break;

            case CLabel::eUserSubtype:
                *str += kEmptyStr;
                break;

            case CLabel::eType:
                *str += "Entrez Record";
                break;

            case CLabel::eDescriptionBrief:
            case CLabel::eDescription:
                {
                    // XPath query cannot be executed on a node
                    xml::node::const_iterator itDesc;
                    do {
                        itDesc = xmlNode->GetNode().find("AssemblyDescription");
                        if (itDesc != xmlNode->GetNode().end())
                            break;

                        itDesc = xmlNode->GetNode().find("Description");
                        if (itDesc != xmlNode->GetNode().end())
                            break;

                        itDesc = xmlNode->GetNode().find("Title");
                    }
                    while(false);
                    if (itDesc == xmlNode->GetNode().end())
                        break;
                    *str += itDesc->get_content();
                    break;
                }
            case CLabel::eContent:
                {
                    // XPath query cannot be executed on a node
                    xml::node::const_iterator itName;
                    do {
                        itName = xmlNode->GetNode().find("AssemblyName");
                        if (itName != xmlNode->GetNode().end())
                            break;

                        itName = xmlNode->GetNode().find("Name");
                        if (itName != xmlNode->GetNode().end())
                            break;

                        itName = xmlNode->GetNode().find("Caption");
                    }
                    while(false);
                    if (itName == xmlNode->GetNode().end())
                        break;
                    *str += itName->get_content();
                    break;
                }

            case CLabel::eUserTypeAndContent:
                {
                    GetLabel(obj, str, CLabel::eUserType, scope);
                    *str += "\n";
                    GetLabel(obj, str, CLabel::eContent, scope);
                    break;
                }
            default:
                break;
            }
        }
    }
};

void CEntrezDB::GetDbNames(vector<string>& names)
{
    ITERATE(TEntrezDbMap, it, sc_EntrezDbMap)   {
        names.push_back(it->first);
    }
}

void CEntrezDB::GetDbNames(vector<TNamePair>& name_pairs)
{
    ITERATE(TEntrezDbMap, it, sc_EntrezDbMap)   {
        name_pairs.push_back(TNamePair(it->first, it->second));
    }
}


string CEntrezDB::GetVisibleName(const string& db_name)
{
    TEntrezDbMap::const_iterator it = sc_EntrezDbMap.find(db_name.c_str());
    return (it == sc_EntrezDbMap.end()) ? kEmptyStr : it->second;
}


string CEntrezDB::GetDbName(const string& visible_name)
{
    ITERATE (TEntrezDbMap, iter, sc_EntrezDbMap) {
        if (iter->second == visible_name) {
            return iter->first;
        }
    }
    return kEmptyStr;
}

void CEntrezDB::Query(const string& db_name,
    const string& terms,
    size_t& total_uids,
    xml::document &docsums,
    size_t max_return)
{
    // Register 
    string type = typeid(CXmlNodeObject).name();
    if ( ! CLabel::HasHandler(type)) {
        CLabel::RegisterLabelHandler(type, *new CXmlNodeObjectLabelHandler());
    }

	// prepare eSearch request
    // uid can be GI or 32-bit id depending on db_name
    // always use TGi, since TGi fits for everything
    vector<TGi> uids;
    try {
		CSeqUtils::ESearchQuery(db_name, terms, uids, total_uids, (int) max_return);
    }
    catch (const CException& e) {
        LOG_POST(Error << "CEntrezDB::Query(): error executing search Entrez query: " << terms << ": " << e.GetMsg());
    }
    
    if (!total_uids)
        return;
    
    CGuiEutilsClient ecli;
	if (max_return)
		ecli.SetMaxReturn(max_return);

#ifdef NCBI_STRICT_GI
    vector<TEntrezId> entrez_uids;
    for (TGi gi : uids) {
        entrez_uids.push_back(GI_TO(TEntrezId, gi));
    }
#else
    const vector<TGi> &entrez_uids = uids;
#endif

    try {
        ecli.Summary(db_name, entrez_uids, docsums, "2.0");
    }
    catch (const CException& e) {
        LOG_POST(Error << "CEntrezDB::Query(): error executing summary Entrez query: " << terms << ": " << e.GetMsg());
    }
}


CRef<CSeq_annot> CEntrezDB::CreateAnnot_Nuc_Prot(const string& query_str,
                                                 const xml::node_set& ds_set)
{
    CRef<CSeq_annot> annot(new CSeq_annot());
    ITERATE (xml::node_set, iter, ds_set) {
        CRef<CSeq_id> id = CreateId_Nuc_Prot(*iter);
        annot->SetData().SetIds().push_back(id);
    }

    string title("Query: ");
    title += query_str;
    title += ", database: ";
    //TODO title += GetVisibleDbName();
    title += ": ";
    title += NStr::SizetToString(ds_set.size(), NStr::fWithCommas);
    title += " sequence";
    if (ds_set.size() != 1) {
        title += "s";
    }
    annot->SetNameDesc(title);
    annot->SetCreateDate(CTime(CTime::eCurrent));
    return annot;
}


CRef<CSeq_id> CEntrezDB::CreateId_Nuc_Prot(const xml::node& ds)
{
    CRef<CSeq_id> result;
    int gi = GetUidAttr(ds);
    if (0 != gi) {
        result.Reset(new CSeq_id());
        result->SetGi(GI_FROM(int, gi));
    }
    return result;
}


CRef<objects::CEntrezgene> CEntrezDB::CreateGene_Gene(const xml::node& ds)
{
    CRef<CEntrezgene> egene;

    int uid = GetUidAttr(ds);
    if (0 == uid)
        return egene;

    vector<int> uids;
    uids.push_back(uid);

    // prepare eFetch request
    CGuiEutilsClient ecli;

    CNcbiStrstream asnGene;

    egene.Reset(new CEntrezgene());
    try {
        ecli.Fetch("gene", uids, asnGene, "asn.1");
        asnGene >> MSerial_AsnText >> *egene;
    }
    catch (const CException& e) {
        LOG_POST(Error << "CEntrezDB::CreateGene_Gene(): error fetching ID " << uid << ": " << e.GetMsg());
        egene.Reset();
    }

    return egene;
}


CRef<CEntrezgene_Set> CEntrezDB::CreateGeneSet_Gene(const xml::node_set& ds_set)
{
    CRef<CEntrezgene_Set> gene_set;

    vector<int> uids;

    // prepare eFetch request
    CGuiEutilsClient ecli;

    CNcbiStrstream asnGeneSet;

    /// for each docsum, retrieve the Entrezgene object
    xml::node_set::const_iterator iter = ds_set.begin();
    do {
        int uid = GetUidAttr(*iter);
        if (0 != uid)
            uids.push_back(uid);

        ++iter;
        if (uids.size() % 5 == 4  ||  iter == ds_set.end()) {

            CNcbiStrstream asnGene;
            try {
                ecli.Fetch("gene", uids, asnGene, "asn.1");
            }
            catch (const CException& e) {
                LOG_POST(Error << "CEntrezDB_Gene::CreateGeneSet_Gene(): error retrieving IDs " << CSeqUtils::CreateIdStr(uids) << ": " << e.GetMsg());
                uids.clear();
                break;
            }

            while(true) {
                try {
                    CRef<CEntrezgene> egene(new CEntrezgene());

                    asnGene >> MSerial_AsnText >> *egene;

                    if ( !gene_set ) {
                        gene_set.Reset(new CEntrezgene_Set);
                    }
                    gene_set->Set().push_back(egene);
                }
                catch (const CEofException&) {
                    /// ignore end-of-file - we expect this on completion
                    break;
                }
                catch (const CException& e) {
                    LOG_POST(Error << "CEntrezDB_Gene::CreateGeneSet_Gene(): error retrieving IDs " << CSeqUtils::CreateIdStr(uids) << ": " << e.GetMsg());
                    break;
                }
                catch (const std::exception& e) {
                    LOG_POST(Error << "CEntrezDB_Gene::CreateGeneSet_Gene(): error retrieving IDs " << CSeqUtils::CreateIdStr(uids) << ": " << e.what());
                    break;
                }
            }
            uids.clear();
        }
    }
    while (iter != ds_set.end());
    if ( gene_set  &&  gene_set->IsSet()  &&  !gene_set->Get().size() ) {
        gene_set.Reset();
    }

    return gene_set;
}

int CEntrezDB::GetUidAttr(const xml::node& ds)
{
    xml::attributes::const_iterator itUid = ds.find_attribute("uid");
    if (itUid == ds.get_attributes().end())
        return 0;
    return NStr::StringToInt(itUid->get_value());
}

END_NCBI_SCOPE
