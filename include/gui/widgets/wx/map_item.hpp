#ifndef GUI_WIDGETS_WX___MAP_ITEM__HPP
#define GUI_WIDGETS_WX___MAP_ITEM__HPP

/*  $Id: map_item.hpp 24466 2011-09-28 16:19:36Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/utils/event_handler.hpp>

#include <gui/widgets/wx/imap_item.hpp>

class wxDC;
class wxEvtHandler;

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CMapItem
/// This is a base class for IwxMapItem implementations. Provides basic services.
class NCBI_GUIWIDGETS_WX_EXPORT CMapItem :
    public CObject,
    public IwxMapItem
{
public:
    CMapItem();
    CMapItem(const string& label, wxBitmap& icon,
             const string& descr, bool separator = false);
    virtual ~CMapItem();

    const string& GetLabel() const { return m_Label; }
    const string& GetDescr() const { return m_Descr; }

    void    SetIcon(wxBitmap& image);

    /// enables sending commands on default action; set target to NULL
    /// to disable
    virtual void    SetCommand(TCmdID cmd, wxEvtHandler* target);

    /// @name IwxMapItem implementation
    /// @{
    virtual void    SetRect(const wxRect& rc);
    virtual void    GetRect(wxRect& rc) const;
    virtual void    Layout(wxDC& dc, SwxMapItemProperties& props);
    virtual int  PreferredHeight(wxDC& dc, SwxMapItemProperties& props, int width);
    virtual bool IsGroupSeparator() const;
    virtual void    Draw(wxDC& dc, int state, SwxMapItemProperties& props);
    virtual void    OnDefaultAction();
    virtual bool    OnHotTrack(const wxPoint& ms_pos) { return false; }
    virtual void    OnMouseDown(const wxPoint& ms_pos) {}
    virtual void    OnLeftDoubleClick(const wxPoint& ms_pos) {}
    /// @}

protected:
    void    x_Init();

    virtual int     x_GetItemShift() const;
    virtual int     x_GetTextAreaWidth(SwxMapItemProperties& props, int width) const;
    virtual void    x_UpdatePreferredHeights(wxDC& dc, SwxMapItemProperties& props, int width);
    virtual int     x_PreferredSeparatorHeight(wxDC& dc) const;
    virtual int     x_PreferredLabelHeight(wxDC& dc, SwxMapItemProperties& props, int text_w) const;
    virtual int     x_CalculateRealTextWidth(wxDC& dc, SwxMapItemProperties& props);

    virtual void    x_DrawFocusRect(wxDC& dc, const wxRect& rc, int state, SwxMapItemProperties& props);
    virtual void    x_DrawBackground(wxDC& dc, const wxRect& rc, int state, SwxMapItemProperties& props);
    virtual void    x_DrawSeparator(wxDC& dc, const wxRect& rc, SwxMapItemProperties& props);
    virtual void    x_DrawText(wxDC& dc, const string& text, const wxRect& rc, bool selected, bool focused,
                               bool highlighted,
                               SwxMapItemProperties& props);
protected:
    string      m_Label;
    wxBitmap    m_Icon;
    string      m_Descr;
    bool        m_Separator;

    TCmdID  m_Cmd;
    wxEvtHandler* m_CmdTarget;

    // Geometry
    bool    m_PrefsDirty; // preferred sizes are out-of-date
    int m_LabelPrefH;

    wxRect  m_Rect; // item rect set by the control
    wxPoint m_IconPos; // top left corner
    wxRect  m_LabelRect;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___MAP_ITEM__HPP
