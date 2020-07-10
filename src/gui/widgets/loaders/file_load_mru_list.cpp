/*  $Id: file_load_mru_list.cpp 39692 2017-10-26 17:25:16Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

#include <corelib/ncbitime.hpp>

#include <gui/widgets/loaders/file_load_mru_list.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

void CFileLoadMRUList::LoadFromStrings(const vector<string>& values)
{
    Clear();
    CTimeFormat format =
        CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);

    for (size_t i = 0; i + 3 < values.size();  i += 4) {
        // process two elements at once
        string s_time = values[i];
        wxString filename      = FnToWxString(values[i + 1]);
        wxString manager_label = FnToWxString(values[i + 2]);
        string manager_id      = values[i + 3];

        CTime time(s_time, format);
        time_t t = time.GetTimeT();
        Add(CFileLoadDescriptor(filename, manager_label, manager_id), t);
    }
}

void CFileLoadMRUList::SaveToStrings(vector<string>& values) const
{
    CTimeFormat format =
        CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);

    const TTimeToTMap& map = GetMap();
    ITERATE(TTimeToTMap, it, map)  {
        CTime time(it->first);
        time.ToLocalTime();
        string s_time = time.AsString(format);
        string filename      = FnToStdString(it->second.GetFileName());
        string manager_label = FnToStdString(it->second.GetFileLoaderLabel());
        string manager_id    = it->second.GetFileLoaderId();
        values.push_back(s_time);
        values.push_back(filename);
        values.push_back(manager_label);
        values.push_back(manager_id);
    }
}

END_NCBI_SCOPE
