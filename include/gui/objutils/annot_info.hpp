#ifndef GUI_OBJUTILS___ANNOT_INFO__HPP
#define GUI_OBJUTILS___ANNOT_INFO__HPP

/*  $Id: annot_info.hpp 37943 2017-03-07 18:20:39Z katargir $
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
 *    Annotation meta-data data structures.
 *
 */

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <string>
#include <set>
#include <gui/gui.hpp>

/** @addtogroup GUI_OBJUTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// Data structure for holding basic data track information.
class NCBI_GUIOBJUTILS_EXPORT CTrackInfo : public CObject
{
public:
    CTrackInfo()
        : m_Order(-1)
    {}

    typedef list< CRef<CTrackInfo> > TTrackInfoList;
    /// data indentifier.
    /// Different data source may have different data id format.
    /// For example, for SADB, it will be NA accession. For ID or
    /// other user local data, it will be just annotation name. And
    /// for cSRA, it will be just cSRA file path.
    string  m_DataId;
    string  m_Dbname;   ///< data source such as ID, SADB, cSRA and SRA
    string  m_Title;    ///< annotation's title, optional
    string  m_Descr;    ///< detail info about this annot
    string  m_AnnotType;///< annot type, such as ftable, seq-table, align
    string  m_Subtype;  ///< track-specific type.
    string  m_Group;
    string  m_Subgroup;
    string  m_SettingGroup;
    string  m_Filter;
    string  m_TrackId;  ///< track id (may be generated)
    int     m_Order;
};


///////////////////////////////////////////////////////////////////////////////
/// Data structure for holding meta information for an annotaion.
class NCBI_GUIOBJUTILS_EXPORT CAnnotMetaData : public CObject
{
public:
    // flag whether the links are present, absent or never retrieved (i.e. presence of links is not defined)
    enum ELinksStatus {
        ELinksStatus_Undef,
        ELinksStatus_Present,
        ELinksStatus_Absent
    };
    typedef map<string, string> TLinks;

    bool operator < (const CAnnotMetaData& meta) const
    {
        return m_Name < meta.m_Name;
    }

    string  m_Name;     ///< annotation name, e.g. named annotation accession
    string  m_Id;       ///< annotation id. It may only apply to NAs from SADB
    string  m_Title;    ///< annotation title
    string  m_Descr;    ///< annotation description (or comment)
    string  m_AssmAcc;  ///< GenColl assembly accession
    string  m_xClass;   ///< annotation category
    string  m_SubCategory;   ///< annotation subcategory
    string  m_AnnotType;///< annot type, such as ftable, seq-table, align
    string  m_AnnotChain;

    /// NAA-specific info.
    string  m_Scope;
    string  m_OtherName;

    /// feature/track types (track-specific)
    set<string>  m_Subtypes;
    ELinksStatus m_LinksStatus;
    TLinks       m_Links;

    bool         m_Shown;

    CAnnotMetaData() : m_LinksStatus(ELinksStatus_Undef), m_Shown(false) {}
};


inline
bool operator< (const CRef<CAnnotMetaData>& a1, const CRef<CAnnotMetaData>& a2)
{
    return a1  &&  a2  &&  *a1 < *a2;
}


END_NCBI_SCOPE

/* @} */

#endif  /// GUI_OBJUTILS___ANNOT_INFO__HPP
