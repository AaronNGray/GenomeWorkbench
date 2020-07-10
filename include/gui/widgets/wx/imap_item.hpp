#ifndef GUI_WIDGETS_WX___IMAP_ITEM__HPP
#define GUI_WIDGETS_WX___IMAP_ITEM__HPP

/*  $Id: imap_item.hpp 30858 2014-07-31 14:05:43Z ucko $
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

//#include <gui/utils/command.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/bitmap.h>

class wxDC;

BEGIN_NCBI_SCOPE

struct SwxMapItemProperties;

///////////////////////////////////////////////////////////////////////////////
/// IwxMapItem
/// Abstarct item of the Map Control.
class IwxMapItem
{
public:
    virtual ~IwxMapItem() {};

    virtual void    SetRect(const wxRect& rc) = 0;
    virtual void    GetRect(wxRect& rc) const = 0;

    // layouts the internals based on the current rectangle
    virtual void    Layout(wxDC& dc, SwxMapItemProperties& props) = 0;
    virtual int  PreferredHeight(wxDC& dc, SwxMapItemProperties& props, int width) = 0;
    virtual bool IsGroupSeparator() const = 0;

    /// Draws item in the given rectangle, state is a combination of EItemState flags
    virtual void    Draw(wxDC& dc, int state, SwxMapItemProperties& props) = 0;

    /// a callback to perform a default action associated with the item
    virtual void    OnDefaultAction() = 0;

    virtual bool    OnHotTrack(const wxPoint& ms_pos) = 0;
    virtual void    OnMouseDown(const wxPoint& ms_pos) = 0;
    virtual void    OnLeftDoubleClick(const wxPoint& ms_pos) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// SwxMapItemProperties - properties of the IwxMapItem
struct SwxMapItemProperties
{
    wxColour m_TextColor;
    wxColour m_BackColor;
    wxColour m_SelTextColor;
    wxColour m_SelBackColor;
    wxColour m_HiTextColor;

    wxFont  m_Font;
    wxFont  m_MinorFont;

    int     m_Border; // the border around the item (not painted)
    int     m_HorzMargin; // margin - the offset around icons and text
    int     m_VertMargin;
    bool    m_CenterLabelAndIcon;

    SwxMapItemProperties()
    :   m_TextColor( 0, 0, 0),
        m_BackColor(255, 255, 255),
        m_SelTextColor(255, 255, 255),
        m_SelBackColor(128, 128, 128),
        m_HiTextColor(0, 0, 196),
        m_Font(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
        m_MinorFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
        m_Border(1),
        m_HorzMargin(3),
        m_VertMargin(3),
        m_CenterLabelAndIcon(true)
    {
    }
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___IMAP_ITEM__HPP
