/*  $Id: cell_hyperlink.cpp 27002 2012-12-05 23:18:37Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/grid_widget/cell_hyperlink.hpp>

BEGIN_NCBI_SCOPE

// ----------------------------------------------------------------------------
// CCellHyperlinks
// ----------------------------------------------------------------------------

const CCellHyperlinks::CHyperlink* CCellHyperlinks::HitTest(const wxPoint& pt) const
{
    ITERATE(vector<CHyperlink>, it, m_Links) {
        if (it->HitTest(pt)) {
            return &*it;
        }
    }

    return 0;
}

CGridHyperlinks::~CGridHyperlinks()
{
    ITERATE(vector<CColHyperlinks*>, it, m_ColLinks) {
        delete *it;
    }
}

void CGridHyperlinks::SetLink(int col, int row, CCellHyperlinks* links)
{
    if (col < 0 || row < 0) {
        delete links;
        return;
    }
    if (m_ColLinks.size() <= (size_t)col) {
        m_ColLinks.resize(col + 1, 0);
    }
    if (m_ColLinks[col] == 0) {
        m_ColLinks[col] = new CColHyperlinks;
    }
    m_ColLinks[col]->SetLink(row, links);
}

const CCellHyperlinks* CGridHyperlinks::GetLink(int col, int row) const
{
    if (col < 0 || row < 0)
        return 0;

    if (m_ColLinks.size() <= (size_t)col || m_ColLinks[col] == 0)
        return 0;

    return m_ColLinks[col]->GetLink(row);
}

CGridHyperlinks::CColHyperlinks::~CColHyperlinks()
{
    ITERATE(vector<CCellHyperlinks*>, it, m_CellLinks) {
        delete *it;
    }
}

void CGridHyperlinks::CColHyperlinks::SetLink(int row, CCellHyperlinks* links)
{
    if (row < 0) {
        delete links;
        return;
    }
    if (m_CellLinks.size() <= (size_t)row) {
        m_CellLinks.resize(row + 1, 0);
    }

    delete m_CellLinks[row];
    m_CellLinks[row] = links;
}

const CCellHyperlinks* CGridHyperlinks::CColHyperlinks::GetLink(int row) const
{
    if (row < 0)
        return 0;

    if (m_CellLinks.size() <= (size_t)row)
        return 0;

    return m_CellLinks[row];
}

END_NCBI_SCOPE
