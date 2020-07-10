#ifndef GUI_WIDGETS_EDIT___RNA_PANEL__HPP
#define GUI_WIDGETS_EDIT___RNA_PANEL__HPP

/*  $Id: rna_panel.hpp 35569 2016-05-25 13:47:34Z asztalos $
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
#include "wx/choicebk.h"
////@end includes

#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxChoicebook;
////@end forward declarations

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
    class CTrna_ext;
    class CSeq_id;
END_SCOPE(objects)

class CGBQualPanel;
/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CRNAPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CRNAPANEL_TITLE _("RNA Panel")
#define SYMBOL_CRNAPANEL_IDNAME ID_CRNAPANEL
#define SYMBOL_CRNAPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRNAPANEL_POSITION wxDefaultPosition
////@end control identifiers


class CSerialObject;

/*!
 * CRNAPanel class declaration
 */

class CRNAPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CRNAPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRNAPanel();
    CRNAPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope,
               wxWindowID id = SYMBOL_CRNAPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRNAPANEL_POSITION, const wxSize& size = SYMBOL_CRNAPANEL_SIZE, long style = SYMBOL_CRNAPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRNAPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRNAPANEL_POSITION, const wxSize& size = SYMBOL_CRNAPANEL_SIZE, long style = SYMBOL_CRNAPANEL_STYLE );

    /// Destructor
    ~CRNAPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRNAPanel event handler declarations

////@end CRNAPanel event handler declarations

////@begin CRNAPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRNAPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRNAPanel member variables
    wxChoicebook* m_Choicebook;
    wxComboBox* m_TranscriptID;
    /// Control identifiers
    enum {
        ID_CRNAPANEL = 10017,
        ID_CHOICEBOOK1 = 10018,
        ID_COMBOBOX1 = 10024
    };
////@end CRNAPanel member variables

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();
    static void x_GetLabelForSeqID(const objects::CSeq_id& id, string& id_label);

    void SetGBQualPanel(CGBQualPanel* gbqual_panel);

private:
    void x_PopulateTranscriptID();
    CSerialObject* m_Object;
    objects::CScope* m_Scope;
    CRef<objects::CTrna_ext> m_trna;
    CRef<objects::CRNA_gen> m_ncRNAGen;
    CRef<objects::CRNA_gen> m_tmRNAGen;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___RNA_PANEL__HPP
