/*  $Id: reg_file.cpp 39959 2017-11-30 17:10:19Z katargir $
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
#include <corelib/ncbireg.hpp>
#include <gui/objutils/reg_file.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_field.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const string kConfigObjClass("NCBI");
static const string kConfigObjType("GUI-App-Registry");
static const string kMetaSuffix = "-meta";
static const string kDecimalDot = ".";


CRegistryFile::CRegistryFile()
{
    SetRegistry();
}


CRegistryFile::CRegistryFile(CNcbiIstream& istr)
{
    Read(istr);
}


CRegistryFile::CRegistryFile(const objects::CUser_object& obj)
{
    SetRegistry(obj);
}

CRegistryReadView CRegistryFile::GetReadView(const string& section) const
{
    return CRegistryReadView(GetField(section));
}

CRegistryWriteView CRegistryFile::GetWriteView(const string& section)
{
    return CRegistryWriteView(section, *this);
}

/// recursively navigate an object tree, retrieving an existing named key
static CConstRef<CUser_field> s_GetFieldAt(const CUser_object& obj, const string& key, const string& delim)
{
#ifdef _DEBUG
    {{
            _ASSERT(key.find(delim + delim) == string::npos);
            _ASSERT(key.find_first_of(delim) != 0);
            _ASSERT(key.size() && key.find_first_of(delim) != key.size() - 1);
        }}
#endif

    string first_key(key);
    string remainder;
    string::size_type pos = key.find_first_of(delim);
    if (pos != string::npos) {
        first_key = key.substr(0, pos);
        remainder = key.substr(pos + 1, key.length() - pos - 1);
    }

    CConstRef<CUser_field>  field_ref;
    ITERATE(CUser_object::TData, field_iter, obj.GetData()) {
        const CUser_field& f = **field_iter;
        if (f.GetLabel().GetStr() == first_key) {
            if (remainder.empty()) {
                field_ref.Reset(&f);
                break;
            }
            else if (f.GetData().IsFields()) {
                field_ref = f.GetFieldRef(remainder, delim);
                if (field_ref) {
                    break;
                }
            }
        }
    }
    return field_ref;
}

/// recursively navigate an object tree, setting keys where necessary to get
/// a named level
static CRef<CUser_field> s_SetFieldAt(CUser_object& obj, const string& key, const string& delim)
{
#ifdef _DEBUG
    {{
            _ASSERT(key.find(delim + delim) == string::npos);
            _ASSERT(key.find_first_of(delim) != 0);
            _ASSERT(key.size() && key.find_first_of(delim) != key.size() - 1);
        }}
#endif

    string first_key(key);
    string remainder;
    string::size_type pos = key.find_first_of(delim);
    if (pos != string::npos) {
        first_key = key.substr(0, pos);
        remainder = key.substr(pos + 1, key.length() - pos - 1);
    }

    CRef<CUser_field>  field_ref;
    NON_CONST_ITERATE(CUser_object::TData, field_iter, obj.SetData()) {
        CUser_field& f = **field_iter;
        if (f.GetLabel().GetStr() == first_key) {
            if (remainder.empty()) {
                field_ref.Reset(&f);
                break;
            }
            else {
                field_ref = f.SetFieldRef(remainder, delim);
                if (field_ref) {
                    break;
                }
            }
        }
    }

    if (!field_ref) {
        /// create the field for this level
        field_ref.Reset(new CUser_field());
        field_ref->SetLabel().SetStr(first_key);
        obj.SetData().push_back(field_ref);

        if (!remainder.empty()) {
            CRef<CUser_field> f = field_ref->SetFieldRef(remainder, delim);
            field_ref = f;
        }
    }

    return field_ref;
}

void CRegistryFile::FromConfigFile(CNcbiIstream& istr)
{
    // in the incoming registry, meta sections will end with this:
    static string kMetaKey(".meta");

    CNcbiRegistry reg(istr, IRegistry::fInternalSpaces);

    list<string> meta_sections;
    list<string> sections;
    reg.EnumerateSections(&sections, IRegistry::fInternalSpaces);
    ITERATE (list<string>, iter, sections) {
        string section(*iter);
        string::size_type meta_pos = iter->find(kMetaKey);
        if (meta_pos == iter->length() - kMetaKey.length()) {
            // replace NcbiRegistry's meta key with CGuiRegistry's meta suffix.
            string non_meta = section.erase(meta_pos);
            section = non_meta + kMetaSuffix;
            meta_sections.push_back(section);

            CConstRef<CUser_field> cf = s_GetFieldAt(GetRegistry(), non_meta, kDecimalDot);
            if ( !cf ) {
                /// add an empty string field
                CRef<CUser_field> f = s_SetFieldAt(SetRegistry(), non_meta, kDecimalDot);
                f->SetData().SetStr();
            }
        }

        list<string> entries;
        reg.EnumerateEntries(*iter, &entries, IRegistry::fInternalSpaces);
        ITERATE (list<string>, ent_iter, entries) {

            string val = reg.Get(*iter, *ent_iter, IRegistry::fInternalSpaces);

            CRef<CUser_field> obj = s_SetFieldAt(SetRegistry(), section, kDecimalDot);
            _ASSERT(obj);

            try {
                int i = NStr::StringToInt(val);
                obj->AddField(*ent_iter, i);
                continue;
            }
            catch (std::exception&) {
            }

            try {
                double d = NStr::StringToDouble(val);
                obj->AddField(*ent_iter, d);
                continue;
            }
            catch (std::exception&) {
            }

            try {
                bool b = NStr::StringToBool(val);
                obj->AddField(*ent_iter, b);
                continue;
            }
            catch (std::exception&) {
            }

            obj->AddField(*ent_iter, val);
        }
    }

    ITERATE (list<string>, iter, meta_sections) {
        // we currently only post-process the type field
        CConstRef<CUser_field> type_field = GetField(*iter + ".Type");
        if ( !type_field  ||  !type_field->GetData().IsStr() ) {
            continue;
        }
        string type = type_field->GetData().GetStr();
        if (type.empty()) {
            continue;
        }

        string::size_type pos;
        string section(*iter);

        // erase the '-meta'
        pos = section.rfind(kMetaSuffix);
        section.erase(pos);

        CRef<CUser_field> field = SetField(section);
        if ( !field ) {
            continue;
        }

        if (type == "intvec") {
            vector<int> val;

            switch (field->GetData().Which()) {
            case CUser_field::TData::e_Str:
                {{
                    string str = field->GetData().GetStr();
                    list<string> toks;
                    NStr::Split(str, ",", toks, NStr::fSplit_Tokenize);
                    ITERATE (list<string>, it, toks) {
                        val.push_back
                            (NStr::StringToInt(NStr::TruncateSpaces(*it)));
                    }
                    field->SetData().SetInts() = val;
                }}
                break;

            case CUser_field::TData::e_Int:
                val.push_back(field->GetData().GetInt());
                field->SetData().SetInts() = val;
                break;

            case CUser_field::TData::e_Object:
                break;

            default:
                LOG_POST(Error << "unexpected type in conversion to int-vec");
                break;
            }

        } else if (type == "realvec") {
            vector<double> val;

            switch (field->GetData().Which()) {
            case CUser_field::TData::e_Str:
                {{
                    string str = field->GetData().GetStr();
                    list<string> toks;
                    NStr::Split(str, ",", toks, NStr::fSplit_Tokenize);
                    ITERATE (list<string>, it, toks) {
                        val.push_back
                            (NStr::StringToDouble(NStr::TruncateSpaces(*it)));
                    }
                    field->SetData().SetReals() = val;
                }}
                break;

            case CUser_field::TData::e_Real:
                val.push_back(field->GetData().GetReal());
                field->SetData().SetReals() = val;
                break;

            case CUser_field::TData::e_Object:
                break;

            default:
                LOG_POST(Error << "unexpected type in conversion to int-vec");
                break;
            }

        } else if (type == "stringvec") {
            CUser_field_Base::C_Data::TStrs val;

            switch (field->GetData().Which()) {
            case CUser_field::TData::e_Str:
                {{
                    string str = field->GetData().GetStr();
                    list<string> toks;
                    NStr::Split(str, ",", toks, NStr::fSplit_Tokenize);
                    ITERATE (list<string>, it, toks) {
                        val.push_back(NStr::TruncateSpaces(*it));
                    }
                    field->SetData().SetStrs() = val;
                }}
                break;

            case CUser_field::TData::e_Int:
                val.push_back(NStr::IntToString(field->GetData().GetInt()));
                field->SetData().SetStrs() = val;
                break;

            case CUser_field::TData::e_Object:
                break;

            default:
                LOG_POST(Error << "unexpected type in conversion to string-vec");
                break;
            }
        }
    }
}


static void s_ScanRecursive(const CUser_object& obj,
                            CNcbiRegistry& reg,
                            const string& field_tag = kEmptyStr)
{
    ITERATE (CUser_object::TData, iter, obj.GetData()) {

        const CUser_field& field = **iter;

        switch (field.GetData().Which()) {
        case CUser_field::TData::e_Object:
            {{
                string tag(field_tag);
                if ( !tag.empty() ) {
                    tag += '.';
                }
                tag += field.GetLabel().GetStr();
                s_ScanRecursive(field.GetData().GetObject(), reg, tag);
            }}
            break;

        case CUser_field::TData::e_Bool:
            reg.Set(field_tag, field.GetLabel().GetStr(),
                    NStr::BoolToString(field.GetData().GetBool()),
                    CNcbiRegistry::ePersistent);
            break;

        case CUser_field::TData::e_Int:
            reg.Set(field_tag, field.GetLabel().GetStr(),
                    NStr::IntToString(field.GetData().GetInt()),
                    CNcbiRegistry::ePersistent);
            break;

        case CUser_field::TData::e_Ints:
            {{
                string str;
                ITERATE (CUser_field::TData::TInts, iter, field.GetData().GetInts()) {
                    if ( !str.empty() ) {
                        str += ", ";
                    }
                    str += NStr::IntToString(*iter);
                }
                reg.Set(field_tag, field.GetLabel().GetStr(), str,
                        CNcbiRegistry::ePersistent);
            }}
            break;

        case CUser_field::TData::e_Real:
            reg.Set(field_tag, field.GetLabel().GetStr(),
                    NStr::DoubleToString(field.GetData().GetReal()),
                    CNcbiRegistry::ePersistent);
            break;

        case CUser_field::TData::e_Reals:
            {{
                string str;
                ITERATE (CUser_field::TData::TReals, iter, field.GetData().GetReals()) {
                    if ( !str.empty() ) {
                        str += ", ";
                    }
                    str += NStr::DoubleToString(*iter);
                }
                reg.Set(field_tag, field.GetLabel().GetStr(), str,
                        CNcbiRegistry::ePersistent);
            }}
            break;

        case CUser_field::TData::e_Str:
            reg.Set(field_tag, field.GetLabel().GetStr(),
                    field.GetData().GetStr(),
                    CNcbiRegistry::ePersistent);
            break;

        case CUser_field::TData::e_Strs:
            {{
                string str;
                ITERATE (CUser_field::TData::TStrs, iter, field.GetData().GetStrs()) {
                    if ( !str.empty() ) {
                        str += ", ";
                    }
                    str += *iter;
                }
                reg.Set(field_tag, field.GetLabel().GetStr(), str,
                        CNcbiRegistry::ePersistent);
            }}
            break;

        default:
            LOG_POST(Warning << "unhandled tag: "
                << field_tag << kDecimalDot << field.GetLabel().GetStr());
            break;
        }
    }
}

void CRegistryFile::ToConfigFile(CNcbiOstream& ostr) const
{
    if ( !m_Registry ) {
        return;
    }

    CNcbiRegistry reg;

    s_ScanRecursive(*m_Registry, reg);
    reg.Write(ostr);
}


void CRegistryFile::Read(CNcbiIstream& istr)
{
    istr >> MSerial_AsnText >> SetRegistry();
}

void CRegistryFile::Write(CNcbiOstream& ostr) const
{
    if (m_Registry) {
        ostr << MSerial_AsnText << GetRegistry();
    }
}


bool CRegistryFile::HasField(const string& key) const
{
    if ( !m_Registry ) {
        return false;
    }

    CConstRef<CUser_field> obj = s_GetFieldAt(*m_Registry, key, kDecimalDot);
    return obj ? true : false;
}


CConstRef<CUser_field> CRegistryFile::GetField(const string& key) const
{
    CConstRef<CUser_field> f;
    if (m_Registry) {
        f = s_GetFieldAt(*m_Registry, key, kDecimalDot);
    }
    return f;
}



CRef<CUser_field> CRegistryFile::SetField(const string& key)
{
    return s_SetFieldAt(SetRegistry(), key, kDecimalDot);
}


bool CRegistryFile::DeleteField(const string& key)
{
    string first_key, remainder;
    if (!NStr::SplitInTwo(key, kDecimalDot, first_key, remainder)) {
        // should be no need to delete top-level keys.
        // if there is, need to implement it in CUser_object.
        return false;
    }

    CRef<CUser_field> parent_field = SetField(first_key);
    if (! parent_field) {
        return false;
    }
    return parent_field->DeleteField(remainder, kDecimalDot);
}


CUser_object& CRegistryFile::SetRegistry()
{
    if ( !m_Registry ) {
        m_Registry = NewRegistryObject();
    }
    return *m_Registry;
}



void CRegistryFile::SetRegistry(const objects::CUser_object& reg)
{
    _ASSERT(IsRegistry(reg));

    m_Registry.Reset();
    SetRegistry();
    m_Registry->Assign(reg);
}


const CUser_object& CRegistryFile::GetRegistry() const
{
    _ASSERT(m_Registry);
    return *m_Registry;
}


CRef<CUser_field> CRegistryFile::SetFieldToValue(const string& key)
{
    CRef<CUser_field> field = SetField(key);
    if (field->GetData().IsFields()) {
        // field is a place for subkeys, not for a value.
        NCBI_THROW(CException, eUnknown,
                   "Too few components in key \"" + key + "\"");
    }
    return field;
}


bool CRegistryFile::IsRegistry(const objects::CUser_object& uo)
{
    return uo.CanGetClass()  &&  uo.GetClass() == kConfigObjClass  &&
           uo.GetType().IsStr()  &&  uo.GetType().GetStr() == kConfigObjType ;
}


CRef<objects::CUser_object> CRegistryFile::NewRegistryObject()
{
    CRef<objects::CUser_object> uo_ref;

    uo_ref.Reset(new CUser_object());
    uo_ref->SetClass(kConfigObjClass);
    uo_ref->SetType().SetStr(kConfigObjType);
    uo_ref->SetData();
    return uo_ref;
}


END_NCBI_SCOPE
