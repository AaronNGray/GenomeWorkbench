#ifndef GUI_WIDGETS_SEQ_GRAPHIC___COORDINATE_MAPPER__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___COORDINATE_MAPPER__HPP

/*  $Id: coord_mapper.hpp 19073 2009-04-03 13:44:29Z wuliangs $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *
 */

#include <gui/gui.hpp>
#include <objmgr/seq_loc_mapper.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// ICoordMapper interface for converting location between two coordinate
/// systems.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ICoordMapper
{
public:
    virtual ~ICoordMapper() {};

    /// Converting source location to destination location.
    /// The relation between src coord. and destination coords. is defined
    /// by users.
    virtual CRef<objects::CSeq_loc>
        Map(const objects::CSeq_loc& src_loc) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// CCoordMapper_SeqLocMapper coordinate mapper based on CSeq_Loc_Mapper.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CCoordMapper_SeqLocMapper
    : public CObject
    , public ICoordMapper
{
public:
    CCoordMapper_SeqLocMapper(const objects::CSeq_align& map_align,
        const objects::CSeq_id&  to_id, objects::CScope* scope)
        : m_Mapper(map_align, to_id, scope) {}

    CCoordMapper_SeqLocMapper(const objects::CSeq_loc& source,
        const objects::CSeq_loc& target, objects::CScope* scope)
        : m_Mapper(source, target, scope) {}

    virtual CRef<objects::CSeq_loc>
        Map(const objects::CSeq_loc& src_loc)
    {
        CRef<objects::CSeq_loc> loc = m_Mapper.Map(src_loc);
        return loc;
    }

private:
    objects::CSeq_loc_Mapper     m_Mapper;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___COORDINATE_MAPPER__HPP
