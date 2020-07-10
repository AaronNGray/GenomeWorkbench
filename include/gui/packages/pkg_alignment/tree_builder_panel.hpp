#ifndef PKG_ALIGNMENT___TREE_BUILDER_PANEL__HPP
#define PKG_ALIGNMENT___TREE_BUILDER_PANEL__HPP

/*  $Id: tree_builder_panel.hpp 32655 2015-04-07 18:11:22Z evgeniev $
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
 * Authors:  Roman Katargin, Vladimir Tereshkov
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

#include <corelib/ncbiobj.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/tree_builder_params.hpp>

#include <wx/panel.h>
#include <wx/choice.h>

#include "wx/valtext.h"

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTreeBuilderPanel_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CTreeBuilderPanel_TITLE wxT("Find Overlap Panel")
#define SYMBOL_CTreeBuilderPanel_IDNAME ID_CTreeBuilderPanel
#define SYMBOL_CTreeBuilderPanel_SIZE wxSize(400, 300)
#define SYMBOL_CTreeBuilderPanel_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CObjectListWidget;

/*!
 * CTreeBuilderPanel class declaration
 */

class CTreeBuilderPanel: public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CTreeBuilderPanel )
    DECLARE_EVENT_TABLE()

public:
    CTreeBuilderPanel();
    CTreeBuilderPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTreeBuilderPanel_IDNAME, const wxPoint& pos = SYMBOL_CTreeBuilderPanel_POSITION, const wxSize& size = SYMBOL_CTreeBuilderPanel_SIZE, long style = SYMBOL_CTreeBuilderPanel_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTreeBuilderPanel_IDNAME, const wxPoint& pos = SYMBOL_CTreeBuilderPanel_POSITION, const wxSize& size = SYMBOL_CTreeBuilderPanel_SIZE, long style = SYMBOL_CTreeBuilderPanel_STYLE );

    ~CTreeBuilderPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

////@begin CTreeBuilderPanel event handler declarations

////@end CTreeBuilderPanel event handler declarations

////@begin CTreeBuilderPanel member function declarations

    CTreeBuilderParams& GetData() { return m_data; }
    const CTreeBuilderParams& GetData() const { return m_data; }
    void SetData(const CTreeBuilderParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CTreeBuilderPanel member function declarations

    void SetObjects(TConstScopedObjects* objects) { m_InputObjects = objects; }

    static bool ShowToolTips();

////@begin CTreeBuilderPanel member variables
    CObjectListWidget* m_Seq1List;    
    CTreeBuilderParams m_data;
    enum {
        ID_CTreeBuilderPanel = 10021,
        ID_LISTCTRL3 = 10022,
        ID_LISTCTRL4 = 10023,
        ID_TEXTCTRL2 = 10024,
        ID_TEXTCTRL3 = 10025,
        ID_TEXTCTRL4 = 10026,
        ID_TEXTCTRL5 = 10027
    };
////@end CTreeBuilderPanel member variables

    TConstScopedObjects*    m_InputObjects;
    wxChoice * m_pDistance;
    wxChoice * m_pConstruct;
    wxChoice * m_pLabels;
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___TREE_BUILDER_PANEL__HPP
