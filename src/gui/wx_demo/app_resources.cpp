/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:
// Modified by:
// Created:     23/04/2007 16:47:14
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "app_resources.h"

////@begin XPM images
////@end XPM images

/*!
 * Resource functions
 */

////@begin AppResources resource functions

/*!
 * Menu creation function for ID_TEXT_EDIT_POPUP
 */

wxMenu* AppResources::CreateMenuMenu()
{
    wxMenu* itemMenu1 = new wxMenu;
    itemMenu1->Append(wxID_COPY, _("Copy"), wxEmptyString, wxITEM_NORMAL);
    itemMenu1->Append(wxID_PASTE, _("Paste"), wxEmptyString, wxITEM_NORMAL);
    itemMenu1->Append(wxID_CUT, _("Cut"), wxEmptyString, wxITEM_NORMAL);
    return itemMenu1;
}

////@end AppResources resource functions

/*!
 * Get bitmap resources
 */

wxBitmap AppResources::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin AppResources bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end AppResources bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon AppResources::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    // Icon retrieval
////@begin AppResources icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end AppResources icon retrieval
}
