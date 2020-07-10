/*  $Id: secondary_structure_panel.hpp 28513 2013-07-23 17:21:28Z bollin $
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
#ifndef _SECONDARY_STRUCTURE_PANEL_H_
#define _SECONDARY_STRUCTURE_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

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

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSECONDARYSTRUCTUREPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSECONDARYSTRUCTUREPANEL_TITLE _("SecondaryStructurePanel")
#define SYMBOL_CSECONDARYSTRUCTUREPANEL_IDNAME ID_CSECONDARYSTRUCTUREPANEL
#define SYMBOL_CSECONDARYSTRUCTUREPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSECONDARYSTRUCTUREPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSecondaryStructurePanel class declaration
 */

class CSecondaryStructurePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSecondaryStructurePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSecondaryStructurePanel();
    CSecondaryStructurePanel( wxWindow* parent, wxWindowID id = SYMBOL_CSECONDARYSTRUCTUREPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSECONDARYSTRUCTUREPANEL_POSITION, const wxSize& size = SYMBOL_CSECONDARYSTRUCTUREPANEL_SIZE, long style = SYMBOL_CSECONDARYSTRUCTUREPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSECONDARYSTRUCTUREPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSECONDARYSTRUCTUREPANEL_POSITION, const wxSize& size = SYMBOL_CSECONDARYSTRUCTUREPANEL_SIZE, long style = SYMBOL_CSECONDARYSTRUCTUREPANEL_STYLE );

    /// Destructor
    ~CSecondaryStructurePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSecondaryStructurePanel event handler declarations

////@end CSecondaryStructurePanel event handler declarations

////@begin CSecondaryStructurePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSecondaryStructurePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSecondaryStructurePanel member variables
    wxChoice* m_Type;
    /// Control identifiers
    enum {
        ID_CSECONDARYSTRUCTUREPANEL = 10158,
        ID_CHOICE17 = 10159
    };
////@end CSecondaryStructurePanel member variables

    objects::CSeqFeatData::EPsec_str GetSecondaryStructure();
    void SetSecondaryStructure(objects::CSeqFeatData::EPsec_str struc);
};

END_NCBI_SCOPE

#endif
    // _SECONDARY_STRUCTURE_PANEL_H_
