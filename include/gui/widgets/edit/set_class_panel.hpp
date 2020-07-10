/*  $Id: set_class_panel.hpp 31595 2014-10-27 14:51:04Z bollin $
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
#ifndef _SET_CLASS_PANEL_H_
#define _SET_CLASS_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <gui/gui_export.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT ISetClassNotify
{
public:
    virtual void ChangeNotify(void) = 0;
};


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSETCLASSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSETCLASSPANEL_TITLE _("SetClassPanel")
#define SYMBOL_CSETCLASSPANEL_IDNAME ID_CSETCLASSPANEL
#define SYMBOL_CSETCLASSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSETCLASSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSetClassPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CSetClassPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSetClassPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSetClassPanel();
    CSetClassPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSETCLASSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSETCLASSPANEL_POSITION, const wxSize& size = SYMBOL_CSETCLASSPANEL_SIZE, long style = SYMBOL_CSETCLASSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSETCLASSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSETCLASSPANEL_POSITION, const wxSize& size = SYMBOL_CSETCLASSPANEL_SIZE, long style = SYMBOL_CSETCLASSPANEL_STYLE );

    /// Destructor
    ~CSetClassPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSetClassPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE19
    void OnChoice19Selected( wxCommandEvent& event );

////@end CSetClassPanel event handler declarations

////@begin CSetClassPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSetClassPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSetClassPanel member variables
    wxChoice* m_SetClass;
    /// Control identifiers
    enum {
        ID_CSETCLASSPANEL = 10183,
        ID_CHOICE19 = 10184
    };
////@end CSetClassPanel member variables

    void SetClass(objects::CBioseq_set::EClass class_type);
    objects::CBioseq_set::EClass GetClass();
    static string GetClassName(objects::CBioseq_set::TClass class_type);

protected:
    void x_SetDefaultTypes();
};

END_NCBI_SCOPE

#endif
    // _SET_CLASS_PANEL_H_
