#ifndef PKG_ALIGNMENT___FIND_OVERLAP_PANEL__HPP
#define PKG_ALIGNMENT___FIND_OVERLAP_PANEL__HPP

/*  $Id: find_overlap_panel.hpp 32655 2015-04-07 18:11:22Z evgeniev $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

#include <corelib/ncbiobj.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/find_overlap_params.hpp>

#include "wx/valtext.h"

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFINDOVERLAPPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFINDOVERLAPPANEL_TITLE _("Find Overlap Panel")
#define SYMBOL_CFINDOVERLAPPANEL_IDNAME ID_CFINDOVERLAPPANEL
#define SYMBOL_CFINDOVERLAPPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CFINDOVERLAPPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CObjectListWidget;

/*!
 * CFindOverlapPanel class declaration
 */

class CFindOverlapPanel: public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CFindOverlapPanel )
    DECLARE_EVENT_TABLE()

public:
    CFindOverlapPanel();
    CFindOverlapPanel( wxWindow* parent, wxWindowID id = SYMBOL_CFINDOVERLAPPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFINDOVERLAPPANEL_POSITION, const wxSize& size = SYMBOL_CFINDOVERLAPPANEL_SIZE, long style = SYMBOL_CFINDOVERLAPPANEL_STYLE, bool visible = true );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFINDOVERLAPPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFINDOVERLAPPANEL_POSITION, const wxSize& size = SYMBOL_CFINDOVERLAPPANEL_SIZE, long style = SYMBOL_CFINDOVERLAPPANEL_STYLE, bool visible = true );

    ~CFindOverlapPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

////@begin CFindOverlapPanel event handler declarations

////@end CFindOverlapPanel event handler declarations

////@begin CFindOverlapPanel member function declarations

    CFindOverlapParams& GetData() { return m_data; }
    const CFindOverlapParams& GetData() const { return m_data; }
    void SetData(const CFindOverlapParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CFindOverlapPanel member function declarations

    void SetObjects(TConstScopedObjects* objects) { m_InputObjects = objects; }

    static bool ShowToolTips();

////@begin CFindOverlapPanel member variables
    CObjectListWidget* m_Seq1List;
    CObjectListWidget* m_Seq2List;
    CFindOverlapParams m_data;
    enum {
        ID_CFINDOVERLAPPANEL = 10021,
        ID_LISTCTRL3 = 10022,
        ID_LISTCTRL4 = 10023,
        ID_TEXTCTRL2 = 10024,
        ID_TEXTCTRL4 = 10026,
        ID_TEXTCTRL5 = 10027
    };
////@end CFindOverlapPanel member variables

    TConstScopedObjects*    m_InputObjects;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___FIND_OVERLAP_PANEL__HPP
