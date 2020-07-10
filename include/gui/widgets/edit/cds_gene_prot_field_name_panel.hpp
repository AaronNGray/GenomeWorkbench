#ifndef GUI_WIDGETS_EDIT___CDSGENEPROTFIELDNAMEPANEL__HPP
#define GUI_WIDGETS_EDIT___CDSGENEPROTFIELDNAMEPANEL__HPP
/*  $Id: cds_gene_prot_field_name_panel.hpp 44691 2020-02-21 19:19:39Z asztalos $
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

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/field_name_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/listbox.h>

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
#define SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_TITLE _("CDSGeneProtFieldNamePanel")
#define SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_IDNAME ID_CCDSGENEPROTFIELDNAMEPANEL
#define SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCDSGeneProtFieldNamePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CCDSGeneProtFieldNamePanel : public CFieldNamePanel
{    
    DECLARE_DYNAMIC_CLASS( CCDSGeneProtFieldNamePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCDSGeneProtFieldNamePanel();
    CCDSGeneProtFieldNamePanel(wxWindow* parent, 
        wxWindowID id = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_STYLE);

    /// Creation
    bool Create(wxWindow* parent, 
        wxWindowID id = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_SIZE, 
        long style = SYMBOL_CCDSGENEPROTFIELDNAMEPANEL_STYLE);

    /// Destructor
    ~CCDSGeneProtFieldNamePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCDSGeneProtFieldNamePanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_FIELD
    void OnFieldSelected( wxCommandEvent& event );

////@end CCDSGeneProtFieldNamePanel event handler declarations

////@begin CCDSGeneProtFieldNamePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCDSGeneProtFieldNamePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCDSGeneProtFieldNamePanel member variables
    wxListBox* m_Field;
////@end CCDSGeneProtFieldNamePanel member variables

    virtual string GetFieldName(const bool subfield = false);
    virtual bool SetFieldName(const string& field);
    virtual void ClearValues(void);
    virtual vector<string> GetChoices(bool& allow_other);
    virtual string GetMacroFieldName(const string &target, const string& selected_field = kEmptyStr);

    static vector<string> GetStrings();
    enum {
        ID_CCDSGENEPROTFIELDNAMEPANEL = 6080,
        ID_CCDSGENEPROTFIELDNAME_FIELD
    };
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___CDSGENEPROTFIELDNAMEPANEL__HPP
