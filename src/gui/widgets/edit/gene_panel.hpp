#ifndef GUI_WIDGETS_EDIT___GENE_PANEL__HPP
#define GUI_WIDGETS_EDIT___GENE_PANEL__HPP

/*  $Id: gene_panel.hpp 30569 2014-06-12 18:33:02Z asztalos $
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

#include <corelib/ncbiobj.hpp>

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
#define SYMBOL_CGENEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGENEPANEL_TITLE _("Gene")
#define SYMBOL_CGENEPANEL_IDNAME ID_CGENEPANEL
#define SYMBOL_CGENEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGENEPANEL_POSITION wxDefaultPosition
////@end control identifiers


class CSerialObject;

/*!
 * CGenePanel class declaration
 */

class CGenePanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CGenePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGenePanel();
    CGenePanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope,
                wxWindowID id = SYMBOL_CGENEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGENEPANEL_POSITION, const wxSize& size = SYMBOL_CGENEPANEL_SIZE, long style = SYMBOL_CGENEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGENEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGENEPANEL_POSITION, const wxSize& size = SYMBOL_CGENEPANEL_SIZE, long style = SYMBOL_CGENEPANEL_STYLE );

    /// Destructor
    ~CGenePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CGenePanel event handler declarations

////@end CGenePanel event handler declarations

////@begin CGenePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGenePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGenePanel member variables
    /// Control identifiers
    enum {
        ID_CGENEPANEL = 10003,
        ID_TEXT1_GENEPNL = 10004,
        ID_TEXT2_GENEPNL = 10005,
        ID_TEXT3_GENEPNL = 10006,
        ID_TEXT4_GENEPNL = 10174,
        ID_TEXT5_GENEPNL = 10008
    };
////@end CGenePanel member variables

private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___GENE_PANEL__HPP
