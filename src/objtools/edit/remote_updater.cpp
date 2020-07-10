/*  $Id: remote_updater.cpp 605109 2020-04-07 11:01:53Z ivanov $
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
* Authors:  Sergiy Gotvyanskyy, NCBI
*           Colleen Bolin, NCBI
*
* File Description:
*   Front-end class for making remote request to MLA and taxon
* 
* ===========================================================================
*/
#include <ncbi_pch.hpp>

#include <objects/taxon3/taxon3.hpp>
#include <objects/mla/mla_client.hpp>

#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/seq/Pubdesc.hpp>

#include <objects/seqfeat/Org_ref.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Bioseq.hpp>

#include <objmgr/seq_descr_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>

// new
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

#include <objtools/edit/remote_updater.hpp>

#include <common/test_assert.h>  /* This header must go last */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(edit)

DEFINE_CLASS_STATIC_MUTEX(CRemoteUpdater::m_static_mutex);

namespace
{

int FindPMID(CMLAClient& mlaClient, const CPub_equiv::Tdata& arr)
{
    for (auto pPub : arr) {
        if (pPub->IsPmid()) {
            return pPub->GetPmid().Get();
        }

    }
    return 0;
}

// the method is not used at the momment
void CreatePubPMID(CMLAClient& mlaClient, CPub_equiv::Tdata& arr, int id)
{
    try {
        CPubMedId req(id);
        CRef<CPub> new_pub = mlaClient.AskGetpubpmid(req);
        if (new_pub.NotEmpty())
        {
            // authors come back in a weird format that we need
            // to convert to ISO
            if (new_pub->IsSetAuthors())
               CRemoteUpdater::ConvertToStandardAuthors((CAuth_list&)new_pub->GetAuthors());

            arr.clear();
            CRef<CPub> new_pmid(new CPub);
            new_pmid->SetPmid().Set(id);
            arr.push_back(new_pmid);
            arr.push_back(new_pub);
        }
    } catch(...) {
        // don't worry if we can't look it up
    }

}

}// end anonymous namespace


class CCachedTaxon3_impl
{
public:
    typedef map<string, CRef<CT3Reply> > CCachedReplyMap;

    void Init()
    {
        if (m_taxon.get() == 0)
        {
            m_taxon.reset(new CTaxon3);
            m_taxon->Init();
            m_cache.reset(new CCachedReplyMap);
        }
    }

    void ClearCache()
    {
        if (m_cache.get() != 0)
        {
            m_cache->clear();
        }
    }

    CRef<COrg_ref> GetOrg(const COrg_ref& org, CRemoteUpdater::FLogger f_logger)
    {
        CRef<COrg_ref> result;
        CRef<CT3Reply> reply = GetOrgReply(org);
        if (reply->IsError() && f_logger)
        {
            const string& error_message = 
                "Taxon update: " +
                (org.IsSetTaxname() ? org.GetTaxname() : NStr::IntToString(org.GetTaxId())) + ": " +
                reply->GetError().GetMessage();

/*
            logger->PutError(*auto_ptr<CLineError>(
                CLineError::Create(ILineError::eProblem_Unset, eDiag_Warning, "", 0,
                string("Taxon update: ") + 
                (org.IsSetTaxname() ? org.GetTaxname() : NStr::IntToString(org.GetTaxId())) + ": " +
                reply->GetError().GetMessage())));
            */
        }
        else
        if (reply->IsData() && reply->SetData().IsSetOrg())
        {
            result.Reset(&reply->SetData().SetOrg());
        }
        return result;
    }

    CRef<CT3Reply> GetOrgReply(const COrg_ref& in_org)
    {

#if 0
        string id;
        NStr::IntToString(id, in_org.GetTaxId());
        if (in_org.IsSetTaxname())
            id += in_org.GetTaxname();
        CRef<CT3Reply>& reply = (*m_cache)[id];
#else
        CNcbiStrstream os;
        os << MSerial_AsnText << in_org;
        CRef<CT3Reply>& reply = (*m_cache)[os.str()];
#endif
        if (reply.Empty())
        {
            CTaxon3_request request;

            CRef<CT3Request> rq(new CT3Request);                    
            CRef<COrg_ref> org(new COrg_ref);
            org->Assign(in_org);
            rq->SetOrg(*org);

            request.SetRequest().push_back(rq);
            CRef<CTaxon3_reply> result = m_taxon->SendRequest (request);
            reply = *result->SetReply().begin();
            if (reply->IsData() && reply->SetData().IsSetOrg())
            {
                reply->SetData().SetOrg().ResetSyn();
            }

        }
        else
        {
#ifdef _DEBUG
            //cerr << "Using cache for:" << os.str() << endl;
#endif
        }
        return reply;
    }

    CRef<CTaxon3_reply> SendOrgRefList(const vector< CRef<COrg_ref> >& query, bool use_cache)
    {
        if (!use_cache)
            return m_taxon->SendOrgRefList(query);

        CRef<CTaxon3_reply> result(new CTaxon3_reply);

        ITERATE (vector<CRef< COrg_ref> >, it, query)
        {            
            result->SetReply().push_back(GetOrgReply(**it));
        }

        return result;
    }  
protected:
    auto_ptr<CTaxon3> m_taxon;
    auto_ptr<CCachedReplyMap> m_cache;
};

void CRemoteUpdater::UpdateOrgFromTaxon(FLogger logger, objects::CSeqdesc& obj)
{
    if (obj.IsOrg())
    {
        xUpdateOrgTaxname(logger, obj.SetOrg());
    }
    else
    if (obj.IsSource() && obj.GetSource().IsSetOrg())
    {
        xUpdateOrgTaxname(logger, obj.SetSource().SetOrg());
    }
}

void CRemoteUpdater::xUpdateOrgTaxname(FLogger logger, COrg_ref& org)
{
    CMutexGuard guard(m_Mutex);

    int taxid = org.GetTaxId();
    if (taxid == 0 && !org.IsSetTaxname())
        return;

    if (m_taxClient.get() == 0)
    {
        m_taxClient.reset(new CCachedTaxon3_impl);
        m_taxClient->Init();
    }
        
    CRef<COrg_ref> new_org = m_taxClient->GetOrg(org, logger);
    if (new_org.NotEmpty())
    {
        org.Assign(*new_org);
    }
}

CRemoteUpdater& CRemoteUpdater::GetInstance()
{
    CMutexGuard guard(m_static_mutex);
    
    static CRemoteUpdater instance;

    return instance;
}

CRemoteUpdater::CRemoteUpdater(bool enable_caching)
    :m_enable_caching(enable_caching)
{
}

CRemoteUpdater::~CRemoteUpdater()
{
}

void CRemoteUpdater::ClearCache()
{
    CMutexGuard guard(m_Mutex);

    if (m_taxClient.get() != 0)
    {
        m_taxClient->ClearCache();
    }
}

void CRemoteUpdater::UpdatePubReferences(objects::CSeq_entry_EditHandle& obj)
{
    for (CBioseq_CI it(obj); it; ++it)
    {
        xUpdatePubReferences(it->GetEditHandle().SetDescr());
    }
}

void CRemoteUpdater::UpdatePubReferences(CSerialObject& obj)
{
    if (obj.GetThisTypeInfo()->IsType(CSeq_entry::GetTypeInfo()))
    {
        CSeq_entry* entry = (CSeq_entry*)(&obj);
        xUpdatePubReferences(*entry);
    }
    else
    if (obj.GetThisTypeInfo()->IsType(CSeq_submit::GetTypeInfo()))
    {
        CSeq_submit* submit = (CSeq_submit*)(&obj);
        NON_CONST_ITERATE(CSeq_submit::TData::TEntrys, it, submit->SetData().SetEntrys())
        {
            xUpdatePubReferences(**it);
        }
    }
    else
    if (obj.GetThisTypeInfo()->IsType(CSeq_descr::GetTypeInfo()))
    {
        CSeq_descr* desc = (CSeq_descr*)(&obj);
        xUpdatePubReferences(*desc);
    }
    else
    if (obj.GetThisTypeInfo()->IsType(CSeqdesc::GetTypeInfo()))
    {
        CSeqdesc* desc = (CSeqdesc*)(&obj);
        CSeq_descr tmp;
        tmp.Set().push_back(CRef<CSeqdesc>(desc));
        xUpdatePubReferences(tmp);
    }
}

void CRemoteUpdater::xUpdatePubReferences(CSeq_entry& entry)
{
    if (entry.IsSet())
    {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, it2, entry.SetSet().SetSeq_set())
        {
            xUpdatePubReferences(**it2);
        }
    }

    if (!entry.IsSetDescr())
        return;

    xUpdatePubReferences(entry.SetDescr());
}

void CRemoteUpdater::xUpdatePubReferences(objects::CSeq_descr& seq_descr)
{
    CMutexGuard guard(m_Mutex);

    CSeq_descr::Tdata& descr = seq_descr.Set();
    size_t count = descr.size();
    CSeq_descr::Tdata::iterator it = descr.begin();

    for (size_t i=0; i<count; ++it,  ++i)
    {
        if (! ( (**it).IsPub() && (**it).GetPub().IsSetPub() ) )
            continue;

        CPub_equiv::Tdata& arr = (**it).SetPub().SetPub().Set();
        if (m_mlaClient.Empty())
            m_mlaClient.Reset(new CMLAClient);

        int id = FindPMID(*m_mlaClient, arr);
        if (id>0)
        {
            CreatePubPMID(*m_mlaClient, arr, id);
        }
        else
        // nothing was found
        NON_CONST_ITERATE(CPub_equiv::Tdata, item_it, arr)
        {
            if ((**item_it).IsArticle())
            try
            {
                id = m_mlaClient->AskCitmatchpmid(**item_it);
                if (id>0)
                {
                    CreatePubPMID(*m_mlaClient, arr, id);
                    break;
                }
            }
            catch(CException& /*ex*/)
            {
            }
        }
    }
}

namespace
{
    typedef set<CRef< CSeqdesc >* > TOwnerSet;
    typedef struct { TOwnerSet owner; CRef<COrg_ref> org_ref; } TOwner;
    typedef map<string, TOwner > TOrgMap;
    void _UpdateOrgFromTaxon(CRemoteUpdater::FLogger logger, objects::CSeq_entry& entry, TOrgMap& m)
    {
        if (entry.IsSet())
        {
            NON_CONST_ITERATE(CSeq_entry::TSet::TSeq_set, it, entry.SetSet().SetSeq_set())
            {
                _UpdateOrgFromTaxon(logger, **it, m);
            }
        }

        if (!entry.IsSetDescr())
            return;

        NON_CONST_ITERATE(CSeq_descr::Tdata, it, entry.SetDescr().Set())
        {
            CRef<CSeqdesc>* owner = &*it;
            CSeqdesc& desc = **owner;
            CRef<COrg_ref> org_ref;
            if (desc.IsOrg())
            {
                //xUpdateOrgTaxname(logger, desc.SetOrg());
                org_ref.Reset(&desc.SetOrg());
            }
            else
            if (desc.IsSource() && desc.GetSource().IsSetOrg())
            {
                //xUpdateOrgTaxname(logger, desc.SetSource().SetOrg());
                org_ref.Reset(&desc.SetSource().SetOrg());
            }
            if (org_ref)
            {
                string id;
                CNcbiStrstream os;
                os << MSerial_AsnText << *org_ref;
                id = os.str();
                TOwner& v = m[id];
                v.owner.insert(owner);
                v.org_ref = org_ref;
            }
        }
    }

    void xUpdate(TOwnerSet& owner, COrg_ref& org_ref)
    {
        NON_CONST_ITERATE(TOwnerSet, owner_it, owner)
        {
            if ((**owner_it)->IsOrg())
            {
                (**owner_it)->SetOrg(org_ref);
            }
            else
                if ((**owner_it)->IsSource())
                {
                    (**owner_it)->SetSource().SetOrg(org_ref);
                }
        }
    }
}
void CRemoteUpdater::UpdateOrgFromTaxon(FLogger logger, objects::CSeq_entry& entry)
{   
    TOrgMap org_to_update;

    _UpdateOrgFromTaxon(logger, entry, org_to_update); 
    if (org_to_update.empty())
        return;

    CTaxon3 taxon;
    taxon.Init();

    NON_CONST_ITERATE(TOrgMap, it, org_to_update)
    {
        vector<CRef<COrg_ref> > reflist;
        reflist.push_back(it->second.org_ref);
        CRef<CTaxon3_reply> reply = taxon.SendOrgRefList(reflist);

        if (reply.NotNull())
        {
            CTaxon3_reply::TReply::iterator reply_it = reply->SetReply().begin();
            {
                if ((*reply_it)->IsData() && (*reply_it)->SetData().IsSetOrg())
                {
                    (*reply_it)->SetData().SetOrg().ResetSyn();
                    (*reply_it)->SetData().SetOrg().SetOrgname().SetFormalNameFlag(false);

                    xUpdate(it->second.owner, (*reply_it)->SetData().SetOrg());
                }
            }
        }
    }
}

void CRemoteUpdater::UpdateOrgFromTaxon(FLogger logger, objects::CSeq_entry_EditHandle& obj)
{
    for (CBioseq_CI bioseq_it(obj); bioseq_it; ++bioseq_it)
    {
        for (CSeqdesc_CI desc_it(bioseq_it->GetEditHandle()); desc_it; ++desc_it)
        {
            UpdateOrgFromTaxon(logger, (CSeqdesc&)*desc_it);
        }
    }
}

namespace
{
bool s_IsAllCaps(const string& str)
{
    ITERATE(string, it, str) {
        if (!isalpha(*it) || !isupper(*it)) {
            return false;
        }
    }
    return true;
}


string s_GetInitials(vector<string>& tokens)
{
    string init = "";
    if (tokens.size() > 1) {
        string val = tokens.back();
        if (s_IsAllCaps(val)) {
            init = val;
            tokens.pop_back();
            if (tokens.size() > 1) {
                val = tokens.back();
                if (s_IsAllCaps(val)) {
                    init = val + init;
                    tokens.pop_back();
                }
            }
        }
    }
    return init;
}

CRef<CAuthor> StdAuthorFromMl(const string& val)
{
    CRef<CAuthor> new_auth(new CAuthor());
    vector<string> tokens;
    NStr::Split(val, " ", tokens);
    string suffix = "";
    string init = s_GetInitials(tokens);
    if (NStr::IsBlank(init) && tokens.size() > 1) {
        suffix = tokens.back();
        tokens.pop_back();
        init = s_GetInitials(tokens);
    }
    string last = NStr::Join(tokens, " ");
    new_auth->SetName().SetName().SetLast(last);
    if (!NStr::IsBlank(suffix)) {
        new_auth->SetName().SetName().SetSuffix(suffix);
    }
    if (!NStr::IsBlank(init)) {                
        new_auth->SetName().SetName().SetFirst(init.substr(0, 1));
        vector<string> letters;
        NStr::Split(init, "", letters);
        string initials = NStr::Join(letters, ".");
        new_auth->SetName().SetName().SetInitials(initials);
    }
    return new_auth;
}

void FixMedLineList(CAuth_list& auth_list)
{
    list< CRef< CAuthor > > standard_names;

    ITERATE(CAuth_list::TNames::TMl, it, auth_list.GetNames().GetMl()) {
        if (!NStr::IsBlank(*it)) {
            CRef<CAuthor> new_auth = StdAuthorFromMl(*it);
            standard_names.push_back(new_auth);
        }
    }
    auth_list.SetNames().Reset();
    auth_list.SetNames().SetStd().insert(auth_list.SetNames().SetStd().begin(), standard_names.begin(), standard_names.end());
}

 
}

void CRemoteUpdater::ConvertToStandardAuthors(CAuth_list& auth_list)
{
    if (!auth_list.IsSetNames()) {
        return;
    }
/*
    const bool fix_suffix = false;
    if (auth_list.GetNames().IsMl()) {
        list<CRef<CAuthor>> new_authors;
        for (const string& ml_name : auth_list.GetNames().GetMl()) {
            new_authors.push_back(CAuthor::ConvertMlToStandard(ml_name, fix_suffix));
        }
        auth_list.SetNames().Reset();
        auth_list.SetNames().SetStd().insert(auth_list.SetNames().SetStd().begin(), new_authors.begin(), new_authors.end());
        return;
    }

    if (auth_list.GetNames().IsStd()) {
        for (CRef<CAuthor> author : auth_list.SetNames().SetStd()) {
            if (author->GetName().IsMl()) {
                CRef<CAuthor> new_author = CAuthor::ConvertMlToStandard(*author, fix_suffix);
                author->Assign(*new_author);
            }
        }
    }
*/
    if (auth_list.GetNames().IsMl()) {
        FixMedLineList(auth_list);
        return;
    } else if (auth_list.GetNames().IsStd()) {
        list< CRef<CAuthor> > authors_with_affil;
        NON_CONST_ITERATE(CAuth_list::TNames::TStd, it, auth_list.SetNames().SetStd()) {
            if ((*it)->GetName().IsMl()) {
                CRef<CAuthor> new_auth = StdAuthorFromMl((*it)->GetName().GetMl());
                (*it)->SetName(new_auth->SetName());
            }
            if ((*it)->IsSetAffil()) {
                authors_with_affil.push_back(*it);
            }
        }

        if (authors_with_affil.size() == 1) {
            // we may need to hoist an affiliation
            if (auth_list.IsSetAffil()) {
                ERR_POST(Error << "publication contains multiple affiliations");
            }
            else {
                auth_list.SetAffil(authors_with_affil.front()->SetAffil());
                authors_with_affil.front()->ResetAffil();
            }
        }
    }
}


void CRemoteUpdater::PostProcessPubs(CSeq_entry& obj)
{
    if (obj.IsSet())
    {
        NON_CONST_ITERATE(CSeq_entry::TSet::TSeq_set, it, obj.SetSet().SetSeq_set())
        {
            PostProcessPubs(**it);
        }
    }
    else
    if (obj.IsSeq() && obj.IsSetDescr())
    {
        NON_CONST_ITERATE(CSeq_descr::Tdata, desc_it, obj.SetSeq().SetDescr().Set())
        {
            if ((**desc_it).IsPub())
            {
                PostProcessPubs((**desc_it).SetPub());
            }
        }
    }
}

void CRemoteUpdater::PostProcessPubs(CPubdesc& pubdesc)
{
    if (!pubdesc.IsSetPub())
        return;

    NON_CONST_ITERATE(CPubdesc::TPub::Tdata, it, pubdesc.SetPub().Set())
    {
        if ((**it).IsSetAuthors())
        {
            ConvertToStandardAuthors((CAuth_list&)(**it).GetAuthors());
        }
    }
}

void CRemoteUpdater::PostProcessPubs(CSeq_entry_EditHandle& obj)
{
    for (CBioseq_CI bioseq_it(obj); bioseq_it; ++bioseq_it)
    {
        for (CSeqdesc_CI desc_it(bioseq_it->GetEditHandle(), CSeqdesc::e_Pub); desc_it; ++desc_it)
        {
            PostProcessPubs((CPubdesc&)desc_it->GetPub());
        }
    }
   
}

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE
