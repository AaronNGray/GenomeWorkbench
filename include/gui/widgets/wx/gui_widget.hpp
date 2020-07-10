#ifndef GUI_WIDGETS_WX___GUI_WIDGET_HPP
#define GUI_WIDGETS_WX___GUI_WIDGET_HPP

/*  $Id: gui_widget.hpp 31197 2014-09-10 19:31:59Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/objutils/objects.hpp>

class wxString;
class wxWindow;

BEGIN_NCBI_SCOPE

class ICommandProccessor;

typedef wxWindow* (*FWindowFactory)(wxWindow*);

class IGuiWidgetHost
{
public:
    virtual ~IGuiWidgetHost() {}

    virtual void CloseWidget() = 0;
    virtual void WidgetSelectionChanged() = 0;
    virtual void WidgetActivated() = 0;
    virtual void GetAppSelection(TConstObjects& /*objects*/) {}
    virtual void OnWidgetCommand(const string& command) = 0;
    virtual void ShowView(const string& viewName, FWindowFactory widgetFactory, TConstScopedObjects& objects, bool bFloat) = 0;
};

class IGuiWidget
{
public:
    virtual ~IGuiWidget() {}

    virtual void SetHost(IGuiWidgetHost* host) = 0;
    virtual bool InitWidget(TConstScopedObjects& objects) = 0;
    virtual const CObject* GetOrigObject() const { return 0; }
    virtual void SetWidgetTitle(const wxString& /*title*/) {}
    virtual void SetClosing() {}

    // Selection
    virtual void GetSelectedObjects (TConstObjects& /*objects*/) const {}
    virtual void SetSelectedObjects (const TConstObjects& /*objects*/) {}

    // Data
    virtual void SetUndoManager(ICommandProccessor* cmdProccessor) = 0;
    virtual void DataChanging() {}
    virtual void DataChanged() {}

    // Objects for action
    virtual void GetActiveObjects(vector<TConstScopedObjects>& objects) = 0;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___GUI_WIDGET_HPP
