#ifndef GUI_WIDGETS_WX___NF_BITMAP_BUTTON__HPP
#define GUI_WIDGETS_WX___NF_BITMAP_BUTTON__HPP

/*  $Id: nf_bitmap_button.hpp 37605 2017-01-26 15:53:02Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <wx/window.h>
#include <wx/bitmap.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CNoFocusBitmapButton : public wxWindow
{
public:
    CNoFocusBitmapButton(wxWindow* parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxBitmap& hoverBitmap,
        const wxPoint& pos);

    virtual bool AcceptsFocus() const { return false; }

private:
    void    OnPaint(wxPaintEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseLeave(wxMouseEvent& event);
    void    OnMouseDown(wxMouseEvent& event);
    void    OnMouseUp(wxMouseEvent& event);
    void    OnCaptureLost(wxMouseCaptureLostEvent& event);

    wxBitmap m_Bitmap;
    wxBitmap m_HoverBitmap;
    bool     m_MouseOver;

    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___NF_BITMAP_BUTTON__HPP
