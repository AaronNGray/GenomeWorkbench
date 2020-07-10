#ifndef GUI_UTILS___PANEL_GRID__HPP
#define GUI_UTILS___PANEL_GRID__HPP

/*  $Id: panel_grid.hpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPanelGrid - manage multi-page spread for a PDF document
 */

#include <corelib/ncbiobj.hpp>
#include <gui/print/print_options.hpp>
#include <vector>
#include "panel.hpp"


BEGIN_NCBI_SCOPE


class CPdfObject;
class CIdGenerator;


class CPanelGrid : public CObject
{
private:
    typedef CPrintOptions::TAlignment TAlignment;
    typedef CRef<CPanel> TPanelRef;

public:
    CPanelGrid(CIdGenerator* objid_gen,
               unsigned int cols,
               unsigned int rows
              );
    virtual ~CPanelGrid();

    TPanelRef GetPanel(unsigned int panel) const;
    TPanelRef GetPanel(unsigned int col, unsigned int row) const;
    unsigned int GetNumPanels(void) const;

    TAlignment GetVAlign(unsigned int row) const;
    TAlignment GetHAlign(unsigned int col) const;

    struct SNeighbors
    {
        typedef CRef<CPanel> TPanelRef;
        TPanelRef left;
        TPanelRef right;
        TPanelRef top;
        TPanelRef bottom;
    };

    SNeighbors GetNeighbours(unsigned int row, unsigned int col) const;

protected:
    unsigned int x_GetPanelNumber(unsigned int col, unsigned int row) const;

private:
    CRef<CIdGenerator> m_ObjIdGenerator;

    unsigned int m_Cols;
    unsigned int m_Rows;

    vector<TPanelRef> m_Panels;
};



END_NCBI_SCOPE


#endif // GUI_UTILS___PANEL_GRID__HPP
