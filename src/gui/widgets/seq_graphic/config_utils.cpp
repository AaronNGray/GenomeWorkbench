/*  $Id: config_utils.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/utils/track_info.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

string CSGConfigUtils::m_CommonDefaultStyleName = "Default";

CRegistryReadView
CSGConfigUtils::GetReadView(const CGuiRegistry& reg, const string& base_key,
                            const string& curr_key, const string& def_key1,
                            const string& def_key2, const string& def_key3)
{
    vector <string> keys;
    if ( !def_key3.empty()) {
        keys.push_back(def_key3);
    }
    if ( !def_key2.empty()  &&  def_key2 != def_key3 ) {
        keys.push_back(def_key2);
    }
    if ( !def_key1.empty()  &&  def_key1 != def_key2) {
        keys.push_back(def_key1);
    }
    if (curr_key != def_key1) {
        keys.push_back(curr_key);
    }
    return reg.GetReadView(base_key, keys);
}


CRegistryWriteView
CSGConfigUtils::GetWriteView(CGuiRegistry& reg, const string& base_key,
                                 const string& curr_key, const string& def_key1,
                                 const string& def_key2, const string& def_key3)
{
    vector <string> keys;
    if ( !def_key3.empty()) {
        keys.push_back(def_key3);
    }
    if ( !def_key2.empty()  &&  def_key2 != def_key3 ) {
        keys.push_back(def_key2);
    }
    if ( !def_key1.empty()  &&  def_key1 != def_key2) {
        keys.push_back(def_key1);
    }
    if (curr_key != def_key1) {
        keys.push_back(curr_key);
    }
    return reg.GetWriteView(base_key, keys);
}


void CSGConfigUtils::ParseProfileString(const string& profile_str,
                                        TKeyValuePairs& settings)
{
    typedef vector<string> TTokens;
    TTokens tokens;
    CTrackUtils::TokenizeWithEscape(profile_str, ",", tokens);
    ITERATE (vector<string>, iter, tokens) {
        TTokens pair;
        CTrackUtils::TokenizeWithEscape(*iter, ":", pair);
        if (pair.size() == 2) {
            settings[pair[0]] = pair[1];
        }
    }
}


string CSGConfigUtils::ComposeProfileString(const TKeyValuePairs& settings)
{
    string p_str = kEmptyStr;
    if (!settings.empty()) {
        auto it = settings.begin();
        p_str += it->first;
        p_str += ":";
        p_str += it->second;
        while (++it != settings.end()) {
            p_str += ",";
            p_str += it->first;
            p_str += ":";
            p_str += it->second;
        }
    }
    return p_str;
}


void CSGConfigUtils::GetFont(const CRegistryReadView& view,
                             const string& face_key, const string& size_key,
                             CGlTextureFont& f)
{
    CGlTextureFont::EFontFace font_face = CGlTextureFont::FaceFromString(
        view.GetString(face_key, "Helvetica"));
    unsigned int font_size =  (unsigned int)view.GetInt(size_key, 10);

    f.SetFontFace(font_face);
    f.SetFontSize(font_size);
}


void CSGConfigUtils::SetFont(CRegistryWriteView& view,
                             const string& face_key, const string& size_key,
                             const CGlTextureFont& f)
{
    view.Set(face_key, CGlTextureFont::FaceToString(f.GetFontFace()));
    view.Set(size_key, (int)f.GetFontSize());
}


void CSGConfigUtils::GetTexFont(const CRegistryReadView& view,
                                const string& face_key, const string& size_key,
                                CGlTextureFont& f)
{
    CGlTextureFont::EFontFace font_face = CGlTextureFont::FaceFromString(
        view.GetString(face_key, "Helvetica"));
    unsigned int font_size =  (unsigned int)view.GetInt(size_key, 10);

    f.SetFontFace(font_face);
    f.SetFontSize(font_size);
}


void CSGConfigUtils::SetTexFont(CRegistryWriteView& view,
                                const string& face_key, const string& size_key,
                                const CGlTextureFont& f)
{
    view.Set(face_key, CGlTextureFont::FaceToString(f.GetFontFace()));
    view.Set(size_key, (int)f.GetFontSize());
}


bool CSGConfigUtils::x_UserFieldToColor(const CUser_field& f, CRgbaColor& c)
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

void CSGConfigUtils::x_ColorToUserField(const CRgbaColor& c, CUser_field& f)
{
    // store the color as a vector of ints.
    vector<int> val;
    val.push_back(c.GetRedUC());
    val.push_back(c.GetGreenUC());
    val.push_back(c.GetBlueUC());
    val.push_back(c.GetAlphaUC());

    f.SetData().SetInts() = val;
}

END_NCBI_SCOPE
