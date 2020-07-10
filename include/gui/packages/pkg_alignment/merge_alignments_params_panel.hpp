#ifndef PKG_ALIGNMENT___MERGE_ALIGNMENTS_PARAMS_PANEL__HPP
#define PKG_ALIGNMENT___MERGE_ALIGNMENTS_PARAMS_PANEL__HPP

/*  $Id: merge_alignments_params_panel.hpp 32655 2015-04-07 18:11:22Z evgeniev $
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

#include <corelib/ncbistl.hpp>

#include <gui/core/algo_tool_manager_base.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/objutils/reg_settings.hpp>

////@begin includes
#include "wx/listctrl.h"
////@end includes

#include <wx/panel.h>

////@begin forward declarations
class CObjectListWidget;
////@end forward declarations

class wxCheckBox;

////@begin control identifiers
#define SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_TITLE _("BLAST Search Options Panel")
#define SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_IDNAME ID_CBLASTSEARCHOPTIONSPANEL
#define SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CObjectListWidget;

///////////////////////////////////////////////////////////////////////////////
///

class SMergeAlignmentsParams : public IRegSettings
{
public:
    SMergeAlignmentsParams();

    void Init();

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

public:
    string m_RegPath;

    TConstScopedObjects     m_Alignments;

// CAlnUserOptions
    int     m_Direction;
    int     m_MergeAlgo;
    int     m_MergeFlags;
    bool    m_FillUnaligned;
};

///////////////////////////////////////////////////////////////////////////////
/// CMergeAlignmentsParamsPanel

class CMergeAlignmentsParamsPanel: public CAlgoToolManagerParamsPanel
{
    DECLARE_DYNAMIC_CLASS( CMergeAlignmentsParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    CMergeAlignmentsParamsPanel();
    CMergeAlignmentsParamsPanel( wxWindow* parent, wxWindowID id = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_SIZE, long style = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_POSITION, const wxSize& size = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_SIZE, long style = SYMBOL_CMERGEALIGNMENTSPARAMSPANEL_STYLE );

    ~CMergeAlignmentsParamsPanel();

    void Init();

    void CreateControls();

    bool TransferDataToWindow();

    bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

////@begin CMergeAlignmentsParamsPanel event handler declarations

////@end CMergeAlignmentsParamsPanel event handler declarations

////@begin CMergeAlignmentsParamsPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CMergeAlignmentsParamsPanel member function declarations

    static bool ShowToolTips();

////@begin CMergeAlignmentsParamsPanel member variables
    CObjectListWidget* m_ObjectListCtrl;
    wxCheckBox* m_MergeQuerySeqOnly;
    wxCheckBox* m_AssumeNegStrand;
    wxCheckBox* m_TruncateOverlap;
    wxCheckBox* m_SortInput;
    wxCheckBox* m_FillUnaligned;
    enum {
        ID_CBLASTSEARCHOPTIONSPANEL = 10000,
        ID_LISTCTRL1 = 10003,
        ID_CHECKBOX6 = 10071,
        ID_CHECKBOX7 = 10072,
        ID_CHECKBOX8 = 10073,
        ID_CHECKBOX9 = 10074,
        ID_CHECKBOX10 = 10075
    };
////@end CMergeAlignmentsParamsPanel member variables

public:
    void    SetParams(SMergeAlignmentsParams* params, TConstScopedObjects* objects);

    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;

protected:
    SMergeAlignmentsParams* m_Params;
    TConstScopedObjects*    m_InputObjects;

    bool    m_InputListDirty;

    string  m_RegPath;
};

/* @} */

END_NCBI_SCOPE

#endif // PKG_ALIGNMENT___MERGE_ALIGNMENTS_PARAMS_PANEL__HPP

