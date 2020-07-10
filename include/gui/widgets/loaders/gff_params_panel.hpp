#ifndef GUI_WIDGETS___LOADERS___GFF_PARAMS_PANEL__HPP
#define GUI_WIDGETS___LOADERS___GFF_PARAMS_PANEL__HPP

/*  $Id: gff_params_panel.hpp 38144 2017-04-03 16:55:05Z katargir $
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
#include <gui/gui_export.h>

/*!
 * Includes
 */

#include <wx/panel.h>

#include <gui/widgets/loaders/gff_load_params.hpp>

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGFFPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CGFFPARAMSPANEL_TITLE _("GFF Format Params Panel")
#define SYMBOL_CGFFPARAMSPANEL_IDNAME ID_CGFFPARAMSPANEL
#define SYMBOL_CGFFPARAMSPANEL_SIZE wxSize(400, 500)
#define SYMBOL_CGFFPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CAssemblySelPanel;

/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

/*!
 * CGffParamsPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CGffParamsPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CGffParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGffParamsPanel();
    CGffParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CGFFPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGFFPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CGFFPARAMSPANEL_SIZE, long style = SYMBOL_CGFFPARAMSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGFFPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGFFPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CGFFPARAMSPANEL_SIZE, long style = SYMBOL_CGFFPARAMSPANEL_STYLE );

    /// Destructor
    ~CGffParamsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CGffParamsPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SEQ_ID_NORMAL
    void OnParseSeqIdNormalSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for IDSEQ_ID_NUMERIC
    void OnParseSeqIdNumericSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_SEQ_ID_LOCAL
    void OnParseSeqIdLocalSelected( wxCommandEvent& event );

////@end CGffParamsPanel event handler declarations

////@begin CGffParamsPanel member function declarations

    /// Data access
    CGffLoadParams& GetData() { return m_data; }
    const CGffLoadParams& GetData() const { return m_data; }
    void SetData(const CGffLoadParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGffParamsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGffParamsPanel member variables
    CAssemblySelPanel* m_AssemblyPanel;
    /// The data edited by this window
    CGffLoadParams m_data;
    /// Control identifiers
    enum {
        ID_CGFFPARAMSPANEL = 10003,
        ID_CHOICE2 = 10005,
        ID_TEXTCTRL1 = 10012,
        ID_PANEL = 10004,
        ID_SEQ_ID_NORMAL = 10025,
        IDSEQ_ID_NUMERIC = 10026,
        ID_SEQ_ID_LOCAL = 10027,
        ID_PANEL7 = 10091
    };
////@end CGffParamsPanel member variables

private:
    void x_SetParseSeqId(int val);
};

/* @} */

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___GFF_PARAMS_PANEL__HPP
