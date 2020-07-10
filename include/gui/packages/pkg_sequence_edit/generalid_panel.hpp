/*  $Id: generalid_panel.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _GENERALID_PANEL_H_
#define _GENERALID_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

class wxChoice;
class wxStaticText;
class wxTextCtrl;

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGENERALIDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGENERALIDPANEL_TITLE _("General ID")
#define SYMBOL_CGENERALIDPANEL_IDNAME ID_CGENERALIDPANEL
#define SYMBOL_CGENERALIDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGENERALIDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGeneralIDPanel class declaration
 */

class CGeneralIDPanel: public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CGeneralIDPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGeneralIDPanel();
    CGeneralIDPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CGENERALIDPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CGENERALIDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CGENERALIDPANEL_SIZE, 
        long style = SYMBOL_CGENERALIDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CGENERALIDPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CGENERALIDPANEL_POSITION, 
        const wxSize& size = SYMBOL_CGENERALIDPANEL_SIZE, 
        long style = SYMBOL_CGENERALIDPANEL_STYLE );

    /// Destructor
    ~CGeneralIDPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    
////@begin CGeneralIDPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CGENERALID_CHOICE
    void OnGeneralIDChoice( wxCommandEvent& event );

////@end CGeneralIDPanel event handler declarations

////@begin CGeneralIDPanel member function declarations

    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGeneralIDPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGeneralIDPanel member variables
    wxChoice* m_GenID;
    wxStaticText* m_Dblabel;
    wxTextCtrl* m_DbName;
    /// Control identifiers
    enum {
        ID_CGENERALIDPANEL = 6150,
        ID_CGENERALID_CHOICE = 6151,
        ID_CGENERALID_TEXT = 6152
    };
////@end CGeneralIDPanel member variables
private:
    vector<string> x_GetStrings();
};

END_NCBI_SCOPE

#endif
    // _GENERALID_PANEL_H_
