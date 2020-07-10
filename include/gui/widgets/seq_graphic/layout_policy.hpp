#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_POLICY__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_POLICY__HPP

/* $Id: layout_policy.hpp 34827 2016-02-17 00:59:08Z rudnev $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbiobj.hpp>
#include <gui/opengl/gltypes.hpp>
#include <gui/gui_export.h>
#include <list>

BEGIN_NCBI_SCOPE

class CSeqGlyph;
class CLayoutGroup;

///////////////////////////////////////////////////////////////////////////////
/// class ILayoutPolicy defines the abstract interface required for
/// generating layout based on a set of given CSeqGlyphs (layout objects).
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ILayoutPolicy
{
public:
    typedef list< CRef<CSeqGlyph> > TObjectList;
    typedef vector< CRef<CSeqGlyph> > TLayoutRow;
    typedef vector<TLayoutRow>           TLayout;

    struct SBoundingBox {
        TModelUnit m_X;
        TModelUnit m_Y;
        TModelUnit m_Height;
        TModelUnit m_Width;
    };

    virtual ~ILayoutPolicy() {}

    /// Build layout for a list of layout objects.
    /// @param bound the bounding box for all objects
    virtual void BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const = 0;

protected:
    /// Separate objects into multiple sub-groups.
    /// This is for rendering performance consideration. By creating
    /// multiple groups, the visibility detection can be done more
    /// efficiently based on group boundary such that the whole group
    /// of objects can be skipped from rendering if it is not visible
    /// to users.
    void x_SeparateObjects(CLayoutGroup& group, TLayout& layout,
        SBoundingBox& bound, size_t group_size, int vert_space) const;

    /// Set the objects' position (Y), and update the bounding
    /// box of the group (of all objects).
    virtual void x_SetObjectPos(TLayout& layout, SBoundingBox& bound) const {};

};


END_NCBI_SCOPE

#endif //GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_POLICY__HPP
