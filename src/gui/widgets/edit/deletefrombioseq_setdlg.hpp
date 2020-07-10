#ifndef GUI_WIDGETS_SEQ___DELETEFROMFIOSEQ_SETDLG__HPP
#define GUI_WIDGETS_SEQ___DELETEFROMFIOSEQ_SETDLG__HPP
/*  $Id: deletefrombioseq_setdlg.hpp 43278 2019-06-06 20:04:03Z asztalos $
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

#include <gui/widgets/wx/dialog.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CDELETEFROMBIOSEQ_SETDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CDELETEFROMBIOSEQ_SETDLG_TITLE _("Information applies to multiple sequences")
#define SYMBOL_CDELETEFROMBIOSEQ_SETDLG_IDNAME ID_CDELETEFROMBIOSEQ_SETDLG
#define SYMBOL_CDELETEFROMBIOSEQ_SETDLG_SIZE wxSize(400, 300)
#define SYMBOL_CDELETEFROMBIOSEQ_SETDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CDeleteFromBioseq_setDlg class declaration
 */

class CDeleteFromBioseq_setDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CDeleteFromBioseq_setDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDeleteFromBioseq_setDlg();
    CDeleteFromBioseq_setDlg( wxWindow* parent, wxWindowID id = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_IDNAME, const wxString& caption = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_TITLE, const wxPoint& pos = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_POSITION, const wxSize& size = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_SIZE, long style = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_IDNAME, const wxString& caption = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_TITLE, const wxPoint& pos = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_POSITION, const wxSize& size = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_SIZE, long style = SYMBOL_CDELETEFROMBIOSEQ_SETDLG_STYLE );

    /// Destructor
    ~CDeleteFromBioseq_setDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDeleteFromBioseq_setDlg event handler declarations

////@end CDeleteFromBioseq_setDlg event handler declarations

////@begin CDeleteFromBioseq_setDlg member function declarations

    int GetChoice() const { return m_Choice ; }
    void SetChoice(int value) { m_Choice = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDeleteFromBioseq_setDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDeleteFromBioseq_setDlg member variables
private:
    int m_Choice;
    /// Control identifiers
    enum {
        ID_CDELETEFROMBIOSEQ_SETDLG = 10059,
        ID_RADIOBOX = 10062
    };
////@end CDeleteFromBioseq_setDlg member variables

protected:
    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    virtual bool TransferDataToWindow();
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___DELETEFROMFIOSEQ_SETDLG__HPP
