#ifndef __GUI_WIDGETS_ALNMULTI___ALIGN_ROW_GRAPH_CONT__HPP
#define __GUI_WIDGETS_ALNMULTI___ALIGN_ROW_GRAPH_CONT__HPP

/*  $Id: align_row_graph_cont.hpp 29658 2014-01-23 22:00:59Z wuliangs $
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

#include <gui/opengl/glpane.hpp>
#include <gui/widgets/aln_multiple/alnvec_row_graph.hpp>
#include <gui/widgets/gl/graph_container.hpp>


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CAlignRowGraphCont

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlignRowGraphCont :
    public CGraphContainer,
    public IAlnVecRowGraph
{
public:
    CAlignRowGraphCont();

    //
    /// @name IAlnVecRowGraph implementaion
    /// @{
    virtual ~CAlignRowGraphCont();

    virtual bool    IsCreated() const;
    virtual bool    Create();
    virtual void    Destroy();
    virtual void    Update(double start, double stop);

    virtual const IAlnRowGraphProperties*     GetProperties() const;
    virtual void    SetProperties(IAlnRowGraphProperties* props);

    virtual void    Render(CGlPane& pane, IAlnSegmentIterator& it);
    /// @}

    ///overriding base function
    virtual void    Layout();

protected:
    bool    m_Created;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALIGN_ROW_GRAPH_CONT__HPP
