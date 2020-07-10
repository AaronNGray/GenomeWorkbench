/*  $Id: blast_databases.cpp 40278 2018-01-19 17:43:39Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

#include <set>
#include <corelib/ncbifile.hpp>

#include <gui/objutils/blast_databases.hpp>

#include <algo/blast/api/remote_blast.hpp>
#include <objects/blast/blastclient.hpp>
#include <objects/general/User_field.hpp>

#include <util/compress/stream_util.hpp>

#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/objutils/taxon_cache.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

DEFINE_STATIC_MUTEX(s_Mutex);

CBLASTDatabases& CBLASTDatabases::GetInstance()
{
    CMutexGuard LOCK(s_Mutex);
    static CRef<CBLASTDatabases> instance;
    if (!instance)
        instance.Reset(new CBLASTDatabases());
    return *instance;
}

CBLASTDatabases::CBLASTDatabases()
    : m_JobId(CAppJobDispatcher::eInvalidJobID)
    , m_State(eInitial)
{
}

const CBLASTDatabases::TDbMap& CBLASTDatabases::GetDbMap(bool nuc)
{
    if (m_State != eLoaded) {
        static TDbMap dummy;
        return dummy;
    }
    x_UpdateDbMap(nuc);
    return nuc ? m_NucDbMap : m_ProtDbMap; 
}

const vector<string>& CBLASTDatabases::GetDefaultMRU_DBs(bool nuc_db)
{
    if (m_State != eLoaded) {
        static vector<string> dummy;
        return dummy;
    }
    x_UpdateDbMap(nuc_db);
    return nuc_db ? m_Nuc_DefMRU_DBs : m_Prot_DefMRU_DBs;
}

static const char* kBlastDbDataMD5  = "blast_dbs.md5";
static const char* kBlastDbDataNucl = "blast_dbs.nucl";
static const char* kBlastDbDataProt = "blast_dbs.prot";
static const CCompressStream::EMethod kDbDataCompressMethod = CCompressStream::eLZO;

void CBLASTDatabases::x_UpdateDbMap(bool nuc) 
{
    TDbMap& db = nuc ? m_NucDbMap : m_ProtDbMap;

    if (db.empty()) {
        if (nuc) 
            m_NucDbMap.reserve(200000);

        string path = CDirEntry::ConcatPath(m_Path, nuc ? kBlastDbDataNucl : kBlastDbDataProt);

        CNcbiIfstream is(path.c_str(), ios::in | ios::binary);
        CDecompressIStream istr(is, kDbDataCompressMethod);
        string str_line;
        while (!istr.eof()) {
            str_line.clear();
            NcbiGetlineEOL(istr, str_line);
            NStr::TruncateSpacesInPlace(str_line, NStr::eTrunc_End);
            if (str_line.empty())
                continue; 
            string db_name, db_string;
            if (NStr::SplitInTwo(str_line, "\t", db_name, db_string)) {
                db.insert(TDbMap::value_type(db_name, db_string));

                // if db_string == "nr" or "nt" populate default mru values
                if (db_string.size() > 1 && db_string[0] == 'n' && (db_string[1] == 't' || db_string[1] == 'r')) {
                    vector<string>& mru_dbs = nuc ? m_Nuc_DefMRU_DBs : m_Prot_DefMRU_DBs;
                    mru_dbs.push_back(db_name);
                }
            }
        }
    }
}

BEGIN_EVENT_MAP( CBLASTDatabases, CEventHandler )
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged, &CBLASTDatabases::x_OnJobNotification)
END_EVENT_MAP()

void CBLASTDatabases::x_OnJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);
    if (notn->GetJobID() != m_JobId) return;

    switch (notn->GetState()) {
        case IAppJob::eCompleted:
            m_State = eLoaded;
            break;
        case IAppJob::eCanceled:
        case IAppJob::eFailed:
            m_State = eFailed;
            break;
        default:
            return;
    }

    m_JobId = CAppJobDispatcher::eInvalidJobID;
}

namespace {

class CNetBlastLoadDBListJob : public CJobCancelable
{
public:
    CNetBlastLoadDBListJob(const string& dir, const string& dbtree);
    ~CNetBlastLoadDBListJob() {}

    typedef unordered_map<string, string> TDbMap;

    /// @name IAppJob implementation
    /// @{
    virtual EJobState                   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

private:
    void    x_CategorizeDBs(CRef<CBlast4_get_databases_ex_reply>& dbs);
    void    x_AddCategorizedDBs(CRef<objects::CBlast4_get_databases_ex_reply>& dbs, set<string>& categorized_dbs);
    void    x_SaveBlastDataBases(const string& md5);

    TDbMap  m_NucDbMap;
    TDbMap  m_ProtDbMap;

    string m_Dir;
    string m_DBTreePath;

    CRef<CAppJobError> m_Error;
};

} // namespace

void CBLASTDatabases::Load(const char* path, const char* dbtree)
{
    if (m_State == eLoaded)
        return;

    if (m_JobId != CAppJobDispatcher::eInvalidJobID)
        return;

    m_State = eLoading;

    m_Path = path;

    try {
        CRef<CNetBlastLoadDBListJob> job(new CNetBlastLoadDBListJob(path, dbtree));
        m_JobId = CAppJobDispatcher::GetInstance().StartJob(*job, "ThreadPool", *this);

    } catch (const CException& e) {
        ERR_POST("CBLASTDatabases::Load() - Failed to start BLAST DBs loading job");
        e.ReportAll();
        m_State = eFailed;
    }
}

///
///  CNetBlastLoadDBListJob
///

CNetBlastLoadDBListJob::CNetBlastLoadDBListJob(const string& dir, const string& dbtree)
    : m_Dir(dir), m_DBTreePath(dbtree)
{
}

CConstIRef<IAppJobProgress> CNetBlastLoadDBListJob::GetProgress()
{
    return CConstIRef<IAppJobProgress>();
}

CRef<CObject> CNetBlastLoadDBListJob::GetResult()
{
    return CRef<CObject>();
}

CConstIRef<IAppJobError> CNetBlastLoadDBListJob::GetError()
{
   return CConstIRef<IAppJobError>(m_Error.GetPointer());
}

string CNetBlastLoadDBListJob::GetDescr() const
{
    return "Load BLAST DB list job";
}

static void s_GetBlastDbMd5(CBlast4Client& client, string& md5)
{
    md5.clear();
    CRef<CBlast4_get_protocol_info_reply> protocol_info_reply;
    // get md5 digest
    CRef<CBlast4_get_protocol_info_request> ex_md5_req( new CBlast4_get_protocol_info_request );
    // ask for md5 digest for an extended BLAST DB list
    ex_md5_req->Add(string("REQUEST"), string("INFO/BLAST-DBS-EX-MD5DIGEST") );
    protocol_info_reply = client.AskGet_protocol_info( *ex_md5_req );
    // get data from reply ( check -v flag to see all retirned data )
    CRef< CBlast4_parameter > md5_param = protocol_info_reply->GetParamByName("MD5DIGEST");
    if (!md5_param.Empty() && md5_param->GetValue().IsString()) 
        md5 = md5_param->GetValue().GetString();
}

IAppJob::EJobState CNetBlastLoadDBListJob::Run()
{
    string err_msg, logMsg = "Exception in CNetBlastLoadDBListJob: ";

    try {
        CDirEntry dir(m_Dir);
        if (!dir.IsDir())
            NCBI_THROW(CException, eUnknown, "Invalid directory: " + m_Dir);

        string cached_md5;
        try {
            string md5path = CDirEntry::ConcatPath(m_Dir, kBlastDbDataMD5);
            CFile md5file(md5path);
            if (md5file.Exists()) {
                CNcbiIfstream istr(md5path.c_str());
                istr >> cached_md5;
            }
        } catch (const CException& e) {
            LOG_POST(Error << "CNetBlastLoadDBListJob. Failed to load blast databases checksum: " << e.GetMsg());
        }

        CBlast4Client client;
        client.SetRetryLimit(2);
        string md5;
        s_GetBlastDbMd5(client, md5);

        if (!cached_md5.empty() && cached_md5 == md5) {
            CFile nuclFile(CDirEntry::ConcatPath(m_Dir, kBlastDbDataNucl));
            CFile protFile(CDirEntry::ConcatPath(m_Dir, kBlastDbDataProt));
            if (nuclFile.Exists() && protFile.Exists())
                return eCompleted; 
        }

        CRef <CBlast4_get_databases_ex_request> ex_req( new CBlast4_get_databases_ex_request );
        CRef<CBlast4_get_databases_ex_reply> dbs = client.AskGet_databases_ex(*ex_req);
        if (!dbs) 
            NCBI_THROW(CException, eUnknown, "Failed to read blast databases");

        x_CategorizeDBs(dbs);
        if (IsCanceled()) {
            m_NucDbMap.clear();
            m_ProtDbMap.clear();
            return eCanceled;
        }

        x_SaveBlastDataBases(md5);
        m_NucDbMap.clear();
        m_ProtDbMap.clear();
    } catch (const CException& e) {
        err_msg = logMsg + e.GetMsg();
    } catch (const std::exception& e) {
        err_msg = logMsg + e.what();
    }

    if (IsCanceled())
        return eCanceled;

    if (!err_msg.empty()) {
        LOG_POST(Error << err_msg);
        m_Error.Reset(new CAppJobError(err_msg));
        return eFailed;
    }

    return eCompleted;
}

static void s_SaveDbMap(const string& path, const CNetBlastLoadDBListJob::TDbMap& dbmap)
{
    CNcbiOfstream ostr(path.c_str(), ios::out | ios::binary);
    CCompressOStream os(ostr, kDbDataCompressMethod);
    ITERATE(CNetBlastLoadDBListJob::TDbMap, it, dbmap) {
        string first = it->first, second = it->second;
        NStr::ReplaceInPlace(first, "\t", "");
        NStr::ReplaceInPlace(second, "\t", "");
        os << first << "\t" << second << NcbiEndl;
    }
}

void CNetBlastLoadDBListJob::x_SaveBlastDataBases(const string& md5)
{
    if (md5.empty() || m_NucDbMap.empty()) 
        return;
    LOG_POST(Info << "Net BLAST Data Source - saving blast databases...");
    try {
        string path = CDirEntry::ConcatPath(m_Dir, kBlastDbDataNucl);
        s_SaveDbMap(path, m_NucDbMap);
        path = CDirEntry::ConcatPath(m_Dir, kBlastDbDataProt);
        s_SaveDbMap(path, m_ProtDbMap);
        path = CDirEntry::ConcatPath(m_Dir, kBlastDbDataMD5);
        CNcbiOfstream ostr(path.c_str(), ios::out);
        ostr << md5;
        LOG_POST(Info << "Net BLAST Data Source - finished saving blast databases");
    } catch(CException& e)  {
        LOG_POST(Info << "Net BLAST Data Source - failed to save databases - " << e.GetMsg());
    }
}

void s_GetDBTree(const string& file, list<CConstRef<CUser_field> >& fields)
{
    CStopWatch sw;
    sw.Start();

    if (CFile(file).Exists()) {
        try {
            CRef<CUser_field> field(new CUser_field());
            CNcbiIfstream istr(file.c_str());
            istr >> MSerial_AsnText >> *field;

            /// flatten the hierarchy a bit - this is a triple-nested item
            ITERATE (CUser_field::TData::TFields, outer_it, field->GetData().GetFields()) {
                ITERATE (CUser_field::TData::TFields, inner_it, (*outer_it)->GetData().GetFields()) {
                    ITERATE (CUser_field::TData::TFields, it, (*inner_it)->GetData().GetFields()) {
                        fields.push_back(*it);
                    }
                }
            }
        }
        catch (CException& e) {
            // TODO error reporting
            LOG_POST(Error << "Error while loading blast-db-tree.asn: " << e.GetMsg());
        }
    }
    LOG_POST(Info << "CNetBlastLoadDBListJob.s_GetDBTree()" << ": " << sw.Elapsed() << " seconds");
}

void CNetBlastLoadDBListJob::x_AddCategorizedDBs(CRef<CBlast4_get_databases_ex_reply>& dbs, set<string>& categorized_dbs)
{
    //CStopWatch sw; sw.Start();

    list<CConstRef<CUser_field> > fields;
    s_GetDBTree(m_DBTreePath, fields);
    if (fields.empty())
        return;

//    TDbMap valid_db_names;
    typedef CBlast4_get_databases_reply::Tdata  TDBData;
    CBlast4_get_databases_reply::Tdata& db_data = dbs->Set();
    
    typedef unordered_map<string, pair<bool, string> > TValidDbNames;
    TValidDbNames valid_db_names(db_data.size());

    //TDbMap valid_db_names(db_data.size());

    // iterate by all available Databases
    NON_CONST_ITERATE (TDBData, it, db_data) {
        if (IsCanceled()) {
            return;
        }
        const CBlast4_database_info& info = **it;
        const string& db_name = info.GetDatabase().GetName();
        // insert DB record into a temporary map
        valid_db_names.insert(TValidDbNames::value_type(db_name, 
            pair<bool, string>((info.GetDatabase().GetType() == eBlast4_residue_type_protein), info.GetDatabase().GetName())));
    }

    string label, db_name, desc;

    ITERATE (list< CConstRef<CUser_field> >, it, fields) {
        try {
            const CUser_field& f = **it;
            label = f.GetLabel().GetStr();
            db_name = f.GetField("Db").GetData().GetStr();
            NStr::ReplaceInPlace(db_name, "//", "/");
            desc = " ";
            desc.append(f.GetField("Descr").GetData().GetStr());

            TValidDbNames::const_iterator dbname_iter = valid_db_names.find(db_name);
            //TDbMap::const_iterator dbname_iter = valid_db_names.find(db_name);
            if (dbname_iter == valid_db_names.end()) {
                // TODO
#ifdef _DEBUG      
                //LOG_POST(Info << "Dropping categorized BLAST database: " << db_name << " (" << desc << ")");
#endif
            } else{
                categorized_dbs.insert(db_name);
                TDbMap& db = dbname_iter->second.first ? m_ProtDbMap : m_NucDbMap;
                db.insert(TDbMap::value_type(desc, dbname_iter->second.second));
            }
        }
        catch (CException& e) {
            //TODO error reporting
            LOG_POST(Error << "error processing BLAST databases: " << e.GetMsg());
        }
    }
}

static void sPrepareDBDescr(string& db_desc)
{
    /// escape slashes in the description
    NStr::ReplaceInPlace(db_desc, "/", "\\/");

    /// strip off external quotes
    string::size_type pos = db_desc.find_first_not_of("\"'");
    if (pos != string::npos) {
        db_desc.erase(0, pos);
    }
    pos = db_desc.find_last_not_of("\"'");
    if (pos != string::npos) {
        pos += 1;
        if (pos < db_desc.size()) {
            db_desc.erase(pos);
        }
    }
}

/// Fills sm_ProtDbMap and sm_NucDbMap maps.
/// The keys in the maps represent paths in the DB hierarchy.
void CNetBlastLoadDBListJob::x_CategorizeDBs(CRef<CBlast4_get_databases_ex_reply>& dbs)
{
#       ifdef _DEBUG
        CStopWatch watch( CStopWatch::eStart );
#       endif


//    TDbMap valid_db_names;
    string name, visible_name;
    string db_desc, tax_label;
    string org_name_label = "organism_label";
    string prop_name;

    string visual_priority_label = "interface_priority";


    typedef CBlast4_get_databases_reply::Tdata  TDBData;
    CBlast4_get_databases_reply::Tdata& db_data = dbs->Set();
    m_NucDbMap.reserve(db_data.size());
/*
    // iterate by all available Databases
    NON_CONST_ITERATE (TDBData, it, db_data) {
        if (m_StopRequested.Get()) {
            return;
        }

        const CBlast4_database_info& info = **it;
        const string& db_name = info.GetDatabase().GetName();

        // insert DB record into a temporary map
        valid_db_names.insert(TDbMap::value_type(db_name, *it));
    }
*/
    NON_CONST_ITERATE (TDBData, it, db_data) {
        if (IsCanceled()) 
            return;
        const CBlast4_database_info& info = **it;
        const string& db_name = info.GetDatabase().GetName();
        // 'nr' and 'nt' should never be subcategorized
        if (!(db_name == "nr"  ||  db_name == "nt")) 
            continue;
        db_desc = info.GetDescription();
        sPrepareDBDescr(db_desc);
        name = db_name;
        visible_name = db_name;
        if ( ! db_desc.empty()  &&  db_name.find(db_desc) == string::npos) {
            name += " (" + db_desc + ")";
            string db_desc_trimmed(db_desc);
            visible_name += " (" + db_desc_trimmed + ")";
        }
        visible_name = string(" ") + visible_name;
        bool prot = false;
        if (db_name == "nr") {
            prot = (info.GetDatabase().GetType() == eBlast4_residue_type_protein);
        }
        TDbMap& db = prot ? m_ProtDbMap : m_NucDbMap;
        db.insert(TDbMap::value_type(visible_name, (*it)->GetDatabase().GetName()));

    } // ITERATE 


    // use categorization information to build the final maps
    set<string> categorized_dbs;
    x_AddCategorizedDBs(dbs, categorized_dbs);
    CTaxonCache& tax_cache = CTaxonCache::GetInstance();
    string s;
    string db_desc_trimmed;

    NON_CONST_ITERATE (TDBData, it, db_data) {
        if (IsCanceled()) 
            return;
        CRef<objects::CBlast4_database_info> info = *it;
        CBlast4_database_info_Base::TTaxid tax_id = info->GetTaxid();
        if (tax_id == 0) {
            // TODO: decide what to do with those DBs
            continue;
        }

        const string& db_name = info->GetDatabase().GetName();
        // 'nr' and 'nt' should never be subcategorized
        if (db_name == "nr" || db_name == "nt") 
            continue;
        if (categorized_dbs.find(db_name) != categorized_dbs.end()) 
            continue;

        db_desc = info->GetDescription();
        sPrepareDBDescr(db_desc);
        name = db_name;
        visible_name = db_name;
        NStr::ReplaceInPlace(visible_name, "GPIPE/", kEmptyStr);

        if ( ! db_desc.empty()  &&  db_name.find(db_desc) == string::npos) {
            name.append(" (");
            name.append(db_desc);
            name.append(")");            
            db_desc_trimmed = db_desc;
            visible_name.append(" (");
            visible_name.append(db_desc_trimmed);
            visible_name.append(")");
        }
        s.clear();
        tax_label.clear();

        TDbMap& db = m_NucDbMap;
        // put human and mouse into top category of interest
        if (tax_id == 9606 || tax_id == 10090) {
            s = "A0. ";
        }
        
        // ad-hoc anaysis to understand if taxonomy name is more than two letters, which is frequent 
        // when we dealing with various bacterial strands, so we can put them all under a common name
        // (in the GUI tree)
        //
        vector<string> word_split_vec;

        // check if response contains tax label
        //
        if( (*it)->CanGetExtended() ) {
            const CBlast4_parameters &ext = (*it)->GetExtended();
            list< CRef< CBlast4_parameter > >::const_iterator pit = ext.Get().begin();
            for(; pit != ext.Get().end(); pit++) {
              prop_name = (*pit)->GetName();
              if (prop_name == org_name_label) {
                  if( (*pit)->CanGetValue() ){
                      const CBlast4_value &pval = (*pit)->GetValue();
                      if( pval.IsString() ) 
                          tax_label = pval.GetString();
                  }
              } else 
              if (prop_name == visual_priority_label) {
                  if( (*pit)->CanGetValue() ){
                      const CBlast4_value &pval = (*pit)->GetValue();
                      if( pval.IsInteger() ) {
                          int vp = pval.GetInteger();
                          if (vp > 0 && s.empty()) {
                              s = "A1. ";
                              //ERR_POST(Info << "GUI rankeddb:" << visible_name);
                          }
                      }
                  }
              }
              

            } // for
        } // if

        if( tax_label.empty() ){
            ERR_POST(Warning << "BLAST database without tax name:" << visible_name);
            tax_label = tax_cache.GetLabel(tax_id);
        } else {
/*
#ifdef _DEBUG
            // brief diagnostics if strings are at least somewhat similar
            // (using substring search)
            // BLAST database names can sometimes be rather different from taxa names...
            // (sometimes it is legal because of use of synonims)
            //
            string tl = tax_cache.GetLabel(tax_id);
            {{
                SIZE_TYPE pos = NPOS;
                if( !tl.empty() ){                    
                    pos = NStr::Find( tax_label, tl, NStr::eNocase ); 
                }
                if( pos == NPOS ){
                    pos = NStr::Find( tl, tax_label, NStr::eNocase ); 
                }
                if (pos == NPOS) {
                    LOG_POST(Error << "BLAST tax name is different from tax name:" << tax_id << " " << tl << " - " << tax_label);
                }
            }}
#endif   
*/
        }


        NStr::ReplaceInPlace(tax_label, "/", " ");
        NStr::ReplaceInPlace(tax_label, "//", " ");
        NStr::ReplaceInPlace(tax_label, "'", "");

        if (tax_label.empty()) {
            continue;
        }
        NStr::Split(tax_label, " \t", word_split_vec, NStr::fSplit_Tokenize);
        if (word_split_vec.size() > 2) {
            s.append(word_split_vec[0]);
            s.append(" ");
            s.append(word_split_vec[1]);
            s.append(" /");
        }

        s.append(tax_label);

        s.append("( taxid: ");
        s.append(NStr::IntToString(tax_id));
        s.append(" ) /");
        s.append(visible_name);

        //ERR_POST(Info << s);

        if (IsCanceled()) {
            return;
        }
        
        db.insert(TDbMap::value_type(s, (*it)->GetDatabase().GetName()));

    }

    //NON_CONST_ITERATE (TDbMap, it, valid_db_names) {
    //    it->second.Reset();
    //}

//    valid_db_names.clear();
#       ifdef _DEBUG
        watch.Stop();
        ERR_POST(Info << "x_CategorizeBD() takes " << watch.AsSmartString() );
#       endif

}

END_NCBI_SCOPE
