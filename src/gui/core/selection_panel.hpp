#ifndef GUI_CORE___SELECTION_PANEL__HPP
#define GUI_CORE___SELECTION_PANEL__HPP

/*  $Id: selection_panel.hpp 44904 2020-04-13 17:14:22Z shkeda $
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
 * Authors:  Andrey Yazhuk, Yury Voronov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/object_list/object_text_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/utils/job_future.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/menu.h>

BEGIN_NCBI_SCOPE

class ISelectionClient;
class CSelectionService;
class CSelectionServiceEvent;

///////////////////////////////////////////////////////////////////////////////
/// CSelectionPanel
class CSelectionPanel :
    public wxPanel,
    public IRegSettings
{
    DECLARE_EVENT_TABLE()

public:
    static const int scm_ModeCmd = 13000;

    enum EDisplayMode    {
        eInvalidDisplayMode = -1,
        eTable,
        eBriefText,
        eText
    };

public:
    CSelectionPanel( ISelectionClient* view );

    virtual void Create(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize
    );
    virtual bool Destroy();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetSelectionService( CSelectionService* service );
    void GetSelection( TConstScopedObjects& buf ) const;

    void Update();
    void OnSelServiceStateChange( CSelectionServiceEvent& update );

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name Command handlers
    /// @{
    void OnContextMenu(wxContextMenuEvent& event);
    void OnTargetChanged(wxCommandEvent& event);
    void OnModeChanged(wxCommandEvent& event);
    /// @}

    wxWindow* GetCurrentCtrl();

protected:
    void x_InitCurrentWidget();

    bool x_ResetSelObjects();
    void x_ResetTargetChoice();
    void x_ResetObjectWidget();
    void x_InitTextWidget();

protected:
    typedef map<int, ISelectionClient*> TIndexToClient;

    ISelectionClient*   m_SelView;
    CSelectionService*  m_Service;
    TConstScopedObjects m_SelObjects;

    wxChoice*  m_TargetChoice;

    CWeakIRef<ISelectionClient>  m_CurrentClientWeakPtr;
    //ISelectionClient* m_CurrentClient;

    EDisplayMode m_DisplayMode;

    CObjectListWidget* m_ListWidget;
    CTextItemPanel*    m_TextWidget;

    string m_RegPath; // path to the setting in CGuiRegistry

    std::unique_ptr<async_job> m_Future;
};

END_NCBI_SCOPE;

#endif // GUI_CORE___SELECTION_PANEL__HPP
