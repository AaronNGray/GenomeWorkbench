/*  $Id: track_info.cpp 44124 2019-10-31 19:50:19Z rudnev $
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
 * Author:  Dmitry Rudnev
 *
 * File Description:
 *  Implements the functions in track_info.hpp
 */

#include <ncbi_pch.hpp>

#include <set>
#include <sstream>

#include <gui/utils/track_info.hpp>


BEGIN_NCBI_SCOPE

const size_t TTrackErrorStatus::cShortErrorLength = 27;



const string CTrackUtils::cTrackSettingKey_PrivatePrefix = "###";
const string CTrackUtils::cTrackSettingKey_ErrorSeverity = "###severity";
const string CTrackUtils::cTrackSettingKey_ErrorCode = "###code";
const string CTrackUtils::cTrackSettingKey_ErrorShortMsg = "###short_msg";
const string CTrackUtils::cTrackSettingKey_ErrorMsg = "###msg";
const string CTrackUtils::cTrackSettingKey_AccessType = "###access_type";
const string CTrackUtils::cTrackSettingKey_OrigAccession = "###orig_accession";

const string CTrackUtils::kStdTrackId = "STD";

static
string s_SVURLDecode(const string& src)
{   
    string dst;

    SIZE_TYPE len = src.length();
    if ( !len ) {
        return dst;
    }

    dst.resize(len);

    SIZE_TYPE pdst = 0;
    for (SIZE_TYPE psrc = 0;  psrc < len;  pdst++) {
        switch ( src[psrc] ) {
        case '\\': {
            if (psrc + 2 > len) {
                dst[pdst] = src[psrc++];
            } else {
                int n1 = NStr::HexChar(src[psrc+1]);
                int n2 = NStr::HexChar(src[psrc+2]);
                if (n1 < 0  ||  n1 > 15  || n2 < 0  ||  n2 > 15) {
                    dst[pdst] = src[psrc++];
                } else {
                    dst[pdst] = (n1 << 4) | n2;
                    psrc += 3;
                }
            }
            break;
        }
        default:
            dst[pdst] = src[psrc++];
        }
    }
    if (pdst < len) {
        dst.resize(pdst);
    }
    return dst;
}


void CTrackUtils::ExtractTrackConfig(const string& track_str,
                                   TTrackSettingsSet& settings_set,
                                   list<string> *warning_messages)
{
    settings_set.clear();
    if (track_str.empty() ) return;

    set<string> track_names;
    typedef vector<string> TTokens;
    TTokens tracks;

    CTrackUtils::TokenizeWithEscape(track_str, ";][", tracks, true);
    ITERATE (TTokens, t_iter, tracks) {
        TTokens key_values;
        TTrackSettings settings;
        CTrackUtils::TokenizeWithEscape(*t_iter, ",", key_values, true);
        ITERATE (TTokens, kv_iter, key_values) {
            TTokens key_value_pair;
            CTrackUtils::TokenizeWithEscape(*kv_iter, ":", key_value_pair, true);
            if (key_value_pair.size() == 2) {
                NStr::TruncateSpacesInPlace(key_value_pair[0]);
                //NStr::TruncateSpacesInPlace(key_value_pair[1]);
                if ( !key_value_pair[0].empty()  &&
                     !key_value_pair[1].empty() ) {

                    if (key_value_pair[0] == "name") { // Prevent tracks names duplication
                        string track_name = key_value_pair[1];
                        unsigned name_count = 1;
                        while (track_names.end() != track_names.find(track_name)) {
                            stringstream new_name;
                            new_name << key_value_pair[1] << " (" << ++name_count << ')';
                            track_name = new_name.str();
                        }
                        track_names.insert(track_name);
                        if (key_value_pair[1] != track_name) {
                            if (warning_messages) {
                                stringstream warning;
                                warning << "Track name \"" << key_value_pair[1] << "\" already exists. Using \"" << track_name << "\" instead.";
                                warning_messages->push_back(warning.str());
                            }
                            key_value_pair[1] = track_name;
                        }
                    }

                    //!! temporary hack to preserve "is_scaled" to have the same meaning as "stored_scale" for some time (see SV-2703)
                    if(key_value_pair[0] == "is_scaled") {
                        settings[ "stored_scale" ] = s_SVURLDecode(NStr::URLDecode(key_value_pair[1]));
                    } else {
                        settings[ key_value_pair[0] ] = s_SVURLDecode(NStr::URLDecode(key_value_pair[1]));
                    }
                }
            }
        }
        if ( !settings.empty() ) {
            // if the id of this track is not STD (i.e. TMS), check that a track with the same id has not yet been processed;
            // if it has, merge/override settings from this one
            if(isTMSId(settings)) {
                string sTMSId;
                GetKey(settings, "id", sTMSId);
                TTrackSettingsSetSelection TracksWithTheSameId;
                GetTrackSettingsSetSelection(settings_set, "id", sTMSId, TracksWithTheSameId);
                if(!TracksWithTheSameId.empty()) {
                    TTrackSettings& duplicate_settings(*TracksWithTheSameId.front());
                    for(const auto& iSettings: settings) {
                        duplicate_settings[iSettings.first] = iSettings.second;
                    }
                    continue;
                }
            }
            else {
                string annots;
                GetKey(settings, "annots", annots);
                if (NStr::StartsWith(annots, "http", NStr::eNocase) || NStr::StartsWith(annots, "ftp", NStr::eNocase)) {
                    if (0 == settings.count("remote_path")) {
                       settings["remote_path"] = annots;
                    }
                }
            }
            settings_set.push_back(settings);
        }
    }
}


bool CTrackUtils::HasTrack(const string& track_key,
                         const TTrackSettingsSet& settings_set)
{
    ITERATE (TTrackSettingsSet, iter, settings_set) {
        TTrackSettings::const_iterator k_iter = iter->find("key");
        if (k_iter != iter->end()  &&
            NStr::StartsWith(k_iter->second, track_key, NStr::eNocase)) {
            return true;
        }
    }
    return false;
}


bool CTrackUtils::HasTrack(const string& track_key,
                         const string& track_name,
                         const TTrackSettingsSet& settings_set)
{
    ITERATE (TTrackSettingsSet, iter, settings_set) {
        TTrackSettings::const_iterator k_iter = iter->find("key");
        TTrackSettings::const_iterator n_iter = iter->find("name");
        if (k_iter != iter->end()  &&  n_iter != iter->end()  &&
            n_iter->second == track_name  &&
            NStr::StartsWith(k_iter->second, track_key, NStr::eNocase)) {
            return true;
        }
    }
    return false;
}

/// returns true if the track id in settings looks like a TMS track id (i.e. is present, not empty and does not start with STD)
/// this does not guarantee nor check that it actually exists in TMS
bool CTrackUtils::isTMSId(const TTrackSettings& TrackSettings)
{
    string sTrackID;
    return CTrackUtils::GetKey(TrackSettings, "id", sTrackID) && sTrackID.find(kStdTrackId) != 0 && sTrackID.find("R" + kStdTrackId) != 0;
}



static const string& kBAM_DATA = "BAM";

CTrackUtils::TDataKeys CTrackUtils::GetAllDataKeys(const TTrackSettingsSet& settings_set)
{
    TDataKeys keys;
    ITERATE (TTrackSettingsSet, iter, settings_set) {
        TTrackSettings::const_iterator key_iter = iter->find("data_key");
        if (key_iter != iter->end()) {
            TTrackSettings::const_iterator db_iter = iter->find("dbname");
            if (db_iter != iter->end() && db_iter->second == kBAM_DATA)
                continue;
            vector<string> mkeys;
            NStr::Split(CTrackUtils::RemoveEscape(key_iter->second), "|", mkeys);
            ITERATE (vector<string>, mkey_iter, mkeys) {
                string key = NStr::TruncateSpaces(*mkey_iter);
                if ( !key.empty() ) {
                    keys.push_back(key);
                }
            }
        }
    }
    return keys;
}

/// get all tracks that have a given value in a given key
/// the entries in TrackSettingsSetSelection are valid while the respective entries are not removed from TrackSettingsSet
void CTrackUtils::GetTrackSettingsSetSelection(TTrackSettingsSet& TrackSettingsSet, const string& key, const string& value,
    TTrackSettingsSetSelection& TrackSettingsSetSelection)
{
    TrackSettingsSetSelection.clear();
    NON_CONST_ITERATE(TTrackSettingsSet, iTrackSettingsSet, TrackSettingsSet) {
        TTrackSettings::const_iterator iTrackSettings = iTrackSettingsSet->find(key);
        if (iTrackSettings != iTrackSettingsSet->end() &&
            iTrackSettings->second == value) {
            TrackSettingsSetSelection.push_back(iTrackSettingsSet);
        }
    }
}

/// returns a specified key; if it does not exist, returns false
bool CTrackUtils::GetKey(const TTrackSettings& TrackSettings, const string& key, string& value)
{
    value.clear();
    TTrackSettings::const_iterator key_iter = TrackSettings.find(key);
    if (key_iter == TrackSettings.end()) {
        return false;
    }
    value = key_iter->second;
    return true;
}

void CTrackUtils::SetKey(TTrackSettings& TrackSettings, const string& key, const string& value)
{
    TrackSettings[key] = value;
}



/// set error information in the track configuration
/// error codes and such should correspond to ones in CHTMLActiveArea
void CTrackUtils::SetError(TTrackSettings& TrackSettings,
    TTrackErrorStatus::TErrorSeverity severity,
    TTrackErrorStatus::TErrorCode code,
    const string& short_msg,
    const string& msg)
{
    TrackSettings[cTrackSettingKey_ErrorSeverity] = NStr::IntToString(severity);
    TrackSettings[cTrackSettingKey_ErrorCode] = NStr::IntToString(code);
    TrackSettings[cTrackSettingKey_ErrorShortMsg] = short_msg.substr(0, TTrackErrorStatus::cShortErrorLength);
    TrackSettings[cTrackSettingKey_ErrorMsg] = msg;
    LOG_POST(Warning << "Track error detected, code: " << code << ", message: " << msg);
}

/// check that the track is currently marked as having an error
bool CTrackUtils::isInErrorStatus(const TTrackSettings& TrackSettings)
{
    string severity;
    //!! should warnings be treated as errors?
    return GetKey(TrackSettings, cTrackSettingKey_ErrorSeverity, severity) && severity != NStr::IntToString(TTrackErrorStatus::eErrorSeverity_NoError);
}

string CTrackUtils::RemoveEscape(const string &str)
{
    // Special cases
    if (str.empty()) {
        return str;
    }

    string::size_type len = str.length();
    string out;
    out.resize(len);

    string::size_type pdst = 0;
    bool escape = false;
    for (string::size_type psrc = 0; psrc < len; ++psrc) {
        const char& curr_char = str[psrc];
        if (curr_char == '\\'  &&  !escape ) {
            escape = true;
        } else {
            out[pdst++] = curr_char;
            if (escape) {
                escape = false;
            }
        }
    }

    if (pdst < len) {
        out[pdst] = '\0';
        out.resize(pdst);
    }

    return out;
}


void CTrackUtils::TokenizeWithEscape(const string &str,
                                   const string &delim,
                                   vector<string>& tokens,
                                   bool remove_escape)
{
    // Special cases
    if (str.empty()) {
        return;
    } else if (delim.empty()) {
        tokens.push_back(str);
        return;
    }

    // Tokenization
    //
    string::size_type pos = 0;
    bool escape = false;
    string curr_str = "";
    while (pos < str.size()) {
        const char& curr_char = str[pos];
        if (curr_char == '\\') {
            if ( !escape ) {
                escape = true;
                ++pos;
                continue;
            }
        }

        bool match = delim.find(curr_char) != string::npos;
        if (match) {
            if (escape) {
                if ( !remove_escape ) {
                    curr_str.append(1, '\\');
                }
                curr_str.append(1, curr_char);
                escape = false;
            } else if ( !curr_str.empty() ) {
                tokens.push_back(curr_str);
                curr_str.clear();
            }
        } else {
            if (escape) {
                curr_str.append(1, '\\');
                curr_str.append(1, curr_char);
                escape = false;
            } else {
                curr_str.append(1, curr_char);
            }
        }
        ++pos;
    }

    if ( !curr_str.empty() ) {
        tokens.push_back(curr_str);
    }
}



END_NCBI_SCOPE

