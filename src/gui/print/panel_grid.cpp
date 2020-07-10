/*  $Id: panel_grid.cpp 27010 2012-12-07 16:37:16Z falkrb $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPanelGrid - manage multi-page spread for a PDF document
 *
 * Note:
 *   Column and Row values are zero-based ie. first index is zero
 *   Panel values are one-based ie. first index is one
 */


#include <ncbi_pch.hpp>
#include "panel_grid.hpp"
#include <gui/utils/id_generator.hpp>
#include <gui/print/pdf_object.hpp>


BEGIN_NCBI_SCOPE



CPanelGrid::CPanelGrid(CIdGenerator* objid_gen,
                       unsigned int cols,
                       unsigned int rows
                      )
    : m_ObjIdGenerator(objid_gen),
      m_Cols(cols),
      m_Rows(rows),
      m_Panels(cols * rows)
{
    for (unsigned int r = 0; r < m_Rows; ++r) {
        for (unsigned int c = 0; c < m_Cols; ++c) {
            CRef<CPanel> p(new CPanel());
            p->m_HAlign = GetHAlign(c);
            p->m_VAlign = GetVAlign(r);
            p->m_Col = c;
            p->m_Row = r;
            p->m_PanelNum = r * m_Cols + c + 1;
            p->m_HPageOffset = c;
            p->m_VPageOffset = m_Rows - r - 1;
            p->m_Panel = CRef<CPdfObject>(new CPdfObject(m_ObjIdGenerator->NextId()));

            const unsigned int idx = x_GetPanelNumber(c, r) - 1;
            m_Panels[idx] = p;
        }
    }
}


CPanelGrid::~CPanelGrid()
{
}


CPanelGrid::TAlignment CPanelGrid::GetVAlign(unsigned int row) const
{
    // first row is bottom-aligned
    TAlignment valign = CPrintOptions::eBottom;

    if (m_Rows == 1) {
        valign = CPrintOptions::eMiddle;
    }
    else {
        if (row == m_Rows - 1) {
            valign = CPrintOptions::eTop;
        }
        else if (row > 0) {
            valign = CPrintOptions::eMiddle;
        }
    }

    return valign;
}


CPanelGrid::TAlignment CPanelGrid::GetHAlign(unsigned int col) const
{
    // first column is right-aligned
    TAlignment halign = CPrintOptions::eRight;

    if (m_Cols == 1) {
        halign = CPrintOptions::eCenter;
    }
    else {
        if (col == m_Cols - 1) {
            halign = CPrintOptions::eLeft;
        }
        else if (col > 0) {
            halign = CPrintOptions::eCenter;
        }
    }

    return halign;
}


CRef<CPanel> CPanelGrid::GetPanel(unsigned int panel) const
{
    if (panel == 0) {
        return CRef<CPanel>();
    }
    return m_Panels[panel - 1];
}


CRef<CPanel> CPanelGrid::GetPanel(unsigned int col, unsigned int row) const
{
    return GetPanel(x_GetPanelNumber(col, row));
}


unsigned int CPanelGrid::x_GetPanelNumber(unsigned int col, unsigned int row) const
{
    if (col >= m_Cols || row >= m_Rows)
    {
        return 0;
    }
    return row * m_Cols + col + 1;
}


unsigned int CPanelGrid::GetNumPanels(void) const
{
    return m_Cols * m_Rows;
}


CPanelGrid::SNeighbors CPanelGrid::GetNeighbours(unsigned int col, unsigned int row) const
{
    // MSVC++ didn't like an initializer list for SNeighbors
    SNeighbors n;
    n.left = GetPanel(col - 1, row);
    n.right = GetPanel(col + 1, row);
    n.top = GetPanel(col, row - 1);
    n.bottom = GetPanel(col, row + 1);
    return n;
}


END_NCBI_SCOPE
