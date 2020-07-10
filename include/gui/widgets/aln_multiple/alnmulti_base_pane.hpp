#ifndef __GUI_WIDGETS_ALNMULTI___ALNMULTI_BASE_PANE__HPP
#define __GUI_WIDGETS_ALNMULTI___ALNMULTI_BASE_PANE__HPP

/*  $Id: alnmulti_base_pane.hpp 44930 2020-04-21 17:07:30Z evgeniev $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */
#include <corelib/ncbistl.hpp>

#include <gui/opengl/glpane.hpp>

#include <gui/widgets/aln_multiple/alnmulti_renderer.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////
/// Interface IAlnMultiPaneParent represents a context in which an instance
/// of CAlnMultiPane lives.

class IAlnMultiPaneParent
{
public:
    typedef IAlnMultiDataSource::TNumrow    TNumrow;

public:
    virtual ~IAlnMultiPaneParent()  {};

    virtual const IAlnMultiDataSource*  GetDataSource(void) const = 0;

    // all Y coordinates are OpenGL Viewport coordinates (not widget)
    virtual TNumrow     GetRowNumByLine(int index) const = 0;
    virtual int         GetLineByRowNum(TNumrow row) const = 0;

    virtual void    OnChildResize() = 0;

    virtual void    SetScaleX(TModelUnit scale_x, const TModelPoint& point) = 0;
    virtual void    ZoomRect(const TModelRect& rc) = 0;
    virtual void    ZoomPoint(const TModelPoint& point, TModelUnit factor) = 0;
    virtual void    Scroll(TModelUnit d_x, TModelUnit d_y) = 0;
    virtual void    OnRowChanged(IAlignRow* p_row) = 0;

    virtual void    OnColumnsChanged() = 0;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALNMULTI_BASE_PANE__HPP
