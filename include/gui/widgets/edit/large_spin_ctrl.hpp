#ifndef GUI_WIDGETS_EDIT___LARGE_SPIN_CTRL__HPP
#define GUI_WIDGETS_EDIT___LARGE_SPIN_CTRL__HPP

/*  $Id: large_spin_ctrl.hpp 38182 2017-04-06 18:57:26Z filippov $
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
 * Authors:  Roman Katargin, Igor Filippov
 *
 * File Description:
 *
 */


#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>
#include <wx/control.h>
#include <wx/bitmap.h>

BEGIN_NCBI_SCOPE

DECLARE_EXPORTED_EVENT_TYPE(NCBI_GUIWIDGETS_EDIT_EXPORT, wxEVT_LARGE_SPIN_CTRL_EVENT, -1)

class NCBI_GUIWIDGETS_EDIT_EXPORT CLargeSpinControl : public wxControl
{   
    DECLARE_EVENT_TABLE()
public:
    CLargeSpinControl(wxWindow* parent, wxWindowID id, const wxPoint& pos);
    void Create(wxWindow* parent, wxWindowID id, const wxPoint& pos);
    void UseImageSet(int set);

    wxSize DoGetBestSize() const;

    void OnPaint(wxPaintEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);

    enum EArea
    {
        kNone,
        kBtnUp = 1000,
        kBtnDn
    };

    static const int kWidth  = 34;
    static const int kHeight = 17;

protected:
    EArea x_HitTest(int x, int y);
    void  x_DrawButton(wxDC& dc, int w, int h, wxBitmap& img, bool highlight, bool clicked);

    wxBitmap m_OffScreen;
    wxBitmap m_ArrUp;
    wxBitmap m_ArrDn;
    wxBitmap m_PlusUp;
    wxBitmap m_PlusDown;
    int      m_ImageSet;
    EArea    m_Highlight;
    EArea    m_Clicking;

};

class NCBI_GUIWIDGETS_EDIT_EXPORT CLargeNoTabSpinControl : public CLargeSpinControl
{
public:
    CLargeNoTabSpinControl(wxWindow* parent, wxWindowID id, const wxPoint& pos);

    virtual bool AcceptsFocusFromKeyboard() const { return false; }
};

END_NCBI_SCOPE;

#endif  // GUI_WIDGETS_EDIT___LARGE_SPIN_CTRL__HPP
