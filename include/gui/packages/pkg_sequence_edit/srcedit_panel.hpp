/*  $Id: srcedit_panel.hpp 32655 2015-04-07 18:11:22Z evgeniev $
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
#ifndef _SRCEDIT_PANEL_H_
#define _SRCEDIT_PANEL_H_

#include <corelib/ncbistd.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_params.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/grid.h"
////@end includes

/*!
 * Forward declarations
 */

BEGIN_NCBI_SCOPE

////@begin forward declarations
class CObjectListWidget;
////@end forward declarations

class ICommandProccessor;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSRCEDITPANEL 10011
#define ID_GRID 10012
#define ID_FOREIGN 10002
#define SYMBOL_CSRCEDITPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSRCEDITPANEL_TITLE _("SrcEditPanel")
#define SYMBOL_CSRCEDITPANEL_IDNAME ID_CSRCEDITPANEL
#define SYMBOL_CSRCEDITPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSRCEDITPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSrcEditPanel class declaration
 */

class CSrcEditPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CSrcEditPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSrcEditPanel();
    CSrcEditPanel( wxWindow* parent, wxWindowID id = SYMBOL_CSRCEDITPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCEDITPANEL_POSITION, const wxSize& size = SYMBOL_CSRCEDITPANEL_SIZE, long style = SYMBOL_CSRCEDITPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSRCEDITPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSRCEDITPANEL_POSITION, const wxSize& size = SYMBOL_CSRCEDITPANEL_SIZE, long style = SYMBOL_CSRCEDITPANEL_STYLE );

    /// Destructor
    ~CSrcEditPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// Data access
    CSrcEditParams& GetData() { return m_data; }
    const CSrcEditParams& GetData() const { return m_data; }
    void SetData(const CSrcEditParams& data) { m_data = data; }

////@begin CSrcEditPanel event handler declarations

////@end CSrcEditPanel event handler declarations

////@begin CSrcEditPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSrcEditPanel member function declarations

    void SetObjects(TConstScopedObjects* objects) { m_InputObjects = objects; }

    objects::CSeq_entry_Handle GetSeqEntryHandle();

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSrcEditPanel member variables
    CObjectListWidget* m_ObjectList;
////@end CSrcEditPanel member variables
    TConstScopedObjects*    m_InputObjects;
    /// The data edited by this window
    CSrcEditParams m_data;
    wxGrid* m_Grid;

    void SetUndoManager(ICommandProccessor* cmdProccessor) { m_CmdProccessor = cmdProccessor; }

private:
    ICommandProccessor* m_CmdProccessor;

};


END_NCBI_SCOPE

#endif
    // _SRCEDIT_PANEL_H_
