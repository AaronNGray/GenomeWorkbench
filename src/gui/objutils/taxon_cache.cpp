/*  $Id: taxon_cache.cpp 37498 2017-01-14 01:39:56Z evgeniev $
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
 * Authors:  Roman Katargin, Anatoliy Kuznetsov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <iostream>
#include <corelib/ncbimtx.hpp>

#include <objects/taxon1/taxon1.hpp>
#include <util/line_reader.hpp>

#include <gui/objutils/taxon_cache.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

DEFINE_STATIC_MUTEX(s_Mutex);

string CTaxonCache::m_CachePath;

void CTaxonCache::InitStorage(const char* cache_path)
{
    m_CachePath.assign(cache_path);
}

CTaxonCache::CTaxonCache() 
{
    try {
        x_LoadTaxonCache();
    } catch (std::exception& ex) {
        LOG_POST(Error << "Exception in CTaxonCache::CTaxonCache() : " << ex.what());
    }
}

CTaxonCache::~CTaxonCache()
{
    try {
        LOG_POST(Info << "Saving taxon cache to" << m_CachePath);
        x_SaveTaxonCache();
    } catch (std::exception& ex) { 
        LOG_POST(Error << "Exception in CTaxonCache::~CTaxonCache() : " << ex.what());
    }
}

void CTaxonCache::x_SaveTaxonCache() const
{
    if (m_CachePath.empty())
        return;

    CNcbiOfstream ostr(m_CachePath.c_str());
    for (auto &tax_it : m_Map) {
        ostr << tax_it.first << '\t' << tax_it.second.Label << '\t' << tax_it.second.TaxName << '\t' << tax_it.second.CommonName << '\t' << tax_it.second.BlastName << '\n';
    }    
}

void CTaxonCache::x_LoadTaxonCache() 
{
    if (m_CachePath.empty() || !CFile(m_CachePath).Exists())
        return;

    CRef<ILineReader> line_reader(ILineReader::New(m_CachePath));
    string str_id, label;

    while ( !line_reader->AtEOF() ) {
        line_reader->ReadLine();
        vector<CTempString> strings;
        NStr::Split(line_reader->GetCurrentLine(), "\t", strings, 0);
        if (5 != strings.size()) 
            continue;

        int tax_id = NStr::StringToInt(strings[0]);
        if (0 == tax_id)
            continue;

        STaxonomy tax_info;
        tax_info.Label = strings[1];
        tax_info.TaxName = strings[2];
        tax_info.CommonName = strings[3];
        tax_info.BlastName = strings[4];

        m_Map[tax_id] = tax_info;
    }
}


CTaxonCache& CTaxonCache::GetInstance()
{
    static CRef<CTaxonCache> s_Cache;
    if (!s_Cache) {
        CMutexGuard LOCK(s_Mutex);
        if (!s_Cache) {
            s_Cache.Reset (new CTaxonCache());
        }
    }
    return *s_Cache;
}

void CTaxonCache::Initialize(const vector<int>& tax_ids)
{
    CMutexGuard LOCK(s_Mutex);

    vector<int> to_initialize;
    ITERATE(vector<int>, it, tax_ids) {
        if (m_Map.find(*it) == m_Map.end())
            to_initialize.push_back(*it);
    }

    if (to_initialize.empty())
        return;

    try {
        CTaxon1 taxon;
        taxon.Init();

        ITERATE(vector<int>, it, to_initialize) {
            bool is_species = false;
            bool is_uncultured = false;
            string blast_name;
            CConstRef<COrg_ref> org_ref = taxon.GetOrgRef(*it, is_species, is_uncultured, blast_name);
            if (0 == org_ref)
                continue;

            STaxonomy tax_info;
            org_ref->GetLabel(&tax_info.Label);
            if (org_ref->CanGetTaxname())
                tax_info.TaxName = org_ref->GetTaxname();
            if (org_ref->CanGetCommon())
                tax_info.CommonName = org_ref->GetCommon();
            tax_info.BlastName = blast_name;

            m_Map[*it] = tax_info;
            
        }
    }
    catch (const exception& ex) {
        LOG_POST(Error << "CTaxonCache::Initialize: " << ex.what());
    }
}

void CTaxonCache::ResetConnection()
{
    CMutexGuard LOCK(s_Mutex);
    m_TaxonClient.reset(0);
}

void CTaxonCache::x_QueryTaxonService(int tax_id)
{
    if (m_TaxonClient.get() == 0) {
        m_TaxonClient.reset(new CTaxon1);
        bool conn_res = m_TaxonClient->Init();
        if (conn_res != true) {
            m_TaxonClient.reset(0);
            return;
        }
    }

    CTaxon1& taxon = *(m_TaxonClient.get());

    bool is_species = false;
    bool is_uncultured = false;
    string blast_name;
    CConstRef<COrg_ref> org_ref = taxon.GetOrgRef(tax_id, is_species, is_uncultured, blast_name);
    if (!org_ref)
        return;

    STaxonomy tax_info;
    org_ref->GetLabel(&tax_info.Label);
    if (org_ref->CanGetTaxname())
        tax_info.TaxName = org_ref->GetTaxname();
    if (org_ref->CanGetCommon())
        tax_info.CommonName = org_ref->GetCommon();
    tax_info.BlastName = blast_name;

    m_Map[tax_id] = tax_info;
}

CTaxonCache::TMap::const_iterator CTaxonCache::x_GetItem(int tax_id)
{
    TMap::const_iterator it = m_Map.find(tax_id);
    if (it != m_Map.end())
        return it;
        
    try {
        x_QueryTaxonService(tax_id);
    }
    catch (const exception& ex) {
        LOG_POST(Error << "CTaxonCache::GetLabel: " << ex.what());
        m_TaxonClient.reset(0);
        // second attempt after re-initialization
        try {
            x_QueryTaxonService(tax_id);
        }
        catch (const exception& ex)
        {
            LOG_POST(Error << "Second try of CTaxonCache::GetLabel: " << ex.what());
            m_TaxonClient.reset(0);
        }
    }
    return m_Map.find(tax_id);
}

string CTaxonCache::GetLabel(int tax_id) 
{
    CMutexGuard LOCK(s_Mutex);
    TMap::const_iterator it = x_GetItem(tax_id);
    if (it == m_Map.end())
        return string();
    
    return it->second.Label;
}

string CTaxonCache::GetTaxname(int tax_id) 
{
    CMutexGuard LOCK(s_Mutex);
    TMap::const_iterator it = x_GetItem(tax_id);
    if (it == m_Map.end())
        return string();

    return it->second.TaxName;
}

string CTaxonCache::GetCommon(int tax_id) 
{
    CMutexGuard LOCK(s_Mutex);
    TMap::const_iterator it = x_GetItem(tax_id);
    if (it == m_Map.end())
        return string();

    return it->second.CommonName;
}

string CTaxonCache::GetBlastName(int tax_id)
{
    CMutexGuard LOCK(s_Mutex);
    TMap::const_iterator it = x_GetItem(tax_id);
    if (it == m_Map.end())
        return string();

    return it->second.BlastName;
}

END_NCBI_SCOPE
