/*  $Id: dbxref_name_panel.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Andrea Asztalos
 */
#ifndef _DBXREF_NAME_PANEL_H_
#define _DBXREF_NAME_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CDBXREFNAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CDBXREFNAMEPANEL_TITLE _("DbXref DbName Panel")
#define SYMBOL_CDBXREFNAMEPANEL_IDNAME ID_CDBXREFNAMEPANEL
#define SYMBOL_CDBXREFNAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CDBXREFNAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CDbxrefNamePanel class declaration
 */

class CDbxrefNamePanel: public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CDbxrefNamePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDbxrefNamePanel();
    CDbxrefNamePanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CDBXREFNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CDBXREFNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CDBXREFNAMEPANEL_SIZE, 
        long style = SYMBOL_CDBXREFNAMEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CDBXREFNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CDBXREFNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CDBXREFNAMEPANEL_SIZE, 
        long style = SYMBOL_CDBXREFNAMEPANEL_STYLE );

    /// Destructor
    ~CDbxrefNamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDbxrefNamePanel event handler declarations

////@end CDbxrefNamePanel event handler declarations

////@begin CDbxrefNamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDbxrefNamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);

////@begin CDbxrefNamePanel member variables
    wxTextCtrl* m_DbName;
////@end CDbxrefNamePanel member variables

    enum {
        ID_CDBXREFNAMEPANEL = 6200,
        ID_TXTCTRl_DBXREF
    };
};

END_NCBI_SCOPE

#endif
    // _DBXREF_NAME_PANEL_H_
