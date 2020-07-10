#ifndef GUI_UTILS___TRACK_INFO__HPP
#define GUI_UTILS___TRACK_INFO__HPP

/*  $Id: track_info.hpp 43115 2019-05-15 21:51:27Z rudnev $
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
 * Authors:  Dmitry Rudnev
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_UTILS
 *
 * @{
 */

typedef struct NCBI_GUIUTILS_EXPORT STrackErrorStatus {

    enum EErrorSeverity {
        eErrorSeverity_NoError    = 0,
        eErrorSeverity_Warning    = 1,
        eErrorSeverity_Fatal      = 2
    };
    typedef int TErrorSeverity;

    enum EErrorCode {
        eErrorCode_NoError        = 0,

        // no specific reason for the error
        eErrorCode_Other          = 1,

        // annot parameter does not refer to any known data
        eErrorCode_UnknownAnnot   = 2,

        // no access is permitted to the private data with current credentials
        // (either no cookie or expired cookie)
        eErrorCode_NoAccess       = 3,

        // panfs access test failed
        // since we no more handle panfs-based files, this error will be given to any SRA/BAM file which is located on panfs
        eErrorCode_NoPanfs        = 4,

        // assembly mapper creation failed
        eErrorCode_NoAssmMapper   = 5,

        // incorrectly formatted batch id for AlignDb alignment track
        eErrorCode_IncorrectBatch = 6,

        // Processing a remote file timed out
        eErrorCode_Timeout = 7,

        // Processing a remote file returned an error code
        eErrorCode_ErrorCode = 8,

        // Cannot match the sequence id to a chromosome
        eErrorCode_NoChrMatch = 9,

        // no data in the track (warning)
        eErrorCode_NoDataW = 10,

        // track was skipped because its data is suspected to be inaccessible
        eErrorCode_BlackListed = 11,

    };
    typedef int TErrorCode;

    static const size_t cShortErrorLength;

    TErrorSeverity m_ErrorSeverity;
    TErrorCode m_ErrorCode;
    // optional error messages

    // short message guaranteed not to be longer than cShortErrorLength characters
    string m_ShortErrorMsg;

    // arbitrary length message
    string m_ErrorMsg;

    STrackErrorStatus(TErrorSeverity severity = eErrorSeverity_NoError, 
                      TErrorCode code = eErrorCode_NoError, 
                      const string& sShortMsg = "",
                      const string& sMsg = "") 
    : m_ErrorSeverity(severity), m_ErrorCode(code), m_ShortErrorMsg(sShortMsg), m_ErrorMsg(sMsg) {}
} TTrackErrorStatus;



class NCBI_GUIUTILS_EXPORT CTrackUtils
{
public:
    typedef vector< string >        TDataKeys;

    /// key-value pair for track settings
    typedef map< string, string > 	TTrackSettings;
    typedef list< TTrackSettings > 	TTrackSettingsSet;
    typedef list< TTrackSettingsSet::iterator>  TTrackSettingsSetSelection;

    /// some internal reserved track setting keys
    /// that are used for internal communication
    /// always start with ###
    static const string cTrackSettingKey_PrivatePrefix;
    /// if present and set to anything, but CHTMLActiveArea::eErrorSeverity_NoError, do not attempt to load the track and render it empty
    static const string cTrackSettingKey_ErrorSeverity;
    /// if present contains the error code
    static const string cTrackSettingKey_ErrorCode;
    /// if present contains the short error message
    static const string cTrackSettingKey_ErrorShortMsg;
    /// if present contains the error message
    static const string cTrackSettingKey_ErrorMsg;
    /// set to CDbGapDataAccess::EAccessType
    static const string cTrackSettingKey_AccessType;
    /// if present contains the original private SRA accession
    static const string cTrackSettingKey_OrigAccession;

    /// prefix for STD (as opposed to TMS) tracks
    static const string kStdTrackId;

    /// Extract the track settings from a setting string.
    /// Separate the track settings from each other, and store the settings
    /// for each track in the form of key and value pair in a map.
    static void ExtractTrackConfig(const string& track_str,
                                   TTrackSettingsSet& settings_set,
                                   list<string> *warning_messages = nullptr);

    /// Check if track settings contain the track of a given track key.
    static bool HasTrack(const string& track_key,
                         const TTrackSettingsSet& settings_set);

    /// Check if track settings contain the track of the given track key and name.
    static bool HasTrack(const string& track_key,
                         const string& track_name,
                         const TTrackSettingsSet& settings_set);

    /// returns true if the track id in settings looks like a TMS track id (i.e. is present, not empty and does not start with STD)
    /// this does not guarantee nor check that it actually exists in TMS
    static bool isTMSId(const TTrackSettings& TrackSettings);

    /// Collect all data key values stored in 'data_key'.
    static TDataKeys GetAllDataKeys(const TTrackSettingsSet& settings_set);

    /// get all tracks that have a given value in a given key
    /// the entries in TrackSettingsSetSelection are valid while no operations that invalidate iterators in TrackSettingsSet take place
    static void GetTrackSettingsSetSelection(TTrackSettingsSet& TrackSettingsSet, const string& key, const string& value,
                                             TTrackSettingsSetSelection& TrackSettingsSetSelection);

    /// returns a specified key; if it does not exist, returns false
    static bool GetKey(const TTrackSettings& TrackSettings, const string& key, string& value);
    static void SetKey(TTrackSettings& TrackSettings, const string& key, const string& value);

    /// set error information in the track configuration
    /// error codes and such should correspond to ones in CHTMLActiveArea
    static void SetError(TTrackSettings& TrackSettings,
                         TTrackErrorStatus::TErrorSeverity severity,
                         TTrackErrorStatus::TErrorCode code,
                         const string& short_msg,
                         const string& msg);

    /// check that the track is currently marked as having an error
    static bool isInErrorStatus(const TTrackSettings& TrackSettings);

    /// remove escape characters ('\') used for escaping special characters in track settings values
    static string RemoveEscape(const string &str);

    /// Tokenize a string using the delim.
    /// It skip any escaped ('\') characters.
    static void TokenizeWithEscape(const string &str,
                                   const string &delim,
                                   vector<string>& tokens,
                                   bool remove_escape = false);
};



END_NCBI_SCOPE

/* @} */


#endif  // GUI_UTILS___TRACK_INFO__HPP
