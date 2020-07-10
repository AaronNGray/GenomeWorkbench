#ifndef PKG_ALIGNMENT___CREATE_NEEDLEMAN_WUNSCH_PANEL__HPP
#define PKG_ALIGNMENT___CREATE_NEEDLEMAN_WUNSCH_PANEL__HPP

/*  $Id: create_needleman_wunsch_panel.hpp 42821 2019-04-18 19:32:56Z joukovv $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/needlemanwunsch_tool_params.hpp>

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
class CObjectListWidget;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_TITLE _("Create Needleman-Wunsch Alignment")
#define SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_IDNAME ID_CALIGNNEEDLEMANWUNSCHPANEL
#define SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_POSITION wxDefaultPosition
////@end control identifiers



/*!
 * CAlignNeedlemanWunschPanel class declaration
 */

class CAlignNeedlemanWunschPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CAlignNeedlemanWunschPanel )
    DECLARE_EVENT_TABLE()

public:
    CAlignNeedlemanWunschPanel();
    CAlignNeedlemanWunschPanel( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_POSITION, const wxSize& size = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_SIZE, long style = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_POSITION, const wxSize& size = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_SIZE, long style = SYMBOL_CALIGNNEEDLEMANWUNSCHPANEL_STYLE );

    ~CAlignNeedlemanWunschPanel();

    void Init();

    void CreateControls();

    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;

    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

////@begin CAlignNeedlemanWunschPanel event handler declarations

////@end CAlignNeedlemanWunschPanel event handler declarations

////@begin CAlignNeedlemanWunschPanel member function declarations

    CNeedlemanWunschToolParams& GetData() { return m_data; }
    const CNeedlemanWunschToolParams& GetData() const { return m_data; }
    void SetData(const CNeedlemanWunschToolParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAlignNeedlemanWunschPanel member function declarations

    void SetObjects(TConstScopedObjects* objects);

    static bool ShowToolTips();

////@begin CAlignNeedlemanWunschPanel member variables
    CObjectListWidget* m_ObjectList;
    CNeedlemanWunschToolParams m_data;
    enum {
        ID_CALIGNNEEDLEMANWUNSCHPANEL = 10046,
        ID_LISTCTRL2 = 10014,
        ID_TEXTCTRL = 10000,
        ID_TEXTCTRL1 = 10001,
        ID_TEXTCTRL2 = 10002,
        ID_TEXTCTRL3 = 10004,
        ID_TEXTCTRL10 = 10048,
        ID_CHOICE = 10049,
        ID_CHOICE1 = 10003
    };
////@end CAlignNeedlemanWunschPanel member variables

private:
    string  m_RegPath;
};

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___CREATE_NEEDLEMAN_WUNSCH_PANEL__HPP
