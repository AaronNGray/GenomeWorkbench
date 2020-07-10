/*  $Id: bed_track_record.hpp 543641 2017-08-11 16:19:22Z ludwigf $
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
 * Authors:  Frank Ludwig
 *
 * File Description:  BED file track line data
 *
 */

#ifndef OBJTOOLS_WRITERS___BED_TRACK_RECORD__HPP
#define OBJTOOLS_WRITERS___BED_TRACK_RECORD__HPP

#include <objects/general/User_object.hpp>

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE

//  ============================================================================
/// Encapsulation of a track line in the BED file format. For the most part,
/// BED track line consist of key value pairs. Some of the keys are part of the
/// actual spec and have a defined meaning.
///
class CBedTrackRecord
//  ============================================================================
{
public:
    CBedTrackRecord() {};
    ~CBedTrackRecord() {};

    bool Assign(const CSeq_annot&);
    bool Write(CNcbiOstream&);

    bool UseScore() const;

protected:
    bool xImportKeyValuePairs(
        const CUser_object&);
    std::string xGetKeyValue(
        const std::string&) const;

    string mName;
    string mTitle;
    std::map<std::string, std::string> mKeyValuePairs;
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif  // OBJTOOLS_WRITERS___BED_TRACK_RECORD__HPP
