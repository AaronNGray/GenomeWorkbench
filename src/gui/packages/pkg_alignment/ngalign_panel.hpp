#ifndef PKG_ALIGNMENT___NGALIGN_PANEL__HPP
#define PKG_ALIGNMENT___NGALIGN_PANEL__HPP

/*  $Id: ngalign_panel.hpp 37341 2016-12-27 17:19:44Z katargir $
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

/*!
 * Forward declarations
 */

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CNGALIGNPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CNGALIGNPANEL_TITLE _("NGAlign Tool Panel")
#define SYMBOL_CNGALIGNPANEL_IDNAME ID_CNGALIGNPANEL
#define SYMBOL_CNGALIGNPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CNGALIGNPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CObjectListWidget;

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

/*!
 * CNGAlignPanel class declaration
 */

class CNGAlignPanel: public CAlgoToolManagerParamsPanel
{    
    DECLARE_DYNAMIC_CLASS( CNGAlignPanel )
    DECLARE_EVENT_TABLE()

public:
    CNGAlignPanel();
    CNGAlignPanel( wxWindow* parent );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNGALIGNPANEL_IDNAME );

    ~CNGAlignPanel();

    void Init();

    void CreateControls();

    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    virtual void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    const SConstScopedObject& GetSubject() const { return m_Subject; }
    SConstScopedObject& SetSubject() { return m_Subject; }

    const TConstScopedObjects& GetQueries() const { return m_Queries; }
    TConstScopedObjects& SetQueries() { return m_Queries; }

////@begin CNGAlignPanel event handler declarations

////@end CNGAlignPanel event handler declarations

////@begin CNGAlignPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CNGAlignPanel member function declarations

    void SetObjects(TConstScopedObjects* objects);

    static bool ShowToolTips();

////@begin CNGAlignPanel member variables
    CObjectListWidget* m_QueryListCtrl;
    CObjectListWidget* m_SubjectListCtrl;
    enum {
        ID_CNGALIGNPANEL = 10047,
        ID_QUERY_LIST = 10060,
        ID_SUBJ_LIST = 10061
    };
////@end CNGAlignPanel member variables

    SConstScopedObject m_Subject;
    TConstScopedObjects m_Queries;
};

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___NGALIGN_PANEL__HPP
