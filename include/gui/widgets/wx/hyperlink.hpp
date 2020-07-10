#ifndef GUI_WIDGETS_WX___HYPERLINK_HPP
#define GUI_WIDGETS_WX___HYPERLINK_HPP

/*  $Id: hyperlink.hpp 40971 2018-05-04 17:25:20Z katargir $
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
#include <gui/gui.hpp>

#include <wx/hyperlink.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///
/// CHyperlink is wrapper class for wxHyperlinkCtrl
/// On OSX platform it posts URL click instead of sending it.
/// This fixes OSX crash when wxHyperlinkCtrl control is deleted
/// while proccessing URL click
///
///////////////////////////////////////////////////////////////////////////////

class NCBI_GUIWIDGETS_WX_EXPORT CHyperlink : public wxHyperlinkCtrl
{
public:
    CHyperlink() {}

    CHyperlink(wxWindow *parent,
               wxWindowID id,
               const wxString& label,
               const wxString& url,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxHL_DEFAULT_STYLE,
               const wxString& name = wxHyperlinkCtrlNameStr)
        : wxHyperlinkCtrl(parent, id, label, url, pos, size,
                                    style, name)
    {
    }

    void OnClick(wxHyperlinkEvent& event);

protected:

    wxDECLARE_DYNAMIC_CLASS_NO_COPY(CHyperlink);
    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___HYPERLINK_HPP
