#ifndef GUI_WIDGETS_EDIT___LOCATION_AND_GCODE_PANEL__HPP
#define GUI_WIDGETS_EDIT___LOCATION_AND_GCODE_PANEL__HPP

/*  $Id: loc_gcode_panel.hpp 36541 2016-10-05 21:37:22Z filippov $
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
 * Authors:  Vasuki Palanigobu
 */


#include <objects/seqfeat/BioSource.hpp>
#include <wx/panel.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CLOCANDGCODEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CLOCANDGCODEPANEL_TITLE _("LocationGCodePanel")
#define SYMBOL_CLOCANDGCODEPANEL_IDNAME ID_GENETICCODESPANEL
#define SYMBOL_CLOCANDGCODEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CLOCANDGCODEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLocAndGCodePanel class declaration
 */

class CLocAndGCodePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CLocAndGCodePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLocAndGCodePanel();
    CLocAndGCodePanel( wxWindow* parent, objects::CBioSource& source, wxWindowID id = SYMBOL_CLOCANDGCODEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOCANDGCODEPANEL_POSITION, const wxSize& size = SYMBOL_CLOCANDGCODEPANEL_SIZE, long style = SYMBOL_CLOCANDGCODEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLOCANDGCODEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOCANDGCODEPANEL_POSITION, const wxSize& size = SYMBOL_CLOCANDGCODEPANEL_SIZE, long style = SYMBOL_CLOCANDGCODEPANEL_STYLE );

    /// Destructor
    ~CLocAndGCodePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void PopulateLocAndGC(objects::CBioSource& source);

////@begin CLocAndGCodePanel event handler declarations

////@end CLocAndGCodePanel event handler declarations

////@begin CLocAndGCodePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLocAndGCodePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnChangedTaxname(void);
////@begin CLocAndGCodePanel member variables
    wxComboBox* m_LocCtrl;
    wxComboBox* m_OriginCtrl;
    wxComboBox* m_NuclearCodeCtrl;
    wxComboBox* m_MitoCodeCtrl;
    wxComboBox* m_PlastidCodeCtrl;
    wxCheckBox* m_BioFocusCtrl;
    /// Control identifiers
    enum {
        ID_GENETICCODESPANEL = 10006,
        ID_COMBOBOX5 = 10028,
        ID_COMBOBOX6 = 10029,
        ID_COMBOBOX = 10007,
        ID_COMBOBOX1 = 10008,
        ID_COMBOBOX2 = 10002,
        ID_CHECKBOX1 = 10030
    };
////@end CLocAndGCodePanel member variables
private:
    objects::CBioSource& m_Source;
   //setters to m_Source
    void x_SetPlastidCode();
    void x_SetMitochondrialCode();
    void x_SetGeneticCode();
    void x_SetOrigin();
    void x_SetLocation();
   
   //getters from m_Source
    string x_GetPlastidCode();
    string x_GetMitochondrialCode();
    string x_GetGeneticCode();   
    string x_GetOrigin();
    string x_GetLocation();
    map<int, string> m_CodeToName;
    map<string, int> m_NameToCode;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___LOCATION_AND_GCODE_PANEL__HPP
