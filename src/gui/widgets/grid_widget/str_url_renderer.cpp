/*  $Id: str_url_renderer.cpp 27414 2013-02-12 17:45:21Z katargir $
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

#include <gui/widgets/grid_widget/str_url_renderer.hpp>
#include <gui/widgets/grid_widget/str_url_parser.hpp>
#include <gui/widgets/grid_widget/grid.hpp>
#include <gui/widgets/grid_widget/wxgrid_table_adapter.hpp>

#include "wx/tokenzr.h"
#include <wx/dc.h>
#include <wx/settings.h>

BEGIN_NCBI_SCOPE

// ----------------------------------------------------------------------------
// CStrWithURLRenderer
// ----------------------------------------------------------------------------

void CStrWithURLRenderer::SetTextColoursAndFont(const wxGrid& grid,
                                                const wxGridCellAttr& attr,
                                                wxDC& dc,
                                                bool isSelected)
{
    dc.SetBackgroundMode( wxBRUSHSTYLE_TRANSPARENT );

    // TODO some special colours for attr.IsReadOnly() case?

    // different coloured text when the grid is disabled
    if ( grid.IsThisEnabled() )
    {
        if ( isSelected )
        {
            wxColour clr;
            if ( grid.HasFocus() )
                clr = grid.GetSelectionBackground();
            else
                clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
            dc.SetTextBackground( clr );
            dc.SetTextForeground( grid.GetSelectionForeground() );
        }
        else
        {
            dc.SetTextBackground( attr.GetBackgroundColour() );
            dc.SetTextForeground( attr.GetTextColour() );
        }
    }
    else
    {
        dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }

    dc.SetFont( attr.GetFont() );
}

static wxSize GetLineSize(const wxGridCellAttr& attr,
                          wxDC& dc,
                          const CStrWithUrlContainer::TLine& line,
                          vector<int>& widths)
{
    wxCoord width = 0, height = 0;
    
    wxFont linkFont = attr.GetFont();
    linkFont.SetUnderlined(true);

    ITERATE(CStrWithUrlContainer::TLine, it, line) {
        const CStrWithUrlLink* link = dynamic_cast<const CStrWithUrlLink*>(*it);
        if (link) {
            dc.SetFont(linkFont);
        }
        else {
            dc.SetFont(attr.GetFont());
        }
        wxCoord x = 0, y = 0;
        dc.GetTextExtent((*it)->GetText(), &x, &y);
        width += x;
        height = wxMax(height, y);
        widths.push_back(x);
    }

    dc.SetFont(attr.GetFont());

    return wxSize(width, height);
}

wxSize CStrWithURLRenderer::DoGetBestSize(const wxGridCellAttr& attr,
                                          wxDC& dc,
                                          const wxString& text)
{
    wxCoord max_x = 0, max_y = 0;

    CStrWithUrlParser parser;
    auto_ptr<wxObject> result(parser.Parse(text));
    CStrWithUrlContainer* container = dynamic_cast<CStrWithUrlContainer*>(result.get());

    ITERATE(vector<CStrWithUrlContainer::TLine>, it, container->GetLines()) {
        vector<int> widths;
        wxSize size = GetLineSize(attr, dc, *it, widths);
        max_x = wxMax(max_x, size.x);
        max_y = wxMax(max_y, size.y);
    }

    max_y *= container->GetLines().size(); // multiply by the number of lines.

    return wxSize(max_x, max_y);
}

wxSize CStrWithURLRenderer::GetBestSize(wxGrid& grid,
                                        wxGridCellAttr& attr,
                                        wxDC& dc,
                                        int row, int col)
{
    return DoGetBestSize(attr, dc, x_GetHtmlValue(grid, row, col));
}

void CStrWithURLRenderer::Draw(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               const wxRect& rectCell,
                               int row, int col,
                               bool isSelected)
{
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    wxString value = x_GetHtmlValue(grid, row, col);
    CStrWithUrlParser parser;
    auto_ptr<wxObject> result(parser.Parse(value));
    CStrWithUrlContainer* container = dynamic_cast<CStrWithUrlContainer*>(result.get());

    int hAlign, vAlign;
    attr.GetAlignment(&hAlign, &vAlign);


    // now we only have to draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);

    wxDCClipper clip(dc, rect);

    wxFont linkFont = attr.GetFont();
    linkFont.SetUnderlined(true);

    wxCoord y = rect.GetTop();

    CCellHyperlinks* links = new CCellHyperlinks();

    wxColour textColor = dc.GetTextForeground();

    ITERATE(vector<CStrWithUrlContainer::TLine>, it, container->GetLines()) {
        if (y > rect.GetBottom())
            break;

        vector<int> widths;
        wxSize size = GetLineSize(attr, dc, *it, widths);
        wxCoord x = rect.GetLeft();

        if (hAlign&wxALIGN_RIGHT) {
            x = rect.GetRight() - size.x;
        }
        else if (hAlign&wxALIGN_CENTER_HORIZONTAL) {
            x += (rect.GetWidth() - size.x)/2;
        }

        int i = 0;
        ITERATE(CStrWithUrlContainer::TLine, it2, *it) {
            if (x > rect.GetRight())
                break;
            if (x + widths[i] >= rect.GetLeft()) {
                const CStrWithUrlLink* link = dynamic_cast<const CStrWithUrlLink*>(*it2);

                if (link) {
                    dc.SetFont(linkFont);
                    dc.SetTextForeground(*wxBLUE);
                }
                else {
                    dc.SetFont(attr.GetFont());
                    dc.SetTextForeground(textColor);
                }

                dc.DrawText( (*it2)->GetText(), x, y );

                if (link) {
                    wxRect linkRect(x, y + 2, widths[i], size.y - 2); // Make hot area little smaller to not interfere with row resize
                    linkRect.Intersect(rect);
                    links->Add(linkRect, link->GetUrl());
                }
            }
            x += widths[i++];
        }

        y += size.y;
    }

    dc.SetTextForeground(textColor);
    dc.SetFont(attr.GetFont());

    CGrid* g = dynamic_cast<CGrid*>(&grid);
    if (g == 0) {
        delete links;
        return;
    }

    if (links->Empty()) {
        delete links;
        g->GetHyperlinks().SetLink(col, row, 0);
    } else {
        g->GetHyperlinks().SetLink(col, row, links);
    }
}

wxString CStrWithURLRenderer::x_GetHtmlValue(wxGrid& grid, int row, int col)
{
    CwxGridTableAdapter& adapter = dynamic_cast<CwxGridTableAdapter&>(*grid.GetTable());
    return adapter.GetHtmlValue(row, col);
}


END_NCBI_SCOPE
