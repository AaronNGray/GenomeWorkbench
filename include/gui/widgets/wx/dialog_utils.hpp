#ifndef GUI_WIDGETS_WX___DIALOG_UTILS__HPP
#define GUI_WIDGETS_WX___DIALOG_UTILS__HPP

/*  $Id: dialog_utils.hpp 18397 2008-11-28 19:45:17Z dicuccio $
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
 *      A temporary place for various GUI utitlities
 */

#include <gui/gui.hpp>

#include <wx/string.h>

class wxControlWithItems;


BEGIN_NCBI_SCOPE

class CGlBitmapFont;


/// FillComboBox() initializes wxWidgets controls with a list of text strings;
/// works for wxListBox, wxCheckListBox, wxChoice and wxComboBox
NCBI_GUIWIDGETS_WX_EXPORT
    void  Init_wxControl(wxControlWithItems& combo, const vector<string>& values);

/// Fucntions below convert CGlBitmapFont size and face to strings and back;
/// handy for using with comboboxes and lists, can be used in combination with
/// InitControl()

NCBI_GUIWIDGETS_WX_EXPORT
    wxString GlFontSizeTo_wxString(const CGlBitmapFont& font);
NCBI_GUIWIDGETS_WX_EXPORT
    wxString GlFontFaceTo_wxString(const CGlBitmapFont& font);

NCBI_GUIWIDGETS_WX_EXPORT void  wxStringsToGlFont(CGlBitmapFont& font,
                                                  const wxString& wx_face,
                                                  const wxString& wx_size);

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___DIALOG_UTILS__HPP
