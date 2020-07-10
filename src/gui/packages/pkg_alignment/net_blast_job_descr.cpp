/*  $Id: net_blast_job_descr.cpp 32541 2015-03-18 20:41:59Z katargir $
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
 * Authors:  Andrey Yazhuk, Anatoliy Kuznetsov
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/net_blast_job_descr.hpp>

#include <gui/packages/pkg_alignment/blast_search_params.hpp>
#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/blast_databases.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(blast);

///////////////////////////////////////////////////////////////////////////////
/// CNetBlastJobDescriptorLabelHandler

// TODO this must be more flexible - extension point ?

class CNetBlastJobDescriptorLabelHandler : public ILabelHandler
{
public:
    void GetLabel(const CObject& obj, string* str,
                  CLabel::ELabelType type, objects::CScope* scope) const
    {
        const CNetBlastJobDescriptor* descr =
                dynamic_cast<const CNetBlastJobDescriptor*>(&obj);
        if (descr) {
            switch (type) {
            case CLabel::eUserType:
                *str += "Net BLAST Job";
                break;

            case CLabel::eUserSubtype:
                *str += kEmptyStr;
                break;

            case CLabel::eType:
                *str += "Net BLAST Job Descriptor";
                break;

            case CLabel::eDescriptionBrief:
            case CLabel::eDescription:
            case CLabel::eContent:  {{
                CNetBlastJobDescriptor::EState state = descr->GetState();

                *str += "Job Title: " + descr->GetJobTitle();
                *str += ", State: " + descr->GetStateLabel(state);
                string rid = descr->GetRID();
                if( ! rid.empty())  {
                    *str += ", RID '" + rid + "'";
                }
                *str += descr->GetDescription();
                break;
            }}

            case CLabel::eUserTypeAndContent:   {{
                GetLabel(obj, str, CLabel::eUserType, scope);
                *str += "\n";
                GetLabel(obj, str, CLabel::eContent, scope);
                break;
            }}
            default:
                break;
            }
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
/// CNetBlastJobDescriptor

CNetBlastJobDescriptor::CNetBlastJobDescriptor()
:   m_NetBlastDS(NULL),
    m_State(eInvalidState),
    m_Deleted(false)
{
    // TODO this must be done using an extension point ?
    string type = typeid(CNetBlastJobDescriptor).name();
    if ( ! CLabel::HasHandler(type)) {
        CLabel::RegisterLabelHandler(type, *new CNetBlastJobDescriptorLabelHandler());
    }
    m_SubmitTime = time(0);
}


CNetBlastJobDescriptor::CNetBlastJobDescriptor(CNetBLASTUIDataSource& ds)
:   m_NetBlastDS(&ds),
    m_State(eInvalidState),
    m_Deleted(false)
{
    // TODO this must be done using an extension point ?
    string type = typeid(CNetBlastJobDescriptor).name();
    if ( ! CLabel::HasHandler(type)) {
        CLabel::RegisterLabelHandler(type, *new CNetBlastJobDescriptorLabelHandler());
    }
    m_SubmitTime = time(0);
}


CNetBlastJobDescriptor::~CNetBlastJobDescriptor()
{
}


string CNetBlastJobDescriptor::GetStateLabel(EState state)
{
    static string s_labels[] =
        { "Invalid", "Initial", "Submitted", "Completed", "Failed", "Retrieved", "Expired", "BUG" };
    size_t i = (state <= eExpired) ? state + 1 : eExpired;
    return s_labels[i];
}


CNetBlastJobDescriptor::EState CNetBlastJobDescriptor::GetState() const
{
    return m_State;
}


string CNetBlastJobDescriptor::GetRID() const
{
    CMutexGuard lock(m_Mutex);

    _ASSERT( ! m_RID.empty());
    return m_RID;
}


string CNetBlastJobDescriptor::GetJobTitle() const
{
    CMutexGuard lock(m_Mutex);
    return m_JobTitle;
}


string CNetBlastJobDescriptor::GetDescription() const
{
    CMutexGuard lock(m_Mutex);
    return m_Description;
}


string CNetBlastJobDescriptor::GetErrors() const
{
    CMutexGuard lock(m_Mutex);
    return m_Errors;
}


time_t CNetBlastJobDescriptor::GetSubmitTime() const
{
    CMutexGuard lock(m_Mutex);
    return m_SubmitTime;
}


void CNetBlastJobDescriptor::MarkDeleted(bool deleted)
{
    CMutexGuard lock(m_Mutex);
    if(m_Deleted != deleted)    {
        m_Deleted = deleted;
        x_NotifyDataSource();
    }
}


bool CNetBlastJobDescriptor::IsDeleted() const
{
    return m_Deleted;
}


void CNetBlastJobDescriptor::Init(const CBLASTParams& params, CNetBLASTUIDataSource& ds)
{
    CMutexGuard lock(m_Mutex);

    _ASSERT( ! m_RemoteBlast);
    _ASSERT(m_State == eInvalidState);

    m_NetBlastDS = &ds;

    m_JobTitle = params.GetJobTitle();
    // TODO init description

    CRef<blast::CBlastOptionsHandle> opts = params.ToBlastOptions();

    _ASSERT(opts);

    m_RemoteBlast.Reset(new blast::CRemoteBlast(opts));
    m_RemoteBlast->SetClientId("GBench2");
    m_RemoteBlast->EnableDiskCacheUse();

    CBLASTDatabases& dbs = CBLASTDatabases::GetInstance();
    if (!dbs.Loaded()) {
		string msg = "Cannot submit Net BLAST job - BLAST databases not loaded.";
		NCBI_THROW(CException, eUnknown, msg);
    }

    bool nuc_db = params.IsSubjNucInput();
    const CBLASTDatabases::TDbMap& db_map = dbs.GetDbMap(nuc_db);
    const string& db_path = params.GetCurrDatabase();
    CBLASTDatabases::TDbMap::const_iterator it = db_map.find(db_path);
	if (it == db_map.end()) {
		string msg = "Cannot submit Net BLAST job - invalid database:\n ";
		msg += db_path;
		NCBI_THROW(CException, eUnknown, msg);
	} else {
        m_RemoteBlast->SetDatabase(it->second);			
        /*
		CRef<objects::CBlast4_database_info> blast_db_info(it->second);
		if (blast_db_info) {
			string db_name = blast_db_info->GetDatabase().GetName();
     		m_RemoteBlast->SetDatabase(db_name);			
		}
		else {
	        NCBI_THROW(CException, eUnknown, "Cannot submit Net BLAST job - no database info");
		}
        */
	}

    string eq = params.GetEntrezQuery();
    m_RemoteBlast->SetEntrezQuery(eq.c_str());

    m_RID = "";
    m_State = eInitial;
}


void CNetBlastJobDescriptor::Init(const string& rid, CNetBLASTUIDataSource& ds)
{
    CMutexGuard lock(m_Mutex);

    _ASSERT( ! m_RemoteBlast);
    _ASSERT(m_State == eInvalidState);

    m_NetBlastDS = &ds;

    //TODO init description
    m_RID = rid;
    m_State = eInitial;
}


void CNetBlastJobDescriptor::Submit(CBioseq_set& seq_set)
{
    CMutexGuard lock(m_Mutex);

    if(m_State == eInitial)  {
        _ASSERT(m_RemoteBlast);

        m_RemoteBlast->SetQueries(CRef<CBioseq_set>(&seq_set));

        if( m_RemoteBlast->Submit() ){
            m_SubmitTime = time(0);
            m_RID = m_RemoteBlast->GetRID();
            m_State = eSubmitted;

            x_NotifyDataSource();
        } else {
            NCBI_THROW(CException, eUnknown, "Fail to submit Net BLAST job - " + m_RemoteBlast->GetErrors() );
        }
    } else {
        NCBI_THROW(CException, eUnknown, "Cannot submit Net BLAST job - invalid state");
    }
}


void CNetBlastJobDescriptor::Check()
{
    CMutexGuard lock(m_Mutex);

    if(m_State == eInitial  ||  m_State == eSubmitted)  {
        EState old_state = m_State;

        x_CreateRemoteBlastIfNeeded();

        bool done = m_RemoteBlast->CheckDone();
        m_Errors = m_RemoteBlast->GetErrors();

        if(m_Errors.empty())  {
            if(done)    {
                /// job finished, results are ready, launch loading Task
                m_State = eCompleted;
            }
            // else  - still pending
        } else {
            if(done)    {
                m_State = eFailed;
            } else {
                m_State = eExpired; // unknown or expired RID
            }
        }
        if(m_State != old_state)    {
            x_NotifyDataSource();
        }
    } else {
        NCBI_THROW(CException, eUnknown, "Cannot check Net BLAST job - invalid state");
    }
}


CRef<CSeq_align_set> CNetBlastJobDescriptor::Retrieve()
{
    CMutexGuard lock(m_Mutex);

    CRef<CSeq_align_set> results;

    if(m_State == eCompleted  ||  m_State == eRetrieved)  {
        x_CreateRemoteBlastIfNeeded();

        results = m_RemoteBlast->GetAlignments();
        m_State = eRetrieved;

        x_NotifyDataSource();
    } else {
        NCBI_THROW(CException, eUnknown, "Cannot retrieve Net BLAST job - invalid state");
    }
    return results;
}

CRef<blast::CRemoteBlast> CNetBlastJobDescriptor::GetRemoteBlast()
{
    CMutexGuard lock(m_Mutex);

    CRef<blast::CRemoteBlast> result;

    if(m_State == eCompleted  ||  m_State == eRetrieved)  {
        x_CreateRemoteBlastIfNeeded();

        result = m_RemoteBlast;
        m_State = eRetrieved;

        x_NotifyDataSource();
    } else {
        NCBI_THROW(CException, eUnknown, "Cannot retrieve Net BLAST job - invalid state");
    }
    return result;
}


void CNetBlastJobDescriptor::Release()
{
    CMutexGuard lock(m_Mutex);

    m_RemoteBlast.Reset();
}


static const char* kBLASTJobTag = "BLAST Job";
static const char* kStateTag = "State";
static const char* kRIDTag = "RID";
static const char* kTitleTag = "Title";
static const char* kDescrTag = "Descr";
static const char* kErrorsTag = "Errors";
static const char* kSubmitTimeTag = "Submittime";


CUser_object* CNetBlastJobDescriptor::ToUserObject() const
{
    CRef<CUser_object> obj(new CUser_object());
    obj->SetType().SetStr(kBLASTJobTag);

    obj->AddField(kStateTag, (int) m_State);
    obj->AddField(kRIDTag, m_RID);
    obj->AddField(kTitleTag, m_JobTitle);
    obj->AddField(kDescrTag, m_Description);
    obj->AddField(kErrorsTag, m_Errors);

    CTime time(m_SubmitTime);
    string s_time = time.AsString();
    obj->AddField(kSubmitTimeTag, s_time);

    return obj.Release();
}


void CNetBlastJobDescriptor::FromUserObject(const objects::CUser_object& user_obj)
{
    m_State = (EState) user_obj.GetField(kStateTag).GetData().GetInt();
    m_RID = user_obj.GetField(kRIDTag).GetData().GetStr();
    m_JobTitle = user_obj.GetField(kTitleTag).GetData().GetStr();
    m_Description = user_obj.GetField(kDescrTag).GetData().GetStr();
    m_Errors = user_obj.GetField(kErrorsTag).GetData().GetStr();

    string s_time = user_obj.GetField(kSubmitTimeTag).GetData().GetStr();
    CTime time(s_time);
    m_SubmitTime = time.GetTimeT();

    if( m_RID.empty() ){
        m_State = eInvalidState;
    }
}


void CNetBlastJobDescriptor::x_CreateRemoteBlastIfNeeded()
{
    if( ! m_RemoteBlast)  {
        _ASSERT( ! m_RID.empty());

        m_RemoteBlast.Reset(new CRemoteBlast(m_RID));
        m_RemoteBlast->SetClientId("GBench2");
    }
}


void CNetBlastJobDescriptor::x_NotifyDataSource()
{
    _ASSERT(m_NetBlastDS);

    if(m_NetBlastDS)    {
        m_NetBlastDS->x_OnJobDescrChanged(*this);
    }
}

END_NCBI_SCOPE
