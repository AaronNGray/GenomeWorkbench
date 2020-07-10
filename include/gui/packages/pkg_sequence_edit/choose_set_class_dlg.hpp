/*  $Id: choose_set_class_dlg.hpp 31020 2014-08-20 13:21:30Z bollin $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */
#ifndef _CHOOSE_SET_CLASS_DLG_H_
#define _CHOOSE_SET_CLASS_DLG_H_

#include <corelib/ncbistd.hpp>

#include <objects/seqset/Bioseq_set.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/dialog.h>
#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCHOOSESETCLASSDLG 10407
#define ID_SET_CLASS_CHOICE 10408
#define SYMBOL_CCHOOSESETCLASSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCHOOSESETCLASSDLG_TITLE _("Choose Set Class")
#define SYMBOL_CCHOOSESETCLASSDLG_IDNAME ID_CCHOOSESETCLASSDLG
#define SYMBOL_CCHOOSESETCLASSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCHOOSESETCLASSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CChooseSetClassDlg class declaration
 */

class CChooseSetClassDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CChooseSetClassDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CChooseSetClassDlg();
    CChooseSetClassDlg( wxWindow* parent, wxWindowID id = SYMBOL_CCHOOSESETCLASSDLG_IDNAME, const wxString& caption = SYMBOL_CCHOOSESETCLASSDLG_TITLE, const wxPoint& pos = SYMBOL_CCHOOSESETCLASSDLG_POSITION, const wxSize& size = SYMBOL_CCHOOSESETCLASSDLG_SIZE, long style = SYMBOL_CCHOOSESETCLASSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCHOOSESETCLASSDLG_IDNAME, const wxString& caption = SYMBOL_CCHOOSESETCLASSDLG_TITLE, const wxPoint& pos = SYMBOL_CCHOOSESETCLASSDLG_POSITION, const wxSize& size = SYMBOL_CCHOOSESETCLASSDLG_SIZE, long style = SYMBOL_CCHOOSESETCLASSDLG_STYLE );

    /// Destructor
    ~CChooseSetClassDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CChooseSetClassDlg event handler declarations

////@end CChooseSetClassDlg event handler declarations

////@begin CChooseSetClassDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CChooseSetClassDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CChooseSetClassDlg member variables
    wxChoice* m_SetClassChoice;
////@end CChooseSetClassDlg member variables

    ncbi::objects::CBioseq_set::EClass GetClass();
};

#endif
    // _CHOOSE_SET_CLASS_DLG_H_
