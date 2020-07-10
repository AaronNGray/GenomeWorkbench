#ifndef GUI_FRAMEWORK___EVENT_VIEW__HPP
#define GUI_FRAMEWORK___EVENT_VIEW__HPP

/*  $Id: event_view.hpp 31806 2014-11-17 18:18:01Z katargir $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/framework/view.hpp>
#include <gui/framework/event_log_service_impl.hpp>

#include <gui/widgets/wx/wm_client.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/command.hpp>
#include <gui/utils/extension.hpp>

#include <corelib/ncbitime.hpp>

#include <deque>

#include <wx/panel.h>

BEGIN_NCBI_SCOPE

class CwxTableListCtrl;

///////////////////////////////////////////////////////////////////////////////
/// CEventViewModel - a model for Event View table.

class CEventViewModel : public CwxAbstractTableModel
{
public:
    enum EColumns {
        eNone = -1,
        eTitle,
        eDescription,
        eTime,
        eType,
        eMaxCols    /// this must be the last!
    };

public:
    CEventViewModel(IEventLogService* service);

    virtual int GetNumRows() const;
    virtual int GetNumColumns() const;
    virtual wxVariant GetValueAt(int row, int col) const;
    virtual wxString GetColumnType(int col) const;
    virtual wxString GetColumnName(int col) const;

    void    ShowType(IEventRecord::EType type, bool show, bool up = true);
    bool    IsTypeShown(IEventRecord::EType type);

    void    UpdateTable();

protected:
    typedef CConstIRef<IEventRecord>    TCRefRec;

    string m_Names[eMaxCols];
    CTimeFormat m_TimeFormat;

    IEventLogService*   m_Service;
    vector<bool>        m_Filter;
    std::deque<TCRefRec>     m_Records;
};


///////////////////////////////////////////////////////////////////////////////
/// CEventTablePanel - the table displaying events; used in Event View.

class NCBI_GUIFRAMEWORK_EXPORT CEventTablePanel :
    public wxPanel,
    public IRegSettings
{
    DECLARE_EVENT_TABLE()
public:
    enum ECommands   {
        eCmdDetails = 10000, //TODO use Actions,
        eCmdShowInfo,
        eCmdShowWarnings,
        eCmdShowErrors
    };

public:
    CEventTablePanel();
    virtual ~CEventTablePanel();

    virtual void    Create(wxWindow* parent,
                           wxWindowID id = wxID_ANY,
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize);

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void    CreateModel(IEventLogService* service);
    void    DestroyModel();
    void    UpdateTable();

    CwxTableListCtrl* GetTable() { return m_Table; }

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    virtual void AppendMenuItems( wxMenu& aMenu );

    /// @name Command handlers
    /// @{
    void    OnContextMenu(wxContextMenuEvent& event);
    void    OnItemActivated(wxListEvent& event);
    void    OnShowDetailsDialog(int vis_index);

    void    OnDetails(wxCommandEvent& event);
    void    OnShowErrors(wxCommandEvent& event);
    void    OnShowWarnings(wxCommandEvent& event);
    void    OnShowInfo(wxCommandEvent& event);

    void    OnUpdateDetails(wxUpdateUIEvent& event);
    void    OnUpdateShowErrors(wxUpdateUIEvent& event);
    void    OnUpdateShowWarnings(wxUpdateUIEvent& event);
    void    OnUpdateShowInfo(wxUpdateUIEvent& event);
    /// @}

protected:
    IEventLogService*   m_Service;
    CEventViewModel*    m_Model;
    CwxTableListCtrl*   m_Table;

    // load icons into art provider once per run
    static bool m_IconsInitialized;

    string m_RegPath;
};


///////////////////////////////////////////////////////////////////////////////
/// CEventView - system view that displays events from Event Logging Service.

class NCBI_GUIFRAMEWORK_EXPORT CEventView :
    public CObjectEx,
    public IView,
    public IWMClient,
    public CEventHandler,
    public IRegSettings
{
    friend class CEventViewFactory;
    DECLARE_EVENT_MAP();
public:
    CEventView();
    virtual ~CEventView();

    /// @name IView implementation
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IWMClient implementation
    /// @{
    virtual wxWindow* GetWindow();
    virtual wxEvtHandler* GetCommandHandler();
    virtual string GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const;
    virtual string GetIconAlias() const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor( const CRgbaColor& color );
    virtual const wxMenu* GetMenu();
    virtual void UpdateMenu( wxMenu& root_menu );
    virtual CFingerprint  GetFingerprint() const;
    /// @}

    /// @name IRegSettings implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    void    x_OnNewRecord(CEvent*);

    static CViewTypeDescriptor m_TypeDescr;

    CEventLogService*   m_Service;
    CEventTablePanel*   m_Panel;

    string m_RegPath;
};


///////////////////////////////////////////////////////////////////////////////
/// CEventViewFactory - factory for creating Event View.

class NCBI_GUIFRAMEWORK_EXPORT CEventViewFactory :
    public CObject,
    public IExtension,
    public IViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IViewFactory interface implementation
    /// @{
    virtual void    RegisterIconAliases(wxFileArtProvider& provider);
    virtual const CViewTypeDescriptor&   GetViewTypeDescriptor() const;
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    /// @}
};


END_NCBI_SCOPE

/* @} */

#endif // GUI_FRAMEWORK___EVENT_VIEW__HPP
