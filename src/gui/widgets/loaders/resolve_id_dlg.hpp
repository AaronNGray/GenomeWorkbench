#ifndef GUI_WIDGETS___LOADERS___RESOLVE_ID_DLG__HPP
#define GUI_WIDGETS___LOADERS___RESOLVE_ID_DLG__HPP

/*  $Id: resolve_id_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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

#include <gui/widgets/wx/dialog.hpp>

#include <objects/seq/seq_id_handle.hpp>

////@begin includes
#include "wx/grid.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CRESOLVEIDDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRESOLVEIDDLG_TITLE _("Check Sequence Identifiers")
#define SYMBOL_CRESOLVEIDDLG_IDNAME ID_CRESOLVEIDDLG
#define SYMBOL_CRESOLVEIDDLG_SIZE wxSize(400, 300)
#define SYMBOL_CRESOLVEIDDLG_POSITION wxDefaultPosition
////@end control identifiers

class wxScrolledWindow;

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

/*!
 * CResolveIdDlg class declaration
 */

class CResolveIdDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CResolveIdDlg )
    DECLARE_EVENT_TABLE()

public:
    typedef vector<objects::CSeq_id_Handle> THandles;
    typedef vector<pair<string, THandles> > TData;
    typedef map<objects::CSeq_id_Handle, objects::CSeq_id_Handle> TIdMap;

    /// Constructors
    CResolveIdDlg();
    CResolveIdDlg( wxWindow* parent, wxWindowID id = SYMBOL_CRESOLVEIDDLG_IDNAME, const wxString& caption = SYMBOL_CRESOLVEIDDLG_TITLE, const wxPoint& pos = SYMBOL_CRESOLVEIDDLG_POSITION, const wxSize& size = SYMBOL_CRESOLVEIDDLG_SIZE, long style = SYMBOL_CRESOLVEIDDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRESOLVEIDDLG_IDNAME, const wxString& caption = SYMBOL_CRESOLVEIDDLG_TITLE, const wxPoint& pos = SYMBOL_CRESOLVEIDDLG_POSITION, const wxSize& size = SYMBOL_CRESOLVEIDDLG_SIZE, long style = SYMBOL_CRESOLVEIDDLG_STYLE );

    /// Destructor
    ~CResolveIdDlg();

    /// Initialises member variables
    void Init();

    void SetData(const TData& data, TIdMap& map, objects::CScope& scope);

    /// Creates the controls and sizers
    void CreateControls();

////@begin CResolveIdDlg event handler declarations

////@end CResolveIdDlg event handler declarations

////@begin CResolveIdDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CResolveIdDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CResolveIdDlg member variables
    wxGrid* m_Grid;
    /// Control identifiers
    enum {
        ID_CRESOLVEIDDLG = 10014,
        ID_GRID = 10016
    };
////@end CResolveIdDlg member variables

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;
};

/* @} */

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___RESOLVE_ID_DLG__HPP
