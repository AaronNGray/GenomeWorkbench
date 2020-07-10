/*  $Id: registry.cpp 44845 2020-03-25 22:06:56Z rudnev $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/reg_file.hpp>
#include <algorithm>
#include <iterator>

#include <objects/general/Object_id.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const string CGuiRegistry::kDecimalDot = ".";

CGuiRegistry::CGuiRegistry()
: m_WritableSite(ePriority_Local)
{
}


CGuiRegistry::~CGuiRegistry()
{
}

void CGuiRegistry::SetLocal(CNcbiIstream& istr)
{
    AddSite(istr, ePriority_Local);
}


void CGuiRegistry::FromConfigFile(CNcbiIstream& istr)
{
    m_Registries[ePriority_Local].Reset(new CRegistryFile());
    m_Registries[ePriority_Local]->FromConfigFile(istr);
}


void CGuiRegistry::ToConfigFile(CNcbiOstream& ostr) const
{
    TRegistries::const_iterator it = m_Registries.find(ePriority_Local);
    if (it != m_Registries.end()) {
        it->second->ToConfigFile(ostr);
    }
}


void CGuiRegistry::SetGlobal(CNcbiIstream& istr)
{
    AddSite(istr, ePriority_Global);
}


CRef<CRegistryFile> CGuiRegistry::AddSite(CNcbiIstream& istr, int priority)
{
    CRef<CRegistryFile> site;
    try {
        site.Reset(new CRegistryFile(istr));
        m_Registries[priority] = site;
    }
    catch (CException& e) {
        LOG_POST(Error << "Error reading settings: " << e.GetMsg());
    }
    catch (std::exception& e) {
        LOG_POST(Error << "Unknown error reading local settings; "
                 "the settings will be ignored " << e.what());
    }
    return site;
}


CRef<CRegistryFile> CGuiRegistry::AddSite(const objects::CUser_object& reg, int priority)
{
    CRef<CRegistryFile> site;
    site.Reset(new CRegistryFile(reg));
    m_Registries[priority] = site;
    return site;
}


void  CGuiRegistry::SetWritableSite(int priority)
{
    // this site might not exist. But if it doesn't, it will
    // be created the first time someone writes to it.
    m_WritableSite = priority;
}


int CGuiRegistry::GetWritableSite()
{
    return m_WritableSite;
}


void CGuiRegistry::Write(CNcbiOstream& ostr, int priority) const
{
    CRef<CRegistryFile> reg = x_GetRegistryRef(priority);
    if (reg) {
        reg->Write(ostr);
    }
}


int CGuiRegistry::GetInt(const string& key, int default_val) const
{
    return CRegistryReadView::GetInt(GetField(key), default_val);
}

double CGuiRegistry::GetReal(const string& key, double default_val) const
{
    return CRegistryReadView::GetReal(GetField(key), default_val);
}

bool CGuiRegistry::GetBool(const string& key, bool default_val) const
{
    return CRegistryReadView::GetBool(GetField(key), default_val);
}

string CGuiRegistry::GetString(const string& key, const string& default_val) const
{
    return CRegistryReadView::GetString(GetField(key), default_val);
}

void CGuiRegistry::GetIntVec(const string& key, vector<int>& val) const
{
    CRegistryReadView::GetIntVec(GetField(key), val);
}

void CGuiRegistry::GetRealVec(const string& key, vector<double>& val) const
{
    CRegistryReadView::GetRealVec(GetField(key), val);
}

void CGuiRegistry::GetStringVec(const string& key, vector<string>& val) const
{
    CRegistryReadView::GetStringVec(GetField(key), val);
}

void CGuiRegistry::GetStringList(const string& key, list<string>& val) const
{
    vector<string> v;
    CRegistryReadView::GetStringVec(GetField(key), v);

    val.clear();
    std::copy(v.begin(), v.end(), std::back_inserter(val));
}


void CGuiRegistry::Set(const string& key, int val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(const string& key, double val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(const string& key, bool val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(const string& key, const string& val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(const string& key, const char* val)
{
    Set(key, string(val));
}


void CGuiRegistry::Set(const string& key, const vector<int>& val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(const string& key, const vector<double>& val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(const string& key, const vector<string>& val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(const string& key, const list<string>& val)
{
    Set(GetWritableSite(), key, val);
}


void CGuiRegistry::Set(int site, const string& key, int val)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    CRef<CUser_field> field = write_site.SetFieldToValue(key);
    field->SetData().SetInt(val);
}


void CGuiRegistry::Set(int site, const string& key, double val)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    CRef<CUser_field> field = write_site.SetFieldToValue(key);
    field->SetData().SetReal(val);
}


void CGuiRegistry::Set(int site, const string& key, bool val)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    CRef<CUser_field> field = write_site.SetFieldToValue(key);
    field->SetData().SetBool(val);
}


void CGuiRegistry::Set(int site, const string& key, const string& val)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    CRef<CUser_field> field = write_site.SetFieldToValue(key);
    field->SetData().SetStr(val);
}


void CGuiRegistry::Set(int site, const string& key, const char* val)
{
    Set(site, key, string(val));
}


void CGuiRegistry::Set(int site, const string& key, const vector<int>& val)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    CRef<CUser_field> field = write_site.SetFieldToValue(key);
    field->SetData().SetInts() = val;
}


void CGuiRegistry::Set(int site, const string& key, const vector<double>& val)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    CRef<CUser_field> field = write_site.SetFieldToValue(key);
    field->SetData().SetReals() = val;
}


void CGuiRegistry::Set(int site, const string& key, const vector<string>& val)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    CRef<CUser_field> field = write_site.SetFieldToValue(key);

    CUser_field_Base::C_Data::TStrs& dest = field->SetData().SetStrs();
    dest.clear();
    ITERATE( vector<string>, v, val) {
        dest.push_back(*v);
    }
}


void CGuiRegistry::Set(int site, const string& key, const list<string>& val)
{
    vector<string> v;
    ITERATE(list<string>, it, val)  {
        v.push_back(*it);
    }
    Set(site, key, v);
}

CRef<CUser_field> CGuiRegistry::SetField(const string& key)
{
    return SetField(GetWritableSite(), key);
}


CRef<CUser_field> CGuiRegistry::SetField(int site, const string& key)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    return write_site.SetField(key);
}


CRef<CUser_field> CGuiRegistry::SetFieldToValue(const string& key)
{
    return SetFieldToValue(GetWritableSite(), key);
}


CRef<CUser_field> CGuiRegistry::SetFieldToValue(int site, const string& key)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    return write_site.SetFieldToValue(key);
}

bool CGuiRegistry::DeleteField(const string& key)
{
    return DeleteField(GetWritableSite(), key);
}


bool CGuiRegistry::DeleteField(int site, const string& key)
{
    CRegistryFile& write_site = x_SetRegistry(site);
    return write_site.DeleteField(key);
}


CConstRef<CUser_field> CGuiRegistry::GetField(const string& key) const
{
    list< CConstRef<CUser_field> > fields;
    CConstRef<CUser_field> meta;

    ITERATE (TRegistries, iter, m_Registries) {
        CConstRef<CUser_field> field = iter->second->GetField(key);
        if (field)
            fields.push_back(field);

        field = iter->second->GetField(key + "-meta");
        if (field)
            meta = field;
    }

    return CRegistryReadView::ResolveField(fields, meta);
}


bool CGuiRegistry::HasField(const string& key) const
{
    ITERATE (TRegistries, iter, m_Registries) {
        if (iter->second->HasField(key)) {
            return true;
        }
    }
    return false;
}


/// return the registry at a particular priority.
/// Do not create if it does not exist.
CRef<CRegistryFile> CGuiRegistry::x_GetRegistryRef(int priority) const
{
    TRegistries::const_iterator it = m_Registries.find(priority);
    if (it != m_Registries.end()) {
        return it->second;
    }
    return CRef<CRegistryFile>();
}


/// return a registry at a particular priority, creating it if need be.
CRegistryFile&  CGuiRegistry::x_SetRegistry(int priority)
{
    CRef<CRegistryFile> reg_ref = m_Registries[priority];
    if (! reg_ref) {
        reg_ref.Reset(new CRegistryFile);
        m_Registries[priority] = reg_ref;
    }
    return *reg_ref;
}


CGuiRegistry& CGuiRegistry::GetInstance()
{
    static CRef<CGuiRegistry> s_Registry;
    if (!s_Registry)
        s_Registry.Reset(new CGuiRegistry());
    return *s_Registry;
}

// get an exact snapshot of this registry
// it will be writable even if this registry is read-only 
CRef<CGuiRegistry> CGuiRegistry::Clone() {
    CRef<CGuiRegistry> new_reg(new CGuiRegistry);
    for(auto sitereg: m_Registries) {
        CRef<CRegistryFile> new_reg_file(new CRegistryFile);
        new_reg_file->SetRegistry(sitereg.second->GetRegistry());
        new_reg->m_Registries.emplace(sitereg.first, new_reg_file);
    }
    return new_reg;
}

void CGuiRegistry::x_AppendToView(CRegistryReadView& view,
                                  const string& section) const
{
    ITERATE (TRegistries, iter, m_Registries)
        view += iter->second->GetReadView(section);
}


CRegistryReadView CGuiRegistry::GetReadView(const string& section) const
{
    CRegistryReadView view;
    x_AppendToView(view, section);
    return view;
}


CRegistryReadView
CGuiRegistry::GetReadView(const string& section,
                          const vector<string>& subsections) const
{
    CRegistryReadView view;

    // the section for a view made with subsections ends with
    // the most significant subsection.
    string str(section);
    if ( !section.empty() ) {
        str += kDecimalDot;
    }

    vector<string>::const_reverse_iterator riter(subsections.end());
    vector<string>::const_reverse_iterator rend (subsections.begin());
    for ( ;  riter != rend;  ++riter) {
        string key(*riter);
        if ( !key.empty()) {
            x_AppendToView(view, str + key);
        }
    }

    return view;
}


CRegistryWriteView CGuiRegistry::GetWriteView(const string& section)
{
    if(m_isReadOnly) {
        NCBI_THROW(CException, eUnknown, "Attempt to write to read-only registry!");
    }
    return x_SetRegistry(GetWritableSite()).GetWriteView(section);
}

CRegistryWriteView CGuiRegistry::GetWriteView(const string& section, int writeSite)
{
    if(m_isReadOnly) {
        NCBI_THROW(CException, eUnknown, "Attempt to write to read-only registry!");
    }
    return x_SetRegistry(writeSite).GetWriteView(section);
}

CRegistryWriteView CGuiRegistry::GetWriteView(const string& section,
                                              const vector<string>& subsections)
{
    if(m_isReadOnly) {
        NCBI_THROW(CException, eUnknown, "Attempt to write to read-only registry!");
    }
    // the section for a view made with subsections ends with
    // the most significant subsection.
    string str(section);
    if ( !section.empty() ) {
        str += kDecimalDot;
    }

    if ( ! subsections.empty()) {
        str += subsections.back();
    }

    return x_SetRegistry(GetWritableSite()).GetWriteView(str);
}

/////////////////////////////////////////////////////////////////////////////
///
/// CGuiRegistryUtil


/// create a key from a section and a subkey
string CGuiRegistryUtil::MakeKey(const string& section, const string& key,
                                 const string& delim)
{
    return section + delim + key;
}


END_NCBI_SCOPE
