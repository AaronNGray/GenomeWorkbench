/*  $Id: unaligned_region_dlg.hpp 33153 2015-06-04 21:41:47Z evgeniev $
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
 * Authors: Vladislav Evgeniev
 *
 * File Description: Dialog, displaying unaligned regions.
 *
 */
#ifndef _UNALIGNED_REGION_DLG_H_
#define _UNALIGNED_REGION_DLG_H_


/*!
 * Includes
 */
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
////@begin includes
#include "wx/richtext/richtextctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxRichTextCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_UNALIGNEDREGION 10000
#define ID_SEQUENCE_RICHTEXTCTRL 10001
#define SYMBOL_CUNALIGNEDREGIONDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CUNALIGNEDREGIONDLG_TITLE _("Unaligned Region")
#define SYMBOL_CUNALIGNEDREGIONDLG_IDNAME ID_UNALIGNEDREGION
#define SYMBOL_CUNALIGNEDREGIONDLG_SIZE wxSize(400, 300)
#define SYMBOL_CUNALIGNEDREGIONDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CUnalignedRegionDlg class declaration
 */

class CUnalignedRegionDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CUnalignedRegionDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CUnalignedRegionDlg();
    CUnalignedRegionDlg( wxWindow* parent, wxWindowID id = SYMBOL_CUNALIGNEDREGIONDLG_IDNAME, const wxString& caption = SYMBOL_CUNALIGNEDREGIONDLG_TITLE, const wxPoint& pos = SYMBOL_CUNALIGNEDREGIONDLG_POSITION, const wxSize& size = SYMBOL_CUNALIGNEDREGIONDLG_SIZE, long style = SYMBOL_CUNALIGNEDREGIONDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CUNALIGNEDREGIONDLG_IDNAME, const wxString& caption = SYMBOL_CUNALIGNEDREGIONDLG_TITLE, const wxPoint& pos = SYMBOL_CUNALIGNEDREGIONDLG_POSITION, const wxSize& size = SYMBOL_CUNALIGNEDREGIONDLG_SIZE, long style = SYMBOL_CUNALIGNEDREGIONDLG_STYLE );

    /// Destructor
    ~CUnalignedRegionDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

//    void SetData(const wxString &sequenceText, size_t unalignedRegionFrom, size_t unalignedRegionTo);
    void WriteNormalText(const wxString &text);
    void WriteRedText(const wxString &text);

////@begin CUnalignedRegionDlg event handler declarations

////@end CUnalignedRegionDlg event handler declarations

////@begin CUnalignedRegionDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CUnalignedRegionDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CUnalignedRegionDlg member variables
    wxRichTextCtrl* m_SequenceTextCtrl;
////@end CUnalignedRegionDlg member variables
};

END_NCBI_SCOPE

#endif
    // _UNALIGNED_REGION_DLG_H_
