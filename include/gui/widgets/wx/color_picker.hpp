#ifndef GUI_WIDGETS_WX___COLOR_PICKER__HPP
#define GUI_WIDGETS_WX___COLOR_PICKER__HPP

/*  $Id: color_picker.hpp 21074 2010-03-19 17:58:40Z tereshko $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *      CwxColorButton - and custom version of color picker for wxWidgets
 *                       (to look like a Color Well on Mac OS X)
 */


#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>
#include <wx/clrpicker.h>


BEGIN_NCBI_SCOPE


#ifdef NCBI_OS_DARWIN_NEVER
class NCBI_GUIWIDGETS_WX_EXPORT CColorPickerMac : public wxControl
{   
    DECLARE_DYNAMIC_CLASS(CColorPickerMac)
    DECLARE_EVENT_TABLE()

public:
    CColorPickerMac();
    CColorPickerMac(wxWindow *parent,
            wxWindowID id,
            const wxColour& colour = *wxBLACK,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCLRP_DEFAULT_STYLE,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = "colourpickerctrl");

    wxSize DoGetBestSize() const { return wxSize(24,24); }

    wxColour GetColour () const;
    void     SetColour (const wxColour &colour);

    virtual ~CColorPickerMac();
    void    OnPaint(wxPaintEvent& event);
    void    OnMouseEvent(wxMouseEvent& event);  

protected:
    bool m_Down;
    wxColourData m_ColourData;
};

typedef CColorPickerMac CColorPicker;
#else
typedef wxColourPickerCtrl CColorPicker;
#endif

END_NCBI_SCOPE;

#endif  // GUI_WIDGETS_WX___COLOR_PICKER__HPP
