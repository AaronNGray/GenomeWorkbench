#ifndef GUI_WIDGETS_WX___GUI_WIDGET_DLG__HPP
#define GUI_WIDGETS_WX___GUI_WIDGET_DLG__HPP

/*  $Id: gui_widget_dlg.hpp 30627 2014-07-01 15:47:50Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/gui_widget.hpp>

#include <wx/dialog.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CGuiWidgetDlg: public wxDialog
    , public IGuiWidget
{
    DECLARE_EVENT_TABLE()
public:
    /// Constructors
    CGuiWidgetDlg() : m_Closing(false), m_WidgetHost(), m_CmdProccessor() {}

    /// @name IGuiWidgetHost implementation
    /// @{
    virtual void SetHost(IGuiWidgetHost* host) { m_WidgetHost = host; }
    virtual void SetWidgetTitle(const wxString& title) { SetTitle(title); }
    virtual void SetClosing() { m_Closing = true; }
    virtual void SetUndoManager(ICommandProccessor* cmdProccessor) { m_CmdProccessor = cmdProccessor; }
    /// @}

    void OnClose(wxCloseEvent& event);
    void OnChildFocus(wxChildFocusEvent& event);

    virtual bool Show(bool show = true);

protected:
    void x_CloseDialogView();

    bool m_Closing;
    IGuiWidgetHost*     m_WidgetHost;
    ICommandProccessor* m_CmdProccessor;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___GUI_WIDGET_DLG__HPP
