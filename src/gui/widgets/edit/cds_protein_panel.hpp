#ifndef GUI_WIDGETS_EDIT___CDS_PROTEIN_PANEL__HPP
#define GUI_WIDGETS_EDIT___CDS_PROTEIN_PANEL__HPP

/*  $Id: cds_protein_panel.hpp 25347 2012-03-01 18:22:54Z katargir $
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
 * Authors:  Roman Katargin
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
////@end includes

#include <wx/panel.h>
#include <wx/checkbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class CScope;
END_SCOPE(objects)

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCDSPROTEINPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCDSPROTEINPANEL_TITLE _("CDS Protein Panel")
#define SYMBOL_CCDSPROTEINPANEL_IDNAME ID_CCDSPROTEINPANEL
#define SYMBOL_CCDSPROTEINPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCDSPROTEINPANEL_POSITION wxDefaultPosition
////@end control identifiers

class CSerialObject;

/*!
 * CCDSProteinPanel class declaration
 */

class CCDSProteinPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CCDSProteinPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCDSProteinPanel();
    CCDSProteinPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope,
                      wxWindowID id = SYMBOL_CCDSPROTEINPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCDSPROTEINPANEL_POSITION, const wxSize& size = SYMBOL_CCDSPROTEINPANEL_SIZE, long style = SYMBOL_CCDSPROTEINPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCDSPROTEINPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCDSPROTEINPANEL_POSITION, const wxSize& size = SYMBOL_CCDSPROTEINPANEL_SIZE, long style = SYMBOL_CCDSPROTEINPANEL_STYLE );

    /// Destructor
    ~CCDSProteinPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    bool ShouldUpdatemRNAProduct() { return m_AdjustmRNAProductName->GetValue(); }

////@begin CCDSProteinPanel event handler declarations

////@end CCDSProteinPanel event handler declarations

////@begin CCDSProteinPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCDSProteinPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCDSProteinPanel member variables
    wxCheckBox* m_AdjustmRNAProductName;
    /// Control identifiers
    enum {
        ID_CCDSPROTEINPANEL = 10031,
        ID_WINDOW1 = 10034,
        ID_TEXTCTRL12 = 10033,
        ID_CHECKBOX4 = 10058
    };
////@end CCDSProteinPanel member variables

private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___CDS_PROTEIN_PANEL__HPP
