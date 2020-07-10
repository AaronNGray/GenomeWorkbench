#ifndef GUI_WIDGETS_EDIT___LOCATION_PANEL__HPP
#define GUI_WIDGETS_EDIT___LOCATION_PANEL__HPP

/*  $Id: location_panel.hpp 35777 2016-06-22 15:47:52Z filippov $
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

#include <objmgr/scope.hpp>
#include <objects/seqloc/Packed_seqint.hpp>
#include <gui/widgets/edit/import_export_access.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/checkbox.h>

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
class CLocationListCtrl;
////@end forward declarations

class CSerialObject;

BEGIN_SCOPE(objects)
    class CSeq_loc;
END_SCOPE(objects)

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CLOCATIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CLOCATIONPANEL_TITLE _("Location Panel")
#define SYMBOL_CLOCATIONPANEL_IDNAME ID_CLOCATIONPANEL
#define SYMBOL_CLOCATIONPANEL_SIZE wxDefaultSize
#define SYMBOL_CLOCATIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLocationPanel class declaration
 */

class CLocationPanel: public wxPanel, public CImportExportAccess
{    
    DECLARE_DYNAMIC_CLASS( CLocationPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLocationPanel();
    CLocationPanel( wxWindow* parent, CSerialObject& object, objects::CScope* scope, bool allow_nuc = true, bool allow_prot = true, bool is_cds = false,
                    wxWindowID id = SYMBOL_CLOCATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOCATIONPANEL_POSITION, const wxSize& size = SYMBOL_CLOCATIONPANEL_SIZE, long style = SYMBOL_CLOCATIONPANEL_STYLE );
    CLocationPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope, bool allow_nuc = true, bool allow_prot = true, bool is_cds = false,
                    wxWindowID id = SYMBOL_CLOCATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOCATIONPANEL_POSITION, const wxSize& size = SYMBOL_CLOCATIONPANEL_SIZE, long style = SYMBOL_CLOCATIONPANEL_STYLE );
    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLOCATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOCATIONPANEL_POSITION, const wxSize& size = SYMBOL_CLOCATIONPANEL_SIZE, long style = SYMBOL_CLOCATIONPANEL_STYLE );

    /// Destructor
    ~CLocationPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CLocationPanel event handler declarations

////@end CLocationPanel event handler declarations

////@begin CLocationPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLocationPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CLocationPanel member variables
    CLocationListCtrl* m_LocationCtrl;
    /// Control identifiers
    enum {
        ID_CLOCATIONPANEL = 10036,
        ID_WINDOW2 = 10037
    };
////@end CLocationPanel member variables

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    CRef<objects::CSeq_loc> GetSeq_loc() const;
    void SetSeq_loc(CSerialObject& object);

    virtual bool IsImportEnabled(void);
    virtual bool IsExportEnabled(void);
    virtual CRef<CSerialObject> OnExport(void);
    virtual void OnImport( CNcbiIfstream &istr);
    bool ShouldUpdatemRNALocation() { return m_UpdatemRNASpan ? m_UpdatemRNASpan->GetValue() : false; }
private:
    CSerialObject* m_Object;
    CRef<objects::CScope> m_Scope;
    CRef<objects::CPacked_seqint> m_ToEdit;
    void x_FillList();
    void x_LoadSeqLoc(const objects::CSeq_loc& loc);
    void x_SetJoinOrder();
    bool m_AllowNuc;
    bool m_AllowProt;
    bool m_IsOrdered;
    wxCheckBox* m_UpdatemRNASpan;
    bool m_is_cds;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___LOCATION_PANEL__HPP
