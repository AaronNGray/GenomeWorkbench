#ifndef GUI_WIDGETS_EDIT___PROTEIN_PROPERTIES_PANEL__HPP
#define GUI_WIDGETS_EDIT___PROTEIN_PROPERTIES_PANEL__HPP

/*  $Id: protein_properties.hpp 39095 2017-07-27 19:56:04Z filippov $
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
#include <corelib/ncbiobj.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
/*!
 * Includes
 */

#include <wx/panel.h>


////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class CScope;
END_SCOPE(objects)

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPROTEINPROPERTIES_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROTEINPROPERTIES_TITLE _("Protein properties")
#define SYMBOL_CPROTEINPROPERTIES_IDNAME wxID_ANY
#define SYMBOL_CPROTEINPROPERTIES_SIZE wxDefaultSize
#define SYMBOL_CPROTEINPROPERTIES_POSITION wxDefaultPosition
////@end control identifiers

class CSerialObject;

/*!
 * CProteinPropertiesPanel class declaration
 */

class CProteinPropertiesPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CProteinPropertiesPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProteinPropertiesPanel();
    CProteinPropertiesPanel( wxWindow* parent, CSerialObject& object, objects::CSeq_feat* edited_protein,
                   wxWindowID id = SYMBOL_CPROTEINPROPERTIES_IDNAME, const wxPoint& pos = SYMBOL_CPROTEINPROPERTIES_POSITION, const wxSize& size = SYMBOL_CPROTEINPROPERTIES_SIZE, long style = SYMBOL_CPROTEINPROPERTIES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROTEINPROPERTIES_IDNAME, const wxPoint& pos = SYMBOL_CPROTEINPROPERTIES_POSITION, const wxSize& size = SYMBOL_CPROTEINPROPERTIES_SIZE, long style = SYMBOL_CPROTEINPROPERTIES_STYLE );

    /// Destructor
    ~CProteinPropertiesPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProteinPropertiesPanel event handler declarations

////@end CProteinPropertiesPanel event handler declarations

////@begin CProteinPropertiesPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CProteinPropertiesPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProteinPropertiesPanel member variables
////@end CProteinPropertiesPanel member variables
private:
    CSerialObject* m_Object;
    objects::CSeq_feat* m_edited_protein;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___PROTEIN_PROPERTIES_PANEL__HPP
