/*  $Id: trna_product_panel.hpp 35570 2016-05-25 14:38:34Z asztalos $
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
#ifndef _TRNA_PRODUCT_PANEL_H_
#define _TRNA_PRODUCT_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <wx/panel.h>

class wxRadioBox;
class wxChoice;
class wxTreebook;

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
    class CTrna_ext;
    class CSeq_feat;
END_SCOPE(objects);

class CGBQualPanel;
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTRNAPRODUCTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CTRNAPRODUCTPANEL_TITLE _("tRNA Product Panel")
#define SYMBOL_CTRNAPRODUCTPANEL_IDNAME ID_TRNAPRODUCTPANEL
#define SYMBOL_CTRNAPRODUCTPANEL_SIZE wxDefaultSize
#define SYMBOL_CTRNAPRODUCTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CtRNAProductPanel class declaration
 */

class CtRNAProductPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CtRNAProductPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CtRNAProductPanel();
    CtRNAProductPanel(wxWindow* parent, CRef<objects::CTrna_ext> trna,
                       CConstRef<objects::CSeq_feat> feat,
                       wxWindowID id = SYMBOL_CTRNAPRODUCTPANEL_IDNAME, 
                       const wxPoint& pos = SYMBOL_CTRNAPRODUCTPANEL_POSITION, 
                       const wxSize& size = SYMBOL_CTRNAPRODUCTPANEL_SIZE, 
                       long style = SYMBOL_CTRNAPRODUCTPANEL_STYLE );

    /// Creation
    bool Create(wxWindow* parent, 
                wxWindowID id = SYMBOL_CTRNAPRODUCTPANEL_IDNAME, 
                const wxPoint& pos = SYMBOL_CTRNAPRODUCTPANEL_POSITION, 
                const wxSize& size = SYMBOL_CTRNAPRODUCTPANEL_SIZE, 
                long style = SYMBOL_CTRNAPRODUCTPANEL_STYLE );

    /// Destructor
    ~CtRNAProductPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void SetGBQualPanel(CGBQualPanel* gbqual_panel) { m_GbQualPanel = gbqual_panel; }

////@begin CtRNAProductPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_TRNAPRODUCT_CHOICE
    void OnAminoAcidSelected( wxCommandEvent& event );

    void OnInitiatorSelected( wxCommandEvent& event );


////@end CtRNAProductPanel event handler declarations

////@begin CtRNAProductPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CtRNAProductPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
    void x_SetAminoAcid();
    void x_EnableInitiatorSelection();
    wxTreebook* x_GetTree();

    
////@begin CtRNAProductPanel member variables
    wxChoice* m_AminoAcidCtrl;
    wxRadioBox* m_InitiatorCtrl;

    CRef<objects::CTrna_ext> m_trna;
    CConstRef<objects::CSeq_feat> m_Feat;

    /// Control identifiers
    enum {
        ID_TRNAPRODUCTPANEL = 8000,
        ID_TRNAPRODUCT_CHOICE = 8001,
        ID_TRNAPRODUCT_RDBX = 8002
    };
    CGBQualPanel* m_GbQualPanel;
////@end CtRNAProductPanel member variables

};

END_NCBI_SCOPE

#endif
    // _TRNA_PRODUCT_PANEL_H_
