/*  $Id: reg_view.cpp 39783 2017-11-03 15:04:54Z katargir $
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
#include <gui/objutils/reg_view.hpp>
#include <gui/objutils/reg_file.hpp>

#include <objects/general/Object_id.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const string kDecimalDot = ".";

/////////////////////////////////////////////////////////////////////////////
///
/// CRegistryReadView

CRegistryReadView::CRegistryReadView(const CUser_field* userField)
{
    if (userField)
        m_Fields.push_back(CConstRef<objects::CUser_field>(userField));
}


CRegistryReadView& CRegistryReadView::operator+=(const CRegistryReadView& rhs)
{
    m_Fields.insert(m_Fields.end(), rhs.m_Fields.begin(), rhs.m_Fields.end());
    return *this;
}

void CRegistryReadView::x_AppendToReadView(CRegistryReadView& view, const string& section) const
{
    ITERATE(TFields, iter, m_Fields) {
        const CUser_field& obj = **iter;
        CConstRef<CUser_field> section_obj =
            obj.GetFieldRef(section);
        if (section_obj) {
            view.m_Fields.push_back(section_obj);
        }
    }
}


CRegistryReadView CRegistryReadView::GetReadView(const string& section) const
{
    CRegistryReadView view;
    x_AppendToReadView(view, section);
    return view;
}


CRegistryReadView CRegistryReadView::GetReadView(const string& section,
                                                 const vector<string>& subsections) const
{
    CRegistryReadView view;
    string sec(section);
    if (!sec.empty()) {
        sec += kDecimalDot;
    }

    vector<string>::const_reverse_iterator riter(subsections.end());
    vector<string>::const_reverse_iterator rend(subsections.begin());
    for (; riter != rend; ++riter) {
        string key(*riter);
        if (!key.empty()) {
            x_AppendToReadView(view, sec + key);
        }
    }

    return view;
}


CConstRef<objects::CUser_field> CRegistryReadView::GetField(const string& key) const
{
    list< CConstRef<CUser_field> > fields;
    CConstRef<CUser_field> meta;

    ITERATE(TFields, iter, m_Fields) {
        const CUser_field& obj = **iter;
        CConstRef<CUser_field> field = obj.GetFieldRef(key);
        if (field)
            fields.push_back(field);

        /// access the meta-information for this key, last one takes precedence.
        field = obj.GetFieldRef(key + "-meta");
        if (field)
            meta = field;
    }

    return ResolveField(fields, meta);
}

int CRegistryReadView::GetInt(const objects::CUser_field* field, int default_val)
{
    return (field && field->GetData().IsInt()) ? 
        field->GetData().GetInt() : default_val;
}

double CRegistryReadView::GetReal(const objects::CUser_field* field, double default_val)
{
    return (field && field->GetData().IsReal()) ?
        field->GetData().GetReal() : default_val;
}

bool CRegistryReadView::GetBool(const objects::CUser_field* field, bool default_val)
{
    return (field && field->GetData().IsBool()) ?
        field->GetData().GetBool() : default_val;
}

string CRegistryReadView::GetString(const objects::CUser_field* field, const string& default_val)
{
    return (field && field->GetData().IsStr()) ?
        field->GetData().GetStr() : default_val;
}

void CRegistryReadView::GetIntVec(const objects::CUser_field* field, vector<int>& val)
{
    val.clear();
    if (!field) return;

    if (field->GetData().IsInt()) {
        val.push_back(field->GetData().GetInt());
    }
    else if (field->GetData().IsInts()) {
        val = field->GetData().GetInts();
    }
    else if (field->GetData().IsFields()) {
        ITERATE(CUser_field::TData::TFields, iter, field->GetData().GetFields()) {
            const CUser_field& f = **iter;
            if (f.GetData().IsInt()) {
                val.push_back(f.GetData().GetInt());
            }
            else if (f.GetData().IsInts()) {
                val.insert(val.end(),
                    f.GetData().GetInts().begin(),
                    f.GetData().GetInts().end());
            }
        }
    }
}

void CRegistryReadView::GetRealVec(const objects::CUser_field* field, vector<double>& val)
{
    val.clear();
    if (!field) return;

    if (field->GetData().IsReal()) {
        val.push_back(field->GetData().GetReal());
    }
    else if (field->GetData().IsReals()) {
        val = field->GetData().GetReals();
    }
    else if (field->GetData().IsFields()) {
        ITERATE(CUser_field::TData::TFields, iter, field->GetData().GetFields()) {
            const CUser_field& f = **iter;
            if (f.GetData().IsReal()) {
                val.push_back(f.GetData().GetReal());
            }
            else if (f.GetData().IsReals()) {
                val.insert(val.end(),
                    f.GetData().GetReals().begin(),
                    f.GetData().GetReals().end());
            }
        }
    }
}

void CRegistryReadView::GetStringVec(const objects::CUser_field* field, vector<string>& val)
{
    val.clear();
    if (!field) return;

    if (field->GetData().IsStr()) {
        val.push_back(field->GetData().GetStr());
    }
    else if (field->GetData().IsStrs()) {
        val.insert(val.end(),
            field->GetData().GetStrs().begin(),
            field->GetData().GetStrs().end());
    }
    else if (field->GetData().IsFields()) {
        ITERATE(CUser_field::TData::TFields, iter, field->GetData().GetFields()) {
            const CUser_field& f = **iter;
            if (f.GetData().IsStr()) {
                val.push_back(f.GetData().GetStr());
            }
            else if (f.GetData().IsStrs()) {
                val.insert(val.end(),
                    f.GetData().GetStrs().begin(),
                    f.GetData().GetStrs().end());
            }
        }
    }
}


/// type-specific field accessors
int CRegistryReadView::GetInt(const string& key, int default_val) const
{
    return GetInt(GetField(key), default_val);
}

double CRegistryReadView::GetReal(const string& key, double default_val) const
{
    return GetReal(GetField(key), default_val);
}


bool CRegistryReadView::GetBool(const string& key, bool default_val) const
{
    return GetBool(GetField(key), default_val);
}

string CRegistryReadView::GetString(const string& key, const string& default_val) const
{
    return GetString(GetField(key), default_val);
}


void CRegistryReadView::GetIntVec(const string& key, vector<int>& val) const
{
    GetIntVec(GetField(key), val);
}

void CRegistryReadView::GetRealVec(const string& key, vector<double>& val) const
{
    GetRealVec(GetField(key), val);
}


void CRegistryReadView::GetStringVec(const string& key, vector<string>& val) const
{
    GetStringVec(GetField(key), val);
}

void CRegistryReadView::GetStringList(const string& key, list<string>& val) const
{
    vector<string> v;
    GetStringVec(GetField(key), v);

    val.clear();
    std::copy(v.begin(), v.end(), std::back_inserter(val));
}


void CRegistryReadView::GetTopKeys(TKeys& keys) const
{
    x_GetKeys(keys, false);
}


void CRegistryReadView::GetKeys(TKeys& keys) const
{
    x_GetKeys(keys, true);
}

static void s_ExtractKeys(const CUser_field& field,
    const string& root,
    CRegistryReadView::TKeys& key_info,
    bool recurse)
{
    CRegistryReadView::SKeyInfo info;
    info.key = root;
    if (!info.key.empty()) {
        info.key += ".";
    }
    info.key += field.GetLabel().GetStr();
    info.type = field.GetData().Which();
    key_info.push_back(info);

    if (recurse) {
        switch (field.GetData().Which()) {
        case CUser_field::TData::e_Fields:
        {{
                string subkey = root;
                if (!subkey.empty()) {
                    subkey += ".";
                }
                subkey += field.GetLabel().GetStr();
                ITERATE(CUser_field::TData::TFields, iter,
                    field.GetData().GetFields()) {
                    s_ExtractKeys(**iter, subkey, key_info, true);
                }
            }}
        break;

        default:
            break;
        }
    }
}

void CRegistryReadView::x_GetKeys(TKeys& keys, bool recurse) const
{
    typedef map<string, SKeyInfo> TKeyMap;
    TKeyMap key_map;    // used to sort the final list.

    TFields::const_reverse_iterator riter(m_Fields.rbegin());
    TFields::const_reverse_iterator rend(m_Fields.rend());

    for (; riter != rend; ++riter) {
        const CUser_field& obj = **riter;
        if (obj.GetData().IsFields()) {
            TKeys temp;
            ITERATE(CUser_field::TData::TFields, sub_iter,
                obj.GetData().GetFields()) {
                s_ExtractKeys(**sub_iter, "", temp, recurse);
            }
            ITERATE(TKeys, key_it, temp) {
                key_map[key_it->key] = *key_it;
            }
        }
    }

    keys.clear();
    ITERATE(TKeyMap, iter, key_map)
        keys.push_back(iter->second);
}

bool CRegistryReadView::HasField(const string& key) const
{
    ITERATE(TFields, iter, m_Fields) {
        const CUser_field& obj = **iter;
        if (obj.HasField(key))
            return true;
    }
    return false;
}


bool CRegistryReadView::IsEmpty()
{
    return m_Fields.empty();
}

CConstRef<CUser_field> CRegistryReadView::ResolveField(
    list< CConstRef<CUser_field> >& fields, const CUser_field* meta)
{
    if (fields.empty())
        return CConstRef<CUser_field>();

    if (fields.size() == 1)
        return fields.front();

    int merge_policy = 0;
    if (meta) {
        CConstRef<CUser_field> merge_field = meta->GetFieldRef("MergePolicy");
        if (merge_field && merge_field->GetData().IsStr()) {
            string str = merge_field->GetData().GetStr();
            NStr::ToLower(str);
            if (str == "append") {
                merge_policy = 1;
            }
            else if (str == "prepend") {
                merge_policy = 2;
            }
        }
    }

    //
    // all fields should be objects, so look for sub-fields that
    // meet the criteria we want
    //
    switch (merge_policy) {
    case 1:
        {{
            CRef<CUser_field> field;
            field.Reset(new CUser_field());
            ITERATE(list< CConstRef<CUser_field> >, iter, fields) {
                CRef<CUser_field> f
                    (const_cast<CUser_field*>(iter->GetPointer()));
                field->SetData().SetFields().push_back(f);
            }
            return CConstRef<CUser_field>(field.Release());
        }}

    case 2:
        {{
            CRef<CUser_field> field;
            std::reverse(fields.begin(), fields.end());
            field.Reset(new CUser_field());
            ITERATE(list< CConstRef<CUser_field> >, iter, fields) {
                CRef<CUser_field> f
                    (const_cast<CUser_field*>(iter->GetPointer()));
                field->SetData().SetFields().push_back(f);
            }
            return CConstRef<CUser_field>(field.Release());
        }}
    default:
        return fields.front();
    }
}

void CRegistryReadView::DumpAll(CNcbiOstream& ostr) const
{
    TKeys keys;
    GetKeys(keys);

    ostr << keys.size() << " keys:" << endl;
    ITERATE(TKeys, iter, keys) {
        const SKeyInfo& key = *iter;

        ostr << key.key << "|";

        vector<string> strs;
        vector<int> ints;
        vector<double> reals;
        switch (key.type) {
        case CUser_field::TData::e_Bool:
            ostr << "bool|" << NStr::BoolToString(GetBool(key.key));
            break;
        case CUser_field::TData::e_Int:
            ostr << "int|" << GetInt(key.key);
            break;
        case CUser_field::TData::e_Ints:
            ostr << "ints|";
            GetIntVec(key.key, ints);
            copy(ints.begin(), ints.end(), ostream_iterator<int>(ostr, ";"));
            break;
        case CUser_field::TData::e_Real:
            ostr << "real|" << GetReal(key.key);
            break;
        case CUser_field::TData::e_Reals:
            ostr << "reals|";
            GetRealVec(key.key, reals);
            copy(reals.begin(), reals.end(), ostream_iterator<double>(ostr, ";"));
            break;
        case CUser_field::TData::e_Str:
            ostr << "string|" << GetString(key.key);;
            break;
        case CUser_field::TData::e_Strs:
            ostr << "strings|";
            GetStringVec(key.key, strs);
            copy(strs.begin(), strs.end(), ostream_iterator<string>(ostr, ";"));
            break;
        case CUser_field::TData::e_Fields:
            ostr << "subkey|";
            break;
        default:
            ostr << "unknown|";
            break;
        }
        ostr << endl;

    }
}

/////////////////////////////////////////////////////////////////////////////
///
/// CRegistryWriteView

CRegistryWriteView::CRegistryWriteView(const string &section, CRegistryFile& regFile)
    : m_Section(section), m_RegistryFile(&regFile)
{
}

static const char* kEmptyViewError = "Writing to empty CRegistryWriteView";

CRegistryWriteView CRegistryWriteView::GetWriteView(const string& section)
{
    if (!m_RegistryFile) {
        LOG_POST(Error << kEmptyViewError);
        return CRegistryWriteView();
    }

    return CRegistryWriteView(m_Section + kDecimalDot + section, *m_RegistryFile);
}

/// retrieve a writeable field at a particular section and key
CRef<CUser_field> CRegistryWriteView::SetField(const string& key)
{
    if (!m_RegistryFile) {
        LOG_POST(Error << kEmptyViewError);
        return CRef<CUser_field>(new CUser_field());
    }

    return m_RegistryFile->SetField(m_Section + kDecimalDot + key);
}

CRef<CUser_field> CRegistryWriteView::SetFieldToValue(const string& key)
{
    CRef<CUser_field> section_obj = SetField(key);
    if (section_obj->GetData().IsFields()) {
        // field is a place for subkeys, not for a value.
        NCBI_THROW(CException, eUnknown,
            "Too few components in key \"" + key + "\"");
    }
    return section_obj;
}

bool CRegistryWriteView::DeleteField(const string& key)
{
    if (!m_RegistryFile) {
        LOG_POST(Error << kEmptyViewError);
        return false;
    }
    return m_RegistryFile->DeleteField(m_Section + kDecimalDot + key);
}

/// set specific values
void CRegistryWriteView::Set(const string& key, int val)
{
    CRef<CUser_field> field = SetFieldToValue(key);
    field->SetData().SetInt(val);
}

void CRegistryWriteView::Set(const string& key, double val)
{
    CRef<CUser_field> field = SetFieldToValue(key);
    field->SetData().SetReal(val);
}

void CRegistryWriteView::Set(const string& key, bool val)
{
    CRef<CUser_field> field = SetFieldToValue(key);
    field->SetData().SetBool(val);
}

void CRegistryWriteView::Set(const string& key, const string& val)
{
    CRef<CUser_field> field = SetFieldToValue(key);
    field->SetData().SetStr(val);
}

void CRegistryWriteView::Set(const string& key, const char* val)
{
    Set(key, string(val));
}

void CRegistryWriteView::Set(const string& key, const vector<int>& val)
{
    CRef<CUser_field> field = SetFieldToValue(key);
    field->SetData().SetInts() = val;
}

void CRegistryWriteView::Set(const string& key, const vector<double>& val)
{
    CRef<CUser_field> field = SetFieldToValue(key);
    field->SetData().SetReals() = val;
}

void CRegistryWriteView::Set(const string& key, const vector<string>& val)
{
    CRef<CUser_field> field = SetFieldToValue(key);
    CUser_field_Base::C_Data::TStrs& dest = field->SetData().SetStrs();
    dest.clear();
    ITERATE(vector<string>, v, val) {
        dest.push_back(*v);
    }
}

void CRegistryWriteView::Set(const string& key, const list<string>& val)
{
    vector<string> v;
    v.reserve(val.size());
    ITERATE(list<string>, it, val) {
        v.push_back(*it);
    }
    Set(key, v);
}

END_NCBI_SCOPE
