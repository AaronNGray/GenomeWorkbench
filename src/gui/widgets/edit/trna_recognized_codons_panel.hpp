/*  $Id: trna_recognized_codons_panel.hpp 25347 2012-03-01 18:22:54Z katargir $
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
#ifndef _TRNA_RECOGNIZED_CODONS_PANEL_H_
#define _TRNA_RECOGNIZED_CODONS_PANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/icon.h>

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
#define SYMBOL_CTRNARECOGNIZEDCODONSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CTRNARECOGNIZEDCODONSPANEL_TITLE _("tRNA Codons Panel")
#define SYMBOL_CTRNARECOGNIZEDCODONSPANEL_IDNAME SYMBOL_CTRNARECOGNIZEDCODONSPANEL_IDNAME
#define SYMBOL_CTRNARECOGNIZEDCODONSPANEL_SIZE wxSize(50, 113)
#define SYMBOL_CTRNARECOGNIZEDCODONSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CtRNARecognizedCodonsPanel class declaration
 */

class CtRNARecognizedCodonsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CtRNARecognizedCodonsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CtRNARecognizedCodonsPanel();
    CtRNARecognizedCodonsPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_POSITION, const wxSize& size = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_SIZE, long style = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_POSITION, const wxSize& size = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_SIZE, long style = SYMBOL_CTRNARECOGNIZEDCODONSPANEL_STYLE );

    /// Destructor
    ~CtRNARecognizedCodonsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CtRNARecognizedCodonsPanel event handler declarations

////@end CtRNARecognizedCodonsPanel event handler declarations

////@begin CtRNARecognizedCodonsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CtRNARecognizedCodonsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CtRNARecognizedCodonsPanel member variables
    /// Control identifiers
    enum {
        SYMBOL_CTRNARECOGNIZEDCODONSPANEL_IDNAME = 10019,
        ID_WINDOW = 10020
    };
////@end CtRNARecognizedCodonsPanel member variables
private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif
    // _TRNA_RECOGNIZED_CODONS_PANEL_H_
