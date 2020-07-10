/* $Id: settings_set.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Author:  Robert G. Smith
 *
 * File Description:
 *   CSettingsSet is an adapter of CGuiRegistry.
 *
 */

#include <ncbi_pch.hpp>
#include <gui/config/settings_set.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <algorithm>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// must be the same used as a default in CGuiRegistry.
const string    CSettingsSet::kDefaultDelim(".");
const string    CSettingsSet::sm_DefaultValuesKey("default_values");

static const string    kSettingsHeadKey("GBPlugins");
static const string    kStartupStyleKey("default_set");
static const string    kCurrentSetKey("current");
static const string    kNameKey("name");
static const string    kFeatureSectionKey("feats");

// a style with this name will not be shown in the list of styles.
// It can not be selected.
static const string    kHiddenName("*");

CSettingsSet::CSettingsSet(const string& key,
                           CGuiRegistry* registry)
: m_Registry( registry ? registry : &CGuiRegistry::GetInstance()),
    m_Type(key)
{
    CRegistryReadView set_view =
        m_Registry->GetReadView(x_GetSection());
    // make sure we find what we expect in the registry.
    if (set_view.IsEmpty()) {
        NCBI_THROW(CException, eUnknown,
                   "No type '" + m_Type + "'  in settings.");
    }
    if (GetTypeDescription().empty()) {
        NCBI_THROW(CException, eUnknown,
                   "No '" + kNameKey + "' in settings: " + m_Type);
    }
    if (set_view.GetField(sm_DefaultValuesKey).IsNull()) {
        NCBI_THROW(CException, eUnknown,
                   "No '" + sm_DefaultValuesKey + "' in settings: " + m_Type);
    }
    string current_style = GetCurrentStyleKey();
    if (current_style == sm_DefaultValuesKey) {
        // in case we SetDefaultCurrentStyle() without RestoreCurrentStyle()
        current_style = kStartupStyleKey;
    }
    SetCurrentStyleKey(current_style);
}


CSettingsSet::~CSettingsSet()
{
}


void CSettingsSet::Uncache()
{
    m_FieldDictionary.clear();
    m_ColorDictionary.clear();
}


string CSettingsSet::x_GetSection() const
{
    string sect(kSettingsHeadKey);
    sect += kDefaultDelim;
    sect += m_Type;
    return sect;
}


string CSettingsSet::x_GetStyleSection() const
{
    string sect(x_GetSection());
    sect += kDefaultDelim;
    sect += GetCurrentStyleKey();
    return sect;
}


const string&  CSettingsSet::GetType(void) const
{
    return m_Type;
}

string CSettingsSet::GetCurrentStyleName(void) const
{
    return x_GetStyleName(GetCurrentStyleKey());
}


void CSettingsSet::SetCurrentStyleName(const string& new_style)
{
    SetCurrentStyleKey(x_GetStyleKey(new_style));
}


string CSettingsSet::GetCurrentStyleKey(void) const
{
    return m_Registry->GetString
        (CGuiRegistryUtil::MakeKey(x_GetSection(), kCurrentSetKey),
         kStartupStyleKey);
}


void CSettingsSet::SetCurrentStyleKey(const string& new_style_key)
{
    m_Registry->Set
        (CGuiRegistryUtil::MakeKey(x_GetSection(), kCurrentSetKey),
         new_style_key);
    if (new_style_key != sm_DefaultValuesKey) {
        m_SavedStyle = new_style_key;
    }
    x_UncacheAll();
}


CConstRef<CUser_field>
CSettingsSet::x_GetCurrentViewField(const string& key) const
{
    CConstRef<CUser_field> ref;
    TFieldDictionary::iterator iter = m_FieldDictionary.find(key);
    if (iter != m_FieldDictionary.end()) {
        ref = iter->second;
    } else {
        ref = x_GetCurrentStyleView().GetField(key);
        m_FieldDictionary[key] = ref;
    }
    return ref;
}

void CSettingsSet::x_Uncache(const string& key) const
{
    m_FieldDictionary.erase(key);
    m_ColorDictionary.erase(key);
}

void CSettingsSet::x_UncacheAll() const
{
    m_FieldDictionary.clear();
    m_ColorDictionary.clear();
}

CRegistryReadView CSettingsSet::x_GetCurrentStyleView() const
{
    return x_GetStyleView(GetCurrentStyleKey());
}



CRegistryReadView
CSettingsSet::x_GetStyleView(const string& style_key) const
{
    vector <string> keys;
    keys.push_back(sm_DefaultValuesKey);
    if (style_key != sm_DefaultValuesKey) {
        keys.push_back(style_key);
    }
    return m_Registry->GetReadView(x_GetSection(), keys);
}


void CSettingsSet::SetDefaultCurrentStyle()
{
    m_SavedStyle = GetCurrentStyleKey();
    SetCurrentStyleKey(sm_DefaultValuesKey);
}


void CSettingsSet::RestoreCurrentStyle()
{
    SetCurrentStyleKey(m_SavedStyle);
}


string CSettingsSet::GetTypeDescription(void) const
{
    return m_Registry->GetString
        (CGuiRegistryUtil::MakeKey(x_GetSection(), kNameKey, kDefaultDelim),
         kEmptyStr);
}


/// every style has a name and a key. Both should be unique within this Type.
/// translate between style names and keys.
string CSettingsSet::x_GetStyleName(const string& key) const
{
    return m_Registry->GetString
        (CGuiRegistryUtil::MakeKey(x_GetSection(), key) +
         kDefaultDelim + kNameKey,
         kEmptyStr);
}


string CSettingsSet::x_GetStyleKey(const string& name) const
{
    CRegistryReadView set_view = m_Registry->GetReadView(x_GetSection());
    list <string> ret_list;
    CRegistryReadView::TKeys keys;
    set_view.GetTopKeys(keys);
    ITERATE(CRegistryReadView::TKeys, key_it, keys) {
        if (key_it->type == CUser_field::TData::e_Fields  &&
            x_GetStyleName(key_it->key) == name ) {
            return key_it->key;
        }
    }
    return kEmptyStr;
}


/// make unique names and keys.
/// result may be different than the argument.
string CSettingsSet::x_MakeUniqueStyleName(const string& name) const
{
    string uniq_name(name);
    unsigned long     suffix_number = 0;
    list<string> names = GetStyleNames();
    list<string>::iterator name_end = names.end();

    while (find(names.begin(), name_end, uniq_name) != name_end) {
        uniq_name = name + " " + NStr::SizetToString(++suffix_number);
    }
    return uniq_name;
}


string CSettingsSet::x_MakeUniqueStyleKey(const string& key) const
{
    string uniq_key(key);
    unsigned long     suffix_number = 0;
    while ( m_Registry->HasField(x_GetSection() + kDefaultDelim + uniq_key) ) {
        uniq_key = key + NStr::SizetToString(++suffix_number);
    }
    return uniq_key;
}


list<string> CSettingsSet::GetStyleNames() const
{
    CRegistryReadView set_view = m_Registry->GetReadView(x_GetSection());
    list <string> ret_list;
    CRegistryReadView::TKeys keys;
    set_view.GetTopKeys(keys);
    ITERATE(CRegistryReadView::TKeys, key_it, keys) {
        if (key_it->type == CUser_field::TData::e_Fields) {
            if (kStartupStyleKey != key_it->key) {
                string name = x_GetStyleName(key_it->key);
                if (name != kHiddenName )
                    ret_list.push_back(name);
            }
        }
    }
    ret_list.sort();
    ret_list.push_front(x_GetStyleName(kStartupStyleKey));

    return ret_list;
}



string CSettingsSet::AddStyle(void)
{
    string new_key("custom style");
    new_key = x_MakeUniqueStyleKey(new_key);
    string new_name("New Default Values");
    new_name = x_MakeUniqueStyleName(new_name);
    m_Registry->Set(x_GetSection() + kDefaultDelim +
                    new_key + kDefaultDelim + kNameKey,
                    new_name);
    return new_name;
}



string CSettingsSet::DuplicateStyle(const string& style)
{
    // To do this need to be able to duplicate a User_field.
    /*
    // make new unique name.
    string  new_name(style + " copy");
    new_name = x_MakeUniqueStyleName(new_name);

    // Some style names (default) may not actually exist
    // so there is nothing to duplicate.
    if (pcc.HasPCV(m_Type, style)) {
    pcc.DupPCV(m_Type, style, new_name);
    } else {
    pcc.SetPCV(m_Type, new_name);
    }
    return new_name;
     */
    return kEmptyStr;
}


string CSettingsSet::RenameStyle(const string& old_style, const string& new_style)
{
    if (new_style.empty() ||
        old_style == new_style ) {
        return kEmptyStr;
    }
    string style_key = x_GetStyleKey(old_style);
    if (style_key.empty())
        return kEmptyStr;

    string uniq_name = x_MakeUniqueStyleName(new_style);
    m_Registry->Set(x_GetSection() + kDefaultDelim +
                    style_key + kDefaultDelim + kNameKey, uniq_name);

    return uniq_name;
}


bool CSettingsSet::CanRenameStyle(const string& style)
{
    return true;
}


bool CSettingsSet::DeleteStyle(const string& style)
{
    string style_key = x_GetStyleKey(style);
    if (style_key.empty()  ||  style_key == sm_DefaultValuesKey)
        return false;

    return m_Registry->DeleteField(x_GetSection() + kDefaultDelim + style_key);
}


string CSettingsSet::GetString(const string& key,
                               const string& default_val) const
{
    CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
    if ( !ref ) {
        return default_val;
    }
    return ref->GetData().GetStr();
}


int CSettingsSet::GetInt(const string& key,
                         int default_val) const
{
    CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
    if ( !ref ) {
        return default_val;
    }
    return ref->GetData().GetInt();
}


double CSettingsSet::GetReal(const string& key,
                             double default_val) const
{
    CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
    if ( !ref ) {
        return default_val;
    }
    return ref->GetData().GetReal();
}


bool CSettingsSet::GetBool(const string& key,
                           bool default_val) const
{
    CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
    if ( !ref ) {
        return default_val;
    }
    return ref->GetData().GetBool();
}


void CSettingsSet::GetIntVec(const string& key,
                             vector<int>& val) const
{
    CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
    if (ref) {
        val = ref->GetData().GetInts();
    }
}


void CSettingsSet::GetRealVec(const string& key,
                              vector<double>& val) const
{
    CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
    if (ref) {
        val = ref->GetData().GetReals();
    }
}


void CSettingsSet::GetStringVec(const string& key,
                                vector<string>& val) const
{
    CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
    if (ref) {
        const CUser_field_Base::C_Data::TStrs& src = ref->GetData().GetStrs();
        val.clear();
        copy( src.begin(), src.end(), back_inserter(val));
    }
}

CRgbaColor CSettingsSet::GetColor(const string& key,
                                  const CRgbaColor& default_val) const
{
    TColorDictionary::const_iterator iter = m_ColorDictionary.find(key);
    if (iter != m_ColorDictionary.end()) {
        return iter->second;
    } else {
        CConstRef<CUser_field> ref = x_GetCurrentViewField(key);
        if ( !ref ) {
            return default_val;
        }
        if (ref) {
            CRgbaColor c;
            if (UserFieldToColor(*ref, c)) {
                m_ColorDictionary[key] = c;
                return c;
            }
        }
        return default_val;
    }
}


/// Get sub-views of the settings.
CRegistryReadView CSettingsSet::x_GetReadView(const string& section) const
{
    if (section.empty()) {
        return x_GetCurrentStyleView();
    }
    return x_GetCurrentStyleView().GetReadView(section);
}


/// retrieve the key name for a given feature subtype
string CSettingsSet::GetFeatReadKey(TFeatSubtype feat_subtype,
                                    const string& section,
                                    const string& subkey) const
{
    string sect(section);
    if (!sect.empty()) {
        sect += kDefaultDelim;
    }
    sect += kFeatureSectionKey;
    sect += kDefaultDelim;

    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    vector<string> feat_hierarchy = feats.GetStoragekeys(feat_subtype);

    // try them in reverse order so the more specific keys get tried first.
    vector<string>::reverse_iterator riter(feat_hierarchy.end());
    vector<string>::reverse_iterator rend(feat_hierarchy.begin());

    for ( ; riter != rend;  ++riter) {
        string s = sect + *riter;
        if ( ! subkey.empty()) {
            s += kDefaultDelim;
            s += subkey;
        }
        CConstRef<CUser_field> f = x_GetCurrentViewField(s);
        if (f) {
            return s;
        }
    }
    // shouldn't get here.
    // means that there wasn't a Master feature item in the global settings file.
    _ASSERT(false);
    return kEmptyStr;
}


string CSettingsSet::GetFeatWriteKey(TFeatSubtype feat_subtype,
                                     const string& section)
{
    string sect(section);
    if (!sect.empty()) {
        sect += kDefaultDelim;
    }
    sect += kFeatureSectionKey;
    sect += kDefaultDelim;

    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    string feat_key = feats.GetStoragekey(feat_subtype);
    sect += feat_key;
    return sect;
}

/// set specific values
void CSettingsSet::Set(const string& key, int val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, double val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, bool val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, const string& val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, const char* val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, const vector<int>& val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, const vector<double>& val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, const vector<string>& val)
{
    m_Registry->Set(x_GetStyleSection() + kDefaultDelim + key, val);
    x_Uncache(key);
}


void CSettingsSet::Set(const string& key, const CRgbaColor& val)
{
    CRef<CUser_field> f =
        m_Registry->SetFieldToValue(x_GetStyleSection() + kDefaultDelim + key);
    ColorToUserField(val, *f);
    x_Uncache(key);
}


bool CSettingsSet::Delete(const string& key)
{
    return m_Registry->DeleteField(x_GetStyleSection() + kDefaultDelim + key);
    //x_Uncache(key);
}


void CSettingsSet::ColorToUserField(const CRgbaColor& c, CUser_field& f)
{
    // store the color as a vector of ints.
    vector<int> val;
    val.push_back(c.GetRedUC());
    val.push_back(c.GetGreenUC());
    val.push_back(c.GetBlueUC());
    val.push_back(c.GetAlphaUC());

    f.SetData().SetInts() = val;
}


bool CSettingsSet::UserFieldToColor(const CUser_field& f, CRgbaColor& c)
{
    switch(f.GetData().Which()) {
    case CUser_field::TData::e_Ints:
        {{
             vector<int> cvi = f.GetData().GetInts();
             if (cvi.size() > 2 ) {
                 vector<int>::const_iterator cv_it = cvi.begin();
                 c.SetRed  (static_cast<unsigned char>(*cv_it++));
                 c.SetGreen(static_cast<unsigned char>(*cv_it++));
                 c.SetBlue (static_cast<unsigned char>(*cv_it++));
                 if (cv_it != cvi.end()) {
                     c.SetAlpha(static_cast<unsigned char>(*cv_it));
                 }
             } else {
                 return false;
             }
             break;
         }}
    case CUser_field::TData::e_Reals:   {{
         vector<double> cvd = f.GetData().GetReals();
         if (cvd.size() > 2 ) {
             vector<double>::const_iterator cv_it = cvd.begin();
             c.SetRed  (static_cast<float>(*cv_it++));
             c.SetGreen(static_cast<float>(*cv_it++));
             c.SetBlue (static_cast<float>(*cv_it++));
             if (cv_it != cvd.end()) {
                 c.SetAlpha(static_cast<float>(*cv_it));
             }
         } else {
             return false;
         }
         break;
     }}
    case CUser_field::TData::e_Str: {{
         string cs = f.GetData().GetStr();
         c.FromString(cs);
         break;
     }}
    default:
        return false;
    }

    return true;
}



void CSettingsSet::DumpAll(CNcbiOstream& ostr)
{
    ostr << "Current style key : " << GetCurrentStyleKey() << endl;
    ostr << "Current style name: " << GetCurrentStyleName() << endl;
    x_GetCurrentStyleView().DumpAll(ostr);
}


END_NCBI_SCOPE
